/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley. Modified by Ralph Campbell for mips.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)mips.h	8.1 (Berkeley) %G%
 *
 * From: @(#)sparc.h	5.1 (Berkeley) 7/8/92
 */

/*
 * offset (in bytes) of the code from the entry address of a routine.
 * (see asgnsamples for use and explanation.)
 */
#define OFFSET_OF_CODE	0
#define	UNITS_TO_CODE	(OFFSET_OF_CODE / sizeof(UNIT))

enum opermodes { dummy };
typedef enum opermodes	operandenum;
