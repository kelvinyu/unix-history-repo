/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)char.h	3.8 (Berkeley) %G%
 */

/*
 * Macros and things to deal with control characters.
 *
 * Unctrl() is just like the standard function, except we don't want
 * to include curses.
 * Isctrl() returns true for all characters less than space and
 * greater than or equal to delete.
 * Isprt() is tab and all characters not isctrl().  It's used
 * by wwwrite().
 * Isunctrl() includes all characters that should be expanded
 * using unctrl() by wwwrite() if ww_unctrl is set.
 */

extern char *_unctrl[];
extern char _cmap[];
#define ctrl(c)		(c & 0x1f)
#define unctrl(c)	(_unctrl[(unsigned char) (c)])
#define _C		0x01
#define _P		0x02
#define _U		0x04
#define isctrl(c)	(_cmap[(unsigned char) (c)] & _C)
#define isprt(c)	(_cmap[(unsigned char) (c)] & _P)
#define isunctrl(c)	(_cmap[(unsigned char) (c)] & _U)
