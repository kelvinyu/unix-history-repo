/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)nfs_vnops.c	7.76 (Berkeley) %G%
 */

/*
 * vnode op calls for sun nfs version 2
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/conf.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/fifo.h>
#include <sys/map.h>

#include <vm/vm.h>

#include <nfs/rpcv2.h>
#include <nfs/nfsv2.h>
#include <nfs/nfs.h>
#include <nfs/nfsnode.h>
#include <nfs/nfsmount.h>
#include <nfs/xdr_subs.h>
#include <nfs/nfsm_subs.h>
#include <nfs/nqnfs.h>

/* Defs */
#define	TRUE	1
#define	FALSE	0

/*
 * Global vfs data structures for nfs
 */
int (**nfsv2_vnodeop_p)();
struct vnodeopv_entry_desc nfsv2_vnodeop_entries[] = {
	{ &vop_default_desc, vn_default_error },
	{ &vop_lookup_desc, nfs_lookup },		/* lookup */
	{ &vop_create_desc, nfs_create },		/* create */
	{ &vop_mknod_desc, nfs_mknod },		/* mknod */
	{ &vop_open_desc, nfs_open },		/* open */
	{ &vop_close_desc, nfs_close },		/* close */
	{ &vop_access_desc, nfs_access },		/* access */
	{ &vop_getattr_desc, nfs_getattr },		/* getattr */
	{ &vop_setattr_desc, nfs_setattr },		/* setattr */
	{ &vop_read_desc, nfs_read },		/* read */
	{ &vop_write_desc, nfs_write },		/* write */
	{ &vop_ioctl_desc, nfs_ioctl },		/* ioctl */
	{ &vop_select_desc, nfs_select },		/* select */
	{ &vop_mmap_desc, nfs_mmap },		/* mmap */
	{ &vop_fsync_desc, nfs_fsync },		/* fsync */
	{ &vop_seek_desc, nfs_seek },		/* seek */
	{ &vop_remove_desc, nfs_remove },		/* remove */
	{ &vop_link_desc, nfs_link },		/* link */
	{ &vop_rename_desc, nfs_rename },		/* rename */
	{ &vop_mkdir_desc, nfs_mkdir },		/* mkdir */
	{ &vop_rmdir_desc, nfs_rmdir },		/* rmdir */
	{ &vop_symlink_desc, nfs_symlink },		/* symlink */
	{ &vop_readdir_desc, nfs_readdir },		/* readdir */
	{ &vop_readlink_desc, nfs_readlink },		/* readlink */
	{ &vop_abortop_desc, nfs_abortop },		/* abortop */
	{ &vop_inactive_desc, nfs_inactive },		/* inactive */
	{ &vop_reclaim_desc, nfs_reclaim },		/* reclaim */
	{ &vop_lock_desc, nfs_lock },		/* lock */
	{ &vop_unlock_desc, nfs_unlock },		/* unlock */
	{ &vop_bmap_desc, nfs_bmap },		/* bmap */
	{ &vop_strategy_desc, nfs_strategy },		/* strategy */
	{ &vop_print_desc, nfs_print },		/* print */
	{ &vop_islocked_desc, nfs_islocked },		/* islocked */
	{ &vop_advlock_desc, nfs_advlock },		/* advlock */
	{ &vop_blkatoff_desc, nfs_blkatoff },		/* blkatoff */
	{ &vop_vget_desc, nfs_vget },		/* vget */
	{ &vop_valloc_desc, nfs_valloc },		/* valloc */
	{ &vop_vfree_desc, nfs_vfree },		/* vfree */
	{ &vop_truncate_desc, nfs_truncate },		/* truncate */
	{ &vop_update_desc, nfs_update },		/* update */
	{ &vop_bwrite_desc, vn_bwrite },
	{ (struct vnodeop_desc*)NULL, (int(*)())NULL }
};
struct vnodeopv_desc nfsv2_vnodeop_opv_desc =
	{ &nfsv2_vnodeop_p, nfsv2_vnodeop_entries };

/*
 * Special device vnode ops
 */
int (**spec_nfsv2nodeop_p)();
struct vnodeopv_entry_desc spec_nfsv2nodeop_entries[] = {
	{ &vop_default_desc, vn_default_error },
	{ &vop_lookup_desc, spec_lookup },		/* lookup */
	{ &vop_create_desc, spec_create },		/* create */
	{ &vop_mknod_desc, spec_mknod },		/* mknod */
	{ &vop_open_desc, spec_open },		/* open */
	{ &vop_close_desc, spec_close },		/* close */
	{ &vop_access_desc, nfs_access },		/* access */
	{ &vop_getattr_desc, nfs_getattr },		/* getattr */
	{ &vop_setattr_desc, nfs_setattr },		/* setattr */
	{ &vop_read_desc, spec_read },		/* read */
	{ &vop_write_desc, spec_write },		/* write */
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
	{ &vop_inactive_desc, nfs_inactive },		/* inactive */
	{ &vop_reclaim_desc, nfs_reclaim },		/* reclaim */
	{ &vop_lock_desc, nfs_lock },		/* lock */
	{ &vop_unlock_desc, nfs_unlock },		/* unlock */
	{ &vop_bmap_desc, spec_bmap },		/* bmap */
	{ &vop_strategy_desc, spec_strategy },		/* strategy */
	{ &vop_print_desc, nfs_print },		/* print */
	{ &vop_islocked_desc, nfs_islocked },		/* islocked */
	{ &vop_advlock_desc, spec_advlock },		/* advlock */
	{ &vop_blkatoff_desc, spec_blkatoff },		/* blkatoff */
	{ &vop_vget_desc, spec_vget },		/* vget */
	{ &vop_valloc_desc, spec_valloc },		/* valloc */
	{ &vop_vfree_desc, spec_vfree },		/* vfree */
	{ &vop_truncate_desc, spec_truncate },		/* truncate */
	{ &vop_update_desc, nfs_update },		/* update */
	{ &vop_bwrite_desc, vn_bwrite },
	{ (struct vnodeop_desc*)NULL, (int(*)())NULL }
};
struct vnodeopv_desc spec_nfsv2nodeop_opv_desc =
	{ &spec_nfsv2nodeop_p, spec_nfsv2nodeop_entries };

#ifdef FIFO
int (**fifo_nfsv2nodeop_p)();
struct vnodeopv_entry_desc fifo_nfsv2nodeop_entries[] = {
	{ &vop_default_desc, vn_default_error },
	{ &vop_lookup_desc, fifo_lookup },		/* lookup */
	{ &vop_create_desc, fifo_create },		/* create */
	{ &vop_mknod_desc, fifo_mknod },		/* mknod */
	{ &vop_open_desc, fifo_open },		/* open */
	{ &vop_close_desc, fifo_close },		/* close */
	{ &vop_access_desc, nfs_access },		/* access */
	{ &vop_getattr_desc, nfs_getattr },		/* getattr */
	{ &vop_setattr_desc, nfs_setattr },		/* setattr */
	{ &vop_read_desc, fifo_read },		/* read */
	{ &vop_write_desc, fifo_write },		/* write */
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
	{ &vop_inactive_desc, nfs_inactive },		/* inactive */
	{ &vop_reclaim_desc, nfs_reclaim },		/* reclaim */
	{ &vop_lock_desc, nfs_lock },		/* lock */
	{ &vop_unlock_desc, nfs_unlock },		/* unlock */
	{ &vop_bmap_desc, fifo_bmap },		/* bmap */
	{ &vop_strategy_desc, fifo_badop },		/* strategy */
	{ &vop_print_desc, nfs_print },		/* print */
	{ &vop_islocked_desc, nfs_islocked },		/* islocked */
	{ &vop_advlock_desc, fifo_advlock },		/* advlock */
	{ &vop_blkatoff_desc, fifo_blkatoff },		/* blkatoff */
	{ &vop_vget_desc, fifo_vget },		/* vget */
	{ &vop_valloc_desc, fifo_valloc },		/* valloc */
	{ &vop_vfree_desc, fifo_vfree },		/* vfree */
	{ &vop_truncate_desc, fifo_truncate },		/* truncate */
	{ &vop_update_desc, nfs_update },		/* update */
	{ &vop_bwrite_desc, vn_bwrite },
	{ (struct vnodeop_desc*)NULL, (int(*)())NULL }
};
struct vnodeopv_desc fifo_nfsv2nodeop_opv_desc =
	{ &fifo_nfsv2nodeop_p, fifo_nfsv2nodeop_entries };
#endif /* FIFO */

/*
 * Global variables
 */
extern u_long nfs_procids[NFS_NPROCS];
extern u_long nfs_prog, nfs_vers;
extern char nfsiobuf[MAXPHYS+NBPG];
struct buf nfs_bqueue;		/* Queue head for nfsiod's */
struct proc *nfs_iodwant[NFS_MAXASYNCDAEMON];
int nfs_numasync = 0;
#define	DIRHDSIZ	(sizeof (struct readdir) - (MAXNAMLEN + 1))

/*
 * nfs null call from vfs.
 */
int
nfs_null(vp, cred, procp)
	struct vnode *vp;
	struct ucred *cred;
	struct proc *procp;
{
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb;
	
	nfsm_reqhead(vp, NFSPROC_NULL, 0);
	nfsm_request(vp, NFSPROC_NULL, procp, cred);
	nfsm_reqdone;
	return (error);
}

/*
 * nfs access vnode op.
 * Essentially just get vattr and then imitate iaccess()
 */
