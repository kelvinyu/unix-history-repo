#ifndef lint
static	char *sccsid = "@(#)wwgets.c	1.6 83/07/28";
#endif

#include "defs.h"

char *ibufp = ibuf;

bread()
{
	register n;
	register char *p;
	int imask;

	while (ibufc == 0) {
		wwflush();
		imask = 1 << 0;
		while (wwforce(&imask) < 0)
			;
		if ((imask & 1 << 0) == 0)
			continue;
		if (ibufc == 0) {
			p = ibufp = ibuf;
			n = sizeof ibuf;
		} else {
			p = ibufp + ibufc;
			n = (ibuf + sizeof ibuf) - p;
		}
		if ((n = read(0, p, n)) > 0) {
			ibufc += n;
			nreadc += n;
		} else if (n == 0)
			nreadz++;
		else
			nreade++;
		nread++;
	}
}

bgets(buf, n, w)
char *buf;
int n;
register struct ww *w;
{
	register char *p = buf;
	register char c;

	for (;;) {
		wwsetcursor(WCurRow(w->ww_win), WCurCol(w->ww_win));
		while ((c = bgetc()) < 0)
			bread();
		if (c == wwoldtty.ww_sgttyb.sg_erase) {
			if (p > buf)
				rub(*--p, w);
			else
				Ding();
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
				Ding();
			else
				wwputs(unctrl(*p++ = c), w);
		}
	}
	*p = 0;
}

rub(c, w)
struct ww *w;
{
	register i;

	for (i = strlen(unctrl(c)); --i >= 0;)
		wwputs("\b \b", w);
}
