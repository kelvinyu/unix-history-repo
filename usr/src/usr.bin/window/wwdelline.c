#ifndef lint
static	char *sccsid = "@(#)wwdelline.c	3.3 83/08/18";
#endif

#include "ww.h"

wwdelline(w, line)
register struct ww *w;
int line;
{
	register i;
	register union ww_char **cpp, **cqq;
	register union ww_char *cp;
	int srow, erow;
	char deleted;
	int visible;

	/*
	 * Scroll first.
	 */
	if ((srow = line - w->ww_scroll) < 0)
		srow = 0;
	if ((erow = w->ww_nline - w->ww_scroll - 1) >= w->ww_w.nr) {
		erow = w->ww_w.nr - 1;
		visible = 0;
	} else
		visible = 1;
	deleted = wwscroll1(w, srow, erow, 1, visible);

	/*
	 * Fix the buffer.
	 * But leave clearing the last line for wwclreol().
	 */
	cpp = &w->ww_buf[line];
	cqq = cpp + 1;
	cp = *cpp;
	for (i = w->ww_nline - line - 1; --i >= 0;)
		*cpp++ = *cqq++;
	*cpp = cp;

	/*
	 * Now clear the last line.
	 */
	if (visible)
		wwclreol1(w, w->ww_nline - 1, 0, deleted);
	else
		for (i = w->ww_w.nc; --i >= 0;)
			cp++->c_w = ' ';
}
