/*
 * Copyright (c) 1992 The Regents of the University of California
 * Copyright (c) 1990, 1992, 1993 Jan-Simon Pendry
 * All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)fdesc_vnops.c	7.7 (Berkeley) %G%
 *
 * $Id: fdesc_vnops.c,v 1.12 1993/04/06 16:17:17 jsp Exp $
 */

/*
 * /dev/fd Filesystem
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/kernel.h>	/* boottime */
#include <sys/resourcevar.h>
#include <sys/filedesc.h>
#include <sys/vnode.h>
#include <sys/malloc.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/buf.h>
#include <sys/dirent.h>
#include <miscfs/fdesc/fdesc.h>

#define cttyvp(p) ((p)->p_flag&SCTTY ? (p)->p_session->s_ttyvp : NULL)

#define FDL_WANT	0x01
#define FDL_LOCKED	0x02
static int fdescvplock;
static struct vnode *fdescvp[FD_MAX];

#if (FD_STDIN != FD_STDOUT-1) || (FD_STDOUT != FD_STDERR-1)
FD_STDIN, FD_STDOUT, FD_STDERR must be a sequence n, n+1, n+2
#endif

fdesc_allocvp(ftype, ix, mp, vpp)
	fdntype ftype;
	int ix;
	struct mount *mp;
	struct vnode **vpp;
{
	struct vnode **nvpp = 0;
	int error = 0;

loop:
	/* get stashed copy of the vnode */
	if (ix >= 0 && ix < FD_MAX) {
		nvpp = &fdescvp[ix];
		if (*nvpp) {
			if (vget(*nvpp))
				goto loop;
			VOP_UNLOCK(*nvpp);
			*vpp = *nvpp;
			return (error);
		}
	}

	/*
	 * otherwise lock the array while we call getnewvnode
	 * since that can block.
	 */ 
	if (fdescvplock & FDL_LOCKED) {
		fdescvplock |= FDL_WANT;
		sleep((caddr_t) &fdescvplock, PINOD);
		goto loop;
	}
	fdescvplock |= FDL_LOCKED;

	error = getnewvnode(VT_UFS, mp, fdesc_vnodeop_p, vpp);
	if (error)
		goto out;
	MALLOC((*vpp)->v_data, void *, sizeof(struct fdescnode), M_TEMP, M_WAITOK);
	if (nvpp)
		*nvpp = *vpp;
	VTOFDESC(*vpp)->fd_type = ftype;
	VTOFDESC(*vpp)->fd_fd = -1;
	VTOFDESC(*vpp)->fd_link = 0;
	VTOFDESC(*vpp)->fd_ix = ix;

out:;
	fdescvplock &= ~FDL_LOCKED;

	if (fdescvplock & FDL_WANT) {
		fdescvplock &= ~FDL_WANT;
		wakeup((caddr_t) &fdescvplock);
	}

	return (error);
}

/*
 * vp is the current namei directory
 * ndp is the name to locate in that directory...
 */
