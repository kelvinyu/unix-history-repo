/*
 * Copyright (c) 1991 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)ufs_inode.c	7.47 (Berkeley) %G%
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/kernel.h>
#include <sys/malloc.h>

#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/ufs_extern.h>

u_long	nextgennumber;		/* Next generation number to assign. */
int	prtactive = 0;		/* 1 => print out reclaim of active vnodes */

int
ufs_init()
{
	static int first = 1;

	if (!first)
		return (0);
	first = 0;

#ifdef DIAGNOSTIC
	if ((sizeof(struct inode) - 1) & sizeof(struct inode))
		printf("ufs_init: bad size %d\n", sizeof(struct inode));
#endif
	ufs_ihashinit();
	dqinit();
	return (0);
}

/*
 * Unlock and decrement the reference count of an inode structure.
 */
void
ufs_iput(ip)
	register struct inode *ip;
{

	if ((ip->i_flag & ILOCKED) == 0)
		panic("iput");
	IUNLOCK(ip);
	vrele(ITOV(ip));
}

/*
 * Reclaim an inode so that it can be used for other purposes.
 */
int
ufs_reclaim (ap)
	struct vop_reclaim_args *ap;
{
	register struct vnode *vp = ap->a_vp;
	register struct inode *ip;
	int i, type;

	if (prtactive && vp->v_usecount != 0)
		vprint("ufs_reclaim: pushing active", vp);
	/*
	 * Remove the inode from its hash chain.
	 */
	ip = VTOI(vp);
	remque(ip);
	/*
	 * Purge old data structures associated with the inode.
	 */
	cache_purge(vp);
	if (ip->i_devvp) {
		vrele(ip->i_devvp);
		ip->i_devvp = 0;
	}
#ifdef QUOTA
	for (i = 0; i < MAXQUOTAS; i++) {
		if (ip->i_dquot[i] != NODQUOT) {
			dqrele(vp, ip->i_dquot[i]);
			ip->i_dquot[i] = NODQUOT;
		}
	}
#endif
	switch (vp->v_mount->mnt_stat.f_type) {
	case MOUNT_UFS:
		type = M_FFSNODE;
		break;
	case MOUNT_MFS:
		type = M_MFSNODE;
		break;
	case MOUNT_LFS:
		type = M_LFSNODE;
		break;
	default:
		panic("ufs_reclaim: not ufs file");
	}
	FREE(vp->v_data, type);
	vp->v_data = NULL;
	return (0);
}

/*
 * Lock an inode. If its already locked, set the WANT bit and sleep.
 */
void
ufs_ilock(ip)
	register struct inode *ip;
{
	struct proc *p = curproc;	/* XXX */

	while (ip->i_flag & ILOCKED) {
		ip->i_flag |= IWANT;
#ifdef DIAGNOSTIC
		if (p) {
			if (p->p_pid == ip->i_lockholder)
				panic("locking against myself");
			ip->i_lockwaiter = p->p_pid;
		}
#endif
		(void) sleep((caddr_t)ip, PINOD);
	}
#ifdef DIAGNOSTIC
	ip->i_lockwaiter = 0;
	if (p)
		ip->i_lockholder = p->p_pid;
#endif
	ip->i_flag |= ILOCKED;
	curproc->p_spare[2]++;
}

/*
 * Unlock an inode.  If WANT bit is on, wakeup.
 */
void
ufs_iunlock(ip)
	register struct inode *ip;
{

	if ((ip->i_flag & ILOCKED) == 0)
		vprint("ufs_iunlock: unlocked inode", ITOV(ip));
#ifdef DIAGNOSTIC
	ip->i_lockholder = 0;
#endif
	ip->i_flag &= ~ILOCKED;
	curproc->p_spare[2]--;
	if (ip->i_flag&IWANT) {
		ip->i_flag &= ~IWANT;
		wakeup((caddr_t)ip);
	}
}
