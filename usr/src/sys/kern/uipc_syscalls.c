/*	uipc_syscalls.c	4.3	81/11/16	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/buf.h"
#include "../h/mbuf.h"
#include "../h/protocol.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/inaddr.h"
#include "../net/inet.h"
#include "../net/inet_systm.h"

/*
 * Socket system call interface.
 *
 * These routines interface the socket routines to UNIX,
 * isolating the system interface from the socket-protocol interface.
 *
 * TODO:
 *	SO_NEWFDONCONN
 *	SO_INTNOTIFY
 */

static	struct in_addr localaddr = { PF_LOCAL };
/*
 * Pipe system call interface.
 */
spipe()
{
	register struct file *rf, *wf;
	struct socket *rso, *wso;
	int r;
	
	u.u_error = socket(&rso, SOCK_STREAM, &localaddr, SO_ACCEPTCONN);
	if (u.u_error)
		return;
	u.u_error = socket(&wso, SOCK_STREAM, &localaddr, 0);
	if (u.u_error)
		goto free;
	rf = falloc();
	if (rf == NULL)
		goto free2;
	r = u.u_r.r_val1;
	rf->f_flag = FREAD|FSOCKET;
	rf->f_socket = rso;
	wf = falloc();
	if (wf == NULL)
		goto free3;
	wf->f_flag = FWRITE|FSOCKET;
	wf->f_socket = wso;
	u.u_r.r_val2 = u.u_r.r_val1;
	u.u_r.r_val1 = r;
	if (pi_connect(rso, wso) == 0)
		goto free4;
	return;
free4:
	wf->f_count = 0;
	u.u_ofile[u.u_r.r_val1] = 0;
free3:
	rf->f_count = 0;
	u.u_ofile[r] = 0;
free2:
	sofree(wso);
free:
	sofree(rso);
}

/*
 * Splice system call interface.
 */
ssplice()
{
	register struct a {
		int	fd1;
		int	fd2;
	} *ap = (struct a *)u.u_ap;
	struct file *f1, *f2;

	f1 = getf(ap->fd1);
	if (f1 == NULL)
		return;
	f2 = getf(ap->fd2);
	if (f2 == NULL)
		return;
	if ((f1->f_flag & FSOCKET) == 0 || (f2->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	if (f1->f_count > 1 || f2->f_count > 1) {
		u.u_error = ETOOMANYREFS;
		return;
	}
	u.u_error = pi_splice(f1->f_socket, f2->f_socket);
	if (u.u_error)
		return;
	u.u_ofile[ap->fd1] = 0;
	u.u_ofile[ap->fd2] = 0;
	f1->f_count = 0;
	f2->f_count = 0;
}

/*
 * Socket system call interface.  Copy in arguments
 * set up file descriptor and call internal socket
 * creation routine.
 */
ssocket()
{
	register struct a {
		int	type;
		struct	in_addr *ain;
		int	options;
	} *uap = (struct a *)u.u_ap;
	struct in_addr in;
	struct socket *so0;
	register struct file *fp;

	if ((fp = falloc()) == NULL)
		return;
	fp->f_flag = FSOCKET|FREAD|FWRITE;
	if (copyin((caddr_t)uap->ain, (caddr_t)&in, sizeof (in))) {
		u.u_error = EFAULT;
		return;
	}
	u.u_error = socket(&so0, uap->type, &in, uap->options);
	if (u.u_error)
		goto bad;
	fp->f_socket = so0;
	return;
bad:
	u.u_ofile[u.u_r.r_val1] = 0;
	fp->f_count = 0;
}

saccept()
{

}

/*
 * Connect socket to foreign peer; system call
 * interface.  Copy in arguments and call internal routine.
 */
sconnect()
{
	register struct a {
		int fdes;
		struct in_addr *a;
	} *uap = (struct a *)u.u_ap;
	in_addr in;
	register struct file *fp;
	register struct socket *so;
	int s;

	if (copyin((caddr_t)uap->a, (caddr_t)&in, sizeof (in))) {
		u.u_error = EFAULT;
		return;
	}
	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	so = fp->f_socket;
	u.u_error = connect(so, &in);
	if (u.u_error)
		return;
	s = splnet();
	if ((so->so_options & SO_NBIO) &&
	    (so->so_state & SS_ISCONNECTING)) {
		u.u_error = EINPROGRESS;
		splx(s);
		return;
	}
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0)
		sleep((caddr_t)&so->so_timeo, PZERO+1);
	u.u_error = so->so_error;
	so->so_error = 0;
	splx(s);
}

/*
 * Disconnect socket from foreign peer; system call
 * interface.  Copy in arguments and call internal routine.
 */
sdisconnect()
{
	register struct a {
		int	fdes;
		in_addr	 *addr;
	} *uap = (struct a *)u.u_ap;
	in_addr in;
	register struct file *fp;
	register struct socket *so;
	int s;

	if (uap->addr &&
	    copyin((caddr_t)uap->addr, (caddr_t)&in, sizeof (in))) {
		u.u_error = EFAULT;
		return;
	}
	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	so = fp->f_socket;
	u.u_error = disconnect(so, uap->addr ? &in : 0);
	if (u.u_error)
		return;
	s = splnet();
	if ((so->so_options&SO_NBIO) && (so->so_state&SS_ISDISCONNECTING)) {
		u.u_error = EINPROGRESS;
		splx(s);
		return;
	}
	while (so->so_state & SS_ISDISCONNECTING)
		sleep((caddr_t)&so->so_timeo, PZERO+1);
	u.u_error = so->so_error;
	so->so_error = 0;
	splx(s);
}

/*
 * Send data on socket.
 */
ssend()
{
	register struct a {
		int	fdes;
		in_addr	*ain;
		caddr_t	cbuf;
		u_int	count;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct in_addr in;

	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	u.u_count = uap->count;
	u.u_segflg = 0;
	if (useracc(uap->cbuf, uap->count, B_READ) == 0 ||
	    uap->ain && copyin((caddr_t)uap->ain, (caddr_t)&in, sizeof (in))) {
		u.u_error = EFAULT;
		return;
	}
	u.u_error = send(fp->f_socket, uap->ain ? &in : 0);
}

/*
 * Receive data on socket.
 */
sreceive()
{
	register struct a {
		int	fdes;
		in_addr	*ain;
		caddr_t	cbuf;
		u_int	count;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct in_addr in;

	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if ((fp->f_flag & FSOCKET) == 0) {
		u.u_error = ENOTSOCK;
		return;
	}
	u.u_count = uap->count;
	u.u_segflg = 0;
	if (useracc(uap->cbuf, uap->count, B_WRITE) == 0 ||
	    uap->ain && copyin((caddr_t)uap->ain, (caddr_t)&in, sizeof (in))) {
		u.u_error = EFAULT;
		return;
	}
	receive(fp->f_socket, uap->ain ? &in : 0);
}