fdesc_lookup(ap)
	struct vop_lookup_args /* {
		struct vnode * a_dvp;
		struct vnode ** a_vpp;
		struct componentname * a_cnp;
	} */ *ap;
{
	struct vnode **vpp = ap->a_vpp;
	struct vnode *dvp = ap->a_dvp;
	char *pname;
	struct proc *p;
	int nfiles;
	unsigned fd;
	int error;
	struct vnode *fvp;
	char *ln;

#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_lookup(%x)\n", ap);
	printf("fdesc_lookup(dp = %x, vpp = %x, cnp = %x)\n", dvp, vpp, ap->a_cnp);
#endif
	pname = ap->a_cnp->cn_nameptr;
#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_lookup(%s)\n", pname);
#endif
	if (ap->a_cnp->cn_namelen == 1 && *pname == '.') {
		*vpp = dvp;
		VREF(dvp);	
		VOP_LOCK(dvp);
		return (0);
	}

	p = ap->a_cnp->cn_proc;
	nfiles = p->p_fd->fd_nfiles;

	switch (VTOFDESC(dvp)->fd_type) {
	default:
	case Flink:
	case Fdesc:
	case Fctty:
		error = ENOTDIR;
		goto bad;

	case Froot:
		if (ap->a_cnp->cn_namelen == 2 && bcmp(pname, "fd", 2) == 0) {
			error = fdesc_allocvp(Fdevfd, FD_DEVFD, dvp->v_mount, &fvp);
			if (error)
				goto bad;
			*vpp = fvp;
			fvp->v_type = VDIR;
			VOP_LOCK(fvp);
#ifdef FDESC_DIAGNOSTIC
			printf("fdesc_lookup: newvp = %x\n", fvp);
#endif
			return (0);
		}

		if (ap->a_cnp->cn_namelen == 3 && bcmp(pname, "tty", 3) == 0) {
			struct vnode *ttyvp = cttyvp(p);
			if (ttyvp == NULL) {
				error = ENXIO;
				goto bad;
			}
			error = fdesc_allocvp(Fctty, FD_CTTY, dvp->v_mount, &fvp);
			if (error)
				goto bad;
			*vpp = fvp;
			fvp->v_type = VFIFO;
			VOP_LOCK(fvp);
#ifdef FDESC_DIAGNOSTIC
			printf("fdesc_lookup: ttyvp = %x\n", fvp);
#endif
			return (0);
		}

		ln = 0;
		switch (ap->a_cnp->cn_namelen) {
		case 5:
			if (bcmp(pname, "stdin", 5) == 0) {
				ln = "fd/0";
				fd = FD_STDIN;
			}
			break;
		case 6:
			if (bcmp(pname, "stdout", 6) == 0) {
				ln = "fd/1";
				fd = FD_STDOUT;
			} else
			if (bcmp(pname, "stderr", 6) == 0) {
				ln = "fd/2";
				fd = FD_STDERR;
			}
			break;
		}

		if (ln) {
#ifdef FDESC_DIAGNOSTIC
			printf("fdesc_lookup: link -> %s\n", ln);
#endif
			error = fdesc_allocvp(Flink, fd, dvp->v_mount, &fvp);
			if (error)
				goto bad;
			VTOFDESC(fvp)->fd_link = ln;
			*vpp = fvp;
			fvp->v_type = VLNK;
			VOP_LOCK(fvp);
#ifdef FDESC_DIAGNOSTIC
			printf("fdesc_lookup: newvp = %x\n", fvp);
#endif
			return (0);
		} else {
			error = ENOENT;
			goto bad;
		}

		/* fall through */

	case Fdevfd:
		if (ap->a_cnp->cn_namelen == 2 && bcmp(pname, "..", 2) == 0) {
			error = fdesc_root(dvp->v_mount, vpp);
			return (error);
		}

		fd = 0;
		while (*pname >= '0' && *pname <= '9') {
			fd = 10 * fd + *pname++ - '0';
			if (fd >= nfiles)
				break;
		}

#ifdef FDESC_DIAGNOSTIC
		printf("fdesc_lookup: fd = %d, *pname = %x\n", fd, *pname);
#endif
		if (*pname != '\0') {
			error = ENOENT;
			goto bad;
		}

		if (fd >= nfiles || p->p_fd->fd_ofiles[fd] == NULL) {
			error = EBADF;
			goto bad;
		}

#ifdef FDESC_DIAGNOSTIC
		printf("fdesc_lookup: allocate new vnode\n");
#endif
		error = fdesc_allocvp(Fdesc, FD_DESC+fd, dvp->v_mount, &fvp);
		if (error)
			goto bad;
		VTOFDESC(fvp)->fd_fd = fd;
		*vpp = fvp;
#ifdef FDESC_DIAGNOSTIC
		printf("fdesc_lookup: newvp = %x\n", fvp);
#endif
		return (0);
	}

bad:;
	*vpp = NULL;
#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_lookup: error = %d\n", error);
#endif
	return (error);
}

