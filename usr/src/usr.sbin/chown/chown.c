/*
 * Copyright (c) 1988, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1988, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)chown.c	8.5 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>

#include <ctype.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fts.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void	a_gid __P((char *));
void	a_uid __P((char *));
void	chownerr __P((char *));
u_long	id __P((char *, char *));
void	usage __P((void));

uid_t uid;
gid_t gid;
int Rflag, ischown, fflag;
char *gname, *myname;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern int optind;
	FTS *ftsp;
	FTSENT *p;
	int Hflag, Lflag, Pflag, ch, fts_options, hflag, rval;
	char *cp;
	
	myname = (cp = rindex(*argv, '/')) ? cp + 1 : *argv;
	ischown = myname[2] == 'o';
	
	Hflag = Lflag = Pflag = hflag = 0;
	while ((ch = getopt(argc, argv, "HLPRfh")) != EOF)
		switch (ch) {
		case 'H':
			Hflag = 1;
			Lflag = Pflag = 0;
			break;
		case 'L':
			Lflag = 1;
			Hflag = Pflag = 0;
			break;
		case 'P':
			Pflag = 1;
			Hflag = Lflag = 0;
			break;
		case 'R':
			Rflag = 1;
			break;
		case 'f':
			fflag = 1;
			break;
		case 'h':
			/*
			 * In System V (and probably POSIX.2) the -h option
			 * causes chown/chgrp to change the owner/group of
			 * the symbolic link.  4.4BSD's symbolic links don't
			 * have owners/groups, so it's an undocumented noop.
			 * Do syntax checking, though.
			 */
			hflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argv += optind;
	argc -= optind;

	if (argc < 2)
		usage();

	fts_options = FTS_NOSTAT | FTS_PHYSICAL;
	if (Rflag) {
		if (hflag)
			errx(1,
		"the -R and -h options may not be specified together.");
		if (Hflag)
			fts_options |= FTS_COMFOLLOW;
		if (Lflag) {
			fts_options &= ~FTS_PHYSICAL;
			fts_options |= FTS_LOGICAL;
		}
	}

	uid = gid = -1;
	if (ischown) {
#ifdef SUPPORT_DOT
		if ((cp = strchr(*argv, '.')) != NULL) {
			*cp++ = '\0';
			a_gid(cp);
		} else
#endif
		if ((cp = strchr(*argv, ':')) != NULL) {
			*cp++ = '\0';
			a_gid(cp);
		} 
		a_uid(*argv);
	} else 
		a_gid(*argv);

	if ((ftsp = fts_open(++argv, fts_options, 0)) == NULL)
		err(1, NULL);

	for (rval = 0; (p = fts_read(ftsp)) != NULL;) {
		switch (p->fts_info) {
		case FTS_D:
			if (Rflag)		/* Change it at FTS_DP. */
				continue;
			fts_set(ftsp, p, FTS_SKIP);
			break;
		case FTS_DNR:			/* Warn, chown, continue. */
			errno = p->fts_errno;
			warn("%s", p->fts_path);
			rval = 1;
			break;
		case FTS_ERR:			/* Warn, continue. */
			errno = p->fts_errno;
			warn("%s", p->fts_path);
			rval = 1;
			continue;
		case FTS_SL:			/* Ignore. */
		case FTS_SLNONE:
			/*
			 * The only symlinks that end up here are ones that
			 * don't point to anything and ones that we found
			 * doing a physical walk.
			 */
			continue;
		default:
			break;
		}
		if (chown(p->fts_accpath, uid, gid) && !fflag) {
			chownerr(p->fts_path);
			rval = 1;
		}
	}
	if (errno)
		err(1, "fts_read");
	exit(rval);
}

void
a_gid(s)
	char *s;
{
	struct group *gr;

	if (*s == '\0')			/* Argument was "uid[:.]". */
		return;
	gname = s;
	gid = ((gr = getgrnam(s)) == NULL) ? id(s, "group") : gr->gr_gid;
}

void
a_uid(s)
	char *s;
{
	struct passwd *pw;

	if (*s == '\0')			/* Argument was "[:.]gid". */
		return;
	uid = ((pw = getpwnam(s)) == NULL) ? id(s, "user") : pw->pw_uid;
}

u_long
id(name, type)
	char *name, *type;
{
	u_long val;
	char *ep;

	/*
	 * XXX
	 * We know that uid_t's and gid_t's are unsigned longs.
	 */
	errno = 0;
	val = strtoul(name, &ep, 10);
	if (errno)
		err(1, "%s", name);
	if (*ep != '\0')
		errx(1, "%s: illegal %s name", name, type);
	return (val);
}

void
chownerr(file)
	char *file;
{
	static int euid = -1, ngroups = -1;
	int groups[NGROUPS];

	/* Check for chown without being root. */
	if (errno != EPERM ||
	    uid != -1 && euid == -1 && (euid = geteuid()) != 0) {
		if (fflag)
			exit(0);
		err(1, "%s", file);
	}

	/* Check group membership; kernel just returns EPERM. */
	if (gid != -1 && ngroups == -1) {
		ngroups = getgroups(NGROUPS, groups);
		while (--ngroups >= 0 && gid != groups[ngroups]);
		if (ngroups < 0) {
			if (fflag)
				exit(0);
			errx(1, "you are not a member of group %s", gname);
		}
	}

	if (!fflag) 
		warn("%s", file);
}

void
usage()
{
	(void)fprintf(stderr,
	    "usage: %s [-R [-H | -L | -P]] [-f] %s file ...\n",
	    myname, ischown ? "[owner][:group]" : "group");
	exit(1);
}
