/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)symtab.h	8.1 (Berkeley) %G%
 */

/*
 * Public definitions for symbol table.
 */

SYMTAB *symtab;

SYMTAB *st_creat();		/* create a symbol table */
int st_destroy();		/* destroy a symbol table, i.e. free storage */
SYM *st_insert();		/* insert a symbol */
SYM *st_lookup();		/* lookup a symbol */
int dumpvars();			/* dump the symbols of a function */
int print_alias();		/* print out currently active aliases */
int enter_alias();		/* create a new name for a command */
SYM *findtype();		/* search symbol table for a type name */
