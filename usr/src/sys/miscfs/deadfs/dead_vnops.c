/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)dead_vnops.c	7.18 (Berkeley) %G%
 */

#include "param.h"
#include "systm.h"
#include "time.h"
#include "vnode.h"
#include "errno.h"
#include "namei.h"
#include "buf.h"

/*
 * Prototypes for dead operations on vnodes.
 */
int	dead_badop(),
	dead_ebadf();
int	dead_lookup __P((
		struct vnode *dvp,
		struct vnode **vpp,
		struct componentname *cnp));
#define dead_create ((int (*) __P(( \
		struct vnode *dvp, \
 		struct vnode **vpp, \
		struct componentname *cnp, \
		struct vattr *vap))) dead_badop)
#define dead_mknod ((int (*) __P(( \
		struct vnode *dvp, \
		struct vnode **vpp, \
		struct componentname *cnp, \
		struct vattr *vap))) dead_badop)
int	dead_open __P((
		struct vnode *vp,
		int mode,
		struct ucred *cred,
		struct proc *p));
#define dead_close ((int (*) __P(( \
		struct vnode *vp, \
		int fflag, \
		struct ucred *cred, \
		struct proc *p))) nullop)
#define dead_access ((int (*) __P(( \
		struct vnode *vp, \
		int mode, \
		struct ucred *cred, \
		struct proc *p))) dead_ebadf)
#define dead_getattr ((int (*) __P(( \
		struct vnode *vp, \
		struct vattr *vap, \
		struct ucred *cred, \
		struct proc *p))) dead_ebadf)
#define dead_setattr ((int (*) __P(( \
		struct vnode *vp, \
		struct vattr *vap, \
		struct ucred *cred, \
		struct proc *p))) dead_ebadf)
int	dead_read __P((
		struct vnode *vp,
		struct uio *uio,
		int ioflag,
		struct ucred *cred));
int	dead_write __P((
		struct vnode *vp,
		struct uio *uio,
		int ioflag,
		struct ucred *cred));
int	dead_ioctl __P((
		struct vnode *vp,
		int command,
		caddr_t data,
		int fflag,
		struct ucred *cred,
		struct proc *p));
int	dead_select __P((
		struct vnode *vp,
		int which,
		int fflags,
		struct ucred *cred,
		struct proc *p));
#define dead_mmap ((int (*) __P(( \
		struct vnode *vp, \
		int fflags, \
		struct ucred *cred, \
		struct proc *p))) dead_badop)
#define dead_fsync ((int (*) __P(( \
		struct vnode *vp, \
		int fflags, \
		struct ucred *cred, \
		int waitfor, \
		struct proc *p))) nullop)
#define dead_seek ((int (*) __P(( \
		struct vnode *vp, \
		off_t oldoff, \
		off_t newoff, \
		struct ucred *cred))) nullop)
#define dead_remove ((int (*) __P(( \
		struct vnode *dvp, \
	        struct vnode *vp, \
		struct componentname *cnp))) dead_badop)
#define dead_link ((int (*) __P(( \
		register struct vnode *vp, \
		struct vnode *tdvp, \
		struct componentname *cnp))) dead_badop)
#define dead_rename ((int (*) __P(( \
		struct vnode *fdvp, \
	        struct vnode *fvp, \
		struct componentname *fcnp, \
		struct vnode *tdvp, \
		struct vnode *tvp, \
		struct componentname *tcnp))) dead_badop)
#define dead_mkdir ((int (*) __P(( \
		struct vnode *dvp, \
		struct vnode **vpp, \
		struct componentname *cnp, \
		struct vattr *vap))) dead_badop)
#define dead_rmdir ((int (*) __P(( \
		struct vnode *dvp, \
		struct vnode *vp, \
		struct componentname *cnp))) dead_badop)
#define dead_symlink ((int (*) __P(( \
		struct vnode *dvp, \
		struct vnode **vpp, \
		struct componentname *cnp, \
		struct vattr *vap, \
		char *target))) dead_badop)
#define dead_readdir ((int (*) __P(( \
		struct vnode *vp, \
		struct uio *uio, \
		struct ucred *cred, \
		int *eofflagp))) dead_ebadf)
#define dead_readlink ((int (*) __P(( \
		struct vnode *vp, \
		struct uio *uio, \
		struct ucred *cred))) dead_ebadf)
#define dead_abortop ((int (*) __P(( \
		struct vnode *dvp, \
		struct componentname *cnp))) dead_badop)
#define dead_inactive ((int (*) __P(( \
		struct vnode *vp, \
		struct proc *p))) nullop)
#define dead_reclaim ((int (*) __P(( \
		struct vnode *vp))) nullop)
int	dead_lock __P((
		struct vnode *vp));
#define dead_unlock ((int (*) __P(( \
		struct vnode *vp))) nullop)
int	dead_bmap __P((
		struct vnode *vp,
		daddr_t bn,
		struct vnode **vpp,
		daddr_t *bnp));
int	dead_strategy __P((
		struct buf *bp));
int	dead_print __P((
		struct vnode *vp));
#define dead_islocked ((int (*) __P(( \
		struct vnode *vp))) nullop)
#define dead_advlock ((int (*) __P(( \
		struct vnode *vp, \
		caddr_t id, \
		int op, \
		struct flock *fl, \
		int flags))) dead_ebadf)
#define dead_blkatoff ((int (*) __P(( \
		struct vnode *vp, \
		off_t offset, \
		char **res, \
		struct buf **bpp))) dead_badop)
#define dead_vget ((int (*) __P(( \
		struct mount *mp, \
		ino_t ino, \
		struct vnode **vpp))) dead_badop)
