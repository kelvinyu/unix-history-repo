/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)ffs_vnops.c	7.76 (Berkeley) %G%
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/resourcevar.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/fifo.h>
#include <sys/malloc.h>

#include <vm/vm.h>

#include <ufs/ufs/lockf.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ufs/ufs_extern.h>

#include <ufs/ffs/fs.h>
#include <ufs/ffs/ffs_extern.h>

/* Global vfs data structures for ufs. */
int (**ffs_vnodeop_p)();
struct vnodeopv_entry_desc ffs_vnodeop_entries[] = {
	{ &vop_default_desc, vn_default_error },
	{ &vop_lookup_desc, ufs_lookup },		/* lookup */
	{ &vop_create_desc, ufs_create },		/* create */
	{ &vop_mknod_desc, ufs_mknod },		/* mknod */
	{ &vop_open_desc, ufs_open },		/* open */
	{ &vop_close_desc, ufs_close },		/* close */
	{ &vop_access_desc, ufs_access },		/* access */
	{ &vop_getattr_desc, ufs_getattr },		/* getattr */
	{ &vop_setattr_desc, ufs_setattr },		/* setattr */
	{ &vop_read_desc, ffs_read },		/* read */
	{ &vop_write_desc, ffs_write },		/* write */
	{ &vop_ioctl_desc, ufs_ioctl },		/* ioctl */
	{ &vop_select_desc, ufs_select },		/* select */
	{ &vop_mmap_desc, ufs_mmap },		/* mmap */
	{ &vop_fsync_desc, ffs_fsync },		/* fsync */
	{ &vop_seek_desc, ufs_seek },		/* seek */
	{ &vop_remove_desc, ufs_remove },		/* remove */
	{ &vop_link_desc, ufs_link },		/* link */
	{ &vop_rename_desc, ufs_rename },		/* rename */
	{ &vop_mkdir_desc, ufs_mkdir },		/* mkdir */
	{ &vop_rmdir_desc, ufs_rmdir },		/* rmdir */
	{ &vop_symlink_desc, ufs_symlink },		/* symlink */
	{ &vop_readdir_desc, ufs_readdir },		/* readdir */
	{ &vop_readlink_desc, ufs_readlink },		/* readlink */
	{ &vop_abortop_desc, ufs_abortop },		/* abortop */
	{ &vop_inactive_desc, ffs_inactive },		/* inactive */
	{ &vop_reclaim_desc, ufs_reclaim },		/* reclaim */
	{ &vop_lock_desc, ufs_lock },		/* lock */
	{ &vop_unlock_desc, ufs_unlock },		/* unlock */
	{ &vop_bmap_desc, ffs_bmap },		/* bmap */
	{ &vop_strategy_desc, ufs_strategy },		/* strategy */
	{ &vop_print_desc, ufs_print },		/* print */
	{ &vop_islocked_desc, ufs_islocked },		/* islocked */
	{ &vop_advlock_desc, ufs_advlock },		/* advlock */
	{ &vop_blkatoff_desc, ffs_blkatoff },		/* blkatoff */
	{ &vop_vget_desc, ffs_vget },		/* vget */
	{ &vop_valloc_desc, ffs_valloc },		/* valloc */
	{ &vop_vfree_desc, ffs_vfree },		/* vfree */
	{ &vop_truncate_desc, ffs_truncate },		/* truncate */
	{ &vop_update_desc, ffs_update },		/* update */
	{ &vop_bwrite_desc, bwrite },
	{ (struct vnodeop_desc*)NULL, (int(*)())NULL }
};
struct vnodeopv_desc ffs_vnodeop_opv_desc =
	{ &ffs_vnodeop_p, ffs_vnodeop_entries };