fdesc_open(ap)
	struct vop_open_args /* {
		struct vnode *a_vp;
		int  a_mode;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap;
{
	struct vnode *vp = ap->a_vp;
	int error = 0;

	switch (VTOFDESC(vp)->fd_type) {
	case Fdesc:
		/*
		 * XXX Kludge: set p->p_dupfd to contain the value of the
		 * the file descriptor being sought for duplication. The error 
		 * return ensures that the vnode for this device will be
		 * released by vn_open. Open will detect this special error and
		 * take the actions in dupfdopen.  Other callers of vn_open or
		 * VOP_OPEN will simply report the error.
		 */
		ap->a_p->p_dupfd = VTOFDESC(vp)->fd_fd;	/* XXX */
		error = ENODEV;
		break;

	case Fctty:
		error = cttyopen(devctty, ap->a_mode, 0, ap->a_p);
		break;
	}

	return (error);
}

static int
fdesc_attr(fd, vap, cred, p)
	int fd;
	struct vattr *vap;
	struct ucred *cred;
	struct proc *p;
{
	struct filedesc *fdp = p->p_fd;
	struct file *fp;
	int error;

#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_attr: fd = %d, nfiles = %d\n", fd, fdp->fd_nfiles);
#endif
	if (fd >= fdp->fd_nfiles || (fp = fdp->fd_ofiles[fd]) == NULL) {
#ifdef FDESC_DIAGNOSTIC
		printf("fdesc_attr: fp = %x (EBADF)\n", fp);
#endif
		return (EBADF);
	}

	/*
	 * Can stat the underlying vnode, but not sockets because
	 * they don't use struct vattrs.  Well, we could convert from
	 * a struct stat back to a struct vattr, later...
	 */
	switch (fp->f_type) {
	case DTYPE_VNODE:
		error = VOP_GETATTR((struct vnode *) fp->f_data, vap, cred, p);
		if (error == 0 && vap->va_type == VDIR) {
			/*
			 * don't allow directories to show up because
			 * that causes loops in the namespace.
			 */
			vap->va_type = VFIFO;
		}
		break;

	case DTYPE_SOCKET:
		error = EOPNOTSUPP;
		break;

	default:
		panic("fdesc attr");
		break;
	}

#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_attr: returns error %d\n", error);
#endif
	return (error);
}

fdesc_getattr(ap)
	struct vop_getattr_args /* {
		struct vnode *a_vp;
		struct vattr *a_vap;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap;
{
	struct vnode *vp = ap->a_vp;
	struct vattr *vap = ap->a_vap;
	unsigned fd;
	int error;

#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_getattr: stat type = %d\n", VTOFDESC(vp)->fd_type);
#endif

	switch (VTOFDESC(vp)->fd_type) {
	case Froot:
	case Fdevfd:
	case Flink:
	case Fctty:
		bzero((caddr_t) vap, sizeof(*vap));
		vattr_null(vap);
		vap->va_fileid = VTOFDESC(vp)->fd_ix;

		switch (VTOFDESC(vp)->fd_type) {
		case Flink:
			vap->va_mode = S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;
			vap->va_type = VLNK;
			vap->va_nlink = 1;
			/* vap->va_qsize = strlen(VTOFDESC(vp)->fd_link); */
			vap->va_size = strlen(VTOFDESC(vp)->fd_link);
			break;

		case Fctty:
			vap->va_mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
			vap->va_type = VFIFO;
			vap->va_nlink = 1;
			/* vap->va_qsize = 0; */
			vap->va_size = 0;
			break;

		default:
			vap->va_mode = S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;
			vap->va_type = VDIR;
			vap->va_nlink = 2;
			/* vap->va_qsize = 0; */
			vap->va_size = DEV_BSIZE;
			break;
		}
		vap->va_uid = 0;
		vap->va_gid = 0;
		vap->va_fsid = vp->v_mount->mnt_stat.f_fsid.val[0];
		vap->va_blocksize = DEV_BSIZE;
		vap->va_atime.ts_sec = boottime.tv_sec;
		vap->va_atime.ts_nsec = 0;
		vap->va_mtime = vap->va_atime;
		vap->va_ctime = vap->va_ctime;
		vap->va_gen = 0;
		vap->va_flags = 0;
		vap->va_rdev = 0;
		/* vap->va_qbytes = 0; */
		vap->va_bytes = 0;
		break;

	case Fdesc:
#ifdef FDESC_DIAGNOSTIC
		printf("fdesc_getattr: stat desc #%d\n", VTOFDESC(vp)->fd_fd);
#endif
		fd = VTOFDESC(vp)->fd_fd;
		error = fdesc_attr(fd, vap, ap->a_cred, ap->a_p);
		break;

	default:
		panic("fdesc_getattr");
		break;	
	}

	if (error == 0)
		vp->v_type = vap->va_type;

#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_getattr: stat returns 0\n");
#endif
	return (error);
}

fdesc_setattr(ap)
	struct vop_setattr_args /* {
		struct vnode *a_vp;
		struct vattr *a_vap;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap;
{
	struct filedesc *fdp = ap->a_p->p_fd;
	struct file *fp;
	unsigned fd;
	int error;

	/*
	 * Can't mess with the root vnode
	 */
	switch (VTOFDESC(ap->a_vp)->fd_type) {
	case Fdesc:
		break;

	case Fctty:
		return (0);

	default:
		return (EACCES);
	}

	fd = VTOFDESC(ap->a_vp)->fd_fd;
#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_setattr: fd = %d, nfiles = %d\n", fd, fdp->fd_nfiles);
#endif
	if (fd >= fdp->fd_nfiles || (fp = fdp->fd_ofiles[fd]) == NULL) {
#ifdef FDESC_DIAGNOSTIC
		printf("fdesc_setattr: fp = %x (EBADF)\n", fp);
#endif
		return (EBADF);
	}

	/*
	 * Can setattr the underlying vnode, but not sockets!
	 */
	switch (fp->f_type) {
	case DTYPE_VNODE:
		error = VOP_SETATTR((struct vnode *) fp->f_data, ap->a_vap, ap->a_cred, ap->a_p);
		break;

	case DTYPE_SOCKET:
		error = 0;
		break;

	default:
		panic("fdesc setattr");
		break;
	}

#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_setattr: returns error %d\n", error);
#endif
	return (error);
}

#define UIO_MX 16

static struct dirtmp {
	u_long d_fileno;
	u_short d_reclen;
	u_short d_namlen;
	char d_name[8];
} rootent[] = {
	{ FD_DEVFD, UIO_MX, 2, "fd" },
	{ FD_STDIN, UIO_MX, 5, "stdin" },
	{ FD_STDOUT, UIO_MX, 6, "stdout" },
	{ FD_STDERR, UIO_MX, 6, "stderr" },
	{ FD_CTTY, UIO_MX, 3, "tty" },
	{ 0 }
};

