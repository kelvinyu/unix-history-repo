/*
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)uipc_syscalls.c	7.12 (Berkeley) %G%
 */

#include "param.h"
/* #include "user.h" */
#include "syscontext.h" /* XXX */
#include "proc.h"
#include "file.h"
#include "buf.h"
#include "malloc.h"
#include "mbuf.h"
#include "protosw.h"
#include "socket.h"
#include "socketvar.h"

/*
 * System call interface to the socket abstraction.
 */

struct	file *getsock();
extern	struct fileops socketops;

socket()
{
	register struct a {
		int	domain;
		int	type;
		int	protocol;
	} *uap = (struct a *)u.u_ap;
	struct socket *so;
	struct file *fp;
	int fd;

	if (u.u_error = falloc(&fp, &fd))
		return;
	fp->f_flag = FREAD|FWRITE;
	fp->f_type = DTYPE_SOCKET;
	fp->f_ops = &socketops;
	u.u_error = socreate(uap->domain, &so, uap->type, uap->protocol);
	if (u.u_error)
		goto bad;
	fp->f_data = (caddr_t)so;
	u.u_r.r_val1 = fd;
	return;
bad:
	u.u_ofile[fd] = 0;
	crfree(fp->f_cred);
	fp->f_count = 0;
}

bind()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct mbuf *nam;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	u.u_error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
	if (u.u_error)
		return;
	u.u_error = sobind((struct socket *)fp->f_data, nam);
	m_freem(nam);
}

listen()
{
	register struct a {
		int	s;
		int	backlog;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	u.u_error = solisten((struct socket *)fp->f_data, uap->backlog);
}

#ifdef COMPAT_43
accept()
{
	accept1(0);
}

oaccept()
{
	accept1(1);
}

accept1(compat_43)
#else
accept()
#endif
{
	register struct a {
		int	s;
		caddr_t	name;
		int	*anamelen;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	struct mbuf *nam;
	int namelen, error, s;
	register struct socket *so;

	if (uap->name == 0)
		goto noname;
	error = copyin((caddr_t)uap->anamelen, (caddr_t)&namelen,
		sizeof (namelen));
	if (error)
		RETURN (error);
	if (useracc((caddr_t)uap->name, (u_int)namelen, B_WRITE) == 0)
		RETURN (EFAULT);
noname:
	fp = getsock(uap->s);
	if (fp == 0)
		return;
	s = splnet();
	so = (struct socket *)fp->f_data;
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		splx(s);
		RETURN (EINVAL);
	}
	if ((so->so_state & SS_NBIO) && so->so_qlen == 0) {
		splx(s);
		RETURN (EWOULDBLOCK);
	}
	while (so->so_qlen == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		if (error = tsleep((caddr_t)&so->so_timeo, PSOCK | PCATCH,
		    netcon, 0)) {
			splx(s);
			RETURN (error);
		}
	}
	if (so->so_error) {
		error = so->so_error;
		so->so_error = 0;
		splx(s);
		RETURN (error);
	}
	if (error = falloc(&fp, &u.u_r.r_val1)) {
		splx(s);
		RETURN (error);
	}
	{ struct socket *aso = so->so_q;
	  if (soqremque(aso, 1) == 0)
		panic("accept");
	  so = aso;
	}
	fp->f_type = DTYPE_SOCKET;
	fp->f_flag = FREAD|FWRITE;
	fp->f_ops = &socketops;
	fp->f_data = (caddr_t)so;
	nam = m_get(M_WAIT, MT_SONAME);
	(void) soaccept(so, nam);
	if (uap->name) {
#ifdef COMPAT_43
		if (compat_43)
			mtod(nam, struct osockaddr *)->sa_family =
			    mtod(nam, struct sockaddr *)->sa_family;
#endif
		if (namelen > nam->m_len)
			namelen = nam->m_len;
		/* SHOULD COPY OUT A CHAIN HERE */
		(void) copyout(mtod(nam, caddr_t), (caddr_t)uap->name,
		    (u_int)namelen);
		(void) copyout((caddr_t)&namelen, (caddr_t)uap->anamelen,
		    sizeof (*uap->anamelen));
	}
	m_freem(nam);
	splx(s);
	RETURN (0);
}

connect()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *nam;
	int error, s;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	so = (struct socket *)fp->f_data;
	if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING))
		RETURN (EALREADY);
	if (error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME))
		RETURN (error);
	error = soconnect(so, nam);
	if (error)
		goto bad;
	if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING)) {
		m_freem(nam);
		RETURN (EINPROGRESS);
	}
	s = splnet();
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0)
		if (error = tsleep((caddr_t)&so->so_timeo, PSOCK | PCATCH,
		    netcon, 0))
			break;
	if (error == 0) {
		error = so->so_error;
		so->so_error = 0;
	}
	splx(s);
