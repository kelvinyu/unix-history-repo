/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)mfs_vnops.c	7.7 (Berkeley) %G%
 */

#include "param.h"
#include "time.h"
#include "kernel.h"
#include "proc.h"
#include "user.h"
#include "buf.h"
#include "vmmac.h"
#include "errno.h"
#include "map.h"
#include "vnode.h"
#include "../ufs/mfsnode.h"
#include "../ufs/mfsiom.h"
#include "machine/vmparam.h"
#include "machine/pte.h"
#include "machine/mtpr.h"

static int mfsmap_want;		/* 1 => need kernel I/O resources */
struct map mfsmap[MFS_MAPSIZE];
extern char mfsiobuf[];

/*
 * mfs vnode operations.
 */
int	mfs_open(),
	mfs_strategy(),
	mfs_ioctl(),
	mfs_close(),
	mfs_inactive(),
	mfs_badop(),
	mfs_nullop();

struct vnodeops mfs_vnodeops = {
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_open,
	mfs_close,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_ioctl,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_inactive,
	mfs_nullop,
	mfs_badop,
	mfs_badop,
	mfs_badop,
	mfs_strategy,
};

/*
 * Vnode Operations.
 *
 * Open called to allow memory filesystem to initialize and
 * validate before actual IO. Record our process identifier
 * so we can tell when we are doing I/O to ourself.
 */
/* ARGSUSED */
mfs_open(vp, mode, cred)
	register struct vnode *vp;
	int mode;
	struct ucred *cred;
{

	if (vp->v_type != VBLK) {
		panic("mfs_ioctl not VBLK");
		/* NOTREACHED */
	}
	return (0);
}

/*
 * Ioctl operation.
 */
/* ARGSUSED */
mfs_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{

	return (-1);
}

/*
 * Pass I/O requests to the memory filesystem process.
 */
mfs_strategy(bp)
	register struct buf *bp;
{
	register struct mfsnode *mfsp = VTOMFS(bp->b_vp);

	if (mfsp->mfs_pid == u.u_procp->p_pid) {
		mfs_doio(bp, mfsp->mfs_baseoff);
	} else {
		bp->av_forw = mfsp->mfs_buflist;
		mfsp->mfs_buflist = bp;
		wakeup((caddr_t)bp->b_vp);
	}
	return (0);
}

/*
 * Memory file system I/O.
 *
 * Essentially play ubasetup() and disk interrupt service routine by
 * doing the copies to or from the memfs process. If doing physio
 * (i.e. pagein), we must map the I/O through the kernel virtual
 * address space.
 */
mfs_doio(bp, base)
	register struct buf *bp;
	caddr_t base;
{
	register struct pte *pte, *ppte;
	register caddr_t vaddr;
	int off, npf, npf2, reg;
	caddr_t kernaddr, offset;

	/*
	 * For phys I/O, map the b_addr into kernel virtual space using
	 * the Mfsiomap pte's.
	 */
	if ((bp->b_flags & B_PHYS) == 0) {
		kernaddr = bp->b_un.b_addr;
	} else {
		if (bp->b_flags & (B_PAGET | B_UAREA | B_DIRTY))
			panic("swap on memfs?");
		off = (int)bp->b_un.b_addr & PGOFSET;
		npf = btoc(bp->b_bcount + off);
		/*
		 * Get some mapping page table entries
		 */
		while ((reg = rmalloc(mfsmap, (long)npf)) == 0) {
			mfsmap_want++;
			sleep((caddr_t)&mfsmap_want, PZERO-1);
		}
		reg--;
		pte = vtopte(bp->b_proc, btop(bp->b_un.b_addr));
		/*
		 * Do vmaccess() but with the Mfsiomap page table.
		 */
		ppte = &Mfsiomap[reg];
		vaddr = &mfsiobuf[reg * NBPG];
		kernaddr = vaddr + off;
		for (npf2 = npf; npf2; npf2--) {
			mapin(ppte, (u_int)vaddr, pte->pg_pfnum,
				(int)(PG_V|PG_KW));
#if defined(tahoe)
			if ((bp->b_flags & B_READ) == 0)
				mtpr(P1DC, vaddr);
#endif
			ppte++;
			pte++;
			vaddr += NBPG;
		}
	}
	offset = base + (bp->b_blkno << DEV_BSHIFT);
	if (bp->b_flags & B_READ)
		bp->b_error = copyin(offset, kernaddr, bp->b_bcount);
	else
		bp->b_error = copyout(kernaddr, offset, bp->b_bcount);
	if (bp->b_error)
		bp->b_flags |= B_ERROR;
	/*
	 * Release pte's used by physical I/O.
	 */
	if (bp->b_flags & B_PHYS) {
		rmfree(mfsmap, (long)npf, (long)++reg);
		if (mfsmap_want) {
			mfsmap_want = 0;
			wakeup((caddr_t)&mfsmap_want);
		}
	}
	biodone(bp);
}

/*
 * Memory filesystem close routine
 */
/* ARGSUSED */
mfs_close(vp, flag, cred)
	register struct vnode *vp;
	int flag;
	struct ucred *cred;
{
	register struct mfsnode *mfsp = VTOMFS(vp);
	register struct buf *bp;

	/*
	 * Finish any pending I/O requests.
	 */
	while (bp = mfsp->mfs_buflist) {
		mfsp->mfs_buflist = bp->av_forw;
		mfs_doio(bp, mfsp->mfs_baseoff);
		wakeup((caddr_t)bp);
	}
	/*
	 * On last close of a memory filesystem
	 * we must invalidate any in core blocks, so that
	 * we can, free up its vnode.
	 */
	bflush(vp->v_mount);
	if (binval(vp->v_mount))
		return (0);
	/*
	 * There should be no way to have any more uses of this
	 * vnode, so if we find any other uses, it is a panic.
	 */
	if (vp->v_count > 1)
		printf("mfs_close: ref count %d > 1\n", vp->v_count);
	if (vp->v_count > 1 || mfsp->mfs_buflist)
		panic("mfs_close");
	/*
	 * Send a request to the filesystem server to exit.
	 */
	mfsp->mfs_buflist = (struct buf *)(-1);
	wakeup((caddr_t)vp);
	return (0);
}

/*
 * Memory filesystem inactive routine
 */
/* ARGSUSED */
mfs_inactive(vp)
	struct vnode *vp;
{

	if (VTOMFS(vp)->mfs_buflist != (struct buf *)(-1))
		panic("mfs_inactive: not inactive");
	return (0);
}

/*
 * Block device bad operation
 */
mfs_badop()
{

	panic("mfs_badop called\n");
	/* NOTREACHED */
}

/*
 * Block device null operation
 */
mfs_nullop()
{

	return (0);
}

/*
 * Memory based filesystem initialization.
 */
mfs_init()
{

	rminit(mfsmap, (long)MFS_MAPREG, (long)1, "mfs mapreg", MFS_MAPSIZE);
}
