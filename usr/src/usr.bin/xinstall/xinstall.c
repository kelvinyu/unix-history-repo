/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)xinstall.c	5.1 (Berkeley) %G%";
#endif not lint

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <a.out.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <ctype.h>

#define	YES		1			/* yes/true */
#define	DEF_GROUP	"root"			/* default group */
#define	DEF_OWNER	"staff"			/* default owner */

static int	docopy, dostrip,
		mode = 0755;
static char	*group = DEF_GROUP,
		*owner = DEF_OWNER;

main(argc, argv)
	int	argc;
	char	**argv;
{
	extern char	*optarg, *sys_errlist[];
	extern int	optind, errno;
	register int	to_fd;
	struct stat	from_sb, to_sb;
	struct passwd	*pp;
	struct group	*gp;
	int	ch;
	char	*path, pbuf[MAXPATHLEN];

	while ((ch = getopt(argc, argv, "cg:m:o:s")) != EOF)
		switch((char)ch) {
		case 'c':
			docopy = YES;
			break;
		case 'g':
			group = optarg;
			break;
		case 'm':
			mode = atoo(optarg);
			break;
		case 'o':
			owner = optarg;
			break;
		case 's':
			dostrip = YES;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc != 2)
		usage();

	/* check source */
	if (stat(argv[0], &from_sb)) {
		fprintf(stderr, "install: fstat: %s: %s\n", argv[0], sys_errlist[errno]);
		exit(1);
	}
	if (!(from_sb.st_mode & S_IFREG)) {
		fprintf(stderr, "install: %s isn't a regular file.\n", argv[0]);
		exit(1);
	}

	/* build target path, find out if target is same as source */
	if (!stat(path = argv[1], &to_sb)) {
		if (to_sb.st_mode & S_IFDIR) {
			(void)sprintf(path = pbuf, "%s/%s", argv[1], argv[0]);
			if (stat(path, &to_sb))
				goto nocompare;
			if (to_sb.st_mode & S_IFDIR) {
				fprintf(stderr, "install: %s is a directory.\n", path);
				exit(1);
			}
		}
		if (to_sb.st_dev == from_sb.st_dev && to_sb.st_ino == from_sb.st_ino) {
			fprintf(stderr, "install: %s and %s are the same file.\n", argv[0], path);
			exit(1);
		}
		/* unlink now... avoid ETXTBSY errors later */
		(void)unlink(path);
	}

nocompare:
	/* get group and owner id's */
	if (!(gp = getgrnam(group))) {
		fprintf(stderr, "install: unknown group %s.\n", group);
		exit(1);
	}
	if (!(pp = getpwnam(owner))) {
		fprintf(stderr, "install: unknown user %s.\n", owner);
		exit(1);
	}

	/* open target, set mode, owner, group */
	if ((to_fd = open(path, O_CREAT|O_WRONLY|O_TRUNC, 0)) < 0) {
		fprintf(stderr, "install: %s: %s\n", path, sys_errlist[errno]);
		exit(1);
	}
	if (fchmod(to_fd, mode)) {
		fprintf(stderr, "install: fchmod: %s: %s\n", path, sys_errlist[errno]);
		(void)unlink(path);
		exit(1);
	}
	if (fchown(to_fd, pp->pw_uid, gp->gr_gid)) {
		fprintf(stderr, "install: fchown: %s: %s\n", path, sys_errlist[errno]);
		(void)unlink(path);
		exit(1);
	}

	if (dostrip) {
		strip(to_fd, argv[0], path);
		if (docopy)
			exit(0);
	}
	else if (docopy) {
		copy(argv[0], to_fd, path);
		exit(0);
	}
	else if (rename(argv[0], path))
		copy(argv[0], to_fd, path);
	(void)unlink(argv[0]);
	exit(0);
}

/*
 * copy --
 *	copy from one file to another
 */
static
copy(from_name, to_fd, to_name)
	register int	to_fd;
	char	*from_name, *to_name;
{
	register int	n, from_fd;
	char	buf[MAXBSIZE];

	if ((from_fd = open(from_name, O_RDONLY, 0)) < 0) {
		fprintf(stderr, "install: open: %s: %s\n", from_name, sys_errlist[errno]);
		(void)unlink(to_name);
		exit(1);
	}
	while ((n = read(from_fd, buf, sizeof(buf))) > 0)
		if (write(to_fd, buf, n) != n) {
			fprintf(stderr, "install: write: %s: %s\n", to_name, sys_errlist[errno]);
			(void)unlink(to_name);
		}
	if (n == -1) {
		fprintf(stderr, "install: read: %s: %s\n", to_name, sys_errlist[errno]);
		(void)unlink(to_name);
	}
}

/*
 * strip --
 *	copy file, strip(1)'ing it at the same time
 */
static
strip(to_fd, from_name, to_name)
	register int	to_fd;
	char	*from_name, *to_name;
{
	typedef struct exec	EXEC;
	register long	size;
	register int	n, from_fd;
	EXEC	head;
	char	buf[MAXBSIZE];

	if ((from_fd = open(from_name, O_RDONLY, 0)) < 0) {
		fprintf(stderr, "install: open: %s: %s\n", from_name, sys_errlist[errno]);
		(void)unlink(to_name);
		exit(1);
	}
	if (read(from_fd, (char *)&head, sizeof(head)) < 0 || N_BADMAG(head)) {
		fprintf(stderr, "install: %s not in a.out format.\n", from_name);
		(void)unlink(to_name);
		exit(1);
	}
	if (head.a_syms || head.a_trsize || head.a_drsize) {
		size = (long)head.a_text + head.a_data;
		head.a_syms = head.a_trsize = head.a_drsize = 0;
		if (head.a_magic == ZMAGIC)
			size += getpagesize() - sizeof(EXEC);
		if (write(to_fd, (char *)&head, sizeof(EXEC)) != sizeof(EXEC)) {
			fprintf(stderr, "install: write: %s: %s\n", to_name, sys_errlist[errno]);
			(void)unlink(to_name);
			exit(1);
		}
		for (; size; size -= n)
			if ((n = read(from_fd, buf, (int)MIN(size, sizeof(buf)))) <= 0)
				break;
			else if (write(to_fd, buf, n) != n) {
				fprintf(stderr, "install: write: %s: %s\n", to_name, sys_errlist[errno]);
				(void)unlink(to_name);
				exit(1);
			}
		if (n == -1) {
			fprintf(stderr, "install: read: %s: %s\n", from_name, sys_errlist[errno]);
			(void)unlink(to_name);
			exit(1);
		}
	}
}

/*
 * atoo --
 *	octal string to int
 */
static
atoo(C)
	register char	*C;		/* argument string */
{
	register int	val;		/* return value */

	for (val = 0; isdigit(*C); ++C)
		val = val * 8 + *C - '0';
	return(val);
}

/*
 * usage --
 *	print a usage message and die
 */
static
usage()
{
	fputs("usage: install [-cs] [-g group] [-m mode] [-o owner] source destination\n", stderr);
	exit(1);
}