bad:
	so->so_state &= ~SS_ISCONNECTING;
	m_freem(nam);
	if (error == ERESTART)
		error = EINTR;
	RETURN (error);
}

socketpair()
{
	register struct a {
		int	domain;
		int	type;
		int	protocol;
		int	*rsv;
	} *uap = (struct a *)u.u_ap;
	struct file *fp1, *fp2;
	struct socket *so1, *so2;
	int fd, sv[2];

	if (useracc((caddr_t)uap->rsv, 2 * sizeof (int), B_WRITE) == 0) {
		u.u_error = EFAULT;
		return;
	}
	u.u_error = socreate(uap->domain, &so1, uap->type, uap->protocol);
	if (u.u_error)
		return;
	u.u_error = socreate(uap->domain, &so2, uap->type, uap->protocol);
	if (u.u_error)
		goto free1;
	if (u.u_error = falloc(&fp1, &fd))
		goto free2;
	sv[0] = fd;
	fp1->f_flag = FREAD|FWRITE;
	fp1->f_type = DTYPE_SOCKET;
	fp1->f_ops = &socketops;
	fp1->f_data = (caddr_t)so1;
	if (u.u_error = falloc(&fp2, &fd))
		goto free3;
	fp2->f_flag = FREAD|FWRITE;
	fp2->f_type = DTYPE_SOCKET;
	fp2->f_ops = &socketops;
	fp2->f_data = (caddr_t)so2;
	sv[1] = fd;
	u.u_error = soconnect2(so1, so2);
	if (u.u_error)
		goto free4;
	if (uap->type == SOCK_DGRAM) {
		/*
		 * Datagram socket connection is asymmetric.
		 */
		 u.u_error = soconnect2(so2, so1);
		 if (u.u_error)
			goto free4;
	}
	(void) copyout((caddr_t)sv, (caddr_t)uap->rsv, 2 * sizeof (int));
	u.u_r.r_val1 = sv[0];
	u.u_r.r_val2 = sv[1];
	return;
free4:
	crfree(fp2->f_cred);
	fp2->f_count = 0;
	u.u_ofile[sv[1]] = 0;
free3:
	crfree(fp1->f_cred);
	fp1->f_count = 0;
	u.u_ofile[sv[0]] = 0;
free2:
	(void)soclose(so2);
free1:
	(void)soclose(so1);
}

sendto()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	to;
		int	tolen;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = uap->to;
	msg.msg_namelen = uap->tolen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	msg.msg_control = 0;
#ifdef COMPAT_43
	msg.msg_flags = 0;
#endif
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	sendit(uap->s, &msg, uap->flags);
}

#ifdef COMPAT_43

osend()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_control = 0;
	msg.msg_flags = 0;
	sendit(uap->s, &msg, uap->flags);
}

