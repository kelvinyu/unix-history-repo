/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getpagesize.c	5.1 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/sysctl.h>

#if __STDC__
int
getpagesize(void)
#else
int
getpagesize()
#endif
{
	int mib[2], size, value;

	mib[0] = CTL_HW;
	mib[1] = HW_PAGESIZE;
	size = sizeof value;
	if (sysctl(mib, 2, &value, &size, NULL, 0) == -1)
		return (-1);
	return (value);
}
