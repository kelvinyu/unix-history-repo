#ifndef lint
static	char *sccsid = "@(#)wwgets.c	3.5 84/04/08";
#endif

#include "ww.h"
#include "char.h"

wwgets(buf, n, w)
char *buf;
int n;
register struct ww *w;
{
	register char *p = buf;
	register char c;

	for (;;) {
		wwcurtowin(w);
		while ((c = wwgetc()) < 0)
			wwiomux();
		if (c == wwoldtty.ww_sgttyb.sg_erase) {
			if (p > buf)
				rub(*--p, w);
			else
				wwputc(ctrl(g), w);
		} else if (c == wwoldtty.ww_sgttyb.sg_kill) {
			while (p > buf)
				rub(*--p, w);
		} else if (c == wwoldtty.ww_ltchars.t_werasc) {
			while (--p >= buf && (*p == ' ' || *p == '\t'))
				rub(*p, w);
			while (p >= buf && *p != ' ' && *p != '\t')
				rub(*p--, w);
			p++;
		} else if (c == '\r' || c == '\n') {
			break;
		} else {
			if (p >= buf + n - 1)
				wwputc(ctrl(g), w);
			else
				wwputs(unctrl(*p++ = c), w);
		}
	}
	*p = 0;
}

static
rub(c, w)
struct ww *w;
{
	register i;

	for (i = strlen(unctrl(c)); --i >= 0;)
		wwwrite(w, "\b \b", 3);
}
