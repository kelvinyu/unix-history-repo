/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)search.h	5.1 (Berkeley) %G%
 */

/* Backward compatibility to hsearch interface */
typedef struct entry { 
	char	*key; 
	char	*data; 
} ENTRY;

typedef enum { FIND, ENTER } ACTION;
