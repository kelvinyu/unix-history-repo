/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)xx.c	3.1 (Berkeley) %G%";
#endif /* not lint */

#include "ww.h"
#include "xx.h"
#include "tt.h"

xxinit()
{
	if (ttinit() < 0)
		return -1;
	xxbufsize = tt.tt_nrow * tt.tt_ncol * 2;
	xxbuf = malloc((unsigned) xxbufsize * sizeof *xxbuf);
	if (xxbuf == 0) {
		wwerrno = WWE_NOMEM;
		return -1;
	}
	xxbufp = xxbuf;
	xxbufe = xxbuf + xxbufsize;
	if (tt.tt_ntoken > 0 && xcinit() < 0)
		return -1;
	return 0;
}

xxstart()
{
	(*tt.tt_start)();
	if (tt.tt_ntoken > 0)
		xcstart();
	xxreset();			/* might be a restart */
}

xxend()
{
	if (tt.tt_scroll_top != 0 || tt.tt_scroll_bot != tt.tt_nrow - 1)
		/* tt.tt_setscroll is known to be defined */
		(*tt.tt_setscroll)(0, tt.tt_nrow - 1);
	if (tt.tt_insert)
		(*tt.tt_setinsert)(0);
	if (tt.tt_modes)
		(*tt.tt_setmodes)(0);
	if (tt.tt_scroll_down)
		(*tt.tt_scroll_down)(1);
	(*tt.tt_move)(tt.tt_nrow - 1, 0);
	(*tt.tt_end)();
	ttflush();
}

struct xx *
xxalloc()
{
	register struct xx *xp;

	if (xxbufp > xxbufe)
		abort();
	if ((xp = xx_freelist) == 0)
		/* XXX can't deal with failure */
		xp = (struct xx *) malloc((unsigned) sizeof *xp);
	else
		xx_freelist = xp->link;
	if (xx_head == 0)
		xx_head = xp;
	else
		xx_tail->link = xp;
	xx_tail = xp;
	xp->link = 0;
	return xp;
}

xxfree(xp)
	register struct xx *xp;
{
	xp->link = xx_freelist;
	xx_freelist = xp;
}

xxmove(row, col)
{
	register struct xx *xp = xx_tail;

	if (xp == 0 || xp->cmd != xc_move) {
		xp = xxalloc();
		xp->cmd = xc_move;
	}
	xp->arg0 = row;
	xp->arg1 = col;
}

xxscroll(dir, top, bot)
{
	register struct xx *xp = xx_tail;

	if (xp != 0 && xp->cmd == xc_scroll &&
	    xp->arg1 == top && xp->arg2 == bot &&
	    (xp->arg0 < 0 && dir < 0 || xp->arg0 > 0 && dir > 0)) {
		xp->arg0 += dir;
		return;
	}
	xp = xxalloc();
	xp->cmd = xc_scroll;
	xp->arg0 = dir;
	xp->arg1 = top;
	xp->arg2 = bot;
}

xxinschar(row, col, c)
{
	register struct xx *xp = xx_tail;
	int m = c >> WWC_MSHIFT;

	if (xxbufp >= xxbufe)
		xxflush(0);
	c &= WWC_CMASK;
	if (xp != 0 && xp->cmd == xc_inschar &&
	    xp->arg0 == row && xp->arg1 + xp->arg2 == col && xp->arg3 == m) {
		xp->buf[xp->arg2++] = c;
		xxbufp++;
		return;
	}
	xp = xxalloc();
	xp->cmd = xc_inschar;
	xp->arg0 = row;
	xp->arg1 = col;
	xp->arg2 = 1;
	xp->arg3 = m;
	xp->buf = xxbufp++;
	*xp->buf = c;
}

xxdelchar(row, col)
{
	register struct xx *xp = xx_tail;

	if (xp != 0 && xp->cmd == xc_delchar &&
	    xp->arg0 == row && xp->arg1 == col) {
		xp->arg2++;
		return;
	}
	xp = xxalloc();
	xp->cmd = xc_delchar;
	xp->arg0 = row;
	xp->arg1 = col;
	xp->arg2 = 1;
}

xxclear()
{
	register struct xx *xp;

	xxreset();
	xp = xxalloc();
	xp->cmd = xc_clear;
}

xxclreos(row, col)
{
	register struct xx *xp = xxalloc();

	xp->cmd = xc_clreos;
	xp->arg0 = row;
	xp->arg1 = col;
}

xxclreol(row, col)
{
	register struct xx *xp = xxalloc();

	xp->cmd = xc_clreol;
	xp->arg0 = row;
	xp->arg1 = col;
}

xxwrite(row, col, p, n, m)
	char *p;
{
	register struct xx *xp;

	if (xxbufp + n > xxbufe)
		xxflush(0);
	xp = xxalloc();
	xp->cmd = xc_write;
	xp->arg0 = row;
	xp->arg1 = col;
	xp->arg2 = n;
	xp->arg3 = m;
	xp->buf = xxbufp;
	bcopy(p, xxbufp, n);
	xxbufp += n;
	if (tt.tt_ntoken > 0)
		xcscan(xp->buf, n, xp->buf - xxbuf);
}

xxreset()
{
	register struct xx *xp, *xq;

	for (xp = xx_head; xp != 0; xp = xq) {
		xq = xp->link;
		xxfree(xp);
	}
	xx_tail = xx_head = 0;
	xxbufp = xxbuf;
	if (tt.tt_ntoken > 0)
		xcreset();
}
