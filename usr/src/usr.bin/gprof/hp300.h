/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)hp300.h	8.1 (Berkeley) %G%
 */

    /*
     *	offset (in bytes) of the code from the entry address of a routine.
     *	(see asgnsamples for use and explanation.)
     */
#define OFFSET_OF_CODE	0
#define	UNITS_TO_CODE	(OFFSET_OF_CODE / sizeof(UNIT))

enum opermodes { dummy };
typedef enum opermodes	operandenum;
