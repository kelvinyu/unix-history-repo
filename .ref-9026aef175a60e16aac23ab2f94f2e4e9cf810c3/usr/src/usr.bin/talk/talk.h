/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)talk.h	8.1 (Berkeley) %G%
 */

#include <curses.h>

extern	int sockt;
extern	int curses_initialized;
extern	int invitation_waiting;

extern	char *current_state;
extern	int current_line;

typedef struct xwin {
	WINDOW	*x_win;
	int	x_nlines;
	int	x_ncols;
	int	x_line;
	int	x_col;
	char	kill;
	char	cerase;
	char	werase;
} xwin_t;

extern	xwin_t my_win;
extern	xwin_t his_win;
extern	WINDOW *line_win;