fdesc_readdir(ap)
	struct vop_readdir_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		struct ucred *a_cred;
	} */ *ap;
{
	struct uio *uio = ap->a_uio;
	struct filedesc *fdp;
	int i;
	int error;

	switch (VTOFDESC(ap->a_vp)->fd_type) {
	case Fctty:
		return (0);

	case Fdesc:
		return (ENOTDIR);

	default:
		break;
	}

	fdp = uio->uio_procp->p_fd;

	if (VTOFDESC(ap->a_vp)->fd_type == Froot) {
		struct dirent d;
		struct dirent *dp = &d;
		struct dirtmp *dt;

		i = uio->uio_offset / UIO_MX;
		error = 0;

		while (uio->uio_resid > 0) {
			dt = &rootent[i];
			if (dt->d_fileno == 0) {
				/**eofflagp = 1;*/
				break;
			}
			i++;
			
			switch (dt->d_fileno) {
			case FD_CTTY:
				if (cttyvp(uio->uio_procp) == NULL)
					continue;
				break;

			case FD_STDIN:
			case FD_STDOUT:
			case FD_STDERR:
				if ((i-FD_STDIN) >= fdp->fd_nfiles)
					continue;
				if (fdp->fd_ofiles[i-FD_STDIN] == NULL)
					continue;
				break;
			}
			bzero(dp, UIO_MX);
			dp->d_fileno = dt->d_fileno;
			dp->d_namlen = dt->d_namlen;
			dp->d_type = DT_UNKNOWN;
			dp->d_reclen = dt->d_reclen;
			bcopy(dt->d_name, dp->d_name, dp->d_namlen+1);
			error = uiomove((caddr_t) dp, UIO_MX, uio);
			if (error)
				break;
		}
		uio->uio_offset = i * UIO_MX;
		return (error);
	}

	i = uio->uio_offset / UIO_MX;
	error = 0;
	while (uio->uio_resid > 0) {
		if (i >= fdp->fd_nfiles) {
			/* *ap->a_eofflagp = 1; */
			break;
		}
		if (fdp->fd_ofiles[i] != NULL) {
			struct dirent d;
			struct dirent *dp = &d;
			char *cp = dp->d_name;
			bzero((caddr_t) dp, UIO_MX);

			dp->d_namlen = sprintf(dp->d_name, "%d", i);
			/*
			 * Fill in the remaining fields
			 */
			dp->d_reclen = UIO_MX;
			dp->d_type = DT_UNKNOWN;
			dp->d_fileno = i + FD_STDIN;
			/*
			 * And ship to userland
			 */
			error = uiomove((caddr_t) dp, UIO_MX, uio);
			if (error)
				break;
		}
		i++;
	}

	uio->uio_offset = i * UIO_MX;
	return (error);
}

int
fdesc_readlink(ap)
	struct vop_readlink_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		struct ucred *a_cred;
	} */ *ap;
{
	register struct vnode *vp = ap->a_vp;
	int error;

	if (vp->v_type != VLNK)
		return (EPERM);

	if (VTOFDESC(vp)->fd_type == Flink) {
		char *ln = VTOFDESC(vp)->fd_link;
		error = uiomove(ln, strlen(ln), ap->a_uio);
	} else {
		error = EOPNOTSUPP;
	}

	return (error);
}

fdesc_read(ap)
	struct vop_read_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		int  a_ioflag;
		struct ucred *a_cred;
	} */ *ap;
{
	int error = EOPNOTSUPP;

	switch (VTOFDESC(ap->a_vp)->fd_type) {
	case Fctty:
		error = cttyread(devctty, ap->a_uio, ap->a_ioflag);
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}
	
	return (error);
}

fdesc_write(ap)
	struct vop_write_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		int  a_ioflag;
		struct ucred *a_cred;
	} */ *ap;
{
	int error = EOPNOTSUPP;

	switch (VTOFDESC(ap->a_vp)->fd_type) {
	case Fctty:
		error = cttywrite(devctty, ap->a_uio, ap->a_ioflag);
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}
	
	return (error);
}

fdesc_ioctl(ap)
	struct vop_ioctl_args /* {
		struct vnode *a_vp;
		int  a_command;
		caddr_t  a_data;
		int  a_fflag;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap;
{
	int error = EOPNOTSUPP;

#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_ioctl: type = %d, command = %x\n",
			VTOFDESC(ap->a_vp)->fd_type, ap->a_command);
#endif
	switch (VTOFDESC(ap->a_vp)->fd_type) {
	case Fctty:
		error = cttyioctl(devctty, ap->a_command, ap->a_data,
					ap->a_fflag, ap->a_p);
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}
	
	return (error);
}

fdesc_select(ap)
	struct vop_select_args /* {
		struct vnode *a_vp;
		int  a_which;
		int  a_fflags;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap;
{
	int error = EOPNOTSUPP;

	switch (VTOFDESC(ap->a_vp)->fd_type) {
	case Fctty:
		error = cttyselect(devctty, ap->a_fflags, ap->a_p);
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}
	
	return (error);
}

fdesc_inactive(ap)
	struct vop_inactive_args /* {
		struct vnode *a_vp;
	} */ *ap;
{
	struct vnode *vp = ap->a_vp;

	/*
	 * Clear out the v_type field to avoid
	 * nasty things happening in vgone().
	 */
	vp->v_type = VNON;
#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_inactive(%x)\n", vp);
#endif
	return (0);
}

