/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)vnode.h	7.34 (Berkeley) %G%
 */

#ifndef KERNEL
#include <machine/endian.h>
#endif

/*
 * The vnode is the focus of all file activity in UNIX.
 * There is a unique vnode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 */

/*
 * vnode types. VNON means no type.
 */
enum vtype 	{ VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VFIFO, VBAD };

/*
 * Vnode tag types.
 * These are for the benefit of external programs only (e.g., pstat)
 * and should NEVER be inspected inside the kernel.
 */
enum vtagtype	{ VT_NON, VT_UFS, VT_NFS, VT_MFS };

/*
 * This defines the maximum size of the private data area
 * permitted for any file system type. A defined constant 
 * is used rather than a union structure to cut down on the
 * number of header files that must be included.
 */
#define VN_MAXPRIVATE	188

struct vnode {
	u_long		v_flag;			/* vnode flags (see below) */
	long		v_usecount;		/* reference count of users */
	long		v_holdcnt;		/* page & buffer references */
	off_t		v_lastr;		/* last read (read-ahead) */
	u_long		v_id;			/* capability identifier */
	struct mount	*v_mount;		/* ptr to vfs we are in */
	struct vnodeops	*v_op;			/* vnode operations */
	struct vnode	*v_freef;		/* vnode freelist forward */
	struct vnode	**v_freeb;		/* vnode freelist back */
	struct vnode	*v_mountf;		/* vnode mountlist forward */
	struct vnode	**v_mountb;		/* vnode mountlist back */
	struct buf	*v_cleanblkhd;		/* clean blocklist head */
	struct buf	*v_dirtyblkhd;		/* dirty blocklist head */
	long		v_numoutput;		/* num of writes in progress */
	enum vtype	v_type;			/* vnode type */
	union {
		struct mount	*vu_mountedhere;/* ptr to mounted vfs (VDIR) */
		struct socket	*vu_socket;	/* unix ipc (VSOCK) */
		caddr_t		vu_vmdata;	/* private data for vm (VREG) */
		struct specinfo	*vu_specinfo;	/* device (VCHR, VBLK) */
		struct fifoinfo	*vu_fifoinfo;	/* fifo (VFIFO) */
	} v_un;
	enum vtagtype	v_tag;			/* type of underlying data */
	char v_data[VN_MAXPRIVATE];		/* private data for fs */
};
#define v_mountedhere v_un.vu_mountedhere
#define v_socket v_un.vu_socket
#define v_vmdata v_un.vu_vmdata
#define v_specinfo v_un.vu_specinfo
#define v_fifoinfo v_un.vu_fifoinfo

/*
 * vnode flags.
 */
#define	VROOT		0x0001	/* root of its file system */
#define	VTEXT		0x0002	/* vnode is a pure text prototype */
#define	VSYSTEM		0x0004	/* vnode being used by kernel */
#define	VXLOCK		0x0100	/* vnode is locked to change underlying type */
#define	VXWANT		0x0200	/* process is waiting for vnode */
#define	VBWAIT		0x0400	/* waiting for output to complete */
#define	VALIASED	0x0800	/* vnode has an alias */

/*
 * Vnode attributes.  A field value of VNOVAL
 * represents a field whose value is unavailable
 * (getattr) or which is not to be changed (setattr).
 */
struct vattr {
	enum vtype	va_type;	/* vnode type (for create) */
	u_short		va_mode;	/* files access mode and type */
	short		va_nlink;	/* number of references to file */
	uid_t		va_uid;		/* owner user id */
	gid_t		va_gid;		/* owner group id */
	long		va_fsid;	/* file system id (dev for now) */
	long		va_fileid;	/* file id */
	u_quad		va_qsize;	/* file size in bytes */
	long		va_blocksize;	/* blocksize preferred for i/o */
	struct timeval	va_atime;	/* time of last access */
	struct timeval	va_mtime;	/* time of last modification */
	struct timeval	va_ctime;	/* time file changed */
	u_long		va_gen;		/* generation number of file */
	u_long		va_flags;	/* flags defined for file */
	dev_t		va_rdev;	/* device the special file represents */
	u_quad		va_qbytes;	/* bytes of disk space held by file */
};
#if BYTE_ORDER == LITTLE_ENDIAN
#define	va_size		va_qsize.val[0]
#define	va_size_rsv	va_qsize.val[1]
#define	va_bytes	va_qbytes.val[0]
#define	va_bytes_rsv	va_qbytes.val[1]
#else
#define	va_size		va_qsize.val[1]
#define	va_size_rsv	va_qsize.val[0]
#define	va_bytes	va_qbytes.val[1]
#define	va_bytes_rsv	va_qbytes.val[0]
#endif

