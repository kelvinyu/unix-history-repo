/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)LINO.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include "h00vars.h"

char ELINO[] = "Statement count limit of %D exceeded\n";

LINO()
{
	if (++_stcnt >= _stlim) {
		ERROR(ELINO, _stcnt);
		return;
	}
}
