/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)nfsnode.h	7.20 (Berkeley) %G%
 */

/*
 * Silly rename structure that hangs off the nfsnode until the name
 * can be removed by nfs_inactive()
 */
struct sillyrename {
	struct	ucred *s_cred;
	struct	vnode *s_dvp;
	long	s_namlen;
	char	s_name[20];
};

/*
 * The nfsnode is the nfs equivalent to ufs's inode. Any similarity
 * is purely coincidental.
 * There is a unique nfsnode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 * An nfsnode is 'named' by its file handle. (nget/nfs_node.c)
 */

struct nfsnode {
	struct	nfsnode *n_chain[2];	/* must be first */
	nfsv2fh_t n_fh;			/* NFS File Handle */
	long	n_flag;			/* Flag for locking.. */
	struct	vnode *n_vnode;		/* vnode associated with this node */
	time_t	n_attrstamp;		/* Time stamp for cached attributes */
	struct	vattr n_vattr;		/* Vnode attribute cache */
	struct	sillyrename *n_sillyrename; /* Ptr to silly rename struct */
	off_t	n_size;			/* Current size of file */
	int	n_error;		/* Save write error value */
	u_long	n_direofoffset;		/* Dir. EOF offset cache */
	union {
		struct {
			time_t	un_mtime; /* Prev modify time. */
			time_t	un_ctime; /* Prev create time. */
		} un_nfs;
		struct {
			u_quad_t un_brev; /* Modify rev when cached */
			u_quad_t un_lrev; /* Modify rev for lease */
			time_t	un_expiry; /* Lease expiry time */
			struct	nfsnode *un_tnext; /* Nqnfs timer chain */
			struct	nfsnode *un_tprev;
		} un_nqnfs;
	} n_un;
	struct	sillyrename n_silly;	/* Silly rename struct */
	long	n_spare[9];		/* Up to a power of 2 */
};

#define	n_mtime		n_un.un_nfs.un_mtime
#define	n_ctime		n_un.un_nfs.un_ctime
#define	n_brev		n_un.un_nqnfs.un_brev
#define	n_lrev		n_un.un_nqnfs.un_lrev
#define	n_expiry	n_un.un_nqnfs.un_expiry
#define	n_tnext		n_un.un_nqnfs.un_tnext
#define	n_tprev		n_un.un_nqnfs.un_tprev

#define	n_forw		n_chain[0]
#define	n_back		n_chain[1]

#ifdef KERNEL
/*
 * Convert between nfsnode pointers and vnode pointers
 */
#define VTONFS(vp)	((struct nfsnode *)(vp)->v_data)
#define NFSTOV(np)	((struct vnode *)(np)->n_vnode)
#endif
/*
 * Flags for n_flag
 */
#define	NMODIFIED	0x0004	/* Might have a modified buffer in bio */
#define	NWRITEERR	0x0008	/* Flag write errors so close will know */
#define	NQNFSNONCACHE	0x0020	/* Non-cachable lease */
#define	NQNFSWRITE	0x0040	/* Write lease */
#define	NQNFSEVICTED	0x0080	/* Has been evicted */

/*
 * Prototypes for NFS vnode operations
 */
int	nfs_lookup __P((
		struct vnode *dvp,
		struct vnode **vpp,
		struct componentname *cnp));
int	nfs_create __P((
		struct vnode *dvp,
		struct vnode **vpp,
		struct componentname *cnp,
		struct vattr *vap));
int	nfs_mknod __P((
		struct vnode *dvp,
		struct vnode **vpp,
		struct componentname *cnp,
		struct vattr *vap));
int	nfs_open __P((
		struct vnode *vp,
		int mode,
		struct ucred *cred,
		struct proc *p));
int	nfs_close __P((
		struct vnode *vp,
		int fflag,
		struct ucred *cred,
		struct proc *p));
int	nfs_access __P((
		struct vnode *vp,
		int mode,
		struct ucred *cred,
		struct proc *p));
int	nfs_getattr __P((
		struct vnode *vp,
		struct vattr *vap,
		struct ucred *cred,
		struct proc *p));
