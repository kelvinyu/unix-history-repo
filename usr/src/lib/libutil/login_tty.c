/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)login_tty.c	1.2 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/ioctl.h>

login_tty(fd)
	int fd;
{
	(void) setsid();
	(void) ioctl(fd, TIOCSCTTY, (char *)NULL);
	(void) close(0); 
	(void) close(1); 
	(void) close(2);
	(void) dup2(fd, 0);
	(void) dup2(fd, 1);
	(void) dup2(fd, 2);
	(void) close(fd);
}
