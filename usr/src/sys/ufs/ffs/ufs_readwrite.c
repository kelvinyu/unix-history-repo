/*-
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)ufs_readwrite.c	8.3 (Berkeley) %G%
 */

#ifdef LFS_READWRITE
#define	BLKSIZE(a, b, c)	blksize(a)
#define	FS			struct lfs
#define	I_FS			i_lfs
#define	READ			lfs_read
#define	WRITE			lfs_write
#define	fs_bsize		lfs_bsize
#define	fs_maxfilesize		lfs_maxfilesize
#else
#define	BLKSIZE(a, b, c)	blksize(a, b, c)
#define	FS			struct fs
#define	I_FS			i_fs
#define	READ			ffs_read
#define	WRITE			ffs_write
#endif

/*
 * Vnode op for reading.
 */
/* ARGSUSED */
READ(ap)
	struct vop_read_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		int a_ioflag;
		struct ucred *a_cred;
	} */ *ap;
{
	register struct vnode *vp;
	register struct inode *ip;
	register struct uio *uio;
	register FS *fs;
	struct buf *bp;
	daddr_t lbn, nextlbn;
	off_t bytesinfile;
	long size, xfersize, blkoffset;
	int type, nextsize, error;
	u_short mode;

	vp = ap->a_vp;
	ip = VTOI(vp);
	mode = ip->i_mode;
	uio = ap->a_uio;

#ifdef DIAGNOSTIC
	if (uio->uio_rw != UIO_READ)
		panic("%s: mode", READ);

	if (vp->v_type == VLNK) {
		if ((int)ip->i_size < vp->v_mount->mnt_maxsymlinklen)
			panic("%s: short symlink", READ);
	} else if (vp->v_type != VREG && vp->v_type != VDIR)
		panic("%s: type", READ);
#endif
	fs = ip->I_FS;
	if ((u_quad_t)uio->uio_offset > fs->fs_maxfilesize)
		return (EFBIG);

	for (error = 0, bp = NULL; uio->uio_resid > 0; bp = NULL) {
		if ((bytesinfile = ip->i_size - uio->uio_offset) <= 0)
			break;
		lbn = lblkno(fs, uio->uio_offset);
		nextlbn = lbn + 1;
		size = BLKSIZE(fs, ip, lbn);
		blkoffset = blkoff(fs, uio->uio_offset);
		xfersize = fs->fs_bsize - blkoffset;
		if (uio->uio_resid < xfersize)
			xfersize = uio->uio_resid;
		if (bytesinfile < xfersize)
			xfersize = bytesinfile;

#ifdef LFS_READWRITE
		(void)lfs_check(vp, lbn);
		error = cluster_read(vp, ip->i_size, lbn, size, NOCRED, &bp);
#else
		if (lblktosize(fs, nextlbn) > ip->i_size) {
			error = bread(vp, lbn, size, NOCRED, &bp);
		} else {
			if (doclusterread) {
				error = cluster_read(vp,
				    ip->i_size, lbn, size, NOCRED, &bp);
			} else if (lbn - 1 == vp->v_lastr) {
				nextsize = BLKSIZE(fs, ip, nextlbn);
				error = breadn(vp, lbn,
				    size, &nextlbn, &nextsize, 1, NOCRED, &bp);
			} else {
				error = bread(vp, lbn, size, NOCRED, &bp);
			}
		}
#endif
		if (error)
			break;
		vp->v_lastr = lbn;

		/*
		 * We should only get non-zero b_resid when an I/O error
		 * has occurred, which should cause us to break above.
		 * However, if the short read did not cause an error,
		 * then we want to ensure that we do not uiomove bad
		 * or uninitialized data.
		 */
		size -= bp->b_resid;
		if (size < xfersize) {
			if (size == 0)
				break;
			xfersize = size;
		}
		if (error =
		    uiomove((char *)bp->b_data + blkoffset, (int)xfersize, uio))
			break;

		if (S_ISREG(mode) && (xfersize + blkoffset == fs->fs_bsize ||
		    uio->uio_offset == ip->i_size))
			bp->b_flags |= B_AGE;
		brelse(bp);
	}
	if (bp != NULL)
		brelse(bp);
	ip->i_flag |= IN_ACCESS;
	return (error);
}

/*
 * Vnode op for writing.
 */
