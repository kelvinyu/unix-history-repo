/*
 * Copyright (c) 1980, 1986 The Regents of the University of California.
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
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1986 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	5.21 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#include <fstab.h>
#include <strings.h>
#include <ctype.h>
#include "fsck.h"

void	catch(), catchquit(), voidquit();
int	returntosingle;

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int ret, maxrun = 0;
	extern int (docheck)(), (checkfilesys)();

	sync();
	while (--argc > 0 && **++argv == '-') {
		switch (*++*argv) {

		case 'p':
			preen++;
			break;

		case 'b':
			if (argv[0][1] != '\0') {
				bflag = atoi(argv[0]+1);
			} else {
				bflag = atoi(*++argv);
				argc--;
			}
			printf("Alternate super block location: %d\n", bflag);
			break;

		case 'c':
			cvtflag++;
			break;

		case 'd':
			debug++;
			break;

		case 'l':
			if (!isdigit(argv[1][0]))
				errexit("-l flag requires a number\n");
			maxrun = atoi(*++argv);
			argc--;
			break;

		case 'm':
			if (!isdigit(argv[1][0]))
				errexit("-m flag requires a mode\n");
			sscanf(*++argv, "%o", &lfmode);
			if (lfmode &~ 07777)
				errexit("bad mode to -m: %o\n", lfmode);
			argc--;
			printf("** lost+found creation mode %o\n", lfmode);
			break;

		case 'n':
		case 'N':
			nflag++;
			yflag = 0;
			break;

		case 'y':
		case 'Y':
			yflag++;
			nflag = 0;
			break;

		default:
			errexit("%c option?\n", **argv);
		}
	}
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void)signal(SIGINT, catch);
	if (preen)
		(void)signal(SIGQUIT, catchquit);
	if (argc) {
		while (argc-- > 0) {
			hotroot = 0;
			checkfilesys(*argv++);
		}
		exit(0);
	}
	ret = checkfstab(preen, maxrun, docheck, checkfilesys);
	if (returntosingle)
		exit(2);
	exit(ret);
}

/*
 * Determine whether a filesystem should be checked.
 */
docheck(fsp)
	register struct fstab *fsp;
{

	if (strcmp(fsp->fs_vfstype, "ufs") ||
	    (strcmp(fsp->fs_type, FSTAB_RW) &&
	     strcmp(fsp->fs_type, FSTAB_RO)) ||
	    fsp->fs_passno == 0)
		return (0);
	return (1);
}

/*
 * Check the specified filesystem.
 */
/* ARGSUSED */
checkfilesys(filesys, mntpt, auxdata)
	char *filesys, *mntpt;
	long auxdata;
{
	daddr_t n_ffree, n_bfree;
	struct dups *dp;
	struct zlncnt *zlnp;

	(void)signal(SIGQUIT, voidquit);
	devname = filesys;
	if (debug && preen)
		pwarn("starting\n");
	if (setup(filesys) == 0) {
		if (preen)
			pfatal("CAN'T CHECK FILE SYSTEM.");
		return (0);
	}
	/*
	 * 1: scan inodes tallying blocks used
	 */
	if (preen == 0) {
		printf("** Last Mounted on %s\n", sblock.fs_fsmnt);
		if (hotroot)
			printf("** Root file system\n");
		printf("** Phase 1 - Check Blocks and Sizes\n");
	}
	pass1();

	/*
	 * 1b: locate first references to duplicates, if any
	 */
	if (duplist) {
		if (preen)
			pfatal("INTERNAL ERROR: dups with -p");
		printf("** Phase 1b - Rescan For More DUPS\n");
		pass1b();
	}

	/*
	 * 2: traverse directories from root to mark all connected directories
	 */
	if (preen == 0)
		printf("** Phase 2 - Check Pathnames\n");
	pass2();

	/*
	 * 3: scan inodes looking for disconnected directories
	 */
	if (preen == 0)
		printf("** Phase 3 - Check Connectivity\n");
	pass3();

	/*
	 * 4: scan inodes looking for disconnected files; check reference counts
	 */
	if (preen == 0)
		printf("** Phase 4 - Check Reference Counts\n");
	pass4();

	/*
	 * 5: check and repair resource counts in cylinder groups
	 */
	if (preen == 0)
		printf("** Phase 5 - Check Cyl groups\n");
	pass5();

	/*
	 * print out summary statistics
	 */
	n_ffree = sblock.fs_cstotal.cs_nffree;
	n_bfree = sblock.fs_cstotal.cs_nbfree;
	pwarn("%d files, %d used, %d free ",
	    n_files, n_blks, n_ffree + sblock.fs_frag * n_bfree);
	printf("(%d frags, %d blocks, %.1f%% fragmentation)\n",
	    n_ffree, n_bfree, (float)(n_ffree * 100) / sblock.fs_dsize);
	if (debug &&
	    (n_files -= maxino - ROOTINO - sblock.fs_cstotal.cs_nifree))
		printf("%d files missing\n", n_files);
	if (debug) {
		n_blks += sblock.fs_ncg *
			(cgdmin(&sblock, 0) - cgsblock(&sblock, 0));
		n_blks += cgsblock(&sblock, 0) - cgbase(&sblock, 0);
		n_blks += howmany(sblock.fs_cssize, sblock.fs_fsize);
		if (n_blks -= maxfsblock - (n_ffree + sblock.fs_frag * n_bfree))
			printf("%d blocks missing\n", n_blks);
		if (duplist != NULL) {
			printf("The following duplicate blocks remain:");
			for (dp = duplist; dp; dp = dp->next)
				printf(" %d,", dp->dup);
			printf("\n");
		}
		if (zlnhead != NULL) {
			printf("The following zero link count inodes remain:");
			for (zlnp = zlnhead; zlnp; zlnp = zlnp->next)
				printf(" %d,", zlnp->zlncnt);
			printf("\n");
		}
	}
	zlnhead = (struct zlncnt *)0;
	duplist = (struct dups *)0;
	inocleanup();
	if (fsmodified) {
		(void)time(&sblock.fs_time);
		sbdirty();
	}
	ckfini();
	free(blockmap);
	free(statemap);
	free((char *)lncntp);
	if (!fsmodified)
		return (0);
	if (!preen) {
		printf("\n***** FILE SYSTEM WAS MODIFIED *****\n");
		if (hotroot)
			printf("\n***** REBOOT UNIX *****\n");
	}
	if (hotroot) {
		sync();
		return (4);
	}
	return (0);
}
