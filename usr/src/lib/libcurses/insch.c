/*
 * Copyright (c) 1981, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)insch.c	8.2 (Berkeley) %G%";
#endif	/* not lint */

#include <string.h>

#include "curses.h"

/*
 * winsch --
 *	Do an insert-char on the line, leaving (cury, curx) unchanged.
 */
int
winsch(win, ch)
	register WINDOW *win;
	int ch;
{

	register __LDATA *end, *temp1, *temp2;

	end = &win->lines[win->cury]->line[win->curx];
	temp1 = &win->lines[win->cury]->line[win->maxx - 1];
	temp2 = temp1 - 1;
	while (temp1 > end) {
		(void)memcpy(temp1, temp2, sizeof(__LDATA));
		temp1--, temp2--;
	}
	temp1->ch = ch;
	temp1->attr &= ~__STANDOUT;
	__touchline(win, win->cury, win->curx, win->maxx - 1, 0);
	if (win->cury == LINES - 1 && 
	    (win->lines[LINES - 1]->line[COLS - 1].ch != ' ' ||
	    win->lines[LINES -1]->line[COLS - 1].attr != 0))
		if (win->flags & __SCROLLOK) {
			wrefresh(win);
			scroll(win);
			win->cury--;
		} else
			return (ERR);
	return (OK);
}
