#ifndef lint
static	char *sccsid = "@(#)wwputs.c	1.4 83/07/19";
#endif

#include "ww.h"

wwputs(s, w)
register char *s;
struct ww *w;
{
	register char *p = s;

	while (*p++)
		;
	wwwrite(w, s, p - s - 1);
}
