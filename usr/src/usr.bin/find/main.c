/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)main.c	5.8 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include "find.h"

time_t now;			/* time find was run */
int ftsoptions;			/* options for the ftsopen(3) call */
int isdeprecated;		/* using deprecated syntax */
int isdepth;			/* do directories on post-order visit */
int isoutput;			/* user specified output operator */
int isrelative;			/* can do -exec/ok on relative path */
int isxargs;			/* don't permit xargs delimiting chars */

static void usage();

main(argc, argv)
	int argc;
	char **argv;
{
	register char **p, **start;
	PLAN *find_formplan();
	int ch;

	(void)time(&now);	/* initialize the time-of-day */

	p = start = argv;
	ftsoptions = FTS_NOSTAT|FTS_PHYSICAL;
	while ((ch = getopt(argc, argv, "df:rsXx")) != EOF)
		switch(ch) {
		case 'd':
			isdepth = 1;
			break;
		case 'f':
			*p++ = optarg;
			break;
		case 'r':
			isrelative = 1;
			break;
		case 's':
			ftsoptions &= ~FTS_PHYSICAL;
			ftsoptions |= FTS_LOGICAL;
			break;
		case 'X':
			isxargs = 1;
			break;
		case 'x':
			ftsoptions &= ~FTS_NOSTAT;
			ftsoptions |= FTS_XDEV;
			break;
		case '?':
		default:
			break;
		}

	argc -= optind;	
	argv += optind;

	/* Find first option to delimit the file list. */
	while (*argv) {
		if (option(*argv))
			break;
		*p++ = *argv++;
	}

	if (p == start)
		usage();
	*p = NULL;

	find_execute(find_formplan(argv), start);
}

static void
usage()
{
	(void)fprintf(stderr,
	    "usage: find [-drsXx] [-f file] [file ...] expression\n");
	exit(1);
}
