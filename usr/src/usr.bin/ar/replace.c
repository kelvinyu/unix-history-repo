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
static char sccsid[] = "@(#)replace.c	5.1 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <ar.h>
#include <stdio.h>
#include "archive.h"

extern CHDR chdr;			/* converted header */
extern char *archive;			/* archive name */
extern char *tname;                     /* temporary file "name" */

/*
 * replace --
 *	Replace or add named members to archive.  Entries already in the
 *	archive are swapped in place.  Others are added before or after 
 *	the key entry, based on the a, b and i options.  If the u option
 *	is specified, modification dates select for replacement.
 */
replace(argv)
	char **argv;
{
	extern char *posname;		/* positioning file name */
	register char *file;
	register int afd, curfd, mods, sfd;
	struct stat sb;
	CF cf;
	off_t size, tsize;
	int err, exists, tfd1, tfd2;
	char *rname();

	/*
	 * If doesn't exist, simply append to the archive.  There's
	 * a race here, but it's pretty short, and not worth fixing.
	 */
	exists = !stat(archive, &sb);
	afd = open_archive(O_CREAT|O_RDWR);

	if (!exists) {
		tfd1 = -1;
		tfd2 = tmp();
		goto append;
	} 

	tfd1 = tmp();			/* Files before key file. */
	tfd2 = tmp();			/* Files after key file. */
	
	mods = (options & (AR_A|AR_B));

	/*
	 * Break archive into two parts -- entries before and after the key
	 * entry.  If positioning before the key, place the key at the
	 * beginning of the after key entries and if positioning after the
	 * key, place the key at the end of the before key entries.  Put it
	 * all back together at the end.
	 */
	for (err = 0, curfd = tfd1; get_header(afd);) {
		if ((file = *argv) && files(argv)) {
			if ((sfd = open(file, O_RDONLY)) < 0) {
				err = 1;
				(void)fprintf(stderr, "ar: %s: %s.\n",
				    file, strerror(errno));
				goto useold;
			}
			(void)fstat(sfd, &sb);
			if (options & AR_U && sb.st_mtime <= chdr.date)
				goto useold;

			if (options & AR_V)
			     (void)printf("r - %s\n", chdr.name);

			SETCF(sfd, file, curfd, tname, WPAD);
			put_header(&cf, &sb);
			copyfile(&cf, sb.st_size);
			(void)close(sfd);
			SKIP(afd, chdr.size, archive);
			continue;
		}

		if (mods && compare(posname)) {
			mods = 0;
			if (options & AR_B)
				curfd = tfd2;
			SETCF(afd, archive, curfd, tname, RPAD|WPAD);
			put_header(&cf, (struct stat *)NULL);
			copyfile(&cf, chdr.size);
			if (options & AR_A)
				curfd = tfd2;
		} else {
useold:			SETCF(afd, archive, curfd, tname, RPAD|WPAD);
			put_header(&cf, (struct stat *)NULL);
			copyfile(&cf, chdr.size);
		}
	}

	if (mods) {
		(void)fprintf(stderr, "ar: %s: archive member not found.\n",
		    posname);
                close_archive(afd);
                return(1);
        }

	/* Append any left-over arguments to the end of the after file. */
append:	while (file = *argv++) {
		if (options & AR_V)
			(void)printf("a - %s\n", rname(file));
		if ((sfd = open(file, O_RDONLY)) < 0) {
			err = 1;
			(void)fprintf(stderr, "ar: %s: %s.\n",
			    file, strerror(errno));
			continue;
		}
		(void)fstat(sfd, &sb);
		SETCF(sfd, file, tfd2, tname, WPAD);
		put_header(&cf, &sb);
		copyfile(&cf, sb.st_size);
		(void)close(sfd);
	}
	
	(void)lseek(afd, (off_t)SARMAG, SEEK_SET);

	SETCF(tfd1, tname, afd, archive, RPAD|WPAD);
	if (tfd1 != -1) {
		tsize = size = lseek(tfd1, (off_t)0, SEEK_CUR);
		(void)lseek(tfd1, (off_t)0, SEEK_SET);
		copyfile(&cf, size);
	} else
		tsize = 0;

	tsize += size = lseek(tfd2, (off_t)0, SEEK_CUR);
	(void)lseek(tfd2, (off_t)0, SEEK_SET);
	cf.rfd = tfd2;
	copyfile(&cf, size);

	(void)ftruncate(afd, tsize + SARMAG);
	close_archive(afd);
	return(err);
}	