int	nfs_setattr __P((
		struct vnode *vp,
		struct vattr *vap,
		struct ucred *cred,
		struct proc *p));
int	nfs_read __P((
		struct vnode *vp,
		struct uio *uio,
		int ioflag,
		struct ucred *cred));
int	nfs_write __P((
		struct vnode *vp,
		struct uio *uio,
		int ioflag,
		struct ucred *cred));
#define nfs_ioctl ((int (*) __P(( \
		struct vnode *vp, \
		int command, \
		caddr_t data, \
		int fflag, \
		struct ucred *cred, \
		struct proc *p))) enoioctl)
#define nfs_select ((int (*) __P(( \
		struct vnode *vp, \
		int which, \
		int fflags, \
		struct ucred *cred, \
		struct proc *p))) seltrue)
int	nfs_mmap __P((
		struct vnode *vp,
		int fflags,
		struct ucred *cred,
		struct proc *p));
int	nfs_fsync __P((
		struct vnode *vp,
		int fflags,
		struct ucred *cred,
		int waitfor,
		struct proc *p));
#define nfs_seek ((int (*) __P(( \
		struct vnode *vp, \
		off_t oldoff, \
		off_t newoff, \
		struct ucred *cred))) nullop)
int	nfs_remove __P((
		struct vnode *dvp,
		struct vnode *vp,
		struct componentname *cnp));
int	nfs_link __P((
		register struct vnode *vp,
		struct vnode *tdvp,
		struct componentname *cnp));
int	nfs_rename __P((
		struct vnode *fdvp,
		struct vnode *fvp,
		struct componentname *fcnp,
		struct vnode *tdvp,
		struct vnode *tvp,
		struct componentname *tcnp));
int	nfs_mkdir __P((
		struct vnode *dvp,
		struct vnode **vpp,
		struct componentname *cnp,
		struct vattr *vap));
int	nfs_rmdir __P((
		struct vnode *dvp,
		struct vnode *vp,
		struct componentname *cnp));
int	nfs_symlink __P((
		struct vnode *dvp,
		struct vnode **vpp,
		struct componentname *cnp,
		struct vattr *vap,
		char *nm));
int	nfs_readdir __P((
		struct vnode *vp,
		struct uio *uio,
		struct ucred *cred,
		int *eofflagp));
int	nfs_readlink __P((
		struct vnode *vp,
		struct uio *uio,
		struct ucred *cred));
int	nfs_abortop __P((
		struct vnode *,
		struct componentname *));
int	nfs_inactive __P((
		struct vnode *vp,
		struct proc *p));
int	nfs_reclaim __P((
		struct vnode *vp));
int	nfs_lock __P((
		struct vnode *vp));
int	nfs_unlock __P((
		struct vnode *vp));
int	nfs_bmap __P((
		struct vnode *vp,
		daddr_t bn,
		struct vnode **vpp,
		daddr_t *bnp));
int	nfs_strategy __P((
		struct buf *bp));
int	nfs_print __P((
		struct vnode *vp));
int	nfs_islocked __P((
		struct vnode *vp));
int	nfs_advlock __P((
		struct vnode *vp,
		caddr_t id,
		int op,
		struct flock *fl,
		int flags));
int	nfs_blkatoff __P((
		struct vnode *vp,
		off_t offset,
		char **res,
		struct buf **bpp));
int	nfs_vget __P((
		struct mount *mp,
		ino_t ino,
		struct vnode **vpp));
int	nfs_valloc __P((
		struct vnode *pvp,
		int mode,
		struct ucred *cred,
		struct vnode **vpp));
void	nfs_vfree __P((
		struct vnode *pvp,
		ino_t ino,
		int mode));
int	nfs_truncate __P((
		struct vnode *vp,
		off_t length,
		int flags,
		struct ucred *cred));
int	nfs_update __P((
		struct vnode *vp,
		struct timeval *ta,
		struct timeval *tm,
		int waitfor));
int	bwrite();		/* NFS needs a bwrite routine */