int
nfs_access (ap)
	struct vop_access_args *ap;
{
	USES_VOP_GETATTR;
	register struct vattr *vap;
	register gid_t *gp;
	struct vattr vattr;
	register int i;
	int error;

	/*
	 * If you're the super-user,
	 * you always get access.
	 */
	if (ap->a_cred->cr_uid == 0)
		return (0);
	vap = &vattr;
	if (error = VOP_GETATTR(ap->a_vp, vap, ap->a_cred, ap->a_p))
		return (error);
	/*
	 * Access check is based on only one of owner, group, public.
	 * If not owner, then check group. If not a member of the
	 * group, then check public access.
	 */
	if (ap->a_cred->cr_uid != vap->va_uid) {
		ap->a_mode >>= 3;
		gp = ap->a_cred->cr_groups;
		for (i = 0; i < ap->a_cred->cr_ngroups; i++, gp++)
			if (vap->va_gid == *gp)
				goto found;
		ap->a_mode >>= 3;
found:
		;
	}
	if ((vap->va_mode & ap->a_mode) != 0)
		return (0);
	return (EACCES);
}

/*
 * nfs open vnode op
 * Just check to see if the type is ok
 * and that deletion is not in progress.
 */
/* ARGSUSED */
int
nfs_open (ap)
	struct vop_open_args *ap;
{

	if (ap->a_vp->v_type != VREG && ap->a_vp->v_type != VDIR && ap->a_vp->v_type != VLNK)
		return (EACCES);
	if ((VFSTONFS(ap->a_vp->v_mount)->nm_flag & NFSMNT_NQNFS) == 0)
		VTONFS(ap->a_vp)->n_attrstamp = 0; /* For Open/Close consistency */
	return (0);
}

/*
 * nfs close vnode op
 * For reg files, invalidate any buffer cache entries.
 */
/* ARGSUSED */
int
nfs_close (ap)
	struct vop_close_args *ap;
{
	register struct nfsnode *np = VTONFS(ap->a_vp);
	int error = 0;

	if ((np->n_flag & NMODIFIED) &&
	    (VFSTONFS(ap->a_vp->v_mount)->nm_flag & NFSMNT_NQNFS) == 0 &&
	    ap->a_vp->v_type == VREG) {
		np->n_flag &= ~NMODIFIED;
		vinvalbuf(ap->a_vp, TRUE);
		np->n_attrstamp = 0;
		if (np->n_flag & NWRITEERR) {
			np->n_flag &= ~NWRITEERR;
			error = np->n_error;
		}
	}
	return (error);
}

/*
 * nfs getattr call from vfs.
 */
int
nfs_getattr (ap)
	struct vop_getattr_args *ap;
{
	register caddr_t cp;
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	
	/* First look in the cache.. */
	if (nfs_getattrcache(ap->a_vp, ap->a_vap) == 0)
		return (0);
	nfsstats.rpccnt[NFSPROC_GETATTR]++;
	nfsm_reqhead(ap->a_vp, NFSPROC_GETATTR, NFSX_FH);
	nfsm_fhtom(ap->a_vp);
	nfsm_request(ap->a_vp, NFSPROC_GETATTR, ap->a_p, ap->a_cred);
	nfsm_loadattr(ap->a_vp, ap->a_vap);
	nfsm_reqdone;
	return (error);
}

/*
 * nfs setattr call.
 */
int
nfs_setattr (ap)
	struct vop_setattr_args *ap;
{
	register struct nfsv2_sattr *sp;
	register caddr_t cp;
	register long t1;
	caddr_t bpos, dpos, cp2;
	u_long *tl;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	struct nfsnode *np = VTONFS(ap->a_vp);
	u_quad_t frev;

	nfsstats.rpccnt[NFSPROC_SETATTR]++;
	nfsm_reqhead(ap->a_vp, NFSPROC_SETATTR, NFSX_FH+NFSX_SATTR);
	nfsm_fhtom(ap->a_vp);
	nfsm_build(sp, struct nfsv2_sattr *, NFSX_SATTR);
	if (ap->a_vap->va_mode == 0xffff)
		sp->sa_mode = VNOVAL;
	else
		sp->sa_mode = vtonfs_mode(ap->a_vp->v_type, ap->a_vap->va_mode);
	if (ap->a_vap->va_uid == 0xffff)
		sp->sa_uid = VNOVAL;
	else
		sp->sa_uid = txdr_unsigned(ap->a_vap->va_uid);
	if (ap->a_vap->va_gid == 0xffff)
		sp->sa_gid = VNOVAL;
	else
		sp->sa_gid = txdr_unsigned(ap->a_vap->va_gid);
	sp->sa_size = txdr_unsigned(ap->a_vap->va_size);
	sp->sa_atime.tv_sec = txdr_unsigned(ap->a_vap->va_atime.tv_sec);
	sp->sa_atime.tv_usec = txdr_unsigned(ap->a_vap->va_flags);
	txdr_time(&ap->a_vap->va_mtime, &sp->sa_mtime);
	if (ap->a_vap->va_size != VNOVAL || ap->a_vap->va_mtime.tv_sec != VNOVAL ||
	    ap->a_vap->va_atime.tv_sec != VNOVAL) {
		if (np->n_flag & NMODIFIED) {
			np->n_flag &= ~NMODIFIED;
			if (ap->a_vap->va_size == 0)
				vinvalbuf(ap->a_vp, FALSE);
			else
				vinvalbuf(ap->a_vp, TRUE);
		}
	}
	nfsm_request(ap->a_vp, NFSPROC_SETATTR, ap->a_p, ap->a_cred);
	nfsm_loadattr(ap->a_vp, (struct vattr *)0);
	if ((VFSTONFS(ap->a_vp->v_mount)->nm_flag & NFSMNT_NQNFS) &&
	    NQNFS_CKCACHABLE(ap->a_vp, NQL_WRITE)) {
		nfsm_dissect(tl, u_long *, 2*NFSX_UNSIGNED);
		fxdr_hyper(tl, &frev);
		if (QUADGT(frev, np->n_brev))
			np->n_brev = frev;
	}
	nfsm_reqdone;
	return (error);
}

/*
 * nfs lookup call, one step at a time...
 * First look in cache
 * If not found, unlock the directory nfsnode and do the rpc
 */
int
nfs_lookup (ap)
	struct vop_lookup_args *ap;
{
	USES_VOP_GETATTR;
	register struct vnode *vdp;
	register u_long *tl;
	register caddr_t cp;
	register long t1, t2;
	struct nfsmount *nmp;
	struct nfsnode *tp;
	caddr_t bpos, dpos, cp2;
	time_t reqtime;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	struct vnode *newvp;
	long len;
	nfsv2fh_t *fhp;
	struct nfsnode *np;
	int lockparent, wantparent, error = 0;
	int nqlflag, cachable;
	u_quad_t frev;

	*ap->a_vpp = NULL;
	if (ap->a_dvp->v_type != VDIR)
		return (ENOTDIR);
	lockparent = ap->a_cnp->cn_flags & LOCKPARENT;
	wantparent = ap->a_cnp->cn_flags & (LOCKPARENT|WANTPARENT);
	nmp = VFSTONFS(ap->a_dvp->v_mount);
	np = VTONFS(ap->a_dvp);
	if ((error = cache_lookup(ap->a_dvp, ap->a_vpp, ap->a_cnp)) && error != ENOENT) {
		struct vattr vattr;
		int vpid;

		vdp = *ap->a_vpp;
		vpid = vdp->v_id;
		/*
		 * See the comment starting `Step through' in ufs/ufs_lookup.c
		 * for an explanation of the locking protocol
		 */
		if (ap->a_dvp == vdp) {
			VREF(vdp);
			error = 0;
		} else
			error = vget(vdp);
		if (!error) {
			if (vpid == vdp->v_id) {
			   if (nmp->nm_flag & NFSMNT_NQNFS) {
			        if (NQNFS_CKCACHABLE(ap->a_dvp, NQL_READ)) {
					if (QUADNE(np->n_lrev, np->n_brev) ||
					    (np->n_flag & NMODIFIED)) {
						np->n_direofoffset = 0;
						cache_purge(ap->a_dvp);
						np->n_flag &= ~NMODIFIED;
						vinvalbuf(ap->a_dvp, FALSE);
						np->n_brev = np->n_lrev;
					} else {
						nfsstats.lookupcache_hits++;
						if (ap->a_cnp->cn_nameiop != LOOKUP &&
						    (ap->a_cnp->cn_flags&ISLASTCN))
						    ap->a_cnp->cn_flags |= SAVENAME;
						return (0);
					}
				}
			   } else if (!VOP_GETATTR(vdp, &vattr, ap->a_cnp->cn_cred, ap->a_cnp->cn_proc) &&
			       vattr.va_ctime.tv_sec == VTONFS(vdp)->n_ctime) {
				nfsstats.lookupcache_hits++;
				if (ap->a_cnp->cn_nameiop != LOOKUP && (ap->a_cnp->cn_flags&ISLASTCN))
					ap->a_cnp->cn_flags |= SAVENAME;
				return (0);
			   }
			   cache_purge(vdp);
			}
			vrele(vdp);
		}
		*ap->a_vpp = NULLVP;
	}
	error = 0;
	nfsstats.lookupcache_misses++;
	nfsstats.rpccnt[NFSPROC_LOOKUP]++;
	len = ap->a_cnp->cn_namelen;
	nfsm_reqhead(ap->a_dvp, NFSPROC_LOOKUP, NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(len));

