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
static char sccsid[] = "@(#)sys_term.c	5.4 (Berkeley) %G%";
#endif /* not lint */

#include "telnetd.h"
#include "pathnames.h"

#ifdef	NEWINIT
#include <initreq.h>
#else	/* NEWINIT*/
#include <utmp.h>
struct	utmp wtmp;

# ifndef CRAY
char	wtmpf[]	= "/usr/adm/wtmp";
char	utmpf[] = "/etc/utmp";
# else	/* CRAY */
char	wtmpf[]	= "/etc/wtmp";
# endif	/* CRAY */
#endif	/* NEWINIT */

#define SCPYN(a, b)	(void) strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

#include <sys/tty.h>
#ifdef	t_erase
#undef	t_erase
#undef	t_kill
#undef	t_intrc
#undef	t_quitc
#undef	t_startc
#undef	t_stopc
#undef	t_eofc
#undef	t_brkc
#undef	t_suspc
#undef	t_dsuspc
#undef	t_rprntc
#undef	t_flushc
#undef	t_werasc
#undef	t_lnextc
#endif

#ifndef	USE_TERMIO
struct termbuf {
	struct sgttyb sg;
	struct tchars tc;
	struct ltchars ltc;
	int state;
	int lflags;
} termbuf, termbuf2;
#else	/* USE_TERMIO */
# ifndef EXTPROC
# define EXTPROC 0400
# endif
# ifdef	SYSV_TERMIO
#	define termios termio
# endif
# ifndef TCSETA
# define TCSETA TIOCSETA
# define TCGETA TIOCGETA
# endif /* 4.4BSD */
struct termios termbuf, termbuf2;	/* pty control structure */
#endif	/* USE_TERMIO */

/*
 * init_termbuf()
 * copy_termbuf(cp)
 * set_termbuf()
 *
 * These three routines are used to get and set the "termbuf" structure
 * to and from the kernel.  init_termbuf() gets the current settings.
 * copy_termbuf() hands in a new "termbuf" to write to the kernel, and
 * set_termbuf() writes the structure into the kernel.
 */

init_termbuf()
{
#ifndef	USE_TERMIO
	(void) ioctl(pty, TIOCGETP, (char *)&termbuf.sg);
	(void) ioctl(pty, TIOCGETC, (char *)&termbuf.tc);
	(void) ioctl(pty, TIOCGLTC, (char *)&termbuf.ltc);
# ifdef	TIOCGSTATE
	(void) ioctl(pty, TIOCGSTATE, (char *)&termbuf.state);
# endif
#else
	(void) ioctl(pty, TCGETA, (char *)&termbuf);
#endif
	termbuf2 = termbuf;
}

#if	defined(LINEMODE) && defined(TIOCPKT_IOCTL)
copy_termbuf(cp, len)
char *cp;
int len;
{
	if (len > sizeof(termbuf))
		len = sizeof(termbuf);
	bcopy(cp, (char *)&termbuf, len);
	termbuf2 = termbuf;
}
#endif	/* defined(LINEMODE) && defined(TIOCPKT_IOCTL) */

set_termbuf()
{
	/*
	 * Only make the necessary changes.
	 */
#ifndef	USE_TERMIO
	if (bcmp((char *)&termbuf.sg, (char *)&termbuf2.sg, sizeof(termbuf.sg)))
		(void) ioctl(pty, TIOCSETP, (char *)&termbuf.sg);
	if (bcmp((char *)&termbuf.tc, (char *)&termbuf2.tc, sizeof(termbuf.tc)))
		(void) ioctl(pty, TIOCSETC, (char *)&termbuf.tc);
	if (bcmp((char *)&termbuf.ltc, (char *)&termbuf2.ltc,
							sizeof(termbuf.ltc)))
		(void) ioctl(pty, TIOCSLTC, (char *)&termbuf.ltc);
	if (termbuf.lflags != termbuf2.lflags)
		(void) ioctl(pty, TIOCLSET, (char *)&termbuf.lflags);
#else	/* USE_TERMIO */
	if (bcmp((char *)&termbuf, (char *)&termbuf2, sizeof(termbuf)))
		(void) ioctl(pty, TCSETA, (char *)&termbuf);
# ifdef	CRAY2
	needtermstat = 1;
# endif
#endif	/* USE_TERMIO */
}


