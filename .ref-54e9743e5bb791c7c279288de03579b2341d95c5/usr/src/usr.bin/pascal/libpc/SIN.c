/*-
 * Copyright (c) 1982, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)SIN.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include <math.h>
extern int errno;

double
SIN(value)
	double	value;
{
	double result;

	errno = 0;
	result = sin(value);
	if (errno != 0) {
		ERROR("Cannot compute sin(%e)\n", value);
	}
	return result;
}