#define MSG_OSENDMSG	0x8000
osendmsg()
{
	register struct a {
		int	s;
		caddr_t	msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];

	u.u_error = copyin(uap->msg, (caddr_t)&msg, sizeof (struct omsghdr));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		u.u_error = EMSGSIZE;
		return;
	}
	u.u_error = copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
		(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	if (u.u_error)
		return;
	msg.msg_flags = MSG_OSENDMSG;
	sendit(uap->s, &msg, uap->flags);
}
#endif

sendmsg()
{
	register struct a {
		int	s;
		caddr_t	msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];

	if (u.u_error = copyin(uap->msg, (caddr_t)&msg, sizeof (msg)))
		return;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		u.u_error = EMSGSIZE;
		return;
	}
	if (msg.msg_iovlen &&
	    (u.u_error = copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
				(unsigned)(msg.msg_iovlen * sizeof (aiov[0])))))
		return;
	msg.msg_iov = aiov;
#ifdef COMPAT_43
	msg.msg_flags = 0;
#endif
	sendit(uap->s, &msg, uap->flags);
}

sendit(s, mp, flags)
	int s;
	register struct msghdr *mp;
	int flags;
{
	register struct file *fp;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *to, *control;
	int len;
	
	fp = getsock(s);
	if (fp == 0)
		return;
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_WRITE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0) {
			u.u_error = EINVAL;
			return;
		}
		if (iov->iov_len == 0)
			continue;
		if (useracc(iov->iov_base, (u_int)iov->iov_len, B_READ) == 0) {
			u.u_error = EFAULT;
			return;
		}
		auio.uio_resid += iov->iov_len;
	}
	if (mp->msg_name) {
		u.u_error =
		    sockargs(&to, mp->msg_name, mp->msg_namelen, MT_SONAME);
		if (u.u_error)
			return;
	} else
		to = 0;
	if (mp->msg_control) {
		u.u_error =
		    sockargs(&control, mp->msg_control, mp->msg_controllen,
		    MT_CONTROL);
		if (u.u_error)
			goto bad;
#ifdef COMPAT_43
		if (mp->msg_flags) {
			register struct cmsghdr *cm;
			M_PREPEND(control, sizeof(*cm), M_WAIT);
			if (control == 0)
				u.u_error = ENOBUFS;
			else {
				cm = mtod(control, struct cmsghdr *);
				cm->cmsg_len = control->m_len;
				cm->cmsg_level = SOL_SOCKET;
				cm->cmsg_type = SCM_RIGHTS;
			}
		}
#endif
	} else
		control = 0;
	len = auio.uio_resid;
	u.u_error = sosend((struct socket *)fp->f_data, to, &auio,
	    flags, control);
	if ((u.u_error == EINTR || u.u_error == ERESTART ||
	    u.u_error == EWOULDBLOCK) && auio.uio_resid != len)
		u.u_error = 0;
	u.u_r.r_val1 = len - auio.uio_resid;
	if (control)
		m_freem(control);
bad:
	if (to)
		m_freem(to);
}

#ifdef COMPAT_43
recvfrom()
{
	recvfrom1(0);
}

orecvfrom()
{
	recvfrom1(1);
}

recvfrom1(compat_43)
{	/* vi will want an extra } to be happy! */
#else
recvfrom()
{
	int compat_43 = 0;
#endif
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	from;
		int	*fromlenaddr;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;
	int len;

	if (uap->fromlenaddr) {
		u.u_error = copyin((caddr_t)uap->fromlenaddr, (caddr_t)&len,
		   sizeof (len));
		if (u.u_error)
			return;
	} else
		len = 0;
	msg.msg_name = uap->from;
	msg.msg_namelen = len;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_control = 0;
	msg.msg_flags = uap->flags;
	recvit(uap->s, &msg, (caddr_t)uap->fromlenaddr, (caddr_t)0, compat_43);
}
#ifdef COMPAT_43
orecv()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_control = 0;
	msg.msg_flags = uap->flags;
	recvit(uap->s, &msg, (caddr_t)0, (caddr_t)0, 0);
}

