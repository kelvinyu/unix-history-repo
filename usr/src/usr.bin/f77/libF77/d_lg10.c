/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)d_lg10.c	5.2	%G%
 */

double d_lg10(x)
double *x;
{
double log();

return( log10(*x) );
}