/*
 * spcset(func, valp, valpp)
 *
 * This function takes various special characters (func), and
 * sets *valp to the current value of that character, and
 * *valpp to point to where in the "termbuf" structure that
 * value is kept.
 *
 * It returns the SLC_ level of support for this function.
 */

#ifndef	USE_TERMIO
spcset(func, valp, valpp)
int func;
unsigned char *valp;
unsigned char **valpp;
{
	switch(func) {
	case SLC_EOF:
		*valp = termbuf.tc.t_eofc;
		*valpp = (unsigned char *)&termbuf.tc.t_eofc;
		return(SLC_VARIABLE);
	case SLC_EC:
		*valp = termbuf.sg.sg_erase;
		*valpp = (unsigned char *)&termbuf.sg.sg_erase;
		return(SLC_VARIABLE);
	case SLC_EL:
		*valp = termbuf.sg.sg_kill;
		*valpp = (unsigned char *)&termbuf.sg.sg_kill;
		return(SLC_VARIABLE);
	case SLC_IP:
		*valp = termbuf.tc.t_intrc;
		*valpp = (unsigned char *)&termbuf.tc.t_intrc;
		return(SLC_VARIABLE|SLC_FLUSHIN|SLC_FLUSHOUT);
	case SLC_ABORT:
		*valp = termbuf.tc.t_quitc;
		*valpp = (unsigned char *)&termbuf.tc.t_quitc;
		return(SLC_VARIABLE|SLC_FLUSHIN|SLC_FLUSHOUT);
	case SLC_XON:
		*valp = termbuf.tc.t_startc;
		*valpp = (unsigned char *)&termbuf.tc.t_startc;
		return(SLC_VARIABLE);
	case SLC_XOFF:
		*valp = termbuf.tc.t_stopc;
		*valpp = (unsigned char *)&termbuf.tc.t_stopc;
		return(SLC_VARIABLE);
	case SLC_AO:
		*valp = termbuf.ltc.t_flushc;
		*valpp = (unsigned char *)&termbuf.ltc.t_flushc;
		return(SLC_VARIABLE);
	case SLC_SUSP:
		*valp = termbuf.ltc.t_suspc;
		*valpp = (unsigned char *)&termbuf.ltc.t_suspc;
		return(SLC_VARIABLE);
	case SLC_EW:
		*valp = termbuf.ltc.t_werasc;
		*valpp = (unsigned char *)&termbuf.ltc.t_werasc;
		return(SLC_VARIABLE);
	case SLC_RP:
		*valp = termbuf.ltc.t_rprntc;
		*valpp = (unsigned char *)&termbuf.ltc.t_rprntc;
		return(SLC_VARIABLE);
	case SLC_LNEXT:
		*valp = termbuf.ltc.t_lnextc;
		*valpp = (unsigned char *)&termbuf.ltc.t_lnextc;
		return(SLC_VARIABLE);
	case SLC_BRK:
	case SLC_SYNCH:
	case SLC_AYT:
	case SLC_EOR:
		*valp = 0;
		*valpp = 0;
		return(SLC_DEFAULT);
	default:
		*valp = 0;
		*valpp = 0;
		return(SLC_NOSUPPORT);
	}
}

#else	/* USE_TERMIO */

spcset(func, valp, valpp)
int func;
unsigned char *valp;
unsigned char **valpp;
{

#define	setval(a, b)	*valp = termbuf.c_cc[a]; \
			*valpp = &termbuf.c_cc[a]; \
			return(b);
#define	defval(a)	*valp = (a); *valpp = 0; return(SLC_DEFAULT);

	switch(func) {
	case SLC_EOF:
		setval(VEOF, SLC_VARIABLE);
	case SLC_EC:
		setval(VERASE, SLC_VARIABLE);
	case SLC_EL:
		setval(VKILL, SLC_VARIABLE);
	case SLC_IP:
		setval(VINTR, SLC_VARIABLE|SLC_FLUSHIN|SLC_FLUSHOUT);
	case SLC_ABORT:
		setval(VQUIT, SLC_VARIABLE|SLC_FLUSHIN|SLC_FLUSHOUT);
	case SLC_XON:
#ifdef	VSTART
		setval(VSTART, SLC_VARIABLE);
#else
		defval(0x13);
#endif
	case SLC_XOFF:
#ifdef	VSTOP
		setval(VSTOP, SLC_VARIABLE);
#else
		defval(0x11);
#endif
	case SLC_EW:
#ifdef	VWERASE
		setval(VWERASE, SLC_VARIABLE);
#else
		defval(0);
#endif
	case SLC_RP:
#ifdef	VREPRINT
		setval(VREPRINT, SLC_VARIABLE);
#else
		defval(0);
#endif
	case SLC_LNEXT:
#ifdef	VLNEXT
		setval(VLNEXT, SLC_VARIABLE);
#else
		defval(0);
#endif
	case SLC_AO:
#ifdef	VFLUSHO
		setval(VFLUSHO, SLC_VARIABLE|SLC_FLUSHOUT);
#else
		defval(0);
#endif
	case SLC_SUSP:
#ifdef	VSUSP
		setval(VSUSP, SLC_VARIABLE|SLC_FLUSHIN);
#else
		defval(0);
#endif

	case SLC_BRK:
	case SLC_SYNCH:
	case SLC_AYT:
	case SLC_EOR:
		defval(0);

	default:
		*valp = 0;
		*valpp = 0;
		return(SLC_NOSUPPORT);
	}
}
#endif	/* USE_TERMIO */