orecvmsg()
{
	register struct a {
		int	s;
		struct	omsghdr *msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];

	u.u_error = copyin((caddr_t)uap->msg, (caddr_t)&msg,
	    sizeof (struct omsghdr));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		u.u_error = EMSGSIZE;
		return;
	}
	msg.msg_flags = uap->flags;
	u.u_error = copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
		(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	if (u.u_error)
		return;
	msg.msg_iov = aiov;
	if (msg.msg_control)
		if (useracc((caddr_t)msg.msg_control,
		    (unsigned)msg.msg_controllen, B_WRITE) == 0) {
			    u.u_error = EFAULT;
			    return;
		}
	    
	recvit(uap->s, &msg, (caddr_t)&uap->msg->msg_namelen,
	    (caddr_t)&uap->msg->msg_accrightslen, /* compat_43 */1);
}
#endif

recvmsg()
{
	register struct a {
		int	s;
		struct	msghdr *msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN], *uiov;
	register int error1;

	u.u_error = copyin((caddr_t)uap->msg, (caddr_t)&msg, sizeof (msg));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		u.u_error = EMSGSIZE;
		return;
	}
	msg.msg_flags = uap->flags;
	uiov = msg.msg_iov;
	msg.msg_iov = aiov;
	u.u_error = copyin((caddr_t)uiov, (caddr_t)aiov,
		(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	if (u.u_error)
		return;
	if (msg.msg_control)
		if (useracc((caddr_t)msg.msg_control,
		    (unsigned)msg.msg_controllen, B_WRITE) == 0) {
			u.u_error = EFAULT;
			return;
		}
	recvit(uap->s, &msg, (caddr_t)0, (caddr_t)0, 0);
	msg.msg_iov = uiov;
	error1 = copyout((caddr_t)&msg, (caddr_t)uap->msg, sizeof(msg));
	if (error1 && u.u_error == 0)
		u.u_error = error1;
}

/* ARGSUSED */
recvit(s, mp, namelenp, rightslenp, compat_43)
	int s, compat_43;
	register struct msghdr *mp;
	caddr_t namelenp, rightslenp;
{
	register struct file *fp;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	int len;
	struct mbuf *from = 0, *rights = 0, *control = 0;
	
	fp = getsock(s);
	if (fp == 0)
		return;
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0) {
			u.u_error = EINVAL;
			return;
		}
		if (iov->iov_len == 0)
			continue;
		if (useracc(iov->iov_base, (u_int)iov->iov_len, B_WRITE) == 0) {
			u.u_error = EFAULT;
			return;
		}
		auio.uio_resid += iov->iov_len;
	}
	len = auio.uio_resid;
	u.u_error = soreceive((struct socket *)fp->f_data, &from, &auio,
	    &mp->msg_flags, &rights, &control);
	if ((u.u_error == EINTR || u.u_error == ERESTART ||
	    u.u_error == EWOULDBLOCK) && auio.uio_resid != len)
		u.u_error = 0;
	u.u_r.r_val1 = len - auio.uio_resid;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
#ifdef COMPAT_43
			if (compat_43)
				mtod(from, struct osockaddr *)->sa_family =
				    mtod(from, struct sockaddr *)->sa_family;