	/*
	 * For nqnfs optionally piggyback a getlease request for the name
	 * being looked up.
	 */
	if (nmp->nm_flag & NFSMNT_NQNFS) {
		if ((nmp->nm_flag & NFSMNT_NQLOOKLEASE) &&
		    ((ap->a_cnp->cn_flags&MAKEENTRY) && (ap->a_cnp->cn_nameiop != DELETE || !(ap->a_cnp->cn_flags&ISLASTCN)))) {
			nfsm_build(tl, u_long *, 2*NFSX_UNSIGNED);
			*tl++ = txdr_unsigned(NQL_READ);
			*tl = txdr_unsigned(nmp->nm_leaseterm);
		} else {
			nfsm_build(tl, u_long *, NFSX_UNSIGNED);
			*tl = 0;
		}
	}
	nfsm_fhtom(ap->a_dvp);
	nfsm_strtom(ap->a_cnp->cn_nameptr, len, NFS_MAXNAMLEN);
	reqtime = time.tv_sec;
	nfsm_request(ap->a_dvp, NFSPROC_LOOKUP, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
nfsmout:
	if (error) {
		if ((ap->a_cnp->cn_nameiop == CREATE || ap->a_cnp->cn_nameiop == RENAME) &&
		    (ap->a_cnp->cn_flags & ISLASTCN) && error == ENOENT)
			error = EJUSTRETURN;
		if (ap->a_cnp->cn_nameiop != LOOKUP && (ap->a_cnp->cn_flags&ISLASTCN))
			ap->a_cnp->cn_flags |= SAVENAME;
		return (error);
	}
	if (nmp->nm_flag & NFSMNT_NQNFS) {
		nfsm_dissect(tl, u_long *, NFSX_UNSIGNED);
		if (*tl) {
			nqlflag = fxdr_unsigned(int, *tl);
			nfsm_dissect(tl, u_long *, 4*NFSX_UNSIGNED);
			cachable = fxdr_unsigned(int, *tl++);
			reqtime += fxdr_unsigned(int, *tl++);
			fxdr_hyper(tl, &frev);
		} else
			nqlflag = 0;
	}
	nfsm_dissect(fhp, nfsv2fh_t *, NFSX_FH);

	/*
	 * Handle RENAME case...
	 */
	if (ap->a_cnp->cn_nameiop == RENAME && wantparent && (ap->a_cnp->cn_flags&ISLASTCN)) {
		if (!bcmp(np->n_fh.fh_bytes, (caddr_t)fhp, NFSX_FH)) {
			m_freem(mrep);
			return (EISDIR);
		}
		if (error = nfs_nget(ap->a_dvp->v_mount, fhp, &np)) {
			m_freem(mrep);
			return (error);
		}
		newvp = NFSTOV(np);
		if (error =
		    nfs_loadattrcache(&newvp, &md, &dpos, (struct vattr *)0)) {
			vrele(newvp);
			m_freem(mrep);
			return (error);
		}
		*ap->a_vpp = newvp;
		m_freem(mrep);
		ap->a_cnp->cn_flags |= SAVENAME;
		return (0);
	}

	if (!bcmp(np->n_fh.fh_bytes, (caddr_t)fhp, NFSX_FH)) {
		VREF(ap->a_dvp);
		newvp = ap->a_dvp;
	} else {
		if (error = nfs_nget(ap->a_dvp->v_mount, fhp, &np)) {
			m_freem(mrep);
			return (error);
		}
		newvp = NFSTOV(np);
	}
	if (error = nfs_loadattrcache(&newvp, &md, &dpos, (struct vattr *)0)) {
		vrele(newvp);
		m_freem(mrep);
		return (error);
	}
	m_freem(mrep);
	*ap->a_vpp = newvp;
	if (ap->a_cnp->cn_nameiop != LOOKUP && (ap->a_cnp->cn_flags&ISLASTCN))
		ap->a_cnp->cn_flags |= SAVENAME;
	if ((ap->a_cnp->cn_flags&MAKEENTRY) && (ap->a_cnp->cn_nameiop != DELETE || !(ap->a_cnp->cn_flags&ISLASTCN))) {
		if ((nmp->nm_flag & NFSMNT_NQNFS) == 0)
			np->n_ctime = np->n_vattr.va_ctime.tv_sec;
		else if (nqlflag && reqtime > time.tv_sec) {
			if (np->n_tnext) {
				if (np->n_tnext == (struct nfsnode *)nmp)
					nmp->nm_tprev = np->n_tprev;
				else
					np->n_tnext->n_tprev = np->n_tprev;
				if (np->n_tprev == (struct nfsnode *)nmp)
					nmp->nm_tnext = np->n_tnext;
				else
					np->n_tprev->n_tnext = np->n_tnext;
				if (nqlflag == NQL_WRITE)
					np->n_flag |= NQNFSWRITE;
			} else if (nqlflag == NQL_READ)
				np->n_flag &= ~NQNFSWRITE;
			else
				np->n_flag |= NQNFSWRITE;
			if (cachable)
				np->n_flag &= ~NQNFSNONCACHE;
			else
				np->n_flag |= NQNFSNONCACHE;
			np->n_expiry = reqtime;
			np->n_lrev = frev;
			tp = nmp->nm_tprev;
			while (tp != (struct nfsnode *)nmp && tp->n_expiry > np->n_expiry)
				tp = tp->n_tprev;
			if (tp == (struct nfsnode *)nmp) {
				np->n_tnext = nmp->nm_tnext;
				nmp->nm_tnext = np;
			} else {
				np->n_tnext = tp->n_tnext;
				tp->n_tnext = np;
			}
			np->n_tprev = tp;
			if (np->n_tnext == (struct nfsnode *)nmp)
				nmp->nm_tprev = np;
			else
				np->n_tnext->n_tprev = np;
		}
		cache_enter(ap->a_dvp, *ap->a_vpp, ap->a_cnp);
	}
	return (0);
}

/*
 * nfs read call.
 * Just call nfs_bioread() to do the work.
 */
int
nfs_read (ap)
	struct vop_read_args *ap;
{
	if (ap->a_vp->v_type != VREG)
		return (EPERM);
	return (nfs_bioread(ap->a_vp, ap->a_uio, ap->a_ioflag, ap->a_cred));
}

/*
 * nfs readlink call
 */
int
nfs_readlink (ap)
	struct vop_readlink_args *ap;
{
	if (ap->a_vp->v_type != VLNK)
		return (EPERM);
	return (nfs_bioread(ap->a_vp, ap->a_uio, 0, ap->a_cred));
}

/*
 * Do a readlink rpc.
 * Called by nfs_doio() from below the buffer cache.
 */
int
nfs_readlinkrpc(vp, uiop, cred)
	register struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	register u_long *tl;
	register caddr_t cp;
	register long t1;
	caddr_t bpos, dpos, cp2;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	long len;

	nfsstats.rpccnt[NFSPROC_READLINK]++;
	nfsm_reqhead(vp, NFSPROC_READLINK, NFSX_FH);
	nfsm_fhtom(vp);
	nfsm_request(vp, NFSPROC_READLINK, uiop->uio_procp, cred);
	nfsm_strsiz(len, NFS_MAXPATHLEN);
	nfsm_mtouio(uiop, len);
	nfsm_reqdone;
	return (error);
}

/*
 * nfs read rpc call
 * Ditto above
 */
int
nfs_readrpc(vp, uiop, cred)
	register struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	register u_long *tl;
	register caddr_t cp;
	register long t1;
	caddr_t bpos, dpos, cp2;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	struct nfsmount *nmp;
	long len, retlen, tsiz;

	nmp = VFSTONFS(vp->v_mount);
	tsiz = uiop->uio_resid;
	while (tsiz > 0) {
		nfsstats.rpccnt[NFSPROC_READ]++;
		len = (tsiz > nmp->nm_rsize) ? nmp->nm_rsize : tsiz;
		nfsm_reqhead(vp, NFSPROC_READ, NFSX_FH+NFSX_UNSIGNED*3);
		nfsm_fhtom(vp);
		nfsm_build(tl, u_long *, NFSX_UNSIGNED*3);
		*tl++ = txdr_unsigned(uiop->uio_offset);
		*tl++ = txdr_unsigned(len);
		*tl = 0;
		nfsm_request(vp, NFSPROC_READ, uiop->uio_procp, cred);
		nfsm_loadattr(vp, (struct vattr *)0);
		nfsm_strsiz(retlen, nmp->nm_rsize);
		nfsm_mtouio(uiop, retlen);
		m_freem(mrep);
		if (retlen < len)
			tsiz = 0;
		else
			tsiz -= len;
	}
nfsmout:
	return (error);
}

/*
 * nfs write call
 */
int
nfs_writerpc(vp, uiop, cred)
	register struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	register u_long *tl;
	register caddr_t cp;
	register long t1;
	caddr_t bpos, dpos, cp2;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	struct nfsmount *nmp;
	struct nfsnode *np = VTONFS(vp);
	u_quad_t frev;
	long len, tsiz;

	nmp = VFSTONFS(vp->v_mount);
	tsiz = uiop->uio_resid;
	while (tsiz > 0) {
		nfsstats.rpccnt[NFSPROC_WRITE]++;
		len = (tsiz > nmp->nm_wsize) ? nmp->nm_wsize : tsiz;
		nfsm_reqhead(vp, NFSPROC_WRITE,
			NFSX_FH+NFSX_UNSIGNED*4+nfsm_rndup(len));
		nfsm_fhtom(vp);
		nfsm_build(tl, u_long *, NFSX_UNSIGNED*4);
		*(tl+1) = txdr_unsigned(uiop->uio_offset);
		*(tl+3) = txdr_unsigned(len);
		nfsm_uiotom(uiop, len);
		nfsm_request(vp, NFSPROC_WRITE, uiop->uio_procp, cred);
		nfsm_loadattr(vp, (struct vattr *)0);
		if (nmp->nm_flag & NFSMNT_MYWRITE)
			VTONFS(vp)->n_mtime = VTONFS(vp)->n_vattr.va_mtime.tv_sec;
		else if ((nmp->nm_flag & NFSMNT_NQNFS) &&
			 NQNFS_CKCACHABLE(vp, NQL_WRITE)) {
			nfsm_dissect(tl, u_long *, 2*NFSX_UNSIGNED);
			fxdr_hyper(tl, &frev);
			if (QUADGT(frev, np->n_brev))
				np->n_brev = frev;
		}
		m_freem(mrep);
		tsiz -= len;
	}
