/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)RELTLE.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include "h00vars.h"

bool
RELTLE(bytecnt, left, right)

	long		bytecnt;
	register long	*left;
	register long	*right;
{
	register int	longcnt;

	longcnt = bytecnt >> 2;
	do	{
		if ((*left++ & ~*right++) != 0) 
			return FALSE;
	} while (--longcnt);
	return TRUE;
}