#endif
			if (len > from->m_len)		/* ??? */
				len = from->m_len;
			(void) copyout(mtod(from, caddr_t),
			    (caddr_t)mp->msg_name, (unsigned)len);
		}
		mp->msg_namelen = len;
		if (namelenp)
			(void) copyout((caddr_t)&len, namelenp, sizeof (int));
	}
	if (rightslenp) {	/* only from orecvmsg() */
		len = mp->msg_controllen;
		if (len <= 0 || rights == 0)
			len = 0;
		else {
			rights->m_len -= sizeof (struct cmsghdr);
			rights->m_data += sizeof (struct cmsghdr);
			if (len > rights->m_len)
				len = rights->m_len;
			(void) copyout((caddr_t)mtod(rights, caddr_t),
			    (caddr_t)mp->msg_control, (unsigned)len);
			m_freem(rights);
			rights = 0;
		}
		(void) copyout((caddr_t)&len, rightslenp, sizeof (int));
		mp->msg_control = 0;
	}
	if (mp->msg_control) {
		if (rights) {
			if (control) {
				rights->m_next = control;
				rights = m_pullup(rights,
						rights->m_len + control->m_len);
				if (rights == 0)
					mp->msg_flags |= MSG_CTRUNC;
			}
			control = rights;
			rights = 0;
		}
		len = mp->msg_controllen;
		if (len <= 0 || control == 0)
			len = 0;
		else {
			if (len >= control->m_len)
				len = control->m_len;
			else
				mp->msg_flags |= MSG_CTRUNC;
			(void) copyout((caddr_t)mtod(control, caddr_t),
			    (caddr_t)mp->msg_control, (unsigned)len);
		}
		mp->msg_controllen = len;
	}
	if (rights)
		m_freem(rights);
	if (from)
		m_freem(from);
	if (control)
		m_freem(control);
}

shutdown()
{
	struct a {
		int	s;
		int	how;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	u.u_error = soshutdown((struct socket *)fp->f_data, uap->how);
}

setsockopt()
{
	struct a {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	valsize;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	struct mbuf *m = NULL;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	if (uap->valsize > MLEN) {
		u.u_error = EINVAL;
		return;
	}
	if (uap->val) {
		m = m_get(M_WAIT, MT_SOOPTS);
		if (m == NULL) {
			u.u_error = ENOBUFS;
			return;
		}
		u.u_error =
		    copyin(uap->val, mtod(m, caddr_t), (u_int)uap->valsize);
		if (u.u_error) {
			(void) m_free(m);
			return;
		}
		m->m_len = uap->valsize;
	}
	u.u_error =
	    sosetopt((struct socket *)fp->f_data, uap->level, uap->name, m);
}

getsockopt()
{
	struct a {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	*avalsize;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	struct mbuf *m = NULL;
	int valsize;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	if (uap->val) {
		u.u_error = copyin((caddr_t)uap->avalsize, (caddr_t)&valsize,
			sizeof (valsize));
		if (u.u_error)
			return;
	} else
		valsize = 0;
	u.u_error =
	    sogetopt((struct socket *)fp->f_data, uap->level, uap->name, &m);
	if (u.u_error)
		goto bad;
	if (uap->val && valsize && m != NULL) {
		if (valsize > m->m_len)
			valsize = m->m_len;
		u.u_error = copyout(mtod(m, caddr_t), uap->val, (u_int)valsize);
		if (u.u_error)
			goto bad;
		u.u_error = copyout((caddr_t)&valsize, (caddr_t)uap->avalsize,
		    sizeof (valsize));
	}
bad:
	if (m != NULL)
		(void) m_free(m);
}

pipe()
{
	struct file *rf, *wf;
	struct socket *rso, *wso;
	int fd;

	u.u_error = socreate(AF_UNIX, &rso, SOCK_STREAM, 0);
	if (u.u_error)
		return;
	u.u_error = socreate(AF_UNIX, &wso, SOCK_STREAM, 0);
	if (u.u_error)
		goto free1;
	if (u.u_error = falloc(&rf, &fd))
		goto free2;
	u.u_r.r_val1 = fd;
	rf->f_flag = FREAD;
	rf->f_type = DTYPE_SOCKET;
	rf->f_ops = &socketops;
	rf->f_data = (caddr_t)rso;
	if (u.u_error = falloc(&wf, &fd))
		goto free3;
	wf->f_flag = FWRITE;
	wf->f_type = DTYPE_SOCKET;
	wf->f_ops = &socketops;
	wf->f_data = (caddr_t)wso;
	u.u_r.r_val2 = fd;
	if (u.u_error = unp_connect2(wso, rso))
		goto free4;
	return;
free4:
	wf->f_count = 0;
	u.u_ofile[u.u_r.r_val2] = 0;
free3:
	rf->f_count = 0;
	u.u_ofile[u.u_r.r_val1] = 0;
free2:
	(void)soclose(wso);
free1:
	(void)soclose(rso);
}

/*
 * Get socket name.
 */
#ifdef COMPAT_43
getsockname()
{
	getsockname1(0);
}

ogetsockname()
{
	getsockname1(1);
}

getsockname1(compat_43)
#else
getsockname()
#endif
{
	register struct a {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len;

	fp = getsock(uap->fdes);
	if (fp == 0)
		return;
	u.u_error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len));
	if (u.u_error)
		return;
	so = (struct socket *)fp->f_data;
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		u.u_error = ENOBUFS;
		return;
	}
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR, 0, m, 0);
	if (u.u_error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
#ifdef COMPAT_43
	if (compat_43)
		mtod(m, struct osockaddr *)->sa_family =
		    mtod(m, struct sockaddr *)->sa_family;
#endif
	u.u_error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (u.u_error == 0)
		u.u_error = copyout((caddr_t)&len, (caddr_t)uap->alen,
		    sizeof (len));
bad:
	m_freem(m);
}

/*
 * Get name of peer for connected socket.
 */
#ifdef COMPAT_43
getpeername()
{
	getpeername1(0);
}

ogetpeername()
{
	getpeername1(1);
}

getpeername1(compat_43)
#else
getpeername()
#endif
{
	register struct a {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len;

	fp = getsock(uap->fdes);
	if (fp == 0)
		return;
	so = (struct socket *)fp->f_data;
	if ((so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING)) == 0) {
		u.u_error = ENOTCONN;
		return;
	}
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		u.u_error = ENOBUFS;
		return;
	}
	u.u_error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len));
	if (u.u_error)
		return;
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_PEERADDR, 0, m, 0);
	if (u.u_error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
#ifdef COMPAT_43
	if (compat_43)
		mtod(m, struct osockaddr *)->sa_family =
		    mtod(m, struct sockaddr *)->sa_family;
#endif
	u.u_error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (u.u_error)
		goto bad;
	u.u_error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
bad:
	m_freem(m);
}