nfsmout:
	if (error)
		uiop->uio_resid = tsiz;
	return (error);
}

/*
 * nfs mknod call
 * This is a kludge. Use a create rpc but with the IFMT bits of the mode
 * set to specify the file type and the size field for rdev.
 */
/* ARGSUSED */
int
nfs_mknod (ap)
	struct vop_mknod_args *ap;
{
	USES_VOP_ABORTOP;
	register struct nfsv2_sattr *sp;
	register u_long *tl;
	register caddr_t cp;
	register long t2;
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	u_long rdev;

	if (ap->a_vap->va_type == VCHR || ap->a_vap->va_type == VBLK)
		rdev = txdr_unsigned(ap->a_vap->va_rdev);
#ifdef FIFO
	else if (ap->a_vap->va_type == VFIFO)
		rdev = 0xffffffff;
#endif /* FIFO */
	else {
		VOP_ABORTOP(ap->a_dvp, ap->a_cnp);
		vput(ap->a_dvp);
		return (EOPNOTSUPP);
	}
	nfsstats.rpccnt[NFSPROC_CREATE]++;
	nfsm_reqhead(ap->a_dvp, NFSPROC_CREATE,
	  NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(ap->a_cnp->cn_namelen)+NFSX_SATTR);
	nfsm_fhtom(ap->a_dvp);
	nfsm_strtom(ap->a_cnp->cn_nameptr, ap->a_cnp->cn_namelen, NFS_MAXNAMLEN);
	nfsm_build(sp, struct nfsv2_sattr *, NFSX_SATTR);
	sp->sa_mode = vtonfs_mode(ap->a_vap->va_type, ap->a_vap->va_mode);
	sp->sa_uid = txdr_unsigned(ap->a_cnp->cn_cred->cr_uid);
	sp->sa_gid = txdr_unsigned(ap->a_cnp->cn_cred->cr_gid);
	sp->sa_size = rdev;
	/* or should these be VNOVAL ?? */
	txdr_time(&ap->a_vap->va_atime, &sp->sa_atime);
	txdr_time(&ap->a_vap->va_mtime, &sp->sa_mtime);
	nfsm_request(ap->a_dvp, NFSPROC_CREATE, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
	nfsm_reqdone;
	FREE(ap->a_cnp->cn_pnbuf, M_NAMEI);
	VTONFS(ap->a_dvp)->n_flag |= NMODIFIED;
	vrele(ap->a_dvp);
	return (error);
}

/*
 * nfs file create call
 */
int
nfs_create (ap)
	struct vop_create_args *ap;
{
	register struct nfsv2_sattr *sp;
	register u_long *tl;
	register caddr_t cp;
	register long t1, t2;
	caddr_t bpos, dpos, cp2;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	nfsstats.rpccnt[NFSPROC_CREATE]++;
	nfsm_reqhead(ap->a_dvp, NFSPROC_CREATE,
	  NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(ap->a_cnp->cn_namelen)+NFSX_SATTR);
	nfsm_fhtom(ap->a_dvp);
	nfsm_strtom(ap->a_cnp->cn_nameptr, ap->a_cnp->cn_namelen, NFS_MAXNAMLEN);
	nfsm_build(sp, struct nfsv2_sattr *, NFSX_SATTR);
	sp->sa_mode = vtonfs_mode(ap->a_vap->va_type, ap->a_vap->va_mode);
	sp->sa_uid = txdr_unsigned(ap->a_cnp->cn_cred->cr_uid);
	sp->sa_gid = txdr_unsigned(ap->a_cnp->cn_cred->cr_gid);
	sp->sa_size = txdr_unsigned(0);
	/* or should these be VNOVAL ?? */
	txdr_time(&ap->a_vap->va_atime, &sp->sa_atime);
	txdr_time(&ap->a_vap->va_mtime, &sp->sa_mtime);
	nfsm_request(ap->a_dvp, NFSPROC_CREATE, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
	nfsm_mtofh(ap->a_dvp, *ap->a_vpp);
	nfsm_reqdone;
	FREE(ap->a_cnp->cn_pnbuf, M_NAMEI);
	VTONFS(ap->a_dvp)->n_flag |= NMODIFIED;
	vrele(ap->a_dvp);
	return (error);
}

/*
 * nfs file remove call
 * To try and make nfs semantics closer to ufs semantics, a file that has
 * other processes using the vnode is renamed instead of removed and then
 * removed later on the last close.
 * - If v_usecount > 1
 *	  If a rename is not already in the works
 *	     call nfs_sillyrename() to set it up
 *     else
 *	  do the remove rpc
 */
int
nfs_remove (ap)
	struct vop_remove_args *ap;
{
	register struct nfsnode *np = VTONFS(ap->a_vp);
	register u_long *tl;
	register caddr_t cp;
	register long t2;
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	if (ap->a_vp->v_usecount > 1) {
		if (!np->n_sillyrename)
			error = nfs_sillyrename(ap->a_dvp, ap->a_vp, ap->a_cnp);
	} else {
		/*
		 * Purge the name cache so that the chance of a lookup for
		 * the name succeeding while the remove is in progress is
		 * minimized. Without node locking it can still happen, such
		 * that an I/O op returns ESTALE, but since you get this if
		 * another host removes the file..
		 */
		cache_purge(ap->a_vp);
		/*
		 * Throw away biocache buffers. Mainly to avoid
		 * unnecessary delayed writes.
		 */
		vinvalbuf(ap->a_vp, FALSE);
		/* Do the rpc */
		nfsstats.rpccnt[NFSPROC_REMOVE]++;
		nfsm_reqhead(ap->a_dvp, NFSPROC_REMOVE,
			NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(ap->a_cnp->cn_namelen));
		nfsm_fhtom(ap->a_dvp);
		nfsm_strtom(ap->a_cnp->cn_nameptr, ap->a_cnp->cn_namelen, NFS_MAXNAMLEN);
		nfsm_request(ap->a_dvp, NFSPROC_REMOVE, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
		nfsm_reqdone;
		FREE(ap->a_cnp->cn_pnbuf, M_NAMEI);
		VTONFS(ap->a_dvp)->n_flag |= NMODIFIED;
		/*
		 * Kludge City: If the first reply to the remove rpc is lost..
		 *   the reply to the retransmitted request will be ENOENT
		 *   since the file was in fact removed
		 *   Therefore, we cheat and return success.
		 */
		if (error == ENOENT)
			error = 0;
	}
	np->n_attrstamp = 0;
	vrele(ap->a_dvp);
	vrele(ap->a_vp);
	return (error);
}

/*
 * nfs file remove rpc called from nfs_inactive
 */
int
nfs_removeit(sp, procp)
	register struct sillyrename *sp;
	struct proc *procp;
{
	register u_long *tl;
	register caddr_t cp;
	register long t2;
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	nfsstats.rpccnt[NFSPROC_REMOVE]++;
	nfsm_reqhead(sp->s_dvp, NFSPROC_REMOVE,
		NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(sp->s_namlen));
	nfsm_fhtom(sp->s_dvp);
	nfsm_strtom(sp->s_name, sp->s_namlen, NFS_MAXNAMLEN);
	nfsm_request(sp->s_dvp, NFSPROC_REMOVE, procp, sp->s_cred);
	nfsm_reqdone;
	VTONFS(sp->s_dvp)->n_flag |= NMODIFIED;
	return (error);
}

/*
 * nfs file rename call
 */
int
nfs_rename (ap)
	struct vop_rename_args *ap;
{
	register u_long *tl;
	register caddr_t cp;
	register long t2;
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	nfsstats.rpccnt[NFSPROC_RENAME]++;
	nfsm_reqhead(ap->a_fdvp, NFSPROC_RENAME,
		(NFSX_FH+NFSX_UNSIGNED)*2+nfsm_rndup(ap->a_fcnp->cn_namelen)+
		nfsm_rndup(ap->a_fcnp->cn_namelen)); /* or ap->a_fcnp->cn_cred?*/
	nfsm_fhtom(ap->a_fdvp);
	nfsm_strtom(ap->a_fcnp->cn_nameptr, ap->a_fcnp->cn_namelen, NFS_MAXNAMLEN);
	nfsm_fhtom(ap->a_tdvp);
	nfsm_strtom(ap->a_tcnp->cn_nameptr, ap->a_tcnp->cn_namelen, NFS_MAXNAMLEN);
	nfsm_request(ap->a_fdvp, NFSPROC_RENAME, ap->a_tcnp->cn_proc, ap->a_tcnp->cn_cred);
	nfsm_reqdone;
	VTONFS(ap->a_fdvp)->n_flag |= NMODIFIED;
	VTONFS(ap->a_tdvp)->n_flag |= NMODIFIED;
	if (ap->a_fvp->v_type == VDIR) {
		if (ap->a_tvp != NULL && ap->a_tvp->v_type == VDIR)
			cache_purge(ap->a_tdvp);
		cache_purge(ap->a_fdvp);
	}
	if (ap->a_tdvp == ap->a_tvp)
		vrele(ap->a_tdvp);
	else
		vput(ap->a_tdvp);
	if (ap->a_tvp)
		vput(ap->a_tvp);
	vrele(ap->a_fdvp);
	vrele(ap->a_fvp);
	/*
	 * Kludge: Map ENOENT => 0 assuming that it is a reply to a retry.
	 */
	if (error == ENOENT)
		error = 0;
	return (error);
}

/*
 * nfs file rename rpc called from nfs_remove() above
 */
int
nfs_renameit(sdvp, scnp, sp)
	struct vnode *sdvp;
	struct componentname *scnp;
	register struct sillyrename *sp;
{
	register u_long *tl;
	register caddr_t cp;
	register long t2;
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	nfsstats.rpccnt[NFSPROC_RENAME]++;
	nfsm_reqhead(sdvp, NFSPROC_RENAME,
		(NFSX_FH+NFSX_UNSIGNED)*2+nfsm_rndup(scnp->cn_namelen)+
		nfsm_rndup(sp->s_namlen));
	nfsm_fhtom(sdvp);
	nfsm_strtom(scnp->cn_nameptr, scnp->cn_namelen, NFS_MAXNAMLEN);
	nfsm_fhtom(sdvp);
	nfsm_strtom(sp->s_name, sp->s_namlen, NFS_MAXNAMLEN);
	nfsm_request(sdvp, NFSPROC_RENAME, scnp->cn_proc, scnp->cn_cred);
	nfsm_reqdone;
	FREE(scnp->cn_pnbuf, M_NAMEI);
	VTONFS(sdvp)->n_flag |= NMODIFIED;
	return (error);
}

/*
 * nfs hard link create call
 */
int
nfs_link (ap)
	struct vop_link_args *ap;
{
	register u_long *tl;
	register caddr_t cp;
	register long t2;
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	nfsstats.rpccnt[NFSPROC_LINK]++;
	nfsm_reqhead(ap->a_tdvp, NFSPROC_LINK,
		NFSX_FH*2+NFSX_UNSIGNED+nfsm_rndup(ap->a_cnp->cn_namelen));
	nfsm_fhtom(ap->a_tdvp);
	nfsm_fhtom(ap->a_vp);
	nfsm_strtom(ap->a_cnp->cn_nameptr, ap->a_cnp->cn_namelen, NFS_MAXNAMLEN);
	nfsm_request(ap->a_tdvp, NFSPROC_LINK, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
	nfsm_reqdone;
	FREE(ap->a_cnp->cn_pnbuf, M_NAMEI);
	VTONFS(ap->a_tdvp)->n_attrstamp = 0;
	VTONFS(ap->a_vp)->n_flag |= NMODIFIED;
	vrele(ap->a_vp);
	/*
	 * Kludge: Map EEXIST => 0 assuming that it is a reply to a retry.
	 */
	if (error == EEXIST)
		error = 0;
	return (error);
}

/*
 * nfs symbolic link create call
 */
/* start here */
int
nfs_symlink (ap)
	struct vop_symlink_args *ap;
{
	register struct nfsv2_sattr *sp;
	register u_long *tl;
	register caddr_t cp;
	register long t2;
	caddr_t bpos, dpos;
	int slen, error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	nfsstats.rpccnt[NFSPROC_SYMLINK]++;
	slen = strlen(ap->a_target);
	nfsm_reqhead(ap->a_dvp, NFSPROC_SYMLINK,
	 NFSX_FH+2*NFSX_UNSIGNED+nfsm_rndup(ap->a_cnp->cn_namelen)+nfsm_rndup(slen)+NFSX_SATTR);
	nfsm_fhtom(ap->a_dvp);
	nfsm_strtom(ap->a_cnp->cn_nameptr, ap->a_cnp->cn_namelen, NFS_MAXNAMLEN);
	nfsm_strtom(ap->a_target, slen, NFS_MAXPATHLEN);
	nfsm_build(sp, struct nfsv2_sattr *, NFSX_SATTR);
	sp->sa_mode = vtonfs_mode(VLNK, ap->a_vap->va_mode);
	sp->sa_uid = txdr_unsigned(ap->a_cnp->cn_cred->cr_uid);
	sp->sa_gid = txdr_unsigned(ap->a_cnp->cn_cred->cr_gid);
	sp->sa_size = txdr_unsigned(VNOVAL);
	txdr_time(&ap->a_vap->va_atime, &sp->sa_atime);	/* or VNOVAL ?? */
	txdr_time(&ap->a_vap->va_mtime, &sp->sa_mtime);	/* or VNOVAL ?? */
	nfsm_request(ap->a_dvp, NFSPROC_SYMLINK, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
	nfsm_reqdone;
	FREE(ap->a_cnp->cn_pnbuf, M_NAMEI);
	VTONFS(ap->a_dvp)->n_flag |= NMODIFIED;
	vrele(ap->a_dvp);
	/*
	 * Kludge: Map EEXIST => 0 assuming that it is a reply to a retry.
	 */
	if (error == EEXIST)
		error = 0;
	return (error);
}

/*
 * nfs make dir call
 */
int
nfs_mkdir (ap)
	struct vop_mkdir_args *ap;
{
	register struct nfsv2_sattr *sp;
	register u_long *tl;
	register caddr_t cp;
	register long t1, t2;
	register int len;
	caddr_t bpos, dpos, cp2;
	int error = 0, firsttry = 1;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	len = ap->a_cnp->cn_namelen;
	nfsstats.rpccnt[NFSPROC_MKDIR]++;
	nfsm_reqhead(ap->a_dvp, NFSPROC_MKDIR,
	  NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(len)+NFSX_SATTR);
	nfsm_fhtom(ap->a_dvp);
	nfsm_strtom(ap->a_cnp->cn_nameptr, len, NFS_MAXNAMLEN);
	nfsm_build(sp, struct nfsv2_sattr *, NFSX_SATTR);
	sp->sa_mode = vtonfs_mode(VDIR, ap->a_vap->va_mode);
	sp->sa_uid = txdr_unsigned(ap->a_cnp->cn_cred->cr_uid);
	sp->sa_gid = txdr_unsigned(ap->a_cnp->cn_cred->cr_gid);
	sp->sa_size = txdr_unsigned(VNOVAL);
	txdr_time(&ap->a_vap->va_atime, &sp->sa_atime);	/* or VNOVAL ?? */
	txdr_time(&ap->a_vap->va_mtime, &sp->sa_mtime);	/* or VNOVAL ?? */
	nfsm_request(ap->a_dvp, NFSPROC_MKDIR, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
	nfsm_mtofh(ap->a_dvp, *ap->a_vpp);
	nfsm_reqdone;
	VTONFS(ap->a_dvp)->n_flag |= NMODIFIED;
	/*
	 * Kludge: Map EEXIST => 0 assuming that you have a reply to a retry
	 * if we can succeed in looking up the directory.
	 * "firsttry" is necessary since the macros may "goto nfsmout" which
	 * is above the if on errors. (Ugh)
	 */
	if (error == EEXIST && firsttry) {
		firsttry = 0;
		error = 0;
		nfsstats.rpccnt[NFSPROC_LOOKUP]++;
		*ap->a_vpp = NULL;
		nfsm_reqhead(ap->a_dvp, NFSPROC_LOOKUP,
		    NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(len));
		nfsm_fhtom(ap->a_dvp);
		nfsm_strtom(ap->a_cnp->cn_nameptr, len, NFS_MAXNAMLEN);
		nfsm_request(ap->a_dvp, NFSPROC_LOOKUP, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
		nfsm_mtofh(ap->a_dvp, *ap->a_vpp);
		if ((*ap->a_vpp)->v_type != VDIR) {
			vput(*ap->a_vpp);
			error = EEXIST;
		}
		m_freem(mrep);
	}
	FREE(ap->a_cnp->cn_pnbuf, M_NAMEI);
	vrele(ap->a_dvp);
	return (error);
}

/*
 * nfs remove directory call
 */
int
nfs_rmdir (ap)
	struct vop_rmdir_args *ap;
{
	register u_long *tl;
	register caddr_t cp;
	register long t2;
	caddr_t bpos, dpos;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;

	if (ap->a_dvp == ap->a_vp) {
		vrele(ap->a_dvp);
		vrele(ap->a_dvp);
		FREE(ap->a_cnp->cn_pnbuf, M_NAMEI);
		return (EINVAL);
	}
	nfsstats.rpccnt[NFSPROC_RMDIR]++;
	nfsm_reqhead(ap->a_dvp, NFSPROC_RMDIR,
		NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(ap->a_cnp->cn_namelen));
	nfsm_fhtom(ap->a_dvp);
	nfsm_strtom(ap->a_cnp->cn_nameptr, ap->a_cnp->cn_namelen, NFS_MAXNAMLEN);
	nfsm_request(ap->a_dvp, NFSPROC_RMDIR, ap->a_cnp->cn_proc, ap->a_cnp->cn_cred);
	nfsm_reqdone;
	FREE(ap->a_cnp->cn_pnbuf, M_NAMEI);
	VTONFS(ap->a_dvp)->n_flag |= NMODIFIED;
	cache_purge(ap->a_dvp);
	cache_purge(ap->a_vp);
	vrele(ap->a_vp);
	vrele(ap->a_dvp);
	/*
	 * Kludge: Map ENOENT => 0 assuming that you have a reply to a retry.
	 */
	if (error == ENOENT)
		error = 0;
	return (error);
}

/*
 * nfs readdir call
 * Although cookie is defined as opaque, I translate it to/from net byte
 * order so that it looks more sensible. This appears consistent with the
 * Ultrix implementation of NFS.
 */
int
nfs_readdir (ap)
	struct vop_readdir_args *ap;
{
	USES_VOP_GETATTR;
	register struct nfsnode *np = VTONFS(ap->a_vp);
	int tresid, error;
	struct vattr vattr;

	if (ap->a_vp->v_type != VDIR)
		return (EPERM);
	/*
	 * First, check for hit on the EOF offset cache
	 */
	if (ap->a_uio->uio_offset != 0 && ap->a_uio->uio_offset == np->n_direofoffset &&
	    (np->n_flag & NMODIFIED) == 0) {
		if (VFSTONFS(ap->a_vp->v_mount)->nm_flag & NFSMNT_NQNFS) {
			if (NQNFS_CKCACHABLE(ap->a_vp, NQL_READ)) {
				*ap->a_eofflagp = 1;
				nfsstats.direofcache_hits++;
				return (0);
			}
		} else if (VOP_GETATTR(ap->a_vp, &vattr, ap->a_cred, ap->a_uio->uio_procp) == 0 &&
			np->n_mtime == vattr.va_mtime.tv_sec) {
			*ap->a_eofflagp = 1;
			nfsstats.direofcache_hits++;
			return (0);
		}
	}

	/*
	 * Call nfs_bioread() to do the real work.
	 */
	tresid = ap->a_uio->uio_resid;
	error = nfs_bioread(ap->a_vp, ap->a_uio, 0, ap->a_cred);

	if (!error && ap->a_uio->uio_resid == tresid) {
		*ap->a_eofflagp = 1;
		nfsstats.direofcache_misses++;
	} else
		*ap->a_eofflagp = 0;
	return (error);
}

/*
 * Readdir rpc call.
 * Called from below the buffer cache by nfs_doio().
 */
int
nfs_readdirrpc(vp, uiop, cred)
	register struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	register long len;
	register struct readdir *dp;
	register u_long *tl;
	register caddr_t cp;
	register long t1;
	long tlen, lastlen;
	caddr_t bpos, dpos, cp2;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	struct mbuf *md2;
	caddr_t dpos2;
	int siz;
	int more_dirs = 1;
	off_t off, savoff;
	struct readdir *savdp;
	struct nfsmount *nmp;
	struct nfsnode *np = VTONFS(vp);
	long tresid;

	nmp = VFSTONFS(vp->v_mount);
	tresid = uiop->uio_resid;
	/*
	 * Loop around doing readdir rpc's of size uio_resid or nm_rsize,
	 * whichever is smaller, truncated to a multiple of NFS_DIRBLKSIZ.
	 * The stopping criteria is EOF or buffer full.
	 */
	while (more_dirs && uiop->uio_resid >= NFS_DIRBLKSIZ) {
		nfsstats.rpccnt[NFSPROC_READDIR]++;
		nfsm_reqhead(vp, NFSPROC_READDIR,
			NFSX_FH+2*NFSX_UNSIGNED);
		nfsm_fhtom(vp);
		nfsm_build(tl, u_long *, 2*NFSX_UNSIGNED);
		*tl++ = txdr_unsigned(uiop->uio_offset);
		*tl = txdr_unsigned(((uiop->uio_resid > nmp->nm_rsize) ?
			nmp->nm_rsize : uiop->uio_resid) & ~(NFS_DIRBLKSIZ-1));
		nfsm_request(vp, NFSPROC_READDIR, uiop->uio_procp, cred);
		siz = 0;
		nfsm_dissect(tl, u_long *, NFSX_UNSIGNED);
		more_dirs = fxdr_unsigned(int, *tl);
	
		/* Save the position so that we can do nfsm_mtouio() later */
		dpos2 = dpos;
		md2 = md;
	
		/* loop thru the dir entries, doctoring them to 4bsd form */
		off = uiop->uio_offset;
#ifdef lint
		dp = (struct readdir *)0;
#endif /* lint */
		while (more_dirs && siz < uiop->uio_resid) {
			savoff = off;		/* Hold onto offset and dp */
			savdp = dp;
			nfsm_dissecton(tl, u_long *, 2*NFSX_UNSIGNED);
			dp = (struct readdir *)tl;
			dp->d_ino = fxdr_unsigned(u_long, *tl++);
			len = fxdr_unsigned(int, *tl);
			if (len <= 0 || len > NFS_MAXNAMLEN) {
				error = EBADRPC;
				m_freem(mrep);
				goto nfsmout;
			}
			dp->d_namlen = (u_short)len;
			nfsm_adv(len);		/* Point past name */
			tlen = nfsm_rndup(len);
			/*
			 * This should not be necessary, but some servers have
			 * broken XDR such that these bytes are not null filled.
			 */
			if (tlen != len) {
				*dpos = '\0';	/* Null-terminate */
				nfsm_adv(tlen - len);
				len = tlen;
			}
			nfsm_dissecton(tl, u_long *, 2*NFSX_UNSIGNED);
			off = fxdr_unsigned(off_t, *tl);
			*tl++ = 0;	/* Ensures null termination of name */
			more_dirs = fxdr_unsigned(int, *tl);
			dp->d_reclen = len+4*NFSX_UNSIGNED;
			siz += dp->d_reclen;
		}
		/*
		 * If at end of rpc data, get the eof boolean
		 */
		if (!more_dirs) {
			nfsm_dissecton(tl, u_long *, NFSX_UNSIGNED);
			more_dirs = (fxdr_unsigned(int, *tl) == 0);

			/*
			 * If at EOF, cache directory offset
			 */
			if (!more_dirs)
				np->n_direofoffset = off;
		}
		/*
		 * If there is too much to fit in the data buffer, use savoff and
		 * savdp to trim off the last record.
		 * --> we are not at eof
		 */
		if (siz > uiop->uio_resid) {
			off = savoff;
			siz -= dp->d_reclen;
			dp = savdp;
			more_dirs = 0;	/* Paranoia */
		}
		if (siz > 0) {
			lastlen = dp->d_reclen;
			md = md2;
			dpos = dpos2;
			nfsm_mtouio(uiop, siz);
			uiop->uio_offset = off;
		} else
			more_dirs = 0;	/* Ugh, never happens, but in case.. */
		m_freem(mrep);
	}
	/*
	 * Fill last record, iff any, out to a multiple of NFS_DIRBLKSIZ
	 * by increasing d_reclen for the last record.
	 */
	if (uiop->uio_resid < tresid) {
		len = uiop->uio_resid & (NFS_DIRBLKSIZ - 1);
		if (len > 0) {
			dp = (struct readdir *)
				(uiop->uio_iov->iov_base - lastlen);
			dp->d_reclen += len;
			uiop->uio_iov->iov_base += len;
			uiop->uio_iov->iov_len -= len;
			uiop->uio_resid -= len;
		}
	}
nfsmout:
	return (error);
}

/*
 * Nqnfs readdir_and_lookup RPC. Used in place of nfs_readdirrpc() when
 * the "rdirlook" mount option is specified.
 */
int
nfs_readdirlookrpc(vp, uiop, cred)
	struct vnode *vp;
	register struct uio *uiop;
	struct ucred *cred;
{
	register int len;
	register struct readdir *dp;
	register u_long *tl;
	register caddr_t cp;
	register long t1;
	caddr_t bpos, dpos, cp2;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	struct nameidata nami, *ndp = &nami;
	struct componentname *cnp = &ndp->ni_cnd;
	off_t off, endoff;
	time_t reqtime, ltime;
	struct nfsmount *nmp;
	struct nfsnode *np, *tp;
	struct vnode *newvp;
	nfsv2fh_t *fhp;
	u_long fileno;
	u_quad_t frev;
	int error = 0, tlen, more_dirs = 1, tresid, doit, bigenough, i;
	int cachable;

