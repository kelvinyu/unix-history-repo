#ifndef lint
static	char *sccsid = "@(#)cmd1.c	1.3 83/07/20";
#endif

#include "defs.h"

struct ww *doopen();

dowindow()
{
	int col, row, xcol, xrow;
	int id;

	if ((id = findid()) < 0) {
		wwputs("Too many windows.  ", cmdwin);
		return;
	}
	wwputs("Upper left corner: ", cmdwin);
	col = 0;
	row = 1;
	for (;;) {
		wwsetcursor(row, col);
		while (bpeekc() < 0)
			bread();
		switch (getpos(&row, &col, 0, 0)) {
		case -1:
			WBoxActive = 0;
			wwputs("\r\nCancelled.  ", cmdwin);
			return;
		case 1:
			break;
		case 0:
			continue;
		}
		break;
	}
	wwputs("\r\nLower right corner: ", cmdwin);
	xcol = col + 1;
	xrow = row + 1;
	for (;;) {
		Wbox(col, row, xcol - col + 1, xrow - row + 1);
		wwsetcursor(xrow, xcol);
		wwflush();
		while (bpeekc() < 0)
			bread();
		switch (getpos(&xrow, &xcol, row + 1, col + 1)) {
		case -1:
			WBoxActive = 0;
			wwputs("\r\nCancelled.  ", cmdwin);
			return;
		case 1:
			break;
		case 0:
			continue;
		}
		break;
	}
	WBoxActive = 0;
	wwputs("\r\n", cmdwin);
	wwsetcursor(WCurRow(cmdwin->ww_win), WCurCol(cmdwin->ww_win));
	if (doopen(id, xrow-row+1, xcol-col+1, row, col) == 0)
		wwputs("Can't open window.  ", cmdwin);
}

findid()
{
	register id;
	char ids[10];
	register struct ww *w;
#define NWINDOW 9

	for (id = 1; id <= NWINDOW; id++)
		ids[id] = 0;
	for (w = wwhead; w; w = w->ww_next) {
		if (w->ww_ident < 1 || w->ww_ident > NWINDOW)
			continue;
		ids[w->ww_ident]++;
	}
	for (id = 1; id <= NWINDOW && ids[id]; id++)
		;
	return id < 10 ? id : -1;
}

getpos(row, col, minrow, mincol)
register int *row, *col, minrow, mincol;
{
	static int scount = 0;
	int count;
	char c;

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
			if ((*col += count) >= WCols)
				*col = WCols - 1;
			break;
		case 'L':
			*col = WCols - 1;
			break;
		case 'j':
			if ((*row += count) >= WRows)
				*row = WRows - 1;
			break;
		case 'J':
			*row = WRows - 1;
			break;
		case 'k':
			if ((*row -= count) < minrow)
				*row = minrow;
			break;
		case 'K':
			*row = minrow;
			break;
		case CTRL([):
			return -1;
		case '\r':
			return 1;
		default:
			wwputs("\r\nType [hjklHJKL] to move, return to enter position, escape to cancel.", cmdwin);
			Ding();
		}
	}
	return 0;
}

struct ww *
doopen(id, nrow, ncol, row, col)
int id, nrow, ncol, row, col;
{
	register struct ww *w;

	if (id < 0 && (id = findid()) < 0)
		return 0;
	if ((w = wwopen(WW_PTY, id, nrow, ncol, row, col)) == 0)
		return 0;
	wwframe(w);
	labelwin(w, 0);
	/*
	reframe();
	*/
	wwflush();
	switch (wwfork(w)) {
	case -1:
		wwclose(w);
		return 0;
	case 0:
		execl("/bin/csh", "csh", 0);
		perror("exec(csh)");
		exit(1);
	}
	if (selwin == 0)
		setselwin(w);
	else
		wwsetcurwin(cmdwin);
	return w;
}