int (**ffs_specop_p)();
struct vnodeopv_entry_desc ffs_specop_entries[] = {
	{ &vop_default_desc, vn_default_error },
	{ &vop_lookup_desc, spec_lookup },		/* lookup */
	{ &vop_create_desc, spec_create },		/* create */
	{ &vop_mknod_desc, spec_mknod },		/* mknod */
	{ &vop_open_desc, spec_open },		/* open */
	{ &vop_close_desc, ufsspec_close },		/* close */
	{ &vop_access_desc, ufs_access },		/* access */
	{ &vop_getattr_desc, ufs_getattr },		/* getattr */
	{ &vop_setattr_desc, ufs_setattr },		/* setattr */
	{ &vop_read_desc, ufsspec_read },		/* read */
	{ &vop_write_desc, ufsspec_write },		/* write */
	{ &vop_ioctl_desc, spec_ioctl },		/* ioctl */
	{ &vop_select_desc, spec_select },		/* select */
	{ &vop_mmap_desc, spec_mmap },		/* mmap */
	{ &vop_fsync_desc, spec_fsync },		/* fsync */
	{ &vop_seek_desc, spec_seek },		/* seek */
	{ &vop_remove_desc, spec_remove },		/* remove */
	{ &vop_link_desc, spec_link },		/* link */
	{ &vop_rename_desc, spec_rename },		/* rename */
	{ &vop_mkdir_desc, spec_mkdir },		/* mkdir */
	{ &vop_rmdir_desc, spec_rmdir },		/* rmdir */
	{ &vop_symlink_desc, spec_symlink },		/* symlink */
	{ &vop_readdir_desc, spec_readdir },		/* readdir */
	{ &vop_readlink_desc, spec_readlink },		/* readlink */
	{ &vop_abortop_desc, spec_abortop },		/* abortop */
	{ &vop_inactive_desc, ffs_inactive },		/* inactive */
	{ &vop_reclaim_desc, ufs_reclaim },		/* reclaim */
	{ &vop_lock_desc, ufs_lock },		/* lock */
	{ &vop_unlock_desc, ufs_unlock },		/* unlock */
	{ &vop_bmap_desc, spec_bmap },		/* bmap */
	{ &vop_strategy_desc, spec_strategy },		/* strategy */
	{ &vop_print_desc, ufs_print },		/* print */
	{ &vop_islocked_desc, ufs_islocked },		/* islocked */
	{ &vop_advlock_desc, spec_advlock },		/* advlock */
	{ &vop_blkatoff_desc, spec_blkatoff },		/* blkatoff */
	{ &vop_vget_desc, spec_vget },		/* vget */
	{ &vop_valloc_desc, spec_valloc },		/* valloc */
	{ &vop_vfree_desc, spec_vfree },		/* vfree */
	{ &vop_truncate_desc, spec_truncate },		/* truncate */
	{ &vop_update_desc, ffs_update },		/* update */
	{ &vop_bwrite_desc, bwrite },
	{ (struct vnodeop_desc*)NULL, (int(*)())NULL }
};
struct vnodeopv_desc ffs_specop_opv_desc =
	{ &ffs_specop_p, ffs_specop_entries };

#ifdef FIFO
int (**ffs_fifoop_p)();
struct vnodeopv_entry_desc ffs_fifoop_entries[] = {
	{ &vop_default_desc, vn_default_error },
	{ &vop_lookup_desc, fifo_lookup },		/* lookup */
	{ &vop_create_desc, fifo_create },		/* create */
	{ &vop_mknod_desc, fifo_mknod },		/* mknod */
	{ &vop_open_desc, fifo_open },		/* open */
	{ &vop_close_desc, ufsfifo_close },		/* close */
	{ &vop_access_desc, ufs_access },		/* access */
	{ &vop_getattr_desc, ufs_getattr },		/* getattr */
	{ &vop_setattr_desc, ufs_setattr },		/* setattr */
	{ &vop_read_desc, ufsfifo_read },		/* read */
	{ &vop_write_desc, ufsfifo_write },		/* write */
	{ &vop_ioctl_desc, fifo_ioctl },		/* ioctl */
	{ &vop_select_desc, fifo_select },		/* select */
	{ &vop_mmap_desc, fifo_mmap },		/* mmap */
	{ &vop_fsync_desc, fifo_fsync },		/* fsync */
	{ &vop_seek_desc, fifo_seek },		/* seek */
	{ &vop_remove_desc, fifo_remove },		/* remove */
	{ &vop_link_desc, fifo_link },		/* link */
	{ &vop_rename_desc, fifo_rename },		/* rename */
	{ &vop_mkdir_desc, fifo_mkdir },		/* mkdir */
	{ &vop_rmdir_desc, fifo_rmdir },		/* rmdir */
	{ &vop_symlink_desc, fifo_symlink },		/* symlink */
	{ &vop_readdir_desc, fifo_readdir },		/* readdir */
	{ &vop_readlink_desc, fifo_readlink },		/* readlink */
	{ &vop_abortop_desc, fifo_abortop },		/* abortop */
	{ &vop_inactive_desc, ffs_inactive },		/* inactive */
	{ &vop_reclaim_desc, ufs_reclaim },		/* reclaim */
	{ &vop_lock_desc, ufs_lock },		/* lock */
	{ &vop_unlock_desc, ufs_unlock },		/* unlock */
	{ &vop_bmap_desc, fifo_bmap },		/* bmap */
	{ &vop_strategy_desc, fifo_strategy },		/* strategy */
	{ &vop_print_desc, ufs_print },		/* print */
	{ &vop_islocked_desc, ufs_islocked },		/* islocked */
	{ &vop_advlock_desc, fifo_advlock },		/* advlock */
	{ &vop_blkatoff_desc, fifo_blkatoff },		/* blkatoff */
	{ &vop_vget_desc, fifo_vget },		/* vget */
	{ &vop_valloc_desc, fifo_valloc },		/* valloc */
	{ &vop_vfree_desc, fifo_vfree },		/* vfree */
	{ &vop_truncate_desc, fifo_truncate },		/* truncate */
	{ &vop_update_desc, ffs_update },		/* update */
	{ &vop_bwrite_desc, bwrite },
	{ (struct vnodeop_desc*)NULL, (int(*)())NULL }
};
struct vnodeopv_desc ffs_fifoop_opv_desc =
	{ &ffs_fifoop_p, ffs_fifoop_entries };
