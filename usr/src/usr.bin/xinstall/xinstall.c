/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)xinstall.c	5.26 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <paths.h>
#include "pathnames.h"

struct passwd *pp;
struct group *gp;
int docopy, dostrip;
int mode = S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;
char *group, *owner, pathbuf[MAXPATHLEN];

void	copy __P((int, char *, int, char *));
void	err __P((const char *, ...));
void	install __P((char *, char *, int));
void	strip __P((char *));
void	usage __P((void));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	struct stat from_sb, to_sb;
	mode_t *set;
	int ch, no_target;
	char *to_name;

	while ((ch = getopt(argc, argv, "cg:m:o:s")) != EOF)
		switch((char)ch) {
		case 'c':
			docopy = 1;
			break;
		case 'g':
			group = optarg;
			break;
		case 'm':
			if (!(set = setmode(optarg)))
				err("%s: invalid file mode", optarg);
			mode = getmode(set, 0);
			break;
		case 'o':
			owner = optarg;
			break;
		case 's':
			dostrip = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc < 2)
		usage();

	/* get group and owner id's */
	if (group && !(gp = getgrnam(group)))
		err("unknown group %s", group);
	if (owner && !(pp = getpwnam(owner)))
		err("unknown user %s", owner);

	no_target = stat(to_name = argv[argc - 1], &to_sb);
	if (!no_target && (to_sb.st_mode & S_IFMT) == S_IFDIR) {
		for (; *argv != to_name; ++argv)
			install(*argv, to_name, 1);
		exit(0);
	}

	/* can't do file1 file2 directory/file */
	if (argc != 2)
		usage();

	if (!no_target) {
		if (stat(*argv, &from_sb))
			err("%s: %s", *argv, strerror(errno));
		if (!S_ISREG(to_sb.st_mode))
			err("%s: %s", to_name, strerror(EFTYPE));
		if (to_sb.st_dev == from_sb.st_dev &&
		    to_sb.st_ino == from_sb.st_ino)
			err("%s and %s are the same file", *argv, to_name);
		/* unlink now... avoid ETXTBSY errors later */
		(void)unlink(to_name);
	}
	install(*argv, to_name, 0);
	exit(0);
}

/*
 * install --
 *	build a path name and install the file
 */
void
install(from_name, to_name, isdir)
	char *from_name, *to_name;
	int isdir;
{
	struct stat from_sb;
	int devnull, from_fd, to_fd, serrno;
	char *p;

	/* If try to install NULL file to a directory, fails. */
	if (isdir || strcmp(from_name, _PATH_DEVNULL)) {
		if (stat(from_name, &from_sb))
			err("%s: %s", from_name, strerror(errno));
		if (!S_ISREG(from_sb.st_mode))
			err("%s: %s", from_name, strerror(EFTYPE));
		/* Build the target path. */
		if (isdir) {
			(void)snprintf(pathbuf, sizeof(pathbuf), "%s/%s",
			    to_name,
			    (p = rindex(from_name, '/')) ? ++p : from_name);
			to_name = pathbuf;
		}
		devnull = 0;
	} else
		devnull = 1;

	/* Unlink now... avoid ETXTBSY errors later. */
	(void)unlink(to_name);

	/* Create target. */
	if ((to_fd = open(to_name, O_CREAT|O_WRONLY|O_TRUNC, 0600)) < 0)
		err("%s: %s", to_name, strerror(errno));
	if (!devnull) {
		if ((from_fd = open(from_name, O_RDONLY, 0)) < 0) {
			(void)unlink(to_name);
			err("%s: %s", from_name, strerror(errno));
		}
		copy(from_fd, from_name, to_fd, to_name);
		(void)close(from_fd);
	}
	if (dostrip)
		strip(to_name);
	/*
	 * Set owner, group, mode for target; do the chown first,
	 * chown may lose the setuid bits.
	 */
	if ((group || owner) &&
	    fchown(to_fd, owner ? pp->pw_uid : -1, group ? gp->gr_gid : -1) ||
	    fchmod(to_fd, mode)) {
		serrno = errno;
		(void)unlink(to_name);
		err("%s: %s", to_name, strerror(serrno));
	}

	/* Always preserve the flags. */
	if (fchflags(to_fd, from_sb.st_flags)) {
		serrno = errno;
		(void)unlink(to_name);
		err("%s: %s", to_name, strerror(serrno));
	}

	(void)close(to_fd);
	if (!docopy && !devnull && unlink(from_name))
		err("%s: %s", from_name, strerror(errno));
}

/*
 * copy --
 *	copy from one file to another
 */
void
copy(from_fd, from_name, to_fd, to_name)
	register int from_fd, to_fd;
	char *from_name, *to_name;
{
	register int nr, nw;
	int serrno;
	char buf[MAXBSIZE];

	while ((nr = read(from_fd, buf, sizeof(buf))) > 0)
		if ((nw = write(to_fd, buf, nr)) != nr) {
			serrno = errno;
			(void)unlink(to_name);
			err("%s: %s", to_name, strerror(nw > 0 ? EIO : serrno));
		}
	if (nr != 0) {
		serrno = errno;
		(void)unlink(to_name);
		err("%s: %s", from_name, strerror(serrno));
	}
}

/*
 * strip --
 *	use strip(1) to strip the target file
 */
void
strip(to_name)
	char *to_name;
{
	int serrno, status;

	switch (vfork()) {
	case -1:
		serrno = errno;
		(void)unlink(to_name);
		err("forks: %s", strerror(errno));
	case 0:
		execl(_PATH_STRIP, "strip", to_name, NULL);
		err("%s: %s", _PATH_STRIP, strerror(errno));
	default:
		if (wait(&status) == -1 || status)
			(void)unlink(to_name);
	}
}

/*
 * usage --
 *	print a usage message and die
 */
void
usage()
{
	(void)fprintf(stderr,
"usage: install [-cs] [-g group] [-m mode] [-o owner] file1 file2;\n\tor file1 ... fileN directory\n");
	exit(1);
}

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

void
#if __STDC__
err(const char *fmt, ...)
#else
err(fmt, va_alist)
	char *fmt;
        va_dcl
#endif
{
	va_list ap;
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)fprintf(stderr, "install: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	exit(1);
	/* NOTREACHED */
}