/*
 * getpty()
 *
 * Allocate a pty.  As a side effect, the external character
 * array "line" contains the name of the slave side.
 *
 * Returns the file descriptor of the opened pty.
 */
char *line = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

getpty()
{
	register int p;
#ifndef CRAY
	register char c, *p1, *p2;
	register int i;

	(void) sprintf(line, "/dev/ptyXX");
	p1 = &line[8];
	p2 = &line[9];

	for (c = 'p'; c <= 's'; c++) {
		struct stat stb;

		*p1 = c;
		*p2 = '0';
		if (stat(line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			*p2 = "0123456789abcdef"[i];
			p = open(line, 2);
			if (p > 0) {
				line[5] = 't';
				return(p);
			}
		}
	}
#else	/* CRAY */
	register int npty;
	extern lowpty, highpty;

	for (npty = lowpty; npty <= highpty; npty++) {
		(void) sprintf(line, "/dev/pty/%03d", npty);
		p = open(line, 2);
		if (p < 0)
			continue;
		(void) sprintf(line, "/dev/ttyp%03d", npty);
		if (access(line, 6) == 0)
			return(p);
		else {
			/* no tty side to pty so skip it */
			(void) close(p);
		}
	}
#endif	/* CRAY */
	return(-1);
}

#ifdef	LINEMODE
/*
 * tty_flowmode()	Find out if flow control is enabled or disabled.
 * tty_linemode()	Find out if linemode (external processing) is enabled.
 * tty_setlinemod(on)	Turn on/off linemode.
 * tty_isecho()		Find out if echoing is turned on.
 * tty_setecho(on)	Enable/disable character echoing.
 * tty_israw()		Find out if terminal is in RAW mode.
 * tty_binaryin(on)	Turn on/off BINARY on input.
 * tty_binaryout(on)	Turn on/off BINARY on output.
 * tty_isediting()	Find out if line editing is enabled.
 * tty_istrapsig()	Find out if signal trapping is enabled.
 * tty_setedit(on)	Turn on/off line editing.
 * tty_setsig(on)	Turn on/off signal trapping.
 * tty_tspeed(val)	Set transmit speed to val.
 * tty_rspeed(val)	Set receive speed to val.
 */

tty_flowmode()
{
#ifndef USE_TERMIO
	return((termbuf.tc.t_startc) > 0 && (termbuf.tc.t_stopc) > 0);
#else
	return(termbuf.c_iflag & IXON ? 1 : 0);
#endif
}

tty_linemode()
{
#ifndef	USE_TERMIO
	return(termbuf.state & TS_EXTPROC);
#else
	return(termbuf.c_lflag & EXTPROC);
#endif
}

tty_setlinemode(on)
int on;
{
#ifdef	TIOCEXT
	(void) ioctl(pty, TIOCEXT, (char *)&on);
#else	/* !TIOCEXT */
#ifdef	EXTPROC
	if (on)
		termbuf.c_lflag |= EXTPROC;
	else
		termbuf.c_lflag &= ~EXTPROC;
#endif
	set_termbuf();
#endif	/* TIOCEXT */
}

tty_isecho()
{
#ifndef USE_TERMIO
	return (termbuf.sg.sg_flags & ECHO);
#else
	return (termbuf.c_lflag & ECHO);
#endif
}
#endif	/* LINEMODE */

tty_setecho(on)
{
#ifndef	USE_TERMIO
	if (on)
		termbuf.sg.sg_flags |= ECHO|CRMOD;
	else
		termbuf.sg.sg_flags &= ~(ECHO|CRMOD);
#else
	if (on)
		termbuf.c_lflag |= ECHO;
	else
		termbuf.c_lflag &= ~ECHO;
#endif
}

#if	defined(LINEMODE) && defined(KLUDGELINEMODE)
tty_israw()
{
#ifndef USE_TERMIO
	return(termbuf.sg.sg_flags & RAW);
#else
	return(!(termbuf.c_lflag & ICANON));
#endif
}
#endif	/* defined(LINEMODE) && defined(KLUDGELINEMODE) */

tty_binaryin(on)
{
#ifndef	USE_TERMIO
	if (on)
		termbuf.lflags |= LPASS8;
	else
		termbuf.lflags &= ~LPASS8;
#else
	if (on) {
		termbuf.c_lflag &= ~ISTRIP;
	} else {
		termbuf.c_lflag |= ISTRIP;
	}
#endif
}

tty_binaryout(on)
{
#ifndef	USE_TERMIO
	if (on)
		termbuf.lflags |= LLITOUT;
	else
		termbuf.lflags &= ~LLITOUT;
#else
	if (on) {
		termbuf.c_cflag &= ~(CSIZE|PARENB);
		termbuf.c_cflag |= CS8;
		termbuf.c_oflag &= ~OPOST;
	} else {
		termbuf.c_cflag &= ~CSIZE;
		termbuf.c_cflag |= CS7|PARENB;
		termbuf.c_oflag |= OPOST;
	}
#endif
}

tty_isbinaryin()
{
#ifndef	USE_TERMIO
	return(termbuf.lflags & LPASS8);
#else
	return(!(termbuf.c_iflag & ISTRIP));
#endif
}

tty_isbinaryout()
{
#ifndef	USE_TERMIO
	return(termbuf.lflags & LLITOUT);
#else
	return(!(termbuf.c_oflag&OPOST));
#endif
}

#ifdef	LINEMODE
tty_isediting()
{
#ifndef USE_TERMIO
	return(!(termbuf.sg.sg_flags & (CBREAK|RAW)));
#else
	return(termbuf.c_lflag & ICANON);
#endif
}

tty_istrapsig()
{
#ifndef USE_TERMIO
	return(!(termbuf.sg.sg_flags&RAW));
#else
	return(termbuf.c_lflag & ISIG);
#endif
}

tty_setedit(on)
int on;
{
#ifndef USE_TERMIO
	if (on)
		termbuf.sg.sg_flags &= ~CBREAK;
	else
		termbuf.sg.sg_flags |= CBREAK;
#else
	if (on)
		termbuf.c_lflag |= ICANON;
	else
		termbuf.c_lflag &= ~ICANON;
#endif
}

tty_setsig(on)
int on;
{
#ifndef	USE_TERMIO
	if (on)
		;
#else
	if (on)
		termbuf.c_lflag |= ISIG;
	else
		termbuf.c_lflag &= ~ISIG;
#endif
}
#endif	/* LINEMODE */

/*
 * A table of available terminal speeds
 */
struct termspeeds {
	int	speed;
	int	value;
} termspeeds[] = {
	{ 0,     B0 },    { 50,    B50 },   { 75,    B75 },
	{ 110,   B110 },  { 134,   B134 },  { 150,   B150 },
	{ 200,   B200 },  { 300,   B300 },  { 600,   B600 },
	{ 1200,  B1200 }, { 1800,  B1800 }, { 2400,  B2400 },
	{ 4800,  B4800 }, { 9600,  B9600 }, { 19200, B9600 },
	{ 38400, B9600 }, { -1,    B9600 }
};

tty_tspeed(val)
{
	register struct termspeeds *tp;

	for (tp = termspeeds; (tp->speed != -1) && (val > tp->speed); tp++)
		;
#ifndef	USE_TERMIO
	termbuf.sg.sg_ospeed = tp->value;
#else
# ifdef	SYSV_TERMIO
	termbuf.c_cflag &= ~CBAUD;
	termbuf.c_cflag |= tp->value;
# else
	termbuf.c_ospeed = tp->value;
# endif
#endif
}

tty_rspeed(val)
{
	register struct termspeeds *tp;

	for (tp = termspeeds; (tp->speed != -1) && (val > tp->speed); tp++)
		;
#ifndef	USE_TERMIO
	termbuf.sg.sg_ispeed = tp->value;
#else
# ifdef	SYSV_TERMIO
	termbuf.c_cflag &= ~CBAUD;
	termbuf.c_cflag |= tp->value;
# else
	termbuf.c_ispeed = tp->value;
# endif
#endif
}

#ifdef	CRAY2
tty_isnewmap()
{
	return((termbuf.c_oflag & OPOST) && (termbuf.c_oflag & ONLCR) &&
			!(termbuf.c_oflag & ONLRET));
}
#endif

#ifdef	CRAY
# ifndef NEWINIT
extern	struct utmp wtmp;
extern char wtmpf[];
# else	/* NEWINIT */
int	gotalarm;
nologinproc()
{
	gotalarm++;
}
# endif	/* NEWINIT */
#endif /* CRAY */

/*
 * getptyslave()
 *
 * Open the slave side of the pty, and do any initialization
 * that is necessary.  The return value is a file descriptor
 * for the slave side.
 */
getptyslave()
{
	register int t = -1;

#ifndef	CRAY
	/*
	 * Disassociate self from control terminal and open ttyp side.
	 * Set important flags on ttyp and ptyp.
	 */
	t = open(_PATH_TTY, O_RDWR);
	if (t >= 0) {
		(void) ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	}

	t = open(line, O_RDWR);
	if (t < 0)
		fatalperror(net, line);
	if (fchmod(t, 0))
		fatalperror(net, line);
	(void) signal(SIGHUP, SIG_IGN);
	vhangup();
	(void) signal(SIGHUP, SIG_DFL);
	t = open(line, O_RDWR);
	if (t < 0)
		fatalperror(net, line);

	init_termbuf();
#ifndef	USE_TERMIO
	termbuf.sg.sg_flags |= CRMOD|ANYP|ECHO;
	termbuf.sg.sg_ospeed = termbuf.sg.sg_ispeed = B9600;
#else
	termbuf.c_lflag |= ECHO;
	termbuf.c_oflag |= ONLCR|OXTABS;
	termbuf.c_iflag |= ICRNL;
	termbuf.c_iflag &= ~IXOFF;
# ifdef	SYSV_TERMIO
	termbuf.sg.sg_ospeed = termbuf.sg.sg_ispeed = B9600;
# else SYSV_TERMIO
	termbuf.c_ospeed = termbuf.c_ispeed = B9600;
# endif
#endif
	set_termbuf();
#else	/* CRAY */
	(void) chown(line, 0, 0);
	(void) chmod(line, 0600);
#endif	/* CRAY */
	return(t);
}

#ifdef	NEWINIT
char *gen_id = "fe";
#endif

/*
 * startslave(t, host)
 *
 * Given a file descriptor (t) for a tty, and a hostname, do whatever
 * is necessary to startup the login process on the slave side of the pty.
 */

/* ARGSUSED */
startslave(t, host)
int t;
char *host;
{
	register int i;
	long time();

#ifndef	NEWINIT
# ifdef	CRAY
	utmp_sig_init();
# endif	/* CRAY */

	if ((i = fork()) < 0)
		fatalperror(net, "fork");
	if (i) {
# ifdef	CRAY
		/*
		 * Cray parent will create utmp entry for child and send
		 * signal to child to tell when done.  Child waits for signal
		 * before doing anything important.
		 */
		register int pid = i;

		setpgrp();
		(void) signal(SIGUSR1, func);	/* reset handler to default */
		/*
		 * Create utmp entry for child
		 */
		(void) time(&wtmp.ut_time);
		wtmp.ut_type = LOGIN_PROCESS;
		wtmp.ut_pid = pid;
		SCPYN(wtmp.ut_user, "LOGIN");
		SCPYN(wtmp.ut_host, host);
		SCPYN(wtmp.ut_line, line + sizeof("/dev/") - 1);
		SCPYN(wtmp.ut_id, wtmp.ut_line+3);
		pututline(&wtmp);
		endutent();
		if ((i = open(wtmpf, O_WRONLY|O_APPEND)) >= 0) {
			(void) write(i, (char *)&wtmp, sizeof(struct utmp));
			(void) close(i);
		}
		utmp_sig_notify(pid);
# endif	/* CRAY */
		(void) close(t);
	} else {
		start_login(t, host);
		/*NOTREACHED*/
	}
#else	/* NEWINIT */

	extern char *ptyip;
	struct init_request request;
	int nologinproc();
	register int n;

	/*
	 * Init will start up login process if we ask nicely.  We only wait
	 * for it to start up and begin normal telnet operation.
	 */
	if ((i = open(INIT_FIFO, O_WRONLY)) < 0) {
		char tbuf[128];
		(void) sprintf(tbuf, "Can't open %s\n", INIT_FIFO);
		fatalperror(net, tbuf);
	}
	memset((char *)&request, 0, sizeof(request));
	request.magic = INIT_MAGIC;
	SCPYN(request.gen_id, gen_id);
	SCPYN(request.tty_id, &line[8]);
	SCPYN(request.host, host);
	SCPYN(request.term_type, &terminaltype[5]);
	if (write(i, (char *)&request, sizeof(request)) < 0) {
		char tbuf[128];
		(void) sprintf(tbuf, "Can't write to %s\n", INIT_FIFO);
		fatalperror(net, tbuf);
	}
	(void) close(i);
	(void) signal(SIGALRM, nologinproc);
	for (i = 0; ; i++) {
		alarm(15);
		n = read(pty, ptyip, BUFSIZ);
		if (i == 3 || n >= 0 || !gotalarm)
			break;
		gotalarm = 0;
		(void) write(net, "telnetd: waiting for /etc/init to start login process.\r\n", 56);
	}
	if (n < 0 && gotalarm)
		fatal(net, "/etc/init didn't start login process");
	pcc += n;
	alarm(0);
	(void) signal(SIGALRM, SIG_DFL);

	/*
	 * Set tab expansion the way we like, in case init did something
	 * different.
	 */
	init_termbuf();
	termbuf.c_oflag &= ~TABDLY;
	termbuf.c_oflag |= TAB0;
	set_termbuf();
	return;
#endif	/* NEWINIT */
}

#ifndef	NEWINIT
char	*envinit[3];

/*
 * start_login(t, host)
 *
 * Assuming that we are now running as a child processes, this
 * function will turn us into the login process.
 */

start_login(t, host)
int t;
char *host;
{
	extern char *getenv();
	char **envp;

#ifdef	CRAY
	utmp_sig_wait();
# ifndef TCVHUP
	setpgrp();
# endif
	t = open(line, 2);	/* open ttyp */
	if (t < 0)
		fatalperror(net, line);
# ifdef	TCVHUP
	/*
	 * Hangup anybody else using this ttyp, then reopen it for
	 * ourselves.
	 */
	(void) chown(line, 0, 0);
	(void) chmod(line, 0600);
	(void) signal(SIGHUP, SIG_IGN);
	(void) ioctl(t, TCVHUP, (char *)0);
	(void) signal(SIGHUP, SIG_DFL);
	setpgrp();
	i = open(line, 2);
	if (i < 0)
		fatalperror(net, line);
	(void) close(t);
	t = i;
# endif	/* TCVHUP */
	/*
	 * set ttyp modes as we like them to be
	 */
	init_termbuf();
	termbuf.c_oflag = OPOST|ONLCR;
	termbuf.c_iflag = IGNPAR|ISTRIP|ICRNL|IXON;
	termbuf.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
	termbuf.c_cflag = EXTB|HUPCL|CS8;
	set_termbuf();
#endif	/* CRAY */

	/*
	 * set up standard paths before forking to login
	 */
#if	BSD >43
	if (setsid() < 0)
		fatalperror(net, "setsid");
	if (ioctl(t, TIOCSCTTY, (char *)0) < 0)
		fatalperror(net, "ioctl(sctty)");
#endif
	(void) close(net);
	(void) close(pty);
	(void) dup2(t, 0);
	(void) dup2(t, 1);
	(void) dup2(t, 2);
	(void) close(t);
	envp = envinit;
	*envp++ = terminaltype;
	if (*envp = getenv("TZ"))
		*envp++ -= 3;
#ifdef	CRAY
	else
		*envp++ = "TZ=GMT0";
#endif
	*envp = 0;
	environ = envinit;
	/*
	 * -h : pass on name of host.
	 *		WARNING:  -h is accepted by login if and only if
	 *			getuid() == 0.
	 * -p : don't clobber the environment (so terminal type stays set).
	 */
	execl(_PATH_LOGIN, "login", "-h", host,
#ifndef CRAY
					terminaltype ? "-p" : 0,
#endif
								0);
	syslog(LOG_ERR, "%s: %m\n", _PATH_LOGIN);
	fatalperror(net, _PATH_LOGIN);
	/*NOTREACHED*/
}
#endif	NEWINIT

/*
 * cleanup()
 *
 * This is the routine to call when we are all through, to
 * clean up anything that needs to be cleaned up.
 */
cleanup()
{

#ifndef	CRAY
# if BSD > 43
	char *p;

	p = line + sizeof("/dev/") - 1;
	if (logout(p))
		logwtmp(p, "", "");
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
	*p = 'p';
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
# else
	rmut();
	vhangup();	/* XXX */
# endif
	(void) shutdown(net, 2);
#else	/* CRAY */
# ifndef NEWINIT
	rmut(line);
	(void) shutdown(net, 2);
	kill(0, SIGHUP);
# else	/* NEWINIT */
	(void) shutdown(net, 2);
	sleep(5);
# endif	/* NEWINT */
#endif	/* CRAY */
	exit(1);
}

#if	defined(CRAY) && !defined(NEWINIT)
/*
 * _utmp_sig_rcv
 * utmp_sig_init
 * utmp_sig_wait
 *	These three functions are used to coordinate the handling of
 *	the utmp file between the server and the soon-to-be-login shell.
 *	The server actually creates the utmp structure, the child calls
 *	utmp_sig_wait(), until the server calls utmp_sig_notify() and
 *	signals the future-login shell to proceed.
 */
static int caught=0;		/* NZ when signal intercepted */
static void (*func)();		/* address of previous handler */

void
_utmp_sig_rcv(sig)
int sig;
{
	caught = 1;
	(void) signal(SIGUSR1, func);
}

utmp_sig_init()
{
	/*
	 * register signal handler for UTMP creation
	 */
	if ((int)(func = signal(SIGUSR1, _utmp_sig_rcv)) == -1)
		fatalperror(net, "telnetd/signal");
}

utmp_sig_wait()
{
	/*
	 * Wait for parent to write our utmp entry.
	 */
	sigoff();
	while (caught == 0) {
		pause();	/* wait until we get a signal (sigon) */
		sigoff();	/* turn off signals while we check caught */
	}
	sigon();		/* turn on signals again */
}

utmp_sig_notify(pid)
{
	kill(pid, SIGUSR1);
}
#endif	/* defined(CRAY) && !defined(NEWINIT) */

/*
 * rmut()
 *
 * This is the function called by cleanup() to
 * remove the utmp entry for this person.
 */

#if	!defined(CRAY) && BSD <= 43
rmut()
{
	register f;
	int found = 0;
	struct utmp *u, *utmp;
	int nutmp;
	struct stat statbf;
	char *malloc();
	long time();
	off_t lseek();

	f = open(utmpf, O_RDWR);
	if (f >= 0) {
		(void) fstat(f, &statbf);
		utmp = (struct utmp *)malloc((unsigned)statbf.st_size);
		if (!utmp)
			syslog(LOG_ERR, "utmp malloc failed");
		if (statbf.st_size && utmp) {
			nutmp = read(f, (char *)utmp, (int)statbf.st_size);
			nutmp /= sizeof(struct utmp);
		
			for (u = utmp ; u < &utmp[nutmp] ; u++) {
				if (SCMPN(u->ut_line, line+5) ||
				    u->ut_name[0]==0)
					continue;
				(void) lseek(f, ((long)u)-((long)utmp), L_SET);
				SCPYN(u->ut_name, "");
				SCPYN(u->ut_host, "");
				(void) time(&u->ut_time);
				(void) write(f, (char *)u, sizeof(wtmp));
				found++;
			}
		}
		(void) close(f);
	}
	if (found) {
		f = open(wtmpf, O_WRONLY|O_APPEND);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, line+5);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			(void) time(&wtmp.ut_time);
			(void) write(f, (char *)&wtmp, sizeof(wtmp));
			(void) close(f);
		}
	}
	(void) chmod(line, 0666);
	(void) chown(line, 0, 0);
	line[strlen("/dev/")] = 'p';
	(void) chmod(line, 0666);
	(void) chown(line, 0, 0);
}  /* end of rmut */
#endif	/* CRAY */