#endif /* FIFO */


/*
 * Vnode op for reading.
 */
/* ARGSUSED */
ffs_read (ap)
	struct vop_read_args *ap;
#define vp (ap->a_vp)
#define uio (ap->a_uio)
#define ioflag (ap->a_ioflag)
#define cred (ap->a_cred)
{
	register struct inode *ip = VTOI(vp);
	register struct fs *fs;
	struct buf *bp;
	daddr_t lbn, bn, rablock;
	off_t diff;
	int rasize, error = 0;
	long size, n, on;

#ifdef DIAGNOSTIC
	int type;
	if (uio->uio_rw != UIO_READ)
		panic("ffs_read mode");
	type = ip->i_mode & IFMT;
	if (type != IFDIR && type != IFREG && type != IFLNK)
		panic("ffs_read type");
#endif
	if (uio->uio_resid == 0)
		return (0);
	if (uio->uio_offset < 0)
		return (EINVAL);
	ip->i_flag |= IACC;
	fs = ip->i_fs;
	do {
		lbn = lblkno(fs, uio->uio_offset);
		on = blkoff(fs, uio->uio_offset);
		n = MIN((unsigned)(fs->fs_bsize - on), uio->uio_resid);
		diff = ip->i_size - uio->uio_offset;
		if (diff <= 0)
			return (0);
		if (diff < n)
			n = diff;
		size = blksize(fs, ip, lbn);
		rablock = lbn + 1;
		if (vp->v_lastr + 1 == lbn &&
		    lblktosize(fs, rablock) < ip->i_size) {
			rasize = blksize(fs, ip, rablock);
			error = breadn(vp, lbn, size, &rablock,
				&rasize, 1, NOCRED, &bp);
		} else
			error = bread(vp, lbn, size, NOCRED, &bp);
		vp->v_lastr = lbn;
		n = MIN(n, size - bp->b_resid);
		if (error) {
			brelse(bp);
			return (error);
		}
		error = uiomove(bp->b_un.b_addr + on, (int)n, uio);
		if (n + on == fs->fs_bsize || uio->uio_offset == ip->i_size)
			bp->b_flags |= B_AGE;
		brelse(bp);
	} while (error == 0 && uio->uio_resid > 0 && n != 0);
	return (error);
}
#undef vp
#undef uio
#undef ioflag
#undef cred

/*
 * Vnode op for writing.
 */
ffs_write (ap)
	struct vop_write_args *ap;
