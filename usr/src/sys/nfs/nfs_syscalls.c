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
 *	@(#)nfs_syscalls.c	7.1 (Berkeley) %G%
 */

#include "param.h"
#include "systm.h"
#include "user.h"
#include "kernel.h"
#include "file.h"
#include "stat.h"
#include "vnode.h"
#include "mount.h"
#include "proc.h"
#include "uio.h"
#include "malloc.h"
#include "mbuf.h"
#include "socket.h"
#include "socketvar.h"
#include "nfsv2.h"
#include "nfs.h"

/* Global defs. */
extern u_long nfs_prog, nfs_vers;
extern int (*nfsrv_procs[NFS_NPROCS])();
struct file *getsock();

/*
 * NFS server system calls
 * getfh() lives here too, but maybe should move to kern/vfs_syscalls.c
 */
#define RETURN(value)	{ u.u_error = (value); return; }

/*
 * Get file handle system call
 */
getfh()
{
	register struct a {
		char	*fname;
		fhandle_t *fhp;
	} *uap = (struct a *)u.u_ap;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	fhandle_t fh;
	int error;

	/*
	 * Must be super user
	 */
	if (error = suser(u.u_cred, &u.u_acflag))
		RETURN (error);
	ndp->ni_nameiop = LOOKUP | LOCKLEAF | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		RETURN (error);
	vp = ndp->ni_vp;
	bzero((caddr_t)&fh, sizeof(fh));
	fh.fh_fsid = vp->v_mount->m_fsid;
	error = VFS_VPTOFH(vp, &fh.fh_fid);
	vput(vp);
	if (error)
		RETURN (error);
	error = copyout((caddr_t)&fh, (caddr_t)uap->fhp, sizeof (fh));
	RETURN (error);
}

/*
 * Mark a mount point in the filesystem exported
 */
exportfs()
{
	register struct a {
		char	*fname;
		int	rootuid;
		int	exflags;
	} *uap = (struct a *)u.u_ap;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	register struct mount *mp;
	int error;

	/*
	 * Must be super user
	 */
	if (error = suser(u.u_cred, &u.u_acflag))
		RETURN (error);
	ndp->ni_nameiop = LOOKUP | LOCKLEAF | FOLLOW;	/* Or NOFOLLOW ?? */
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		RETURN (error);
	vp = ndp->ni_vp;
	if (vp->v_type != VDIR) {
		vput(vp);
		RETURN (ENOENT);
	}
	mp = vp->v_mount;

	/*
	 * If the filesystem has already been exported, just relax
	 * security as required.
	 * Otherwise export it with the given security
	 */
	if (mp->m_flag & M_EXPORTED) {
		if (uap->rootuid == 0)
			mp->m_exroot = 0;
		if ((uap->exflags & M_EXRDONLY) == 0)
			mp->m_flag &= ~M_EXRDONLY;
	} else {
		mp->m_exroot = uap->rootuid;
		if (uap->exflags & M_EXRDONLY)
			mp->m_flag |= M_EXRDONLY;
		mp->m_flag |= M_EXPORTED;
	}
	vput(vp);
	RETURN (0);
}

/*
 * Nfs server psuedo system call for the nfsd's
 * Never returns unless it fails or gets killed
 */
nfssvc()
{
	register struct a {
		int s;
	} *uap = (struct a *)u.u_ap;
	register struct mbuf *m;
	register int siz;
	register struct ucred *cr;
	struct file *fp;
	struct mbuf *mreq, *mrep, *nam, *md;
	struct socket *so;
	caddr_t dpos;
	int proc;
	u_long retxid;
	int error;

	/*
	 * Must be super user
	 */
	if (error = suser(u.u_cred, &u.u_acflag))
		RETURN (error);
	fp = getsock(uap->s);
	if (fp == 0)
		return;
	so = (struct socket *)fp->f_data;
	cr = u.u_cred = crcopy(u.u_cred);	/* Copy it so others don't see changes */
	/*
	 * Just loop arround doin our stuff until SIGKILL
	 */
	for (;;) {
		if (error = nfs_getreq(so, nfs_prog, nfs_vers, NFS_NPROCS-1,
			&nam, &mrep, &md, &dpos, &retxid, &proc, cr)) {
			m_freem(nam);
			continue;
		}
		if (error = (*(nfsrv_procs[proc]))(mrep, md, dpos,
			cr, retxid, &mreq)) {
			m_freem(nam);
			nfsstats.srv_errs++;
			continue;
		} else
			nfsstats.srvrpccnt[proc]++;
		m = mreq;
		siz = 0;
		while (m) {
			siz += m->m_len;
			m = m->m_next;
		}
		if (siz <= 0 || siz > 9216) {
			printf("mbuf siz=%d\n",siz);
			panic("Bad nfs svc reply");
		}
		error = nfs_udpsend(so, nam, mreq, 0, siz);
		m_freem(nam);
	}
}