WRITE(ap)
	struct vop_write_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		int a_ioflag;
		struct ucred *a_cred;
	} */ *ap;
{
	register struct vnode *vp;
	register struct uio *uio;
	register struct inode *ip;
	register FS *fs;
	struct buf *bp;
	struct proc *p;
	daddr_t lbn;
	off_t osize;
	int blkoffset, error, flags, ioflag, newblock, resid, size, xfersize;

	ioflag = ap->a_ioflag;
	uio = ap->a_uio;
	vp = ap->a_vp;
	ip = VTOI(vp);

#ifdef DIAGNOSTIC
	if (uio->uio_rw != UIO_WRITE)
		panic("%s: mode", WRITE);
#endif

	switch (vp->v_type) {
	case VREG:
		if (ioflag & IO_APPEND)
			uio->uio_offset = ip->i_size;
		if ((ip->i_flags & APPEND) && uio->uio_offset != ip->i_size)
			return (EPERM);
		/* FALLTHROUGH */
	case VLNK:
		break;
	case VDIR:
		if ((ioflag & IO_SYNC) == 0)
			panic("%s: nonsync dir write", WRITE);
		break;
	default:
		panic("%s: type", WRITE);
	}

	fs = ip->I_FS;
	if (uio->uio_offset < 0 ||
	    (u_quad_t)uio->uio_offset + uio->uio_resid > fs->fs_maxfilesize)
		return (EFBIG);
	/*
	 * Maybe this should be above the vnode op call, but so long as
	 * file servers have no limits, I don't think it matters.
	 */
	p = uio->uio_procp;
	if (vp->v_type == VREG && p &&
	    uio->uio_offset + uio->uio_resid >
	    p->p_rlimit[RLIMIT_FSIZE].rlim_cur) {
		psignal(p, SIGXFSZ);
		return (EFBIG);
	}

	resid = uio->uio_resid;
	osize = ip->i_size;
	flags = ioflag & IO_SYNC ? B_SYNC : 0;

	for (error = 0; uio->uio_resid > 0;) {
		lbn = lblkno(fs, uio->uio_offset);
		blkoffset = blkoff(fs, uio->uio_offset);
		xfersize = fs->fs_bsize - blkoffset;
		if (uio->uio_resid < xfersize)
			xfersize = uio->uio_resid;
#ifdef LFS_READWRITE
		(void)lfs_check(vp, lbn);
		error = lfs_balloc(vp, xfersize, lbn, &bp);
#else
		if (fs->fs_bsize > xfersize)
			flags |= B_CLRBUF;
		else
			flags &= ~B_CLRBUF;

		error = ffs_balloc(ip,
		    lbn, blkoffset + xfersize, ap->a_cred, &bp, flags);
#endif
		if (error)
			break;
		if (uio->uio_offset + xfersize > ip->i_size) {
			ip->i_size = uio->uio_offset + xfersize;
			vnode_pager_setsize(vp, (u_long)ip->i_size);
		}
		(void)vnode_pager_uncache(vp);

		size = BLKSIZE(fs, ip, lbn) - bp->b_resid;
		if (size < xfersize)
			xfersize = size;

		error =
		    uiomove((char *)bp->b_data + blkoffset, (int)xfersize, uio);
#ifdef LFS_READWRITE
		(void)VOP_BWRITE(bp);
#else
		if (ioflag & IO_SYNC)
			(void)bwrite(bp);
		else if (xfersize + blkoffset == fs->fs_bsize)
			if (doclusterwrite)
				cluster_write(bp, ip->i_size);
			else {
				bp->b_flags |= B_AGE;
				bawrite(bp);
			}
		else
			bdwrite(bp);
#endif
		if (error || xfersize == 0)
			break;
		ip->i_flag |= IN_CHANGE | IN_UPDATE;
	}
	/*
	 * If we successfully wrote any data, and we are not the superuser
	 * we clear the setuid and setgid bits as a precaution against
	 * tampering.
	 */
	if (resid > uio->uio_resid && ap->a_cred && ap->a_cred->cr_uid != 0)
		ip->i_mode &= ~(ISUID | ISGID);
	if (error) {
		if (ioflag & IO_UNIT) {
			(void)VOP_TRUNCATE(vp, osize,
			    ioflag & IO_SYNC, ap->a_cred, uio->uio_procp);
			uio->uio_offset -= resid - uio->uio_resid;
			uio->uio_resid = resid;
		}
	} else if (resid > uio->uio_resid && (ioflag & IO_SYNC))
		error = VOP_UPDATE(vp, &time, &time, 1);
	return (error);
}
