/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)isatty.c	5.6 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <termios.h>
#include <unistd.h>

int
isatty(fd)
	int fd;
{
	struct termios t;

	return(tcgetattr(fd, &t) != -1);
}
