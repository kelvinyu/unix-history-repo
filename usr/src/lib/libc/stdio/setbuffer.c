/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)setbuffer.c	5.4 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>

void
setbuffer(fp, buf, size)
	register FILE *fp;
	char *buf;
	int size;
{
	(void) setvbuf(fp, buf, buf ? _IONBF : _IOFBF, size);
}

/*
 * set line buffering
 */
setlinebuf(fp)
	FILE *fp;
{
	(void) setvbuf(fp, (char *)NULL, _IOLBF, (size_t)0);
	return (0);	/* ??? */
}
