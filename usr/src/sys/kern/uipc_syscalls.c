/*	uipc_syscalls.c	4.22	82/08/08	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/buf.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../net/in.h"
#include "../net/in_systm.h"
#include "../h/descrip.h"

ssocreate()
{

}

ssobind()
{

}

ssolisten()
{

}

ssoaccept()
{

}

ssoconnect()
{

}

ssocreatepair()
{

}

ssosendto()
{

}

ssosend()
{

}

ssorecvfrom()
{

}

ssorecv()
{

}

ssosendm()
{

}

ssorecvm()
{

}

ssoshutdown()
{

}

/*
 * Socket system call interface.
 *
 * These routines interface the socket routines to UNIX,
 * isolating the system interface from the socket-protocol interface.
 *
 * TODO:
 *	SO_INTNOTIFY
 */

static	struct sockproto localproto = { PF_UNIX, 0 };
/*
 * Pipe system call interface.
 */
spipe()
{
	register struct file *rf, *wf;
	struct socket *rso, *wso;
	int r;
	
	u.u_error = socreate(&rso, SOCK_STREAM,
	    &localproto, (struct sockaddr *)0, 0);
	if (u.u_error)
		return;
	u.u_error = socreate(&wso, SOCK_STREAM,
	    &localproto, (struct sockaddr *)0, 0);
	if (u.u_error)
		goto free;
	rf = falloc();
	if (rf == NULL)
		goto free2;
	r = u.u_r.r_val1;
	rf->f_flag = FREAD;
	rf->f_type = DTYPE_SOCKET;
	rf->f_socket = rso;
	wf = falloc();
	if (wf == NULL)
		goto free3;
	wf->f_flag = FWRITE;
	wf->f_type = DTYPE_SOCKET;
	wf->f_socket = wso;
	u.u_r.r_val2 = u.u_r.r_val1;
	u.u_r.r_val1 = r;
	if (piconnect(wso, rso) == 0)
		goto free4;
	return;
free4:
	wf->f_count = 0;
	u.u_ofile[u.u_r.r_val2] = 0;
free3:
	rf->f_count = 0;
	u.u_ofile[r] = 0;
free2:
	wso->so_state |= SS_NOFDREF;
	sofree(wso);
free:
	rso->so_state |= SS_NOFDREF;
	sofree(rso);
}

/*
 * Socket system call interface.  Copy sa arguments
 * set up file descriptor and call internal socket
 * creation routine.
 */
ssocket()
{
	register struct a {
		int	type;
		struct	sockproto *asp;
		struct	sockaddr *asa;
		int	options;
	} *uap = (struct a *)u.u_ap;
	struct sockproto sp;
	struct sockaddr sa;
	struct socket *so;
	register struct file *fp;

	if ((fp = falloc()) == NULL)
		return;
	fp->f_flag = FREAD|FWRITE;
	fp->f_type = DTYPE_SOCKET;
	if (uap->asp && copyin((caddr_t)uap->asp, (caddr_t)&sp, sizeof (sp)) ||
	    uap->asa && copyin((caddr_t)uap->asa, (caddr_t)&sa, sizeof (sa))) {
		u.u_error = EFAULT;
		goto bad;
	}
	u.u_error = socreate(&so, uap->type,
	    uap->asp ? &sp : 0, uap->asa ? &sa : 0, uap->options);
	if (u.u_error)
		goto bad;
	fp->f_socket = so;
	return;
bad:
	u.u_ofile[u.u_r.r_val1] = 0;
	fp->f_count = 0;
}

/*
 * Accept system call interface.
 */