#define vp (ap->a_vp)
#define uio (ap->a_uio)
#define ioflag (ap->a_ioflag)
#define cred (ap->a_cred)
{
	USES_VOP_TRUNCATE;
	USES_VOP_UPDATE;
	struct proc *p = uio->uio_procp;
	register struct inode *ip = VTOI(vp);
	register struct fs *fs;
	struct buf *bp;
	daddr_t lbn, bn;
	off_t osize;
	int n, on, flags;
	int size, resid, error = 0;

#ifdef DIAGNOSTIC
	if (uio->uio_rw != UIO_WRITE)
		panic("ffs_write mode");
#endif
	switch (vp->v_type) {
	case VREG:
		if (ioflag & IO_APPEND)
			uio->uio_offset = ip->i_size;
		/* fall through */
	case VLNK:
		break;

	case VDIR:
		if ((ioflag & IO_SYNC) == 0)
			panic("ffs_write nonsync dir write");
		break;

	default:
		panic("ffs_write type");
	}
	if (uio->uio_offset < 0)
		return (EINVAL);
	if (uio->uio_resid == 0)
		return (0);
	/*
	 * Maybe this should be above the vnode op call, but so long as
	 * file servers have no limits, i don't think it matters
	 */
	if (vp->v_type == VREG && p &&
	    uio->uio_offset + uio->uio_resid >
	      p->p_rlimit[RLIMIT_FSIZE].rlim_cur) {
		psignal(p, SIGXFSZ);
		return (EFBIG);
	}
	resid = uio->uio_resid;
	osize = ip->i_size;
	fs = ip->i_fs;
	flags = 0;
	if (ioflag & IO_SYNC)
		flags = B_SYNC;
	do {
		lbn = lblkno(fs, uio->uio_offset);
		on = blkoff(fs, uio->uio_offset);
		n = MIN((unsigned)(fs->fs_bsize - on), uio->uio_resid);
		if (n < fs->fs_bsize)
			flags |= B_CLRBUF;
		else
			flags &= ~B_CLRBUF;
		if (error = ffs_balloc(ip, lbn, on + n, cred, &bp, flags))
			break;
		bn = bp->b_blkno;
		if (uio->uio_offset + n > ip->i_size) {
			ip->i_size = uio->uio_offset + n;
			vnode_pager_setsize(vp, (u_long)ip->i_size);
		}
		size = blksize(fs, ip, lbn);
		(void) vnode_pager_uncache(vp);
		n = MIN(n, size - bp->b_resid);
		error = uiomove(bp->b_un.b_addr + on, n, uio);
		if (ioflag & IO_SYNC)
			(void) bwrite(bp);
		else if (n + on == fs->fs_bsize) {
			bp->b_flags |= B_AGE;
			bawrite(bp);
		} else
			bdwrite(bp);
		ip->i_flag |= IUPD|ICHG;
		if (cred->cr_uid != 0)
			ip->i_mode &= ~(ISUID|ISGID);
	} while (error == 0 && uio->uio_resid > 0 && n != 0);
	if (error && (ioflag & IO_UNIT)) {
		(void)VOP_TRUNCATE(vp, osize, ioflag & IO_SYNC, cred);
		uio->uio_offset -= resid - uio->uio_resid;
		uio->uio_resid = resid;
	}
	if (!error && (ioflag & IO_SYNC))
		error = VOP_UPDATE(vp, &time, &time, 1);
	return (error);
}
#undef vp
#undef uio
#undef ioflag
#undef cred

/*
 * Synch an open file.
 */
/* ARGSUSED */
int
ffs_fsync (ap)
	struct vop_fsync_args *ap;
#define vp (ap->a_vp)
#define fflags (ap->a_fflags)
#define cred (ap->a_cred)
#define waitfor (ap->a_waitfor)
#define p (ap->a_p)
{
	USES_VOP_UPDATE;
	struct inode *ip = VTOI(vp);

	if (fflags & FWRITE)
		ip->i_flag |= ICHG;
	vflushbuf(vp, waitfor == MNT_WAIT ? B_SYNC : 0);
	return (VOP_UPDATE(vp, &time, &time, waitfor == MNT_WAIT));
}
#undef vp
#undef fflags
#undef cred
#undef waitfor
#undef p

/*
 * Last reference to an inode, write the inode out and if necessary,
 * truncate and deallocate the file.
 */
int
ffs_inactive (ap)
	struct vop_inactive_args *ap;
#define vp (ap->a_vp)
#define p (ap->a_p)
{
	USES_VOP_TRUNCATE;
	USES_VOP_UPDATE;
	USES_VOP_VFREE;
	register struct inode *ip;
	int mode, error;
	extern int prtactive;

	if (prtactive && vp->v_usecount != 0)
		vprint("ffs_inactive: pushing active", vp);

	/* Get rid of inodes related to stale file handles. */
	ip = VTOI(vp);
	if (ip->i_mode == 0) {
		if ((vp->v_flag & VXLOCK) == 0)
			vgone(vp);
		return (0);
	}

	error = 0;
	ILOCK(ip);
	if (ip->i_nlink <= 0 && (vp->v_mount->mnt_flag & MNT_RDONLY) == 0) {
#ifdef QUOTA
		if (!getinoquota(ip))
			(void)chkiq(ip, -1, NOCRED, 0);
#endif
		error = VOP_TRUNCATE(vp, (off_t)0, 0, NOCRED);
		mode = ip->i_mode;
		ip->i_mode = 0;
		ip->i_rdev = 0;
		ip->i_flag |= IUPD|ICHG;
		VOP_VFREE(vp, ip->i_number, mode);
	}
	if (ip->i_flag&(IUPD|IACC|ICHG|IMOD))
		VOP_UPDATE(vp, &time, &time, 0);
	IUNLOCK(ip);
	ip->i_flag = 0;
	/*
	 * If we are done with the inode, reclaim it
	 * so that it can be reused immediately.
	 */
	if (vp->v_usecount == 0 && ip->i_mode == 0)
		vgone(vp);
	return (error);
}
#undef vp
#undef p
