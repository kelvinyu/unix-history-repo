/*-
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)print.c	8.3 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "archive.h"
#include "extern.h"

/*
 * print --
 *	Prints archive members on stdout - if member names given only
 *	print those members, otherwise print all members.
 */
int
print(argv)
	char **argv;
{
	CF cf;
	int afd, all;
	char *file;

	afd = open_archive(O_RDONLY);

	/* Read from an archive, write to stdout; pad on read. */
	SETCF(afd, archive, STDOUT_FILENO, "stdout", RPAD);
	for (all = !*argv; get_arobj(afd);) {
		if (all)
			file = chdr.name;
		else if (!(file = files(argv))) {
			skip_arobj(afd);
			continue;
		}
		if (options & AR_V) {
			(void)printf("\n<%s>\n\n", file);
			(void)fflush(stdout);
		}
		copy_ar(&cf, chdr.size);
		if (!all && !*argv)
			break;
	}
	close_archive(afd);

	if (*argv) {
		orphans(argv);
		return (1);
	}
	return (0);
}
