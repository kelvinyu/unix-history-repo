/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)shell.h	8.2 (Berkeley) %G%
 */

/*
 * The follow should be set to reflect the type of system you have:
 *	JOBS -> 1 if you have Berkeley job control, 0 otherwise.
 *	SYMLINKS -> 1 if your system includes symbolic links, 0 otherwise.
 *	SHORTNAMES -> 1 if your linker cannot handle long names.
 *	define BSD if you are running 4.2 BSD or later.
 *	define SYSV if you are running under System V.
 *	define DEBUG=1 to compile in debugging (set global "debug" to turn on)
 *	define DEBUG=2 to compile in and turn on debugging.
 *
 * When debugging is on, debugging info will be written to $HOME/trace and
 * a quit signal will generate a core dump.
 */


#define JOBS 1
#define SYMLINKS 1
#ifndef BSD
#define BSD 1
#endif
#define DEBUG 1

#ifdef __STDC__
typedef void *pointer;
#ifndef NULL
#define NULL (void *)0
#endif
#else /* not __STDC__ */
typedef char *pointer;
#ifndef NULL
#define NULL 0
#endif
#endif /*  not __STDC__ */
#define STATIC	/* empty */
#define MKINIT	/* empty */

#include <sys/cdefs.h>

extern char nullstr[1];		/* null string */


#ifdef DEBUG
#define TRACE(param)	trace param
#else
#define TRACE(param)
#endif
