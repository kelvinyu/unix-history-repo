/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)pw_scan.c	8.1 (Berkeley) %G%";
#endif /* not lint */

/*
 * This module is used to "verify" password entries by chpass(1) and
 * pwd_mkdb(8).
 */

#include <sys/param.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern char *progname;

pw_scan(bp, pw)
	char *bp;
	struct passwd *pw;
{
	register long id;
	register int root;
	register char *p, *sh;
	char *getusershell();

	if (!(pw->pw_name = strsep(&bp, ":")))		/* login */
		goto fmt;
	root = !strcmp(pw->pw_name, "root");

	if (!(pw->pw_passwd = strsep(&bp, ":")))	/* passwd */
		goto fmt;

	if (!(p = strsep(&bp, ":")))			/* uid */
		goto fmt;
	id = atol(p);
	if (root && id) {
		(void)fprintf(stderr, "%s: root uid should be 0", progname);
		return(0);
	}
	if (id > USHRT_MAX) {
		(void)fprintf(stderr,
		    "%s: %s > max uid value (%d)", progname, p, USHRT_MAX);
		return(0);
	}
	pw->pw_uid = id;

	if (!(p = strsep(&bp, ":")))			/* gid */
		goto fmt;
	id = atol(p);
	if (id > USHRT_MAX) {
		(void)fprintf(stderr,
		    "%s: %s > max gid value (%d)", progname, p, USHRT_MAX);
		return(0);
	}
	pw->pw_gid = id;

	pw->pw_class = strsep(&bp, ":");		/* class */
	if (!(p = strsep(&bp, ":")))			/* change */
		goto fmt;
	pw->pw_change = atol(p);
	if (!(p = strsep(&bp, ":")))			/* expire */
		goto fmt;
	pw->pw_expire = atol(p);
	pw->pw_gecos = strsep(&bp, ":");		/* gecos */
	pw->pw_dir = strsep(&bp, ":");			/* directory */
	if (!(pw->pw_shell = strsep(&bp, ":")))		/* shell */
		goto fmt;

	p = pw->pw_shell;
	if (root && *p)					/* empty == /bin/sh */
		for (setusershell();;) {
			if (!(sh = getusershell())) {
				(void)fprintf(stderr,
				    "%s: warning, unknown root shell\n",
				    progname);
				break;
			}
			if (!strcmp(p, sh))
				break;	
		}

	if (p = strsep(&bp, ":")) {			/* too many */
fmt:		(void)fprintf(stderr, "%s: corrupted entry\n", progname);
		return(0);
	}
	return(1);
}