	if (uiop->uio_iovcnt != 1)
		panic("nfs rdirlook");
	nmp = VFSTONFS(vp->v_mount);
	tresid = uiop->uio_resid;
	ndp->ni_dvp = vp;
	newvp = NULLVP;
	/*
	 * Loop around doing readdir rpc's of size uio_resid or nm_rsize,
	 * whichever is smaller, truncated to a multiple of NFS_DIRBLKSIZ.
	 * The stopping criteria is EOF or buffer full.
	 */
	while (more_dirs && uiop->uio_resid >= NFS_DIRBLKSIZ) {
		nfsstats.rpccnt[NQNFSPROC_READDIRLOOK]++;
		nfsm_reqhead(vp, NQNFSPROC_READDIRLOOK,
			NFSX_FH+3*NFSX_UNSIGNED);
		nfsm_fhtom(vp);
		nfsm_build(tl, u_long *, 3*NFSX_UNSIGNED);
		*tl++ = txdr_unsigned(uiop->uio_offset);
		*tl++ = txdr_unsigned(((uiop->uio_resid > nmp->nm_rsize) ?
			nmp->nm_rsize : uiop->uio_resid) & ~(NFS_DIRBLKSIZ-1));
		*tl = txdr_unsigned(nmp->nm_leaseterm);
		reqtime = time.tv_sec;
		nfsm_request(vp, NQNFSPROC_READDIRLOOK, uiop->uio_procp, cred);
		nfsm_dissect(tl, u_long *, NFSX_UNSIGNED);
		more_dirs = fxdr_unsigned(int, *tl);
	
		/* loop thru the dir entries, doctoring them to 4bsd form */
		off = uiop->uio_offset;
		bigenough = 1;
		while (more_dirs && bigenough) {
			doit = 1;
			nfsm_dissect(tl, u_long *, 4*NFSX_UNSIGNED);
			cachable = fxdr_unsigned(int, *tl++);
			ltime = reqtime + fxdr_unsigned(int, *tl++);
			fxdr_hyper(tl, &frev);
			nfsm_dissect(fhp, nfsv2fh_t *, NFSX_FH);
			if (!bcmp(VTONFS(vp)->n_fh.fh_bytes, (caddr_t)fhp, NFSX_FH)) {
				VREF(vp);
				newvp = vp;
				np = VTONFS(vp);
			} else {
				if (error = nfs_nget(vp->v_mount, fhp, &np))
					doit = 0;
				newvp = NFSTOV(np);
			}
			if (error = nfs_loadattrcache(&newvp, &md, &dpos,
				(struct vattr *)0))
				doit = 0;
			nfsm_dissect(tl, u_long *, 2*NFSX_UNSIGNED);
			fileno = fxdr_unsigned(u_long, *tl++);
			len = fxdr_unsigned(int, *tl);
			if (len <= 0 || len > NFS_MAXNAMLEN) {
				error = EBADRPC;
				m_freem(mrep);
				goto nfsmout;
			}
			tlen = (len + 4) & ~0x3;
			if ((tlen + DIRHDSIZ) > uiop->uio_resid)
				bigenough = 0;
			if (bigenough && doit) {
				dp = (struct readdir *)uiop->uio_iov->iov_base;
				dp->d_ino = fileno;
				dp->d_namlen = len;
				dp->d_reclen = tlen + DIRHDSIZ;
				uiop->uio_resid -= DIRHDSIZ;
				uiop->uio_iov->iov_base += DIRHDSIZ;
				uiop->uio_iov->iov_len -= DIRHDSIZ;
				cnp->cn_nameptr = uiop->uio_iov->iov_base;
				cnp->cn_namelen = len;
				ndp->ni_vp = newvp;
				nfsm_mtouio(uiop, len);
				cp = uiop->uio_iov->iov_base;
				tlen -= len;
				for (i = 0; i < tlen; i++)
					*cp++ = '\0';
				uiop->uio_iov->iov_base += tlen;
				uiop->uio_iov->iov_len -= tlen;
				uiop->uio_resid -= tlen;
				cnp->cn_hash = 0;
				for (cp = cnp->cn_nameptr, i = 1; i <= len; i++, cp++)
					cnp->cn_hash += (unsigned char)*cp * i;
				if (ltime > time.tv_sec) {
					if (np->n_tnext) {
						if (np->n_tnext == (struct nfsnode *)nmp)
							nmp->nm_tprev = np->n_tprev;
						else
							np->n_tnext->n_tprev = np->n_tprev;
						if (np->n_tprev == (struct nfsnode *)nmp)
							nmp->nm_tnext = np->n_tnext;
						else
							np->n_tprev->n_tnext = np->n_tnext;
					} else
						np->n_flag &= ~NQNFSWRITE;
					if (cachable)
						np->n_flag &= ~NQNFSNONCACHE;
					else
						np->n_flag |= NQNFSNONCACHE;
					np->n_expiry = ltime;
					np->n_lrev = frev;
					tp = nmp->nm_tprev;
					while (tp != (struct nfsnode *)nmp && tp->n_expiry > np->n_expiry)
						tp = tp->n_tprev;
					if (tp == (struct nfsnode *)nmp) {
						np->n_tnext = nmp->nm_tnext;
						nmp->nm_tnext = np;
					} else {
						np->n_tnext = tp->n_tnext;
						tp->n_tnext = np;
					}
					np->n_tprev = tp;
					if (np->n_tnext == (struct nfsnode *)nmp)
						nmp->nm_tprev = np;
					else
						np->n_tnext->n_tprev = np;
					cache_enter(ndp->ni_dvp, ndp->ni_vp, cnp);
				}
			} else {
				nfsm_adv(nfsm_rndup(len));
			}
			if (newvp != NULLVP) {
				vrele(newvp);
				newvp = NULLVP;
			}
			nfsm_dissect(tl, u_long *, 2*NFSX_UNSIGNED);
			if (bigenough)
				endoff = off = fxdr_unsigned(off_t, *tl++);
			else
				endoff = fxdr_unsigned(off_t, *tl++);
			more_dirs = fxdr_unsigned(int, *tl);
		}
		/*
		 * If at end of rpc data, get the eof boolean
		 */
		if (!more_dirs) {
			nfsm_dissect(tl, u_long *, NFSX_UNSIGNED);
			more_dirs = (fxdr_unsigned(int, *tl) == 0);

			/*
			 * If at EOF, cache directory offset
			 */
			if (!more_dirs)
				VTONFS(vp)->n_direofoffset = endoff;
		}
		if (uiop->uio_resid < tresid)
			uiop->uio_offset = off;
		else
			more_dirs = 0;
		m_freem(mrep);
	}
	/*
	 * Fill last record, iff any, out to a multiple of NFS_DIRBLKSIZ
	 * by increasing d_reclen for the last record.
	 */
	if (uiop->uio_resid < tresid) {
		len = uiop->uio_resid & (NFS_DIRBLKSIZ - 1);
		if (len > 0) {
			dp->d_reclen += len;
			uiop->uio_iov->iov_base += len;
			uiop->uio_iov->iov_len -= len;
			uiop->uio_resid -= len;
		}
	}
nfsmout:
	if (newvp != NULLVP)
		vrele(newvp);
	return (error);
}
static char hextoasc[] = "0123456789abcdef";

/*
 * Silly rename. To make the NFS filesystem that is stateless look a little
 * more like the "ufs" a remove of an active vnode is translated to a rename
 * to a funny looking filename that is removed by nfs_inactive on the
 * nfsnode. There is the potential for another process on a different client
 * to create the same funny name between the nfs_lookitup() fails and the
 * nfs_rename() completes, but...
 */
int
nfs_sillyrename(dvp, vp, cnp)
	struct vnode *dvp, *vp;
	struct componentname *cnp;
{
	register struct nfsnode *np;
	register struct sillyrename *sp;
	int error;
	short pid;

