/*-
 * Copyright (c) 1980, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1991 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)tset.c	5.18 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "extern.h"

void	obsolete __P((char *[]));
void	report __P((char *, int, u_int));
void	usage __P((void));

struct termios mode, oldmode;

int	dosetenv;		/* output TERMCAP strings */
int	erasechar;		/* new erase character */
int	intrchar;		/* new interrupt character */
int	isreset;		/* invoked as reset */
int	killchar;		/* new kill character */
int	noinit;			/* don't output initialization string */
int	noset;			/* only report term type */
int	quiet;			/* don't display ctrl key settings */

int	lines, columns;		/* window size */

int
main(argc, argv)
	int argc;
	char *argv[];
{
#ifdef TIOCGWINSZ
	struct winsize win;
#endif
	int ch, csh, usingupper;
	char savech, *p, *t, *tcapbuf, *ttype;

	if (tcgetattr(STDERR_FILENO, &mode) < 0)
		err("standard error: %s", strerror(errno));

	oldmode = mode;
	ospeed = cfgetospeed(&mode);

	if (p = strrchr(*argv, '/'))
		++p;
	else
		p = *argv;
	usingupper = isupper(*p);
	if (!strcasecmp(p, "reset")) {
		isreset = 1;
		reset_mode();
	}

	obsolete(argv);
	while ((ch = getopt(argc, argv, "-e:Ii:k:m:nQs")) != EOF) {
		switch (ch) {
		case '-':
			noset = 1;
			break;
		case 'e':		/* erase character */
			erasechar = optarg[0] == '^' && optarg[1] != '\0' ?
			    optarg[1] == '?' ? '\177' : CTRL(optarg[1]) :
			    optarg[0];
			break;
		case 'I':		/* no initialization */
			noinit = 1;
			break;
		case 'i':		/* interrupt character */
			intrchar = optarg[0] == '^' && optarg[1] != '\0' ?
			    optarg[1] == '?' ? '\177' : CTRL(optarg[1]) :
			    optarg[0];
			break;
		case 'k':		/* kill character */
			killchar = optarg[0] == '^' && optarg[1] != '\0' ?
			    optarg[1] == '?' ? '\177' : CTRL(optarg[1]) :
			    optarg[0];
			break;
		case 'm':		/* map identifier to type */
			add_mapping(optarg);
			break;
		case 'n':		/* Undocumented (obsolete). */
			break;		/* set newtty driver */
		case 'Q':		/* be quiet */
			quiet = 1;
			break;
		case 's':		/* print commands to set environment */
			dosetenv = 1;
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 1)
		usage();

	ttype = get_termcap_entry(*argv, &tcapbuf);

	if (!noset) {
		columns = tgetnum("co");
		lines = tgetnum("li");

#ifdef TIOCGWINSZ
		/* Set window size */
		(void)ioctl(STDERR_FILENO, TIOCGWINSZ, &win);
		if (win.ws_row == 0 && win.ws_col == 0 &&
		    lines > 0 && columns > 0) {
			win.ws_row = lines;
			win.ws_col = columns;
			(void)ioctl(STDERR_FILENO, TIOCSWINSZ, &win);
		}
#endif
		set_control_chars();
		set_conversions(usingupper);

		if (!noinit)
			set_init();

		/* Set the modes if they've changed. */
		if (memcmp(&mode, &oldmode, sizeof(mode)))
			tcsetattr(STDERR_FILENO, TCSADRAIN, &mode);

		/*
		 * If erase, kill and interrupt characters have potentially
		 * been modified and not -Q, display the changes.
		 */
		if (!quiet) {
			report("Erase", VERASE, CERASE);
			report("Kill", VKILL, CKILL);
			report("Interrupt", VINTR, CINTR);
		}
	}

	if (!dosetenv && !noset)
		exit(0);

	/*
	 * The termcap file generally has a two-character name first in each
	 * entry followed by more descriptive names.  If we ended up with the
	 * first one, we switch to the second one for setting or reporting
	 * information.
	 */
	p = strpbrk(tcapbuf, "|:");
	if (p && *p != ':' && !strncmp(ttype, tcapbuf, p - tcapbuf)) {
		t = ++p;
		if (p = strpbrk(p, "|:")) {
			savech = *p;
			*p = '\0';
			if ((ttype = strdup(t)) == NULL)
				err("%s", strerror(errno));
			*p = savech;
		}
	}

	if (noset) {
		(void)printf("%s\n", ttype);
		exit(0);
	}

	/*
	 * Figure out what shell we're using.  A hack, we look for a $SHELL
	 * ending in "csh".
	 */
	csh = (p = getenv("SHELL")) && !strcmp(p + strlen(p) - 3, "csh");
	if (csh)
		(void)printf("set noglob;\nsetenv TERM %s;\nsetenv TERMCAP '",
		    ttype);
	else
		(void)printf("TERM=%s;\nTERMCAP='", ttype);
	wrtermcap(tcapbuf);
	if (csh)
		(void)printf("';\nunset noglob;\n");
	else
		(void)printf("';\nexport TERMCAP TERM;\n");
	exit(0);
}

/*
 * Tell the user if a control key has been changed from the default value.
 */
void
report(name, which, def)
	char *name;
	int which;
	u_int def;
{
	u_int old, new;
	char *bp, buf[1024];

	new = mode.c_cc[which];
	old = oldmode.c_cc[which];

	if (old == new && old == def)
		return;

	(void)fprintf(stderr, "%s %s ", name, old == new ? "is" : "set to");

	bp = buf;
	if (tgetstr("kb", &bp) && new == buf[0] && buf[1] == '\0')
		(void)fprintf(stderr, "backspace.\n");
	else if (new == 0177)
		(void)fprintf(stderr, "delete.\n");
	else if (new < 040) {
		new ^= 0100;
		(void)fprintf(stderr, "control-%c (^%c).\n", new, new);
	} else
		(void)fprintf(stderr, "%c.\n", new);
}

/*
 * Convert the obsolete argument form into something that getopt can handle.
 * This means that -e, -i and -k get default arguments supplied for them.
 */
void
obsolete(argv)
	char *argv[];
{
	for (; *argv; ++argv) {
		if (argv[0][0] != '-' || argv[1] && argv[1][0] != '-' ||
		    argv[0][1] != 'e' && argv[0][1] != 'i' &&
		    argv[0][1] != 'k' || argv[0][2] != '\0')
			continue;
		switch(argv[0][1]) {
		case 'e':
			argv[0] = "-e^H";
			break;
		case 'i':
			argv[0] = "-i^C";
			break;
		case 'k':
			argv[0] = "-k^U";
			break;
		}
	}
}
		
void
usage()
{
	(void)fprintf(stderr,
"usage: tset [-IQs] [-] [-e ch] [-i ch] [-k ch] [-m mapping] [terminal]\n");
	exit(1);
}
