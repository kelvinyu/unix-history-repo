/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.
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
 *	@(#)kern_descrip.c	7.12 (Berkeley) %G%
 */

#include "param.h"
#include "systm.h"
#include "syscontext.h"
#include "kernel.h"
#include "vnode.h"
#include "proc.h"
#include "file.h"
#include "socket.h"
#include "socketvar.h"
#include "stat.h"

#include "ioctl.h"

/*
 * Descriptor management.
 */

/*
 * System calls on descriptors.
 */
/* ARGSUSED */
getdtablesize(p, uap, retval)
	struct proc *p;
	struct args *uap;
	int *retval;
{

	*retval = NOFILE;
	RETURN (0);
}

/*
 * Duplicate a file descriptor.
 */
/* ARGSUSED */
dup(p, uap, retval)
	struct proc *p;
	struct args {
		int	i;
	} *uap;
	int *retval;
{
	struct file *fp;
	int fd, error;

	/*
	 * XXX Compatibility
	 */
	if (uap->i &~ 077) { uap->i &= 077; RETURN (dup2(p, uap, retval)); }

	if ((unsigned)uap->i >= NOFILE || (fp = u.u_ofile[uap->i]) == NULL)
		RETURN (EBADF);
	if (error = ufalloc(0, &fd))
		RETURN (error);
	u.u_ofile[fd] = fp;
	u.u_pofile[fd] = u.u_pofile[uap->i] &~ UF_EXCLOSE;
	fp->f_count++;
	if (fd > u.u_lastfile)
		u.u_lastfile = fd;
	*retval = fd;
	RETURN (0);
}

/*
 * Duplicate a file descriptor to a particular value.
 */
/* ARGSUSED */
dup2(p, uap, retval)
	struct proc *p;
	register struct args {
		int	i;
		int	j;
	} *uap;
	int *retval;
{
	register struct file *fp;
	int error;

	if ((unsigned)uap->i >= NOFILE || (fp = u.u_ofile[uap->i]) == NULL)
		RETURN (EBADF);
	if (uap->j < 0 || uap->j >= NOFILE)
		RETURN (EBADF);
	*retval = uap->j;
	if (uap->i == uap->j)
		RETURN (0);
	if (u.u_ofile[uap->j]) {
		if (u.u_pofile[uap->j] & UF_MAPPED)
			munmapfd(uap->j);
		error = closef(u.u_ofile[uap->j]);
	}
	u.u_ofile[uap->j] = fp;
	u.u_pofile[uap->j] = u.u_pofile[uap->i] &~ UF_EXCLOSE;
	fp->f_count++;
	if (uap->j > u.u_lastfile)
		u.u_lastfile = uap->j;
	/*
	 * dup2() must succeed even though the close had an error.
	 */
	error = 0;		/* XXX */
	RETURN (error);
}

/*
 * The file control system call.
 */
/* ARGSUSED */
fcntl(p, uap, retval)
	struct proc *p;
	register struct args {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;
	int *retval;
{
	register struct file *fp;
	register char *pop;
	int i, error;

	if ((unsigned)uap->fdes >= NOFILE ||
	    (fp = u.u_ofile[uap->fdes]) == NULL)
		RETURN (EBADF);
	pop = &u.u_pofile[uap->fdes];
	switch(uap->cmd) {
	case F_DUPFD:
		if (uap->arg < 0 || uap->arg >= NOFILE)
			RETURN (EINVAL);
		if (error = ufalloc(uap->arg, &i))
			RETURN (error);
		u.u_ofile[i] = fp;
		u.u_pofile[i] = *pop &~ UF_EXCLOSE;
		fp->f_count++;
		if (i > u.u_lastfile)
			u.u_lastfile = i;
		*retval = i;
		RETURN (0);

	case F_GETFD:
		*retval = *pop & 1;
		RETURN (0);

	case F_SETFD:
		*pop = (*pop &~ 1) | (uap->arg & 1);
		RETURN (0);

	case F_GETFL:
		*retval = fp->f_flag + FOPEN;
		RETURN (0);

	case F_SETFL:
		fp->f_flag &= FCNTLCANT;
		fp->f_flag |= (uap->arg-FOPEN) &~ FCNTLCANT;
		if (error = fset(fp, FNDELAY, fp->f_flag & FNDELAY))
			RETURN (error);
		if (error = fset(fp, FASYNC, fp->f_flag & FASYNC))
			(void) fset(fp, FNDELAY, 0);
		RETURN (error);

	case F_GETOWN:
		RETURN (fgetown(fp, retval));

	case F_SETOWN:
		RETURN (fsetown(fp, uap->arg));

	default:
		RETURN (EINVAL);
	}
	/* NOTREACHED */
}

fset(fp, bit, value)
	struct file *fp;
	int bit, value;
{

	if (value)
		fp->f_flag |= bit;
	else
		fp->f_flag &= ~bit;
	return (fioctl(fp, (int)(bit == FNDELAY ? FIONBIO : FIOASYNC),
	    (caddr_t)&value));
}

fgetown(fp, valuep)
	struct file *fp;
	int *valuep;
{
	int error;

	switch (fp->f_type) {

	case DTYPE_SOCKET:
		*valuep = ((struct socket *)fp->f_data)->so_pgid;
		return (0);

	default:
		error = fioctl(fp, (int)TIOCGPGRP, (caddr_t)valuep);
		*valuep = -*valuep;
		return (error);
	}
}

fsetown(fp, value)
	struct file *fp;
	int value;
{

	if (fp->f_type == DTYPE_SOCKET) {
		((struct socket *)fp->f_data)->so_pgid = value;
		return (0);
	}
	if (value > 0) {
		struct proc *p = pfind(value);
		if (p == 0)
			return (ESRCH);
		value = p->p_pgrp->pg_id;
	} else
		value = -value;
	return (fioctl(fp, (int)TIOCSPGRP, (caddr_t)&value));
}

fioctl(fp, cmd, value)
	struct file *fp;
	int cmd;
	caddr_t value;
{

	return ((*fp->f_ops->fo_ioctl)(fp, cmd, value));
}

/*
 * Close a file descriptor.
 */
/* ARGSUSED */
close(p, uap, retval)
	struct proc *p;
	struct args {
		int	fdes;
	} *uap;
	int *retval;
{
	register struct file *fp;
	register u_char *pf;

	if ((unsigned)uap->fdes >= NOFILE ||
	    (fp = u.u_ofile[uap->fdes]) == NULL)
		RETURN (EBADF);
	pf = (u_char *)&u.u_pofile[uap->fdes];
	if (*pf & UF_MAPPED)
		munmapfd(uap->fdes);
	u.u_ofile[uap->fdes] = NULL;
	while (u.u_lastfile >= 0 && u.u_ofile[u.u_lastfile] == NULL)
		u.u_lastfile--;
	*pf = 0;
	RETURN (closef(fp));
}

/*
 * Return status information about a file descriptor.
 */
/* ARGSUSED */
fstat(p, uap, retval)
	struct proc *p;
	register struct args {
		int	fdes;
		struct	stat *sb;
	} *uap;
	int *retval;
{
	register struct file *fp;
	struct stat ub;
	int error;

	if ((unsigned)uap->fdes >= NOFILE ||
	    (fp = u.u_ofile[uap->fdes]) == NULL)
		RETURN (EBADF);
	switch (fp->f_type) {

	case DTYPE_VNODE:
		error = vn_stat((struct vnode *)fp->f_data, &ub);
		break;

	case DTYPE_SOCKET:
		error = soo_stat((struct socket *)fp->f_data, &ub);
		break;

	default:
		panic("fstat");
		/*NOTREACHED*/
	}
	if (error == 0)
		error = copyout((caddr_t)&ub, (caddr_t)uap->sb, sizeof (ub));
	RETURN (error);
}

/*
 * Allocate a user file descriptor.
 */
ufalloc(want, result)
	register int want;
	int *result;
{

	for (; want < NOFILE; want++) {
		if (u.u_ofile[want] == NULL) {
			u.u_pofile[want] = 0;
			if (want > u.u_lastfile)
				u.u_lastfile = want;
			*result = want;
			return (0);
		}
	}
	return (EMFILE);
}

/*
 * Check to see if any user file descriptors are available.
 */
ufavail()
{
	register int i, avail = 0;

	for (i = 0; i < NOFILE; i++)
		if (u.u_ofile[i] == NULL)
			avail++;
	return (avail);
}

struct	file *lastf;
/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 */
falloc(resultfp, resultfd)
	struct file **resultfp;
	int *resultfd;
{
	register struct file *fp;
	int error, i;

	if (error = ufalloc(0, &i))
		return (error);
	if (lastf == 0)
		lastf = file;
	for (fp = lastf; fp < fileNFILE; fp++)
		if (fp->f_count == 0)
			goto slot;
	for (fp = file; fp < lastf; fp++)
		if (fp->f_count == 0)
			goto slot;
	tablefull("file");
	return (ENFILE);
slot:
	u.u_ofile[i] = fp;
	fp->f_count = 1;
	fp->f_data = 0;
	fp->f_offset = 0;
	fp->f_cred = u.u_cred;
	crhold(fp->f_cred);
	lastf = fp + 1;
	if (resultfp)
		*resultfp = fp;
	if (resultfd)
		*resultfd = i;
	return (0);
}

/*
 * Internal form of close.
 * Decrement reference count on file structure.
 */
closef(fp)
	register struct file *fp;
{
	int error;

	if (fp == NULL)
		return (0);
	if (fp->f_count > 1) {
		fp->f_count--;
		return (0);
	}
	if (fp->f_count < 1)
		panic("closef: count < 1");
	error = (*fp->f_ops->fo_close)(fp);
	crfree(fp->f_cred);
	fp->f_count = 0;
	return (error);
}

/*
 * Apply an advisory lock on a file descriptor.
 */
/* ARGSUSED */
flock(p, uap, retval)
	struct proc *p;
	register struct args {
		int	fdes;
		int	how;
	} *uap;
	int *retval;
{
	register struct file *fp;

	if ((unsigned)uap->fdes >= NOFILE ||
	    (fp = u.u_ofile[uap->fdes]) == NULL)
		RETURN (EBADF);
	if (fp->f_type != DTYPE_VNODE)
		RETURN (EOPNOTSUPP);
	if (uap->how & LOCK_UN) {
		vn_unlock(fp, FSHLOCK|FEXLOCK);
		RETURN (0);
	}
	if ((uap->how & (LOCK_SH | LOCK_EX)) == 0)
		RETURN (0);				/* error? */
	if (uap->how & LOCK_EX)
		uap->how &= ~LOCK_SH;
	/* avoid work... */
	if ((fp->f_flag & FEXLOCK) && (uap->how & LOCK_EX) ||
	    (fp->f_flag & FSHLOCK) && (uap->how & LOCK_SH))
		RETURN (0);
	RETURN (vn_lock(fp, uap->how));
}

/*
 * File Descriptor pseudo-device driver (/dev/fd/).
 *
 * Fred Blonder - U of Maryland	11-Sep-1984
 *
 * Opening minor device N dup()s the file (if any) connected to file
 * descriptor N belonging to the calling process.  Note that this driver
 * consists of only the ``open()'' routine, because all subsequent
 * references to this file will be direct to the other driver.
 */
/* ARGSUSED */
fdopen(dev, mode, type)
	dev_t dev;
	int mode, type;
{
	struct file *fp, *wfp;
	int indx, dfd;

	/*
	 * XXX
	 * Horrid kludge: u.u_r.r_val1 contains the value of the new file
	 * descriptor, which was set before the call to vn_open() by copen()
	 * in vfs_syscalls.c.
	 */
	indx = u.u_r.r_val1;
	fp = u.u_ofile[indx];

	/*
	 * File system device minor number is the to-be-dup'd fd number.
	 * If it is greater than the allowed number of file descriptors,
	 * or the fd to be dup'd has already been closed, reject.  Note,
 	 * check for new == old is necessary as u_falloc could allocate
	 * an already closed to-be-dup'd descriptor as the new descriptor.
	 */
	dfd = minor(dev);
	if ((u_int)dfd >= NOFILE || (wfp = u.u_ofile[dfd]) == NULL ||
	    fp == wfp)
		return (EBADF);

	/*
	 * Check that the mode the file is being opened for is a subset 
	 * of the mode of the existing descriptor.
	 */
	if ((mode & (FREAD|FWRITE) | wfp->f_flag) != wfp->f_flag)
		return (EACCES);
	u.u_ofile[indx] = wfp;
	u.u_pofile[indx] = u.u_pofile[dfd];
	wfp->f_count++;
	if (indx > u.u_lastfile)
		u.u_lastfile = indx;

	/*
	 * Delete references to this pseudo-device by returning a special
	 * error (EJUSTRETURN) that will cause all resources to be freed,
	 * then detected and cleared by copen().
	 */
	return (EJUSTRETURN);			/* XXX */
}
