#ifndef lint
static	char *sccsid = "@(#)wwclose.c	3.8 83/12/06";
#endif

#include "ww.h"
#include <signal.h>

wwclose(w)
register struct ww *w;
{
	wwindex[w->ww_index] = 0;
	(void) close(w->ww_pty);
	wwfree((char **)w->ww_win, w->ww_w.t);
	wwfree((char **)w->ww_buf, w->ww_w.t);
	if (w->ww_fmap != 0)
		wwfree((char **)w->ww_fmap, w->ww_w.t);
	free((char *)(w->ww_nvis + w->ww_w.t));
	free((char *)w);
}