	cache_purge(dvp);
	np = VTONFS(vp);
#ifdef SILLYSEPARATE
	MALLOC(sp, struct sillyrename *, sizeof (struct sillyrename),
		M_NFSREQ, M_WAITOK);
#else
	sp = &np->n_silly;
#endif
	sp->s_cred = crdup(cnp->cn_cred);
	sp->s_dvp = dvp;
	VREF(dvp);

	/* Fudge together a funny name */
	pid = cnp->cn_proc->p_pid;
	bcopy(".nfsAxxxx4.4", sp->s_name, 13);
	sp->s_namlen = 12;
	sp->s_name[8] = hextoasc[pid & 0xf];
	sp->s_name[7] = hextoasc[(pid >> 4) & 0xf];
	sp->s_name[6] = hextoasc[(pid >> 8) & 0xf];
	sp->s_name[5] = hextoasc[(pid >> 12) & 0xf];

	/* Try lookitups until we get one that isn't there */
	while (nfs_lookitup(sp, (nfsv2fh_t *)0, cnp->cn_proc) == 0) {
		sp->s_name[4]++;
		if (sp->s_name[4] > 'z') {
			error = EINVAL;
			goto bad;
		}
	}
	if (error = nfs_renameit(dvp, cnp, sp))
		goto bad;
	nfs_lookitup(sp, &np->n_fh, cnp->cn_proc);
	np->n_sillyrename = sp;
	return (0);
bad:
	vrele(sp->s_dvp);
	crfree(sp->s_cred);
#ifdef SILLYSEPARATE
	free((caddr_t)sp, M_NFSREQ);
#endif
	return (error);
}

/*
 * Look up a file name for silly rename stuff.
 * Just like nfs_lookup() except that it doesn't load returned values
 * into the nfsnode table.
 * If fhp != NULL it copies the returned file handle out
 */
int
nfs_lookitup(sp, fhp, procp)
	register struct sillyrename *sp;
	nfsv2fh_t *fhp;
	struct proc *procp;
{
	register struct vnode *vp = sp->s_dvp;
	register u_long *tl;
	register caddr_t cp;
	register long t1, t2;
	caddr_t bpos, dpos, cp2;
	u_long xid;
	int error = 0;
	struct mbuf *mreq, *mrep, *md, *mb, *mb2;
	long len;