fdesc_reclaim(ap)
	struct vop_reclaim_args /* {
		struct vnode *a_vp;
	} */ *ap;
{
	struct vnode *vp = ap->a_vp;
	int ix;

#ifdef FDESC_DIAGNOSTIC
	printf("fdesc_reclaim(%x)\n", vp);
#endif
	ix = VTOFDESC(vp)->fd_ix;
	if (ix >= 0 && ix < FD_MAX) {
		if (fdescvp[ix] != vp)
			panic("fdesc_reclaim");
		fdescvp[ix] = 0;
	}
	if (vp->v_data) {
		FREE(vp->v_data, M_TEMP);
		vp->v_data = 0;
	}
	return (0);
}

/*
 * Print out the contents of a /dev/fd vnode.
 */
/* ARGSUSED */
fdesc_print(ap)
	struct vop_print_args /* {
		struct vnode *a_vp;
	} */ *ap;
{

	printf("tag VT_NON, fdesc vnode\n");
	return (0);
}

/*void*/
fdesc_vfree(ap)
	struct vop_vfree_args /* {
		struct vnode *a_pvp;
		ino_t a_ino;
		int a_mode;
	} */ *ap;
{

	return (0);
}

/*
 * /dev/fd vnode unsupported operation
 */
fdesc_enotsupp()
{

	return (EOPNOTSUPP);
}

/*
 * /dev/fd "should never get here" operation
 */
fdesc_badop()
{

	panic("fdesc: bad op");
	/* NOTREACHED */
}

/*
 * /dev/fd vnode null operation
 */
fdesc_nullop()
{

	return (0);
}

#define fdesc_create ((int (*) __P((struct  vop_create_args *)))fdesc_enotsupp)
#define fdesc_mknod ((int (*) __P((struct  vop_mknod_args *)))fdesc_enotsupp)
#define fdesc_close ((int (*) __P((struct  vop_close_args *)))nullop)
#define fdesc_access ((int (*) __P((struct  vop_access_args *)))nullop)
#define fdesc_mmap ((int (*) __P((struct  vop_mmap_args *)))fdesc_enotsupp)
#define fdesc_fsync ((int (*) __P((struct  vop_fsync_args *)))nullop)
#define fdesc_seek ((int (*) __P((struct  vop_seek_args *)))nullop)
#define fdesc_remove ((int (*) __P((struct  vop_remove_args *)))fdesc_enotsupp)
#define fdesc_link ((int (*) __P((struct  vop_link_args *)))fdesc_enotsupp)
#define fdesc_rename ((int (*) __P((struct  vop_rename_args *)))fdesc_enotsupp)
#define fdesc_mkdir ((int (*) __P((struct  vop_mkdir_args *)))fdesc_enotsupp)
#define fdesc_rmdir ((int (*) __P((struct  vop_rmdir_args *)))fdesc_enotsupp)
#define fdesc_symlink ((int (*) __P((struct vop_symlink_args *)))fdesc_enotsupp)
#define fdesc_abortop ((int (*) __P((struct  vop_abortop_args *)))nullop)
#define fdesc_lock ((int (*) __P((struct  vop_lock_args *)))nullop)
#define fdesc_unlock ((int (*) __P((struct  vop_unlock_args *)))nullop)
#define fdesc_bmap ((int (*) __P((struct  vop_bmap_args *)))fdesc_badop)
#define fdesc_strategy ((int (*) __P((struct  vop_strategy_args *)))fdesc_badop)
#define fdesc_islocked ((int (*) __P((struct  vop_islocked_args *)))nullop)
#define fdesc_advlock ((int (*) __P((struct vop_advlock_args *)))fdesc_enotsupp)
#define fdesc_blkatoff \
	((int (*) __P((struct  vop_blkatoff_args *)))fdesc_enotsupp)
