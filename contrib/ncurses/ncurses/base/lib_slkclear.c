/****************************************************************************
 * Copyright (c) 1998 Free Software Foundation, Inc.                        *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/*
 *	lib_slkclear.c
 *	Soft key routines.
 *      Remove soft labels from the screen.
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkclear.c,v 1.4 1999/03/03 23:44:22 juergen Exp $")

int
slk_clear(void)
{
	T((T_CALLED("slk_clear()")));

	if (SP == NULL || SP->_slk == NULL)
		returnCode(ERR);
	SP->_slk->hidden = TRUE;
	/* For simulated SLK's it's looks much more natural to
	   inherit those attributes from the standard screen */
	SP->_slk->win->_bkgd  = stdscr->_bkgd;
	SP->_slk->win->_attrs = stdscr->_attrs;
        if (SP->_slk->win == stdscr) {
          returnCode(OK);
        }
	else {
	  werase(SP->_slk->win);
	  returnCode(wrefresh(SP->_slk->win));
	}
}
