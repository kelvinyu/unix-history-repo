/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)putchar.c	5.6 (Berkeley) %G%";
#endif	/* not lint */

#include <curses.h>

void
__cputchar(ch)
	int ch;
{

#ifdef DEBUG
	__CTRACE("__cputchar: %s\n", unctrl(ch));
#endif
	(void)putchar(ch);
}
