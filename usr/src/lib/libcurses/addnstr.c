/*
 * Copyright (c) 1993 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)addnstr.c	5.1 (Berkeley) %G%";
#endif	/* not lint */

#include <curses.h>
#include <string.h>

/*
 * waddnstr --
 *	Add a string (at most n characters) to the given window
 *	starting at (_cury, _curx).  If n is negative, add the
 *	entire string.
 */
int
waddnstr(win, s, n)
	WINDOW *win;
	const char *s;
	int n;
{
	size_t len;

	len = strlen(s);

	if (n > 0 && n < len)
		len = n;
	return (waddbytes(win, s, len));
}
