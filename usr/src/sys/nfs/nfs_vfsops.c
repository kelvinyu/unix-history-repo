/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
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
 *	@(#)nfs_vfsops.c	7.13 (Berkeley) %G%
 */

#include "param.h"
#include "signal.h"
#include "user.h"
#include "proc.h"
#include "uio.h"
#include "ucred.h"
#include "systm.h"
#include "../ufs/dir.h"
#include "namei.h"
#include "vnode.h"
#include "mount.h"
#include "errno.h"
#include "malloc.h"
#include "mbuf.h"
#undef	m_data
#include "socket.h"
#include "socketvar.h"
#include "../netinet/in.h"
#include "nfsv2.h"
#include "nfsnode.h"
#include "nfsmount.h"
#include "nfs.h"

#ifndef shouldbe
#include "conf.h"
#endif

/*
 * nfs vfs operations.
 */
int nfs_mount();
int nfs_start();
int nfs_unmount();
int nfs_root();
int nfs_statfs();
int nfs_sync();
int nfs_fhtovp();
int nfs_vptofh();
int nfs_init();

struct vfsops nfs_vfsops = {
	nfs_mount,
	nfs_start,
	nfs_unmount,
	nfs_root,
	nfs_statfs,
	nfs_sync,
	nfs_fhtovp,
	nfs_vptofh,
	nfs_init,
};

extern struct nfsreq nfsreqh;
static u_char nfs_mntid;

/*
 * Called by vfs_mountroot when nfs is going to be mounted as root
 * Not Yet (By a LONG shot)
 */
nfs_mountroot()
{
	return (ENODEV);
}

/*
 * VFS Operations.
 *
 * mount system call
 * It seems a bit dumb to copyinstr() the host and path here and then
 * bcopy() them in mountnfs(), but I wanted to detect errors before
 * doing the sockargs() call because sockargs() allocates an mbuf and
 * an error after that means that I have to release the mbuf.
 */
/* ARGSUSED */
nfs_mount(mp, path, data, ndp)
	struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
{
	int error;
	struct nfs_args args;
	struct mbuf *saddr;
	char pth[MNAMELEN], hst[MNAMELEN];
	int len;
	nfsv2fh_t nfh;

	if (mp->m_flag & M_UPDATE)
		return (0);
	if (error = copyin(data, (caddr_t)&args, sizeof (struct nfs_args)))
		return (error);
	if (error=copyin((caddr_t)args.fh, (caddr_t)&nfh, sizeof (nfsv2fh_t)))
		return (error);
	if (error = copyinstr(path, pth, MNAMELEN-1, &len))
		return (error);
	bzero(&pth[len], MNAMELEN-len);
	if (error = copyinstr(args.hostname, hst, MNAMELEN-1, &len))
		return (error);
	bzero(&hst[len], MNAMELEN-len);
	/* sockargs() call must be after above copyin() calls */
	if (error = sockargs(&saddr, (caddr_t)args.addr,
		sizeof (struct sockaddr_in), MT_SONAME))
		return (error);
	args.fh = &nfh;
	error = mountnfs(&args, mp, saddr, pth, hst);
	return (error);
}

/*
 * Common code for mount and mountroot
 */
mountnfs(argp, mp, saddr, pth, hst)
	register struct nfs_args *argp;
	register struct mount *mp;
	register struct mbuf *saddr;
	char *pth, *hst;
{
	register struct nfsmount *nmp;
	struct nfsnode *np;
	fsid_t tfsid;
	int error;

	nmp = (struct nfsmount *)malloc(sizeof (struct nfsmount), M_NFSMNT,
	    M_WAITOK);
	mp->m_data = (qaddr_t)nmp;
	/*
	 * Generate a unique nfs mount id. The problem is that a dev number
	 * is not unique across multiple systems. The techique is as follows:
	 * 1) Set to nblkdev,0 which will never be used otherwise
	 * 2) Generate a first guess as nblkdev,nfs_mntid where nfs_mntid is
	 *	NOT 0
	 * 3) Loop searching the mount list for another one with same id
	 *	If a match, increment val[0] and try again
	 * NB: I increment val[0] { a long } instead of nfs_mntid { a u_char }
	 *	so that nfs is not limited to 255 mount points
	 *     Incrementing the high order bits does no real harm, since it
	 *     simply makes the major dev number tick up. The upper bound is
	 *     set to major dev 127 to avoid any sign extention problems
	 */
	mp->m_fsid.val[0] = makedev(nblkdev, 0);
	mp->m_fsid.val[1] = MOUNT_NFS;
	if (++nfs_mntid == 0)
		++nfs_mntid;
	tfsid.val[0] = makedev(nblkdev, nfs_mntid);
	tfsid.val[1] = MOUNT_NFS;
	while (getvfs(&tfsid)) {
		tfsid.val[0]++;
		nfs_mntid++;
	}
	if (major(tfsid.val[0]) > 127) {
		error = ENOENT;
		goto bad;
	}
	mp->m_fsid.val[0] = tfsid.val[0];
	nmp->nm_mountp = mp;
	nmp->nm_flag = argp->flags;
	nmp->nm_sockaddr = saddr;
	/* Set up the sockets */
	if (error = socreate(AF_INET, &nmp->nm_so, SOCK_DGRAM, 0))
		goto bad;
	if (error = soconnect(nmp->nm_so, saddr))
		goto bad;
	if ((argp->flags & NFSMNT_TIMEO) && argp->timeo >= 1)
		nmp->nm_timeo = argp->timeo;
	else
		nmp->nm_timeo = NFS_TIMEO;
	if ((argp->flags & NFSMNT_RETRANS) && argp->retrans > 0)
		nmp->nm_retrans = argp->retrans;
	else
		nmp->nm_retrans = NFS_RETRANS;
	if ((argp->flags & NFSMNT_WSIZE) &&
	    argp->wsize <= NFS_MAXDATA && argp->wsize > 0 &&
	   (argp->wsize & 0x1ff) == 0)
		nmp->nm_wsize = argp->wsize;
	else
		nmp->nm_wsize = NFS_WSIZE;
	if ((argp->flags & NFSMNT_RSIZE) &&
	    argp->rsize <= NFS_MAXDATA && argp->rsize > 0 &&
	   (argp->rsize & 0x1ff) == 0)
		nmp->nm_rsize = argp->rsize;
	else
		nmp->nm_rsize = NFS_RSIZE;
	bcopy((caddr_t)argp->fh, (caddr_t)&nmp->nm_fh, sizeof(nfsv2fh_t));
	bcopy(pth, nmp->nm_path, MNAMELEN);
	bcopy(hst, nmp->nm_host, MNAMELEN);
	/*
	 * Set to CLBYTES so that vinifod() doesn't get confused.
	 * Actually any exact multiple of CLBYTES will do
	 */
	mp->m_bsize = mp->m_fsize = CLBYTES;
	/*
	 * A reference count is needed on the nfsnode representing the
	 * remote root.  If this object is not persistent, then backward
	 * traversals of the mount point (i.e. "..") will not work if
	 * the nfsnode gets flushed out of the cache. Ufs does not have
	 * this problem, because one can identify root inodes by their
	 * number == ROOTINO (2).
	 */
	if (error = nfs_nget(mp, &nmp->nm_fh, &np))
		goto bad;
	/*
	 * Unlock it, but keep the reference count.
	 */
	nfs_unlock(NFSTOV(np));
	return (0);
bad:
	m_freem(saddr);
	free((caddr_t)nmp, M_NFSMNT);
	return (error);
}

/*
 * unmount system call
 */
nfs_unmount(mp, flags)
	struct mount *mp;
	int flags;
{
	register struct nfsmount *nmp;
	register struct nfsreq *rep;
	struct nfsreq *rep2;
	struct nfsnode *np;
	int error;
	int s;

	if (flags & MNT_FORCE)
		return (EINVAL);
	nmp = vfs_to_nfs(mp);
	/*
	 * Clear out the buffer cache
	 */
	mntflushbuf(mp, 0);
	if (mntinvalbuf(mp))
		return (EBUSY);
	/*
	 * Goes something like this..
	 * - Decrement reference on the nfsnode representing remote root.
	 *   Must do this first, otherwise vflush will return EBUSY.
	 * - Call vflush() to clear out vnodes for this file system
	 * - Flush out lookup cache
	 * - Close the socket
	 * - Free up the data structures
	 */
	/*
	 * We need to decrement the ref. count on the nfsnode representing
	 * the remote root.  See comment in mountnfs().  The VFS unmount()
	 * has done vput on this vnode, otherwise we would get deadlock!
	 */
	if (error = nfs_nget(mp, &nmp->nm_fh, &np))
		return(error);
	/*
	 * Get rid of two reference counts, and unlock it on the second.
	 */
	vrele(NFSTOV(np));
	vput(NFSTOV(np));
	if (error = vflush(mp, (struct vnode *)0, flags))
		return (error);
	/*
	 * Scan the request list for any requests left hanging about
	 */
	s = splnet();
	rep = nfsreqh.r_next;
	while (rep && rep != &nfsreqh) {
		if (rep->r_mntp == nmp) {
			rep->r_prev->r_next = rep2 = rep->r_next;
			rep->r_next->r_prev = rep->r_prev;
			m_freem(rep->r_mreq);
			if (rep->r_mrep != NULL)
				m_freem(rep->r_mrep);
			free((caddr_t)rep, M_NFSREQ);
			rep = rep2;
		} else
			rep = rep->r_next;
	}
	splx(s);
	soclose(nmp->nm_so);
	m_freem(nmp->nm_sockaddr);
	free((caddr_t)nmp, M_NFSMNT);
	return (0);
}

/*
 * Return root of a filesystem
 */
nfs_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{
	register struct vnode *vp;
	struct nfsmount *nmp;
	struct nfsnode *np;
	int error;

	nmp = vfs_to_nfs(mp);
	if (error = nfs_nget(mp, &nmp->nm_fh, &np))
		return (error);
	vp = NFSTOV(np);
	vp->v_type = VDIR;
	vp->v_flag = VROOT;
	*vpp = vp;
	return (0);
}

extern int syncprt;

/*
 * Flush out the buffer cache
 */
/* ARGSUSED */
nfs_sync(mp, waitfor)
	struct mount *mp;
	int waitfor;
{
	if (syncprt)
		bufstats();
	/*
	 * Force stale buffer cache information to be flushed.
	 */
	mntflushbuf(mp, waitfor == MNT_WAIT ? B_SYNC : 0);
	return (0);
}

/*
 * At this point, this should never happen
 */
/* ARGSUSED */
nfs_fhtovp(mp, fhp, vpp)
	struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{

	return (EINVAL);
}

/*
 * Vnode pointer to File handle, should never happen either
 */
/* ARGSUSED */
nfs_vptofh(mp, fhp, vpp)
	struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{

	return (EINVAL);
}

/*
 * Vfs start routine, a no-op.
 */
/* ARGSUSED */
nfs_start(mp, flags)
	struct mount *mp;
	int flags;
{

	return (0);
}
