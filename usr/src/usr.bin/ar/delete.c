/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)delete.c	5.4 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <ar.h>
#include "archive.h"
#include "pathnames.h"

extern CHDR chdr;			/* converted header */
extern char *archive;			/* archive name */
extern char *tname;                     /* temporary file "name" */

/*-
 * delete --
 *	Deletes named members from the archive.
 */
delete(argv)
	register char **argv;
{
	CF cf;
	off_t size;
	int afd, eval, tfd;
	char *file;

	afd = open_archive(O_RDWR);
	tfd = tmp();

	/* Read and write to an archive; pad on both. */
	SETCF(afd, archive, tfd, tname, RPAD|WPAD);
	while (get_header(afd)) {
		if ((file = *argv) && files(argv)) {
			if (options & AR_V)
				(void)printf("d - %s\n", file);
			skipobj(afd);
			continue;
		}
		put_object(&cf, (struct stat *)NULL);
	}
	eval = 0;
	ORPHANS;

	size = lseek(tfd, (off_t)0, SEEK_CUR);
	(void)lseek(tfd, (off_t)0, SEEK_SET);
	(void)lseek(afd, (off_t)SARMAG, SEEK_SET);
	SETCF(tfd, tname, afd, archive, NOPAD);
	copyfile(&cf, size);
	(void)close(tfd);
	(void)ftruncate(afd, size + SARMAG);
	close_archive(afd);
	return(eval);
}	
