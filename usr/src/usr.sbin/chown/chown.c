/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "@(#)chown.c	5.2 (Berkeley) %G%";
#endif

/*
 * chown [-fR] uid[.gid] file ...
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/dir.h>
#include <grp.h>
#include <strings.h>

struct	passwd *pwd;
struct	passwd *getpwnam();
struct	stat stbuf;
int	uid;
int	status;
int	fflag;
int	rflag;

main(argc, argv)
	char *argv[];
{
	register int c, gid;
	register char *cp, *group;
	struct group *grp;

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'f':
			fflag++;
			break;

		case 'R':
			rflag++;
			break;

		default:
			fatal(255, "unknown option: %c", *cp);
		}
		argv++, argc--;
	}
	if (argc < 2) {
		fprintf(stderr, "usage: chown [-fR] owner[.group] file ...\n");
		exit(-1);
	}
	group = index(argv[0], '.');
	if (group != NULL) {
		*group++ = '\0';
		if (!isnumber(group)) {
			if ((grp = getgrnam(group)) == NULL)
				fatal(255, "unknown group: %s",group);
			gid = grp -> gr_gid;
			endgrent();
		} else
			gid = atoi(group);
	}
	if (!isnumber(argv[0])) {
		if ((pwd = getpwnam(argv[0])) == NULL)
			fatal(255, "unknown user id: %s",argv[0]);
		uid = pwd->pw_uid;
	} else
		uid = atoi(argv[0]);
	for (c = 1; c < argc; c++) {
		/* do stat for directory arguments */
		if (stat(argv[c], &stbuf) < 0) {
			status += error("couldn't access %s", argv[c]);
			continue;
		}
		if (group == NULL)
			gid = stbuf.st_gid;
		if (rflag && stbuf.st_mode&S_IFDIR) {
			status += chownr(argv[c], group != NULL, uid, gid);
			continue;
		}
		if (chown(argv[c], uid, gid)) {
			status += error("couldn't change %s", argv[c]);
			continue;
		}
	}
	exit(status);
}

isnumber(s)
	char *s;
{
	register c;

	while(c = *s++)
		if (!isdigit(c))
			return (0);
	return (1);
}

chownr(dir, dogrp, uid, ogid)
	char *dir;
{
	register DIR *dirp;
	register struct direct *dp;
	register struct stat st;
	char savedir[1024];
	int ecode, gid;

	if (getwd(savedir) == 0)
		fatal(255, "%s", savedir);
	/*
	 * Change what we are given before doing it's contents.
	 */
	if (chown(dir, uid, ogid) < 0 && error("can't change %s", dir))
		return (1);
	if (chdir(dir) < 0)
		return (Perror(dir));
	if ((dirp = opendir(".")) == NULL)
		return (Perror(dir));
	dp = readdir(dirp);
	dp = readdir(dirp); /* read "." and ".." */
	ecode = 0;
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (stat(dp->d_name, &st) < 0) {
			ecode = error("can't access %s", dp->d_name);
			if (ecode)
				break;
			continue;
		}
		if (dogrp)
			gid = ogid;
		else
			gid = st.st_gid;
		if (st.st_mode&S_IFDIR) {
			ecode = chownr(dp->d_name, dogrp, uid, gid);
			if (ecode)
				break;
			continue;
		}
		if (chown(dp->d_name, uid, gid) < 0 &&
		    (ecode = error("can't change %s", dp->d_name)))
			break;
	}
	closedir(dirp);
	if (chdir(savedir) < 0)
		fatal(255, "can't change back to %s", savedir);
	return (ecode);
}

error(fmt, a)
	char *fmt, *a;
{

	if (!fflag) {
		fprintf(stderr, "chown: ");
		fprintf(stderr, fmt, a);
		putc('\n', stderr);
	}
	return (!fflag);
}

fatal(status, fmt, a)
	int status;
	char *fmt, *a;
{

	fflag = 0;
	(void) error(fmt, a);
	exit(status);
}

Perror(s)
	char *s;
{

	fprintf(stderr, "chown: ");
	perror(s);
	return (1);
}
