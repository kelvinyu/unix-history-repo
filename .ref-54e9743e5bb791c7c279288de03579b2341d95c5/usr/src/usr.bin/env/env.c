/*
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)env.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include <stdio.h>
#include <string.h>

main(argc, argv)
	int argc;
	char **argv;
{
	extern char **environ;
	extern int errno, optind;
	register char **ep, *p;
	char *cleanenv[1];
	int ch;

	while ((ch = getopt(argc, argv, "-")) != EOF)
		switch((char)ch) {
		case '-':
			environ = cleanenv;
			cleanenv[0] = NULL;
			break;
		case '?':
		default:
			(void)fprintf(stderr,
			    "usage: env [-] [name=value ...] [command]\n");
			exit(1);
		}
	for (argv += optind; *argv && (p = index(*argv, '=')); ++argv)
		(void)setenv(*argv, ++p, 1);
	if (*argv) {
		execvp(*argv, argv);
		(void)fprintf(stderr, "env: %s: %s\n", *argv,
		    strerror(errno));
		exit(1);
	}
	for (ep = environ; *ep; ep++)
		(void)printf("%s\n", *ep);
	exit(0);
}
