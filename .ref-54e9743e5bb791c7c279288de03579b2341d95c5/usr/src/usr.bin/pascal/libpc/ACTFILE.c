/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)ACTFILE.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include "h00vars.h"

FILE *
ACTFILE(curfile)

	struct iorec	*curfile;
{
	return curfile->fbuf;
}
