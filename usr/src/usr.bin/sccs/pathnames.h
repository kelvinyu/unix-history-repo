/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)pathnames.h	5.2 (Berkeley) %G%
 */

#include <paths.h>

#define	PROGPATH(name)	"/usr/local/name"
#undef _PATH_TMP
#define	_PATH_TMP	"/tmp/sccsXXXXX"
