#ifndef lint
static	char *sccsid = "@(#)cmd1.c	3.17 83/12/01";
#endif

#include "defs.h"

c_window()
{
	int col, row, xcol, xrow;
	int id;

	if ((id = findid()) < 0)
		return;
	if (!terse)
		(void) wwputs("Upper left corner: ", cmdwin);
	col = 0;
	row = 1;
	wwadd(boxwin, framewin->ww_back);
	for (;;) {
		wwbox(boxwin, row - 1, col - 1, 3, 3);
		wwsetcursor(row, col);
		while (bpeekc() < 0)
			bread();
		switch (getpos(&row, &col, 1, 0, wwnrow - 1, wwncol - 1)) {
		case 3:
			wwunbox(boxwin);
			wwdelete(boxwin);
			return;
		case 2:
			wwunbox(boxwin);
			break;
		case 1:
			wwunbox(boxwin);
		case 0:
			continue;
		}
		break;
	}
	if (!terse)
		(void) wwputs("\r\nLower right corner: ", cmdwin);
	xcol = col;
	xrow = row;
	for (;;) {
		wwbox(boxwin, row - 1, col - 1,
			xrow - row + 3, xcol - col + 3);
		wwsetcursor(xrow, xcol);
		wwflush();
		while (bpeekc() < 0)
			bread();
		switch (getpos(&xrow, &xcol, row, col, wwnrow - 1, wwncol - 1))
		{
		case 3:
			wwunbox(boxwin);
			wwdelete(boxwin);
			return;
		case 2:
			wwunbox(boxwin);
			break;
		case 1:
			wwunbox(boxwin);
		case 0:
			continue;
		}
		break;
	}
	wwdelete(boxwin);
	if (!terse)
		(void) wwputs("\r\n", cmdwin);
	wwcurtowin(cmdwin);
	(void) openwin(id, row, col, xrow-row+1, xcol-col+1, nbufline,
		(char *) 0);
}

findid()
{
	register i;

	for (i = 0; i < NWINDOW && window[i] != 0; i++)
		;
	if (i >= NWINDOW) {
		error("Too many windows.");
		return -1;
	}
	return i;
}

getpos(row, col, minrow, mincol, maxrow, maxcol)
register int *row, *col;
int minrow, mincol;
int maxrow, maxcol;
{
	static int scount = 0;
	int count;
	char c;
	int oldrow = *row, oldcol = *col;

	while ((c = bgetc()) >= 0) {
		switch (c) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			scount = scount * 10 + c - '0';
			continue;
		}
		count = scount ? scount : 1;
		scount = 0;
		switch (c) {
		case 'h':
			if ((*col -= count) < mincol)
				*col = mincol;
			break;
		case 'H':
			*col = mincol;
			break;
		case 'l':
			if ((*col += count) > maxcol)
				*col = maxcol;
			break;
		case 'L':
			*col = maxcol;
			break;
		case 'j':
			if ((*row += count) > maxrow)
				*row = maxrow;
			break;
		case 'J':
			*row = maxrow;
			break;
		case 'k':
			if ((*row -= count) < minrow)
				*row = minrow;
			break;
		case 'K':
			*row = minrow;
			break;
		case CTRL([):
			if (!terse)
				(void) wwputs("\r\nCancelled.  ", cmdwin);
			return 3;
		case '\r':
			return 2;
		default:
			if (!terse)
				(void) wwputs("\r\nType [hjklHJKL] to move, return to enter position, escape to cancel.", cmdwin);
			wwbell();
		}
	}
	return oldrow != *row || oldcol != *col;
}

struct ww *
openwin(id, row, col, nrow, ncol, nline, label)
int id, nrow, ncol, row, col;
char *label;
{
	register struct ww *w;

	if (id < 0 && (id = findid()) < 0)
		return 0;
	if (row + nrow <= 0 || row > wwnrow - 1
	    || col + ncol <= 0 || col > wwncol - 1) {
		error("Illegal window position.");
		return 0;
	}
	if ((w = wwopen(WWO_PTY, nrow, ncol, row, col, nline)) == 0) {
		error("%s.", wwerror());
		return 0;
	}
	w->ww_id = id;
	window[id] = w;
	w->ww_hasframe = 1;
	w->ww_altpos.r = 1;
	w->ww_altpos.c = 0;
	if (label != 0 && setlabel(w, label) < 0)
		error("No memory for label.");
	wwcursor(w, 1);
	wwadd(w, framewin);
	selwin = w;
	reframe();
	wwupdate();
	wwflush();
	if (wwspawn(w, shell, shellname, (char *)0) < 0) {
		c_close(w);
		error("%s: %s.", shell, wwerror());
		return 0;
	}
	return w;
}
