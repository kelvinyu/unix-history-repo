/*
 * Copyright (c) 1981, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)overwrite.c	8.2 (Berkeley) %G%";
#endif	/* not lint */

#include <ctype.h>
#include <string.h>

#include "curses.h"

/*
 * overwrite --
 *	Writes win1 on win2 destructively.
 */
int
overwrite(win1, win2)
	register WINDOW *win1, *win2;
{
	register int x, y, endy, endx, starty, startx;

#ifdef DEBUG
	__CTRACE("overwrite: (%0.2o, %0.2o);\n", win1, win2);
#endif
	starty = max(win1->begy, win2->begy);
	startx = max(win1->begx, win2->begx);
	endy = min(win1->maxy + win1->begy, win2->maxy + win2->begx);
	endx = min(win1->maxx + win1->begx, win2->maxx + win2->begx);
	if (starty >= endy || startx >= endx)
		return (OK);
#ifdef DEBUG
	__CTRACE("overwrite: from (%d, %d) to (%d, %d)\n",
	    starty, startx, endy, endx);
#endif
	x = endx - startx;
	for (y = starty; y < endy; y++) {
		(void)memcpy(
		    &win2->lines[y - win2->begy]->line[startx - win2->begx], 
		    &win1->lines[y - win1->begy]->line[startx - win1->begx],
		    x * __LDATASIZE);
		__touchline(win2, y, startx - win2->begx, endx - win2->begx,
		    0);
	}
	return (OK);
}