#define dead_valloc ((int (*) __P(( \
		struct vnode *pvp, \
		int mode, \
		struct ucred *cred, \
		struct vnode **vpp))) dead_badop)
#define dead_vfree ((void (*) __P(( \
		struct vnode *pvp, \
		ino_t ino, \
		int mode))) dead_badop)
#define dead_truncate ((int (*) __P(( \
		struct vnode *vp, \
		off_t length, \
		int flags, \
		struct ucred *cred))) nullop)
#define dead_update ((int (*) __P(( \
		struct vnode *vp, \
		struct timeval *ta, \
		struct timeval *tm, \
		int waitfor))) nullop)
#define dead_bwrite ((int (*) __P(( \
		struct buf *bp))) nullop)

struct vnodeops dead_vnodeops = {
	dead_lookup,	/* lookup */
	dead_create,	/* create */
	dead_mknod,	/* mknod */
	dead_open,	/* open */
	dead_close,	/* close */
	dead_access,	/* access */
	dead_getattr,	/* getattr */
	dead_setattr,	/* setattr */
	dead_read,	/* read */
	dead_write,	/* write */
	dead_ioctl,	/* ioctl */
	dead_select,	/* select */
	dead_mmap,	/* mmap */
	dead_fsync,	/* fsync */
	dead_seek,	/* seek */
	dead_remove,	/* remove */
	dead_link,	/* link */
	dead_rename,	/* rename */
	dead_mkdir,	/* mkdir */
	dead_rmdir,	/* rmdir */
	dead_symlink,	/* symlink */
	dead_readdir,	/* readdir */
	dead_readlink,	/* readlink */
	dead_abortop,	/* abortop */
	dead_inactive,	/* inactive */
	dead_reclaim,	/* reclaim */
	dead_lock,	/* lock */
	dead_unlock,	/* unlock */
	dead_bmap,	/* bmap */
	dead_strategy,	/* strategy */
	dead_print,	/* print */
	dead_islocked,	/* islocked */
	dead_advlock,	/* advlock */
	dead_blkatoff,	/* blkatoff */
	dead_vget,	/* vget */
	dead_valloc,	/* valloc */
	dead_vfree,	/* vfree */
	dead_truncate,	/* truncate */
	dead_update,	/* update */
	dead_bwrite,	/* bwrite */
};

/*
 * Trivial lookup routine that always fails.
 */
/* ARGSUSED */
int
dead_lookup(dvp, vpp, cnp)
	struct vnode *dvp;
	struct vnode **vpp;
	struct componentname *cnp;
{

	*vpp = NULL;
	return (ENOTDIR);
}

/*
 * Open always fails as if device did not exist.
 */
/* ARGSUSED */
dead_open(vp, mode, cred, p)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
	struct proc *p;
{

	return (ENXIO);
}

/*
 * Vnode op for read
 */
/* ARGSUSED */
dead_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{

	if (chkvnlock(vp))
		panic("dead_read: lock");
	/*
	 * Return EOF for character devices, EIO for others
	 */
	if (vp->v_type != VCHR)
		return (EIO);
	return (0);
}

/*
 * Vnode op for write
 */
/* ARGSUSED */
dead_write(vp, uio, ioflag, cred)
	register struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{

	if (chkvnlock(vp))
		panic("dead_write: lock");
	return (EIO);
}

/*
 * Device ioctl operation.
 */
/* ARGSUSED */
dead_ioctl(vp, com, data, fflag, cred, p)
	struct vnode *vp;
	register int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
	struct proc *p;
{

	if (!chkvnlock(vp))
		return (EBADF);
	return (VOP_IOCTL(vp, com, data, fflag, cred, p));
}

/* ARGSUSED */
dead_select(vp, which, fflags, cred, p)
	struct vnode *vp;
	int which, fflags;
	struct ucred *cred;
	struct proc *p;
{

	/*
	 * Let the user find out that the descriptor is gone.
	 */
	return (1);
}

/*
 * Just call the device strategy routine
 */
dead_strategy(bp)
	register struct buf *bp;
{

	if (bp->b_vp == NULL || !chkvnlock(bp->b_vp)) {
		bp->b_flags |= B_ERROR;
		biodone(bp);
		return (EIO);
	}
	return (VOP_STRATEGY(bp));
}

/*
 * Wait until the vnode has finished changing state.
 */
dead_lock(vp)
	struct vnode *vp;
{

	if (!chkvnlock(vp))
		return (0);
	return (VOP_LOCK(vp));
}

/*
 * Wait until the vnode has finished changing state.
 */
dead_bmap(vp, bn, vpp, bnp)
	struct vnode *vp;
	daddr_t bn;
	struct vnode **vpp;
	daddr_t *bnp;
{

	if (!chkvnlock(vp))
		return (EIO);
	return (VOP_BMAP(vp, bn, vpp, bnp));
}

/*
 * Print out the contents of a dead vnode.
 */
/* ARGSUSED */
dead_print(vp)
	struct vnode *vp;
{

	printf("tag VT_NON, dead vnode\n");
}

/*
 * Empty vnode failed operation
 */
dead_ebadf()
{

	return (EBADF);
}

/*
 * Empty vnode bad operation
 */
dead_badop()
{

	panic("dead_badop called");
	/* NOTREACHED */
}

/*
 * Empty vnode null operation
 */
dead_nullop()
{

	return (0);
}

/*
 * We have to wait during times when the vnode is
 * in a state of change.
 */
chkvnlock(vp)
	register struct vnode *vp;
{
	int locked = 0;

	while (vp->v_flag & VXLOCK) {
		vp->v_flag |= VXWANT;
		sleep((caddr_t)vp, PINOD);
		locked = 1;
	}
	return (locked);
}
