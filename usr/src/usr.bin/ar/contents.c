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
static char sccsid[] = "@(#)contents.c	8.3 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <ar.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <tzfile.h>
#include <unistd.h>

#include "archive.h"
#include "extern.h"

/*
 * contents --
 *	Handles t[v] option - opens the archive and then reads headers,
 *	skipping member contents.
 */
int
contents(argv)
	char **argv;
{
	int afd, all;
	struct tm *tp;
	char *file, buf[25];
	
	afd = open_archive(O_RDONLY);

	for (all = !*argv; get_arobj(afd);) {
		if (all)
			file = chdr.name;
		else if (!(file = files(argv)))
			goto next;
		if (options & AR_V) {
			(void)strmode(chdr.mode, buf);
			(void)printf("%s %6d/%-6d %8qd ",
			    buf + 1, chdr.uid, chdr.gid, chdr.size);
			tp = localtime(&chdr.date);
			(void)strftime(buf, sizeof(buf), "%b %e %H:%M %Y", tp);
			(void)printf("%s %s\n", buf, file);
		} else
			(void)printf("%s\n", file);
		if (!all && !*argv)
			break;
next:		skip_arobj(afd);
	} 
	close_archive(afd);

	if (*argv) {
		orphans(argv);
		return (1);
	}
	return (0);
}
