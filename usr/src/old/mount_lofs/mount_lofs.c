/*
 * Copyright (c) 1992 The Regents of the University of California
 * Copyright (c) 1990, 1992 Jan-Simon Pendry
 * All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)mount_lofs.c	5.2 (Berkeley) %G%
 */

#include <sys/param.h>
#include <sys/mount.h>
#include <lofs/lofs.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void usage __P((void));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	struct lofs_args args;
	int ch, mntflags;

	mntflags = 0;
	while ((ch = getopt(argc, argv, "F:")) != EOF)
		switch(ch) {
		case 'F':
			mntflags = atoi(optarg);
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 2)
		usage();

	args.target = argv[0];

	if (mount(MOUNT_LOFS, argv[1], mntflags, &args)) {
		(void)fprintf(stderr, "mount_lofs: %s\n", strerror(errno));
		exit(1);
	}
	exit(0);
}

void
usage()
{
	(void)fprintf(stderr,
	    "usage: mount_lofs [ -F fsoptions ] target_fs mount_point\n");
	exit(1);
}