	nfsstats.rpccnt[NFSPROC_LOOKUP]++;
	len = sp->s_namlen;
	nfsm_reqhead(vp, NFSPROC_LOOKUP, NFSX_FH+NFSX_UNSIGNED+nfsm_rndup(len));
	nfsm_fhtom(vp);
	nfsm_strtom(sp->s_name, len, NFS_MAXNAMLEN);
	nfsm_request(vp, NFSPROC_LOOKUP, procp, sp->s_cred);
	if (fhp != NULL) {
		nfsm_dissect(cp, caddr_t, NFSX_FH);
		bcopy(cp, (caddr_t)fhp, NFSX_FH);
	}
	nfsm_reqdone;
	return (error);
}

/*
 * Kludge City..
 * - make nfs_bmap() essentially a no-op that does no translation
 * - do nfs_strategy() by faking physical I/O with nfs_readrpc/nfs_writerpc
 *   after mapping the physical addresses into Kernel Virtual space in the
 *   nfsiobuf area.
 *   (Maybe I could use the process's page mapping, but I was concerned that
 *    Kernel Write might not be enabled and also figured copyout() would do
 *    a lot more work than bcopy() and also it currently happens in the
 *    context of the swapper process (2).
 */
int
nfs_bmap (ap)
	struct vop_bmap_args *ap;
{
	if (ap->a_vpp != NULL)
		*ap->a_vpp = ap->a_vp;
	if (ap->a_bnp != NULL)
		*ap->a_bnp = ap->a_bn * btodb(ap->a_vp->v_mount->mnt_stat.f_iosize);
	return (0);
}

/*
 * Strategy routine for phys. i/o
 * If the biod's are running, queue a request
 * otherwise just call nfs_doio() to get it done
 */
int
nfs_strategy (ap)
	struct vop_strategy_args *ap;
{
	register struct buf *dp;
	register int i;
	int error = 0;
	int fnd = 0;

