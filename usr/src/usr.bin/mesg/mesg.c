/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mesg.c	5.2 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void err __P((const char *fmt, ...));
void usage __P((void));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	struct stat sbuf;
	char *tty;
	int ch;

	while ((ch = getopt(argc, argv, "")) != EOF)
		switch(ch) {
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (!(tty = ttyname(2)))
		err("ttyname: %s", strerror(errno));
	if (stat(tty, &sbuf) < 0)
		err("%s: %s", strerror(errno));

	if (*argv == NULL) {
		if (sbuf.st_mode & 020) {
			(void)fprintf(stderr, "is y\n");
			exit(0);
		}
		(void)fprintf(stderr, "is n\n");
		exit(1);
	}
#define	OTHER_WRITE	020
	switch(*argv[0]) {
	case 'y':
		if (chmod(tty, sbuf.st_mode | OTHER_WRITE) < 0)
			err("%s: %s", strerror(errno));
		exit(0);
	case 'n':
		if (chmod(tty, sbuf.st_mode &~ OTHER_WRITE) < 0)
			err("%s: %s", strerror(errno));
		exit(1);
	}
	usage();
	/* NOTREACHED */
}

void
usage()
{
	(void)fprintf(stderr, "usage: mesg [y | n]\n");
	exit(2);
}

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

void
#if __STDC__
err(const char *fmt, ...)
#else
err(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	va_list ap;
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)fprintf(stderr, "mesg: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	exit(2);
	/* NOTREACHED */
}
