/*	init_main.c	4.33	82/07/22	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/fs.h"
#include "../h/mount.h"
#include "../h/map.h"
#include "../h/proc.h"
#include "../h/inode.h"
#include "../h/seg.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/mtpr.h"
#include "../h/pte.h"
#include "../h/clock.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "../h/text.h"
#include "../h/vlimit.h"
#include "../h/clist.h"
#ifdef INET
#include "../h/protosw.h"
#endif
#include "../h/quota.h"

/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 2 to page out
 *	     - process 1 execute bootstrap
 *
 * loop at loc 13 (0xd) in user mode -- /etc/init
 *	cannot be executed.
 */
main(firstaddr)
{
	register int i;
	register struct proc *p;
	struct fs *fs;

	rqinit();
#include "loop.h"
	startup(firstaddr);

	/*
	 * set up system process 0 (swapper)
	 */
	p = &proc[0];
	p->p_p0br = (struct pte *)mfpr(P0BR);
	p->p_szpt = 1;
	p->p_addr = uaddr(p);
	p->p_stat = SRUN;
	p->p_flag |= SLOAD|SSYS;
	p->p_nice = NZERO;
	setredzone(p->p_addr, (caddr_t)&u);
	u.u_procp = p;
	u.u_cmask = CMASK;
	for (i = 1; i < sizeof(u.u_limit)/sizeof(u.u_limit[0]); i++)
		switch (i) {

		case LIM_STACK:
			u.u_limit[i] = 512*1024;
			continue;
		case LIM_DATA:
			u.u_limit[i] = ctob(MAXDSIZ);
			continue;
		default:
			u.u_limit[i] = INFINITY;
			continue;
		}
	p->p_maxrss = INFINITY/NBPG;
#ifdef QUOTA
	qtinit();
	p->p_quota = u.u_quota = getquota(0, 0, Q_NDQ);
#endif
	clkstart();

	/*
	 * Initialize tables, protocols, and set up well-known inodes.
	 */
	mbinit();
	cinit();			/* needed by dmc-11 driver */
#ifdef INET
#if NLOOP > 0
	loattach();			/* XXX */
#endif
	ifinit();
	pfinit();			/* must follow interfaces */
#endif
	ihinit();
	bhinit();
	binit();
	bswinit();
#ifdef GPROF
	kmstartup();
#endif

	fs = mountfs(rootdev, 0, 0);
	if (fs == 0)
		panic("iinit");
	bcopy("/", fs->fs_fsmnt, 2);
	clkinit(fs->fs_time);
	bootime = time;

	rootdir = iget(rootdev, fs, (ino_t)ROOTINO);
	iunlock(rootdir);
	u.u_cdir = iget(rootdev, fs, (ino_t)ROOTINO);
	iunlock(u.u_cdir);

	u.u_rdir = NULL;
	u.u_dmap = zdmap;
	u.u_smap = zdmap;

	/*
	 * Set the scan rate and other parameters of the paging subsystem.
	 */
	setupclock();

	/*
	 * make page-out daemon (process 2)
	 * the daemon has ctopt(nswbuf*CLSIZE*KLMAX) pages of page
	 * table so that it can map dirty pages into
	 * its address space during asychronous pushes.
	 */
	mpid = 1;
	proc[0].p_szpt = clrnd(ctopt(nswbuf*CLSIZE*KLMAX + UPAGES));
	proc[1].p_stat = SZOMB;		/* force it to be in proc slot 2 */
	if (newproc(0)) {
		proc[2].p_flag |= SLOAD|SSYS;
		proc[2].p_dsize = u.u_dsize = nswbuf*CLSIZE*KLMAX; 
		pageout();
	}

	/*
	 * make init process and
	 * enter scheduling loop
	 */

	mpid = 0;
	proc[1].p_stat = 0;
	proc[0].p_szpt = CLSIZE;
	if (newproc(0)) {
		expand(clrnd((int)btoc(szicode)), P0BR);
		(void) swpexpand(u.u_dsize, 0, &u.u_dmap, &u.u_smap);
		(void) copyout((caddr_t)icode, (caddr_t)0, (unsigned)szicode);
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
		return;
	}
#ifdef MUSH
	/*
	 * start MUSH daemon
	 *			pid == 3
	 */
	if (newproc(0)) {
		expand(clrnd((int)btoc(szmcode)), P0BR);
		(void) swpexpand(u.u_dsize, 0, &u.u_dmap, &u.u_smap);
		(void) copyout((caddr_t)mcode, (caddr_t)0, (unsigned)szmcode);
		/*
		 * Return goes to loc. 0 of user mush
		 * code just copied out.
		 */
		return;
	}
#endif
	proc[0].p_szpt = 1;
	sched();
}

/*
 * Initialize hash links for buffers.
 */
bhinit()
{
	register int i;
	register struct bufhd *bp;

	for (bp = bufhash, i = 0; i < BUFHSZ; i++, bp++)
		bp->b_forw = bp->b_back = (struct buf *)bp;
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
binit()
{
	register struct buf *bp;
	register struct buf *dp;
	register int i;
	struct bdevsw *bdp;
	struct swdevt *swp;

	for (dp = bfreelist; dp < &bfreelist[BQUEUES]; dp++) {
		dp->b_forw = dp->b_back = dp->av_forw = dp->av_back = dp;
		dp->b_flags = B_HEAD;
	}
	dp--;				/* dp = &bfreelist[BQUEUES-1]; */
	for (i = 0; i < nbuf; i++) {
		bp = &buf[i];
		bp->b_dev = NODEV;
		bp->b_un.b_addr = buffers + i * MAXBSIZE;
		bp->b_bcount = MAXBSIZE;
		bp->b_back = dp;
		bp->b_forw = dp->b_forw;
		dp->b_forw->b_back = bp;
		dp->b_forw = bp;
		bp->b_flags = B_BUSY|B_INVAL;
		brelse(bp);
	}
	for (bdp = bdevsw; bdp->d_open; bdp++)
		nblkdev++;
	/*
	 * Count swap devices, and adjust total swap space available.
	 * Some of this space will not be available until a vswapon()
	 * system is issued, usually when the system goes multi-user.
	 */
	nswdev = 0;
	for (swp = swdevt; swp->sw_dev; swp++)
		nswdev++;
	if (nswdev == 0)
		panic("binit");
	if (nswdev > 1)
		nswap = (nswap/DMMAX)*DMMAX;
	nswap *= nswdev;
	maxpgio *= nswdev;
	swfree(0);
}

/*
 * Initialize linked list of free swap
 * headers. These do not actually point
 * to buffers, but rather to pages that
 * are being swapped in and out.
 */
bswinit()
{
	register int i;
	register struct buf *sp = swbuf;

	bswlist.av_forw = sp;
	for (i=0; i<nswbuf-1; i++, sp++)
		sp->av_forw = sp+1;
	sp->av_forw = NULL;
}

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
cinit()
{
	register int ccp;
	register struct cblock *cp;
	register struct cdevsw *cdp;

	ccp = (int)cfree;
	ccp = (ccp+CROUND) & ~CROUND;
	for(cp=(struct cblock *)ccp; cp < &cfree[nclist-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
		cfreecount += CBSIZE;
	}
	ccp = 0;
	for(cdp = cdevsw; cdp->d_open; cdp++)
		ccp++;
	nchrdev = ccp;
}