	/*
	 * Set b_proc. It seems a bit silly to do it here, but since bread()
	 * doesn't set it, I will.
	 * Set b_proc == NULL for asynchronous ops, since these may still
	 * be hanging about after the process terminates.
	 */
	if ((ap->a_bp->b_flags & B_PHYS) == 0) {
		if (ap->a_bp->b_flags & B_ASYNC)
			ap->a_bp->b_proc = (struct proc *)0;
		else
			ap->a_bp->b_proc = curproc;
	}
	/*
	 * If the op is asynchronous and an i/o daemon is waiting
	 * queue the request, wake it up and wait for completion
	 * otherwise just do it ourselves.
	 */
	if ((ap->a_bp->b_flags & B_ASYNC) == 0 || nfs_numasync == 0)
		return (nfs_doio(ap->a_bp));
	for (i = 0; i < NFS_MAXASYNCDAEMON; i++) {
		if (nfs_iodwant[i]) {
			dp = &nfs_bqueue;
			if (dp->b_actf == NULL) {
				dp->b_actl = ap->a_bp;
				ap->a_bp->b_actf = dp;
			} else {
				dp->b_actf->b_actl = ap->a_bp;
				ap->a_bp->b_actf = dp->b_actf;
			}
			dp->b_actf = ap->a_bp;
			ap->a_bp->b_actl = dp;
			fnd++;
			wakeup((caddr_t)&nfs_iodwant[i]);
			break;
		}
	}
	if (!fnd)
		error = nfs_doio(ap->a_bp);
	return (error);
}

/*
 * Fun and games with i/o
 * Essentially play ubasetup() and disk interrupt service routine by
 * mapping the data buffer into kernel virtual space and doing the
 * nfs read or write rpc's from it.
 * If the nfsiod's are not running, this is just called from nfs_strategy(),
 * otherwise it is called by the nfsiods to do what would normally be
 * partially disk interrupt driven.
 */
int
nfs_doio(bp)
	register struct buf *bp;
{
	register struct uio *uiop;
	register struct vnode *vp;
	struct nfsnode *np;
	struct ucred *cr;
	int error;
	struct uio uio;
	struct iovec io;

	vp = bp->b_vp;
	np = VTONFS(vp);
	uiop = &uio;
	uiop->uio_iov = &io;
	uiop->uio_iovcnt = 1;
	uiop->uio_segflg = UIO_SYSSPACE;
	uiop->uio_procp = bp->b_proc;

	/*
	 * For phys i/o, map the b_addr into kernel virtual space using
	 * the Nfsiomap pte's
	 * Also, add a temporary b_rcred for reading using the process's uid
	 * and a guess at a group
	 */
	if (bp->b_flags & B_PHYS) {
		if (bp->b_flags & B_DIRTY)
			uiop->uio_procp = pageproc;
		cr = crcopy(uiop->uio_procp->p_ucred);
		/* mapping was already done by vmapbuf */
		io.iov_base = bp->b_un.b_addr;

		/*
		 * And do the i/o rpc
		 */
		io.iov_len = uiop->uio_resid = bp->b_bcount;
		uiop->uio_offset = bp->b_blkno * DEV_BSIZE;
		if (bp->b_flags & B_READ) {
			uiop->uio_rw = UIO_READ;
			nfsstats.read_physios++;
			bp->b_error = error = nfs_readrpc(vp, uiop, cr);
			(void) vnode_pager_uncache(vp);
		} else {
			uiop->uio_rw = UIO_WRITE;
			nfsstats.write_physios++;
			bp->b_error = error = nfs_writerpc(vp, uiop, cr);
		}

		/*
		 * Finally, release pte's used by physical i/o
		 */
		crfree(cr);
	} else {
		if (bp->b_flags & B_READ) {
			io.iov_len = uiop->uio_resid = bp->b_bcount;
			io.iov_base = bp->b_un.b_addr;
			uiop->uio_rw = UIO_READ;
			switch (vp->v_type) {
			case VREG:
				uiop->uio_offset = bp->b_blkno * DEV_BSIZE;
				nfsstats.read_bios++;
				error = nfs_readrpc(vp, uiop, bp->b_rcred);
				break;
			case VLNK:
				uiop->uio_offset = 0;
				nfsstats.readlink_bios++;
				error = nfs_readlinkrpc(vp, uiop, bp->b_rcred);
				break;
			case VDIR:
				uiop->uio_offset = bp->b_lblkno;
				nfsstats.readdir_bios++;
				if (VFSTONFS(vp->v_mount)->nm_flag & NFSMNT_RDIRALOOK)
				    error = nfs_readdirlookrpc(vp, uiop, bp->b_rcred);
				else
				    error = nfs_readdirrpc(vp, uiop, bp->b_rcred);
				/*
				 * Save offset cookie in b_blkno.
				 */
				bp->b_blkno = uiop->uio_offset;
				break;
			};
			bp->b_error = error;
		} else {
			io.iov_len = uiop->uio_resid = bp->b_dirtyend
				- bp->b_dirtyoff;
			uiop->uio_offset = (bp->b_blkno * DEV_BSIZE)
				+ bp->b_dirtyoff;
			io.iov_base = bp->b_un.b_addr + bp->b_dirtyoff;
			uiop->uio_rw = UIO_WRITE;
			nfsstats.write_bios++;
			bp->b_error = error = nfs_writerpc(vp, uiop,
				bp->b_wcred);
			if (error) {
				np->n_error = error;
				np->n_flag |= NWRITEERR;
			}
			bp->b_dirtyoff = bp->b_dirtyend = 0;
		}
	}
	if (error)
		bp->b_flags |= B_ERROR;
	bp->b_resid = uiop->uio_resid;
	biodone(bp);
	return (error);
}

/*
 * Mmap a file
 *
 * NB Currently unsupported.
 */
/* ARGSUSED */
int
nfs_mmap (ap)
	struct vop_mmap_args *ap;
{

	return (EINVAL);
}

/*
 * Flush all the blocks associated with a vnode.
 * 	Walk through the buffer pool and push any dirty pages
 *	associated with the vnode.
 */
/* ARGSUSED */
int
nfs_fsync (ap)
	struct vop_fsync_args *ap;
{
	register struct nfsnode *np = VTONFS(ap->a_vp);
	int error = 0;

	if (np->n_flag & NMODIFIED) {
		np->n_flag &= ~NMODIFIED;
		vflushbuf(ap->a_vp, ap->a_waitfor == MNT_WAIT ? B_SYNC : 0);
	}
	if (!error && (np->n_flag & NWRITEERR))
		error = np->n_error;
	return (error);
}

/*
 * NFS advisory byte-level locks.
 * Currently unsupported.
 */
int
nfs_advlock (ap)
	struct vop_advlock_args *ap;
{

	return (EOPNOTSUPP);
}

/*
 * Print out the contents of an nfsnode.
 */
int
nfs_print (ap)
	struct vop_print_args *ap;
{
	register struct nfsnode *np = VTONFS(ap->a_vp);

	printf("tag VT_NFS, fileid %d fsid 0x%x",
		np->n_vattr.va_fileid, np->n_vattr.va_fsid);
#ifdef FIFO
	if (ap->a_vp->v_type == VFIFO)
		fifo_printinfo(ap->a_vp);
#endif /* FIFO */
	printf("\n");
}

/*
 * NFS directory offset lookup.
 * Currently unsupported.
 */
int
nfs_blkatoff (ap)
	struct vop_blkatoff_args *ap;
{

	return (EOPNOTSUPP);
}

/*
 * NFS flat namespace lookup.
 * Currently unsupported.
 */
int
nfs_vget (ap)
	struct vop_vget_args *ap;
{

	return (EOPNOTSUPP);
}

/*
 * NFS flat namespace allocation.
 * Currently unsupported.
 */
int
nfs_valloc (ap)
	struct vop_valloc_args *ap;
{

	return (EOPNOTSUPP);
}

/*
 * NFS flat namespace free.
 * Currently unsupported.
 */
int
nfs_vfree (ap)
	struct vop_vfree_args *ap;
{

	return (EOPNOTSUPP);
}

/*
 * NFS file truncation.
 */
int
nfs_truncate (ap)
	struct vop_truncate_args *ap;
{

	/* Use nfs_setattr */
	printf("nfs_truncate: need to implement!!");
	return (EOPNOTSUPP);
}

/*
 * NFS update.
 */
int
nfs_update (ap)
	struct vop_update_args *ap;
{

	/* Use nfs_setattr */
	printf("nfs_update: need to implement!!");
	return (EOPNOTSUPP);
}
