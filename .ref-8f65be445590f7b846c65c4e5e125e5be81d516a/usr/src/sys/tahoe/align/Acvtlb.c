/*-
 * Copyright (c) 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Computer Consoles Inc.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)Acvtlb.c	7.1 (Berkeley) %G%
 */

#include "align.h"
cvtlb(infop) 	process_info *infop;
/*
/*	Convert long to byte, checks overflow
/*
/****************************************/
{
	register long result;

	result = operand(infop,0)->data;
	if (result < 0 )  negative_1 ; else negative_0;
	if (result == 0 )  zero_1 ; else zero_0;
	carry_1; 
	if (result > 0x7f || result <= -0x80) overflow_1;
	else overflow_0;
	write_back (infop, result, operand(infop,1) );
}
