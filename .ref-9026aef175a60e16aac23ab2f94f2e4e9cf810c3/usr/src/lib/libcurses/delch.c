/*
 * Copyright (c) 1981, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)delch.c	8.2 (Berkeley) %G%";
#endif	/* not lint */

#include <string.h>

#include "curses.h"

/*
 * wdelch --
 *	Do an insert-char on the line, leaving (cury, curx) unchanged.
 */
int
wdelch(win)
	register WINDOW *win;
{
	register __LDATA *end, *temp1, *temp2;

	end = &win->lines[win->cury]->line[win->maxx - 1];
	temp1 = &win->lines[win->cury]->line[win->curx];
	temp2 = temp1 + 1;
	while (temp1 < end) {
		(void)memcpy(temp1, temp2, sizeof(__LDATA));
		temp1++, temp2++;
	}
	temp1->ch = ' ';
	temp1->attr = 0;
	__touchline(win, win->cury, win->curx, win->maxx - 1, 0);
	return (OK);
}