#define fdesc_vget ((int (*) __P((struct  vop_vget_args *)))fdesc_enotsupp)
#define fdesc_valloc ((int(*) __P(( \
		struct vnode *pvp, \
		int mode, \
		struct ucred *cred, \
		struct vnode **vpp))) fdesc_enotsupp)
#define fdesc_truncate \
	((int (*) __P((struct  vop_truncate_args *)))fdesc_enotsupp)
#define fdesc_update ((int (*) __P((struct  vop_update_args *)))fdesc_enotsupp)
#define fdesc_bwrite ((int (*) __P((struct  vop_bwrite_args *)))fdesc_enotsupp)

int (**fdesc_vnodeop_p)();
struct vnodeopv_entry_desc fdesc_vnodeop_entries[] = {
	{ &vop_default_desc, vn_default_error },
	{ &vop_lookup_desc, fdesc_lookup },	/* lookup */
	{ &vop_create_desc, fdesc_create },	/* create */
	{ &vop_mknod_desc, fdesc_mknod },	/* mknod */
	{ &vop_open_desc, fdesc_open },		/* open */
	{ &vop_close_desc, fdesc_close },	/* close */
	{ &vop_access_desc, fdesc_access },	/* access */
	{ &vop_getattr_desc, fdesc_getattr },	/* getattr */
	{ &vop_setattr_desc, fdesc_setattr },	/* setattr */
	{ &vop_read_desc, fdesc_read },		/* read */
	{ &vop_write_desc, fdesc_write },	/* write */
	{ &vop_ioctl_desc, fdesc_ioctl },	/* ioctl */
	{ &vop_select_desc, fdesc_select },	/* select */
	{ &vop_mmap_desc, fdesc_mmap },		/* mmap */
	{ &vop_fsync_desc, fdesc_fsync },	/* fsync */
	{ &vop_seek_desc, fdesc_seek },		/* seek */
	{ &vop_remove_desc, fdesc_remove },	/* remove */
	{ &vop_link_desc, fdesc_link },		/* link */
	{ &vop_rename_desc, fdesc_rename },	/* rename */
	{ &vop_mkdir_desc, fdesc_mkdir },	/* mkdir */
	{ &vop_rmdir_desc, fdesc_rmdir },	/* rmdir */
	{ &vop_symlink_desc, fdesc_symlink },	/* symlink */
	{ &vop_readdir_desc, fdesc_readdir },	/* readdir */
	{ &vop_readlink_desc, fdesc_readlink },	/* readlink */
	{ &vop_abortop_desc, fdesc_abortop },	/* abortop */
	{ &vop_inactive_desc, fdesc_inactive },	/* inactive */
	{ &vop_reclaim_desc, fdesc_reclaim },	/* reclaim */
	{ &vop_lock_desc, fdesc_lock },		/* lock */
	{ &vop_unlock_desc, fdesc_unlock },	/* unlock */
	{ &vop_bmap_desc, fdesc_bmap },		/* bmap */
	{ &vop_strategy_desc, fdesc_strategy },	/* strategy */
	{ &vop_print_desc, fdesc_print },	/* print */
	{ &vop_islocked_desc, fdesc_islocked },	/* islocked */
	{ &vop_advlock_desc, fdesc_advlock },	/* advlock */
	{ &vop_blkatoff_desc, fdesc_blkatoff },	/* blkatoff */
	{ &vop_valloc_desc, fdesc_valloc },	/* valloc */
	{ &vop_vfree_desc, fdesc_vfree },	/* vfree */
	{ &vop_truncate_desc, fdesc_truncate },	/* truncate */
	{ &vop_update_desc, fdesc_update },	/* update */
	{ &vop_bwrite_desc, fdesc_bwrite },	/* bwrite */
	{ (struct vnodeop_desc*)NULL, (int(*)())NULL }
};
struct vnodeopv_desc fdesc_vnodeop_opv_desc =
	{ &fdesc_vnodeop_p, fdesc_vnodeop_entries };
