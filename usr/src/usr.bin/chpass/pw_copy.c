/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)pw_copy.c	5.2 (Berkeley) %G%";
#endif /* not lint */

/*
 * This module is used to copy the master password file, replacing a single
 * record, by chpass(1) and passwd(1).
 */

#include <pwd.h>
#include <stdio.h>
#include <string.h>

extern char *tempname;

pw_copy(ffd, tfd, pw)
	int ffd, tfd;
	struct passwd *pw;
{
	register FILE *from, *to;
	register int done;
	register char *p;
	char buf[8192];

	if (!(from = fdopen(ffd, "r")))
		pw_error(_PATH_MASTERPASSWD, 1, 1);
	if (!(to = fdopen(tfd, "w")))
		pw_error(tempname, 1, 1);

	for (done = 0; fgets(buf, sizeof(buf), from);) {
		if (!index(buf, '\n')) {
			(void)fprintf(stderr, "chpass: %s: line too long\n",
			    _PATH_MASTERPASSWD);
			pw_error((char *)NULL, 0, 1);
		}
		if (done) {
			(void)fprintf(to, "%s", buf);
			continue;
		}
		if (!(p = index(buf, ':'))) {
			(void)fprintf(stderr, "chpass: %s: corrupted entry\n",
			    _PATH_MASTERPASSWD);
			pw_error((char *)NULL, 0, 1);
		}
		*p = '\0';
		if (strcmp(buf, pw->pw_name)) {
			*p = ':';
			(void)fprintf(to, "%s", buf);
			continue;
		}
		(void)fprintf(to, "%s:%s:%d:%d:%s:%ld:%ld:%s:%s:%s\n",
		    pw->pw_name, pw->pw_passwd, pw->pw_uid, pw->pw_gid,
		    pw->pw_class, pw->pw_change, pw->pw_expire, pw->pw_gecos,
		    pw->pw_dir, pw->pw_shell);
		done = 1;
	}
	if (!done)
		(void)fprintf(to, "%s:%s:%d:%d:%s:%ld:%ld:%s:%s:%s\n",
		    pw->pw_name, pw->pw_passwd, pw->pw_uid, pw->pw_gid,
		    pw->pw_class, pw->pw_change, pw->pw_expire, pw->pw_gecos,
		    pw->pw_dir, pw->pw_shell);
	(void)fclose(to);
}