/*
 * Operations on vnodes.
 */
struct file;
struct flock;
struct nameidata;
struct vnodeops {
	int	(*vn_lookup)__P((
			struct vnode *vp,
			struct nameidata *ndp,
			struct proc *p));
	int	(*vn_create)__P((
			struct nameidata *ndp,
			struct vattr *vap,
			struct proc *p));
	int	(*vn_mknod)__P((
			struct nameidata *ndp,
			struct vattr *vap,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_open)__P((
			struct vnode *vp,
			int mode,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_close)__P((
			struct vnode *vp,
			int fflag,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_access)__P((
			struct vnode *vp,
			int mode,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_getattr)__P((
			struct vnode *vp,
			struct vattr *vap,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_setattr)__P((
			struct vnode *vp,
			struct vattr *vap,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_read)__P((
			struct vnode *vp,
			struct uio *uio,
			int ioflag,
			struct ucred *cred));
	int	(*vn_write)__P((
			struct vnode *vp,
			struct uio *uio,
			int ioflag,
			struct ucred *cred));
	int	(*vn_ioctl)__P((
			struct vnode *vp,
			int command,
			caddr_t data,
			int fflag,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_select)__P((
			struct vnode *vp,
			int which,
			int fflags,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_mmap)__P((
			struct vnode *vp,
			int fflags,
			struct ucred *cred,
			struct proc *p));
	int	(*vn_fsync)__P((
			struct vnode *vp,
			int fflags,
			struct ucred *cred,
			int waitfor,
			struct proc *p));
	int	(*vn_seek)__P((
			struct vnode *vp,
			off_t oldoff,
			off_t newoff,
			struct ucred *cred));
	int	(*vn_remove)__P((
			struct nameidata *ndp,
			struct proc *p));
	int	(*vn_link)__P((
			struct vnode *vp,
			struct nameidata *ndp,
			struct proc *p));
	int	(*vn_rename)__P((
			struct nameidata *fndp,
			struct nameidata *tdnp,
			struct proc *p));
	int	(*vn_mkdir)__P((
			struct nameidata *ndp,
			struct vattr *vap,
			struct proc *p));
	int	(*vn_rmdir)__P((
			struct nameidata *ndp,
			struct proc *p));
	int	(*vn_symlink)__P((
			struct nameidata *ndp,
			struct vattr *vap,
			char *target,
			struct proc *p));
	int	(*vn_readdir)__P((
			struct vnode *vp,
			struct uio *uio,
			struct ucred *cred,
			int *eofflagp));
	int	(*vn_readlink)__P((
			struct vnode *vp,
			struct uio *uio,
			struct ucred *cred));
	int	(*vn_abortop)__P((
			struct nameidata *ndp));
	int	(*vn_inactive)__P((
			struct vnode *vp,
			struct proc *p));
	int	(*vn_reclaim)__P((
			struct vnode *vp));
	int	(*vn_lock)__P((
			struct vnode *vp));
	int	(*vn_unlock)__P((
			struct vnode *vp));
	int	(*vn_bmap)__P((
			struct vnode *vp,
			daddr_t bn,
			struct vnode **vpp,
			daddr_t *bnp));
	int	(*vn_strategy)__P((
			struct buf *bp));
	int	(*vn_print)__P((
			struct vnode *vp));
	int	(*vn_islocked)__P((
			struct vnode *vp));
	int	(*vn_advlock)__P((
			struct vnode *vp,
			caddr_t id,
			int op,
			struct flock *fl,
			int flags));
};

/* Macros to call the vnode ops */
#define	VOP_LOOKUP(v,n,p)	(*((v)->v_op->vn_lookup))(v,n,p)
#define	VOP_CREATE(n,a,p)	(*((n)->ni_dvp->v_op->vn_create))(n,a,p)
#define	VOP_MKNOD(n,a,c,p)	(*((n)->ni_dvp->v_op->vn_mknod))(n,a,c,p)
#define	VOP_OPEN(v,f,c,p)	(*((v)->v_op->vn_open))(v,f,c,p)
#define	VOP_CLOSE(v,f,c,p)	(*((v)->v_op->vn_close))(v,f,c,p)
#define	VOP_ACCESS(v,f,c,p)	(*((v)->v_op->vn_access))(v,f,c,p)
#define	VOP_GETATTR(v,a,c,p)	(*((v)->v_op->vn_getattr))(v,a,c,p)
#define	VOP_SETATTR(v,a,c,p)	(*((v)->v_op->vn_setattr))(v,a,c,p)
#define	VOP_READ(v,u,i,c)	(*((v)->v_op->vn_read))(v,u,i,c)
#define	VOP_WRITE(v,u,i,c)	(*((v)->v_op->vn_write))(v,u,i,c)
#define	VOP_IOCTL(v,o,d,f,c,p)	(*((v)->v_op->vn_ioctl))(v,o,d,f,c,p)
#define	VOP_SELECT(v,w,f,c,p)	(*((v)->v_op->vn_select))(v,w,f,c,p)
#define	VOP_MMAP(v,c,p)		(*((v)->v_op->vn_mmap))(v,c,p)
#define	VOP_FSYNC(v,f,c,w,p)	(*((v)->v_op->vn_fsync))(v,f,c,w,p)
#define	VOP_SEEK(v,p,o,w)	(*((v)->v_op->vn_seek))(v,p,o,w)
#define	VOP_REMOVE(n,p)		(*((n)->ni_dvp->v_op->vn_remove))(n,p)
#define	VOP_LINK(v,n,p)		(*((n)->ni_dvp->v_op->vn_link))(v,n,p)
#define	VOP_RENAME(s,t,p)	(*((s)->ni_dvp->v_op->vn_rename))(s,t,p)
#define	VOP_MKDIR(n,a,p)	(*((n)->ni_dvp->v_op->vn_mkdir))(n,a,p)
#define	VOP_RMDIR(n,p)		(*((n)->ni_dvp->v_op->vn_rmdir))(n,p)
#define	VOP_SYMLINK(n,a,m,p)	(*((n)->ni_dvp->v_op->vn_symlink))(n,a,m,p)
#define	VOP_READDIR(v,u,c,e)	(*((v)->v_op->vn_readdir))(v,u,c,e)
#define	VOP_READLINK(v,u,c)	(*((v)->v_op->vn_readlink))(v,u,c)
#define	VOP_ABORTOP(n)		(*((n)->ni_dvp->v_op->vn_abortop))(n)
#define	VOP_INACTIVE(v,p)	(*((v)->v_op->vn_inactive))(v,p)
#define	VOP_RECLAIM(v)		(*((v)->v_op->vn_reclaim))(v)
#define	VOP_LOCK(v)		(*((v)->v_op->vn_lock))(v)
#define	VOP_UNLOCK(v)		(*((v)->v_op->vn_unlock))(v)
#define	VOP_BMAP(v,s,p,n)	(*((v)->v_op->vn_bmap))(v,s,p,n)
#define	VOP_STRATEGY(b)		(*((b)->b_vp->v_op->vn_strategy))(b)
#define	VOP_PRINT(v)		(*((v)->v_op->vn_print))(v)
#define	VOP_ISLOCKED(v)		(((v)->v_flag & VXLOCK) || \
				(*((v)->v_op->vn_islocked))(v))
#define	VOP_ADVLOCK(v,p,o,l,f)	(*((v)->v_op->vn_advlock))(v,p,o,l,f)

/*
 * flags for ioflag
 */
#define IO_UNIT		0x01		/* do I/O as atomic unit */
#define IO_APPEND	0x02		/* append write to end */
#define IO_SYNC		0x04		/* do I/O synchronously */
#define	IO_NODELOCKED	0x08		/* underlying node already locked */
#define	IO_NDELAY	0x10		/* FNDELAY flag set in file table */

/*
 *  Modes. Some values same as Ixxx entries from inode.h for now
 */
#define	VSUID	04000		/* set user id on execution */
#define	VSGID	02000		/* set group id on execution */
#define	VSVTX	01000		/* save swapped text even after use */
#define	VREAD	0400		/* read, write, execute permissions */
#define	VWRITE	0200
#define	VEXEC	0100

/*
 * Token indicating no attribute value yet assigned
 */
#define VNOVAL	((unsigned)0xffffffff)

#ifdef KERNEL
/*
 * public vnode manipulation functions
 */
int 	vn_open __P((			/* open vnode */
		struct nameidata *ndp,
		struct proc *p,
		int fmode,
		int cmode));
int 	vn_rdwr __P((			/* read or write vnode */
		enum uio_rw rw,
		struct vnode *vp,
		caddr_t base,
		int len,
		off_t offset,
		enum uio_seg segflg,
		int ioflg,
		struct ucred *cred,
		int *aresid,
		struct proc *p));
int	vn_read __P((			/* read a vnode into a uio structure */
		struct file *fp,
		struct uio *uio,
		struct ucred *cred));
int	vn_write __P((			/* write a vnode from a uio structure */
		struct file *fp,
		struct uio *uio,
		struct ucred *cred));
int	vn_ioctl __P((			/* do an ioctl operation on a vnode */
		struct file *fp,
		int com,
		caddr_t data,
		struct proc *p));
int	vn_select __P((			/* do a select operation on a vnode */
		struct file *fp,
		int which,
		struct proc *p));
int 	vn_close __P((			/* close vnode */
		struct file *fp,
		struct proc *p));
int 	getnewvnode __P((		/* allocate a new vnode */
		enum vtagtype tag,
		struct mount *mp,
		struct vnodeops *vops,
		struct vnode **vpp));
int 	bdevvp __P((			/* allocate a new special dev vnode */
		int dev,		/* XXX should be type dev_t, not int */
		struct vnode **vpp));
struct 	vnode *checkalias __P((		/* check for special device aliases */
		struct vnode *vp,
		int nvp_rdev,		/* XXX should be type dev_t, not int */
		struct mount *mp));
void 	vattr_null __P((		/* set attributes to null */
		struct vattr *vap));
int 	vcount __P((struct vnode *vp));	/* total references to a device */
int 	vget __P((struct vnode *vp));	/* get first reference to a vnode */
void 	vref __P((struct vnode *vp));	/* increase reference to a vnode */
void 	vput __P((struct vnode *vp));	/* unlock and release vnode */
void 	vrele __P((struct vnode *vp));	/* release vnode */
void 	vgone __P((struct vnode *vp));	/* completely recycle vnode */
void 	vgoneall __P((struct vnode *vp));/* recycle vnode and all its aliases */

/*
 * Flags to various vnode functions.
 */
#define	SKIPSYSTEM	0x0001		/* vflush: skip vnodes marked VSYSTEM */
#define	FORCECLOSE	0x0002		/* vflush: force file closeure */
#define	DOCLOSE		0x0004		/* vclean: close active files */

#ifndef DIAGNOSTIC
#define VREF(vp)    (vp)->v_usecount++	/* increase reference to a vnode */
#define VHOLD(vp)   (vp)->v_holdcnt++	/* increase buf or page ref to vnode */
#define HOLDRELE(vp) (vp)->v_holdcnt--	/* decrease buf or page ref to vnode */
#define	VATTR_NULL(vap) *(vap) = va_null /* initialize a vattr stucture */
#else /* DIAGNOSTIC */
#define VREF(vp)    vref(vp)
#define VHOLD(vp)   vhold(vp)
#define HOLDRELE(vp) holdrele(vp)
#define	VATTR_NULL(vap) vattr_null(vap)
#endif

#define	NULLVP	((struct vnode *)0)

/*
 * Global vnode data.
 */
extern	struct vnode *rootdir;		/* root (i.e. "/") vnode */
extern	int desiredvnodes;		/* number of vnodes desired */
extern	struct vattr va_null;		/* predefined null vattr structure */
#endif
