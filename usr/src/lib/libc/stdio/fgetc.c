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
static char sccsid[] = "@(#)fgetc.c	5.3 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>

fgetc(fp)
	FILE *fp;
{
	return (__sgetc(fp));
}
