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
static char sccsid[] = "@(#)printf.c	5.6 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#if __STDC__
printf(char const *fmt, ...)
#else
printf(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	int ret;
	va_list ap;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return (ret);
}
