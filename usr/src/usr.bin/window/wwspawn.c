#ifndef lint
static	char *sccsid = "@(#)wwspawn.c	2.1 83/07/30";
#endif

#include "ww.h"

extern int _wwdtablesize;
extern char _wwtermcap[];

wwfork(wp)
register struct ww *wp;
{
	switch (wp->ww_pid = fork()) {
	case -1:
		return -1;
	case 0:
		wp->ww_state = WW_INCHILD;
		wwenviron(wp);
		return 0;
	default:
		wp->ww_state = WW_HASPROC;
		close(wp->ww_tty);
		wp->ww_tty = -1;
		return wp->ww_pid;
	}
}

#define TERM	"TERM=window"
#define TERMCAP	"TERMCAP=WW|window|window package:\
	:cr=^M:nl=^J:bl=^G:\
	:al=\\EL:am:le=^H:bs:cd=\\EJ:ce=\\EK:cl=\\EE:cm=\\EY%%+ %%+ :\
	:co#%d:dc=\\EN:dl=\\EM:do=\\EB:ei=\\EO:ho=\\EH:li#%d:im=\\E@:mi:\
	:nd=\\EC:ta=^I:pt:up=\\EA:"
static char *env[100];
static char buf[1024];
extern char **environ;
extern char _wwkeys[];

wwenviron(wp)
register struct ww *wp;
{
	register i;
	register char **p, **q;
	char **termcap = 0;

	dup2(wp->ww_tty, 0);
	dup2(wp->ww_tty, 1);
	dup2(wp->ww_tty, 2);
	for (i = _wwdtablesize - 1; i > 2; i--)
		close(i);

	i = open("/dev/tty");
	ioctl(i, TIOCNOTTY, 0);
	close(i);
	open(wp->ww_ttyname, 0);

	for (p = environ, q = env; *p; p++, q++) {
		if (strncmp(*p, "TERM=", 5) == 0)
			*q = TERM;
		else if (strncmp(*p, "TERMCAP=", 8) == 0)
			termcap = q;
		else
			*q = *p;
	}
	if (termcap == 0)
		termcap = q++;
	*q = 0;
	*termcap = sprintf(buf, TERMCAP, wp->ww_i.ncol, wp->ww_i.nrow);
	strcat(buf, _wwkeys);
	environ = env;
}
