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
static char sccsid[] = "@(#)vsprintf.c	5.4 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <limits.h>

vsprintf(str, fmt, ap)
	char *str;
	char *fmt;
	_VA_LIST_ ap;
{
	int ret;
	FILE f;

	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = INT_MAX;
	ret = vfprintf(&f, fmt, ap);
	*f._p = 0;
	return (ret);
}
