/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)STLIM.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include "h00vars.h"

STLIM(limit)

	long	limit;
{
	if (_stcnt >= limit) {
		ERROR("Statement count limit of %D exceeded\n", _stcnt);
		return;
	}
	_stlim = limit;
}