sockargs(aname, name, namelen, type)
	struct mbuf **aname;
	caddr_t name;
	int namelen, type;
{
	register struct mbuf *m;
	int error;

	if ((u_int)namelen > MLEN) {
#ifdef COMPAT_43
		if (type == MT_SONAME && (u_int)namelen <= 112)
			namelen = MLEN;		/* unix domain compat. hack */
		else
#endif
		return (EINVAL);
	}
	m = m_get(M_WAIT, type);
	if (m == NULL)
		return (ENOBUFS);
	m->m_len = namelen;
	error = copyin(name, mtod(m, caddr_t), (u_int)namelen);
	if (error)
		(void) m_free(m);
	else
		*aname = m;
	if (type == MT_SONAME) {
		register struct sockaddr *sa = mtod(m, struct sockaddr *);
#if defined(COMPAT_43) && BYTE_ORDER != BIG_ENDIAN
		if (sa->sa_family == 0 && sa->sa_len < AF_MAX)
			sa->sa_family = sa->sa_len;
#endif
		sa->sa_len = namelen;
	}
	return (error);
}

struct file *
getsock(fdes)
	int fdes;
{
	register struct file *fp;

	if ((unsigned)fdes >= NOFILE || (fp = u.u_ofile[fdes]) == NULL) {
		u.u_error = EBADF;
		return (0);
	}
	if (fp->f_type != DTYPE_SOCKET) {
		u.u_error = ENOTSOCK;
		return (0);
	}
	return (fp);
}
