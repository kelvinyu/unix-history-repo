/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)r_log.c	5.1	%G%
 */

double r_log(x)
float *x;
{
double log();
return( log(*x) );
}