saccept()
{
	register struct a {
		int	fdes;
		struct	sockaddr *asa;
	} *uap = (struct a *)u.u_ap;
	struct sockaddr sa;
	register struct file *fp;
	struct socket *so;
	int s;

	if (uap->asa && useracc((caddr_t)uap->asa, sizeof (sa), B_WRITE)==0) {
		u.u_error = EFAULT;
		return;
	}
	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if (fp->f_type != DTYPE_SOCKET) {
		u.u_error = ENOTSOCK;
		return;
	}
	s = splnet();
	so = fp->f_socket;
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		u.u_error = EINVAL;
		splx(s);
		return;
	}
	if ((so->so_state & SS_NBIO) && so->so_qlen == 0) {
		u.u_error = EWOULDBLOCK;
		splx(s);
		return;
	}
	while (so->so_qlen == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		sleep((caddr_t)&so->so_timeo, PZERO+1);
	}
	if (so->so_error) {
		u.u_error = so->so_error;
		splx(s);
		return;
	}
	if ((so->so_options & SO_NEWFDONCONN) == 0) {
		struct socket *nso = so->so_q;
		(void) soqremque(nso, 1);
		soclose(so, 1);
		fp->f_socket = nso;
		nso->so_q = 0;
		so = nso;
		goto ret;
	}
	if (ufalloc() < 0) {
		splx(s);
		return;
	}
	fp = falloc();
	if (fp == 0) {
		u.u_ofile[u.u_r.r_val1] = 0;
		splx(s);
		return;
	}
	fp->f_type = DTYPE_SOCKET;
	fp->f_flag = FREAD|FWRITE;
	fp->f_socket = so->so_q;
	so->so_q = so->so_q->so_q;
	so->so_qlen--;
ret:
	soaccept(so, &sa);
	if (uap->asa)
		(void) copyout((caddr_t)&sa, (caddr_t)uap->asa, sizeof (sa));
	splx(s);
}

/*
 * Connect socket to foreign peer; system call
 * interface.  Copy sa arguments and call internal routine.
 */
sconnect()
{
	register struct a {
		int	fdes;
		struct	sockaddr *a;
	} *uap = (struct a *)u.u_ap;
	struct sockaddr sa;
	register struct file *fp;
	register struct socket *so;
	int s;

	if (copyin((caddr_t)uap->a, (caddr_t)&sa, sizeof (sa))) {
		u.u_error = EFAULT;
		return;
	}
	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if (fp->f_type != DTYPE_SOCKET) {
		u.u_error = ENOTSOCK;
		return;
	}
	so = fp->f_socket;
	u.u_error = soconnect(so, &sa);
	if (u.u_error)
		return;
	s = splnet();
	if ((so->so_state & SS_NBIO) &&
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
 * Send data on socket.
 */
ssend()
{
	register struct a {
		int	fdes;
		struct	sockaddr *asa;
		caddr_t	cbuf;
		unsigned count;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct sockaddr sa;

	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if (fp->f_type != DTYPE_SOCKET) {
		u.u_error = ENOTSOCK;
		return;
	}
	u.u_base = uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	if (useracc(uap->cbuf, uap->count, B_READ) == 0 ||
	    uap->asa && copyin((caddr_t)uap->asa, (caddr_t)&sa, sizeof (sa))) {
		u.u_error = EFAULT;
		return;
	}
	u.u_error = sosend(fp->f_socket, uap->asa ? &sa : 0);
	u.u_r.r_val1 = uap->count - u.u_count;
}

/*
 * Receive data on socket.
 */
sreceive()
{
	register struct a {
		int	fdes;
		struct	sockaddr *asa;
		caddr_t	cbuf;
		u_int	count;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct sockaddr sa;

	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if (fp->f_type != DTYPE_SOCKET) {
		u.u_error = ENOTSOCK;
		return;
	}
	u.u_base = uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	if (useracc(uap->cbuf, uap->count, B_WRITE) == 0 ||
	    uap->asa && copyin((caddr_t)uap->asa, (caddr_t)&sa, sizeof (sa))) {
		u.u_error = EFAULT;
		return;
	}
	u.u_error = soreceive(fp->f_socket, uap->asa ? &sa : 0);
	if (u.u_error)
		return;
	if (uap->asa)
		(void) copyout((caddr_t)&sa, (caddr_t)uap->asa, sizeof (sa));
	u.u_r.r_val1 = uap->count - u.u_count;
}

/*
 * Get socket address.
 */
ssocketaddr()
{
	register struct a {
		int	fdes;
		struct	sockaddr *asa;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct sockaddr addr;

	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	if (fp->f_type != DTYPE_SOCKET) {
		u.u_error = ENOTSOCK;
		return;
	}
	so = fp->f_socket;
	u.u_error =
		(*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR, 0, (caddr_t)&addr);
	if (u.u_error)
		return;
	if (copyout((caddr_t)&addr, (caddr_t)uap->asa, sizeof (addr)))
		u.u_error = EFAULT;
}
