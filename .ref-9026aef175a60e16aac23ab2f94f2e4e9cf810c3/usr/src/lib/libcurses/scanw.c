/*
 * Copyright (c) 1981, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)scanw.c	8.3 (Berkeley) %G%";
#endif	/* not lint */

/*
 * scanw and friends.
 */

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "curses.h"

/*
 * scanw --
 *	Implement a scanf on the standard screen.
 */
int
#ifdef __STDC__
scanw(const char *fmt, ...)
#else
scanw(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = vwscanw(stdscr, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * wscanw --
 *	Implements a scanf on the given window.
 */
int
#ifdef __STDC__
wscanw(WINDOW *win, const char *fmt, ...)
#else
wscanw(win, fmt, va_alist)
	WINDOW *win;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = vwscanw(win, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * mvscanw, mvwscanw -- 
 *	Implement the mvscanw commands.  Due to the variable number of
 *	arguments, they cannot be macros.  Another sigh....
 */
int
#ifdef __STDC__
mvscanw(register int y, register int x, const char *fmt,...)
#else
mvscanw(y, x, fmt, va_alist)
	register int y, x;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

	if (move(y, x) != OK)
		return (ERR);
#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = vwscanw(stdscr, fmt, ap);
	va_end(ap);
	return (ret);
}

int
#ifdef __STDC__
mvwscanw(register WINDOW * win, register int y, register int x,
    const char *fmt, ...)
#else
mvwscanw(win, y, x, fmt, va_alist)
	register WINDOW *win;
	register int y, x;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
	int ret;

	if (move(y, x) != OK)
		return (ERR);
#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	ret = vwscanw(win, fmt, ap);
	va_end(ap);
	return (ret);
}

/*
 * vwscanw --
 *	This routine actually executes the scanf from the window.
 */
int
vwscanw(win, fmt, ap)
	WINDOW *win;
	const char *fmt;
	va_list ap;
{

	char buf[1024];

	return (wgetstr(win, buf) == OK ?
	    vsscanf(buf, fmt, ap) : ERR);
}
