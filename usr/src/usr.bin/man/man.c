/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1987, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)man.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pathnames.h"

extern int errno;

int f_all, f_cat, f_how, f_where;
char *command, *machine, *p_augment, *p_path, *pager, *progname;
extern char **arorder, *pathbuf;

main(argc, argv)
	int argc;
	register char **argv;
{
	extern char *optarg;
	extern int optind;
	int ch, res;
	char *section[2], *check_pager(), *getpath(), **getorder(), *tmp;

	progname = "man";
	while ((ch = getopt(argc, argv, "-acfhkM:m:P:w")) != EOF)
		switch((char)ch) {
		case 'a':
			f_all = 1;
			break;
		case 'c':
		case '-':		/* deprecated */
			f_cat = 1;
			break;
		case 'h':
			f_how = 1;
			break;
		case 'm':
			p_augment = optarg;
			break;
		case 'M':
		case 'P':		/* backward compatibility */
			p_path = optarg;
			break;
		/*
		 * "man -f" and "man -k" are backward compatible, undocumented
		 * ways of calling whatis(1) and apropos(1).
		 */
		case 'f':
			jump(argv, "-f", "whatis");
			/* NOTREACHED */
		case 'k':
			jump(argv, "-k", "apropos");
			/* NOTREACHED */
		case 'w':
			f_all = f_where = 1;
			break;
		case '?':
		default:
			usage();
		}
	argv += optind;

	if (!*argv)
		usage();

	if (!f_cat && !f_how)
		if (!isatty(1))
			f_cat = 1;
		else if (pager = getenv("PAGER"))
			pager = check_pager(pager);
		else
			pager = _PATH_PAGER;

	if (!(machine = getenv("MACHINE")))
		machine = MACHINE;

	/* see if checking in a specific section */
	if (argc > 1 && getsection(*argv)) {
		section[0] = *argv++;
		section[1] = (char *)NULL;
	} else {
		section[0] = "_default";
		section[1] = (char *)NULL;
	}

	arorder = getorder();
	if (p_path || (p_path = getenv("MANPATH"))) {
		char buf[MAXPATHLEN], **av;

		tmp = strtok(p_path, ":");
		while (tmp) {
			(void)snprintf(buf, sizeof(buf), "%s/", tmp);
			for (av = arorder; *av; ++av)
				cadd(buf, strlen(buf), *av);
			tmp = strtok(NULL, ":");
		}
		p_path = pathbuf;
	} else if (!(p_path = getpath(section)) && !p_augment) {
		(void)fprintf(stderr,
			"man: no place to search for those manual pages.\n");
		exit(1);
	}

	for (; *argv; ++argv) {
		if (p_augment)
			res = manual(p_augment, *argv);
		res = manual(p_path, *argv);
		if (!res && !f_where)
			(void)fprintf(stderr,
			    "man: no entry for %s in the manual.\n", *argv);
	}

	/* use system(3) in case someone's pager is "pager arg1 arg2" */
	if (command)
		(void)system(command);
	exit(0);
}

/*
 * manual --
 *	given a path, a directory list and a file name, find a file
 *	that matches; check ${directory}/${dir}/{file name} and
 *	${directory}/${dir}/${machine}/${file name}.
 */
manual(path, name)
	char *path, *name;
{
	register int res;
	register char *cp;
	char fname[MAXPATHLEN + 1];

	for (res = 0; path != NULL && *path != '\0'; path = cp) {
		if (cp = strchr(path, ':')) {
			if (cp == path + 1) {		/* foo::bar */
				++cp;
				continue;
			}
			*cp = '\0';
		}
		(void)snprintf(fname, sizeof(fname), "%s/%s.0", path, name);
		if (access(fname, R_OK)) {
			(void)snprintf(fname, sizeof(fname),
			    "%s/%s/%s.0", path, machine, name);
			if (access(fname, R_OK)) {
				if (cp != NULL)
					*cp++ = ':';
				continue;
			}
		}

		if (f_where)
			(void)printf("man: found in %s.\n", fname);
		else if (f_cat)
			cat(fname);
		else if (f_how)
			how(fname);
		else
			add(fname);
		if (!f_all)
			return(1);
		res = 1;
		if (cp != NULL)
			*cp++ = ':';
	}
	return(res);
}

/*
 * how --
 *	display how information
 */
how(fname)
	char *fname;
{
	register FILE *fp;

	register int lcnt, print;
	register char *p;
	char buf[BUFSIZ];

	if (!(fp = fopen(fname, "r"))) {
		(void)fprintf(stderr, "man: %s: %s\n", fname, strerror(errno));
		exit(1);
	}
#define	S1	"SYNOPSIS"
#define	S2	"S\bSY\bYN\bNO\bOP\bPS\bSI\bIS\bS"
#define	D1	"DESCRIPTION"
#define	D2	"D\bDE\bES\bSC\bCR\bRI\bIP\bPT\bTI\bIO\bON\bN"
	for (lcnt = print = 0; fgets(buf, sizeof(buf), fp);) {
		if (!strncmp(buf, S1, sizeof(S1) - 1) ||
		    !strncmp(buf, S2, sizeof(S2) - 1)) {
			print = 1;
			continue;
		} else if (!strncmp(buf, D1, sizeof(D1) - 1) ||
		    !strncmp(buf, D2, sizeof(D2) - 1))
			return;
		if (!print)
			continue;
		if (*buf == '\n')
			++lcnt;
		else {
			for(; lcnt; --lcnt)
				(void)putchar('\n');
			for (p = buf; isspace(*p); ++p);
			(void)fputs(p, stdout);
		}
	}
	(void)fclose(fp);
}
/*
 * cat --
 *	cat out the file
 */
cat(fname)
	char *fname;
{
	register int fd, n;
	char buf[BUFSIZ];

	if ((fd = open(fname, O_RDONLY, 0)) < 0) {
		(void)fprintf(stderr, "man: %s: %s\n", fname, strerror(errno));
		exit(1);
	}
	while ((n = read(fd, buf, sizeof(buf))) > 0)
		if (write(1, buf, n) != n) {
			(void)fprintf(stderr,
			    "man: write: %s\n", strerror(errno));
			exit(1);
		}
	if (n == -1) {
		(void)fprintf(stderr, "man: read: %s\n", strerror(errno));
		exit(1);
	}
	(void)close(fd);
}

/*
 * add --
 *	add a file name to the list for future paging
 */
add(fname)
	char *fname;
{
	static u_int buflen;
	static int len;
	static char *cp;
	int flen;

	if (!command) {
		if (!(command = malloc(buflen = 1024)))
			enomem();
		len = strlen(strcpy(command, pager));
		cp = command + len;
	}
	flen = strlen(fname);
	if (len + flen + 2 > buflen) {		/* +2 == space, EOS */
		if (!(command = realloc(command, buflen += 1024)))
			enomem();
		cp = command + len;
	}
	*cp++ = ' ';
	len += flen + 1;			/* +1 = space */
	(void)strcpy(cp, fname);
	cp += flen;
}

/*
 * check_pager --
 *	check the user supplied page information
 */
char *
check_pager(name)
	char *name;
{
	register char *p;
	char *save;

	/*
	 * if the user uses "more", we make it "more -s"; watch out for
	 * PAGER = "mypager /usr/ucb/more"
	 */
	for (p = name; *p && !isspace(*p); ++p);
	for (; p > name && *p != '/'; --p);
	if (p != name)
		++p;

	/* make sure it's "more", not "morex" */
	if (!strncmp(p, "more", 4) && (!p[4] || isspace(p[4]))){
		save = name;
		/* allocate space to add the "-s" */
		if (!(name =
		    malloc((u_int)(strlen(save) + sizeof("-s") + 1))))
			enomem();
		(void)sprintf(name, "%s %s", save, "-s");
	}
	return(name);
}

/*
 * jump --
 *	strip out flag argument and jump
 */
jump(argv, flag, name)
	char **argv, *name;
	register char *flag;
{
	register char **arg;

	argv[0] = name;
	for (arg = argv + 1; *arg; ++arg)
		if (!strcmp(*arg, flag))
			break;
	for (; *arg; ++arg)
		arg[0] = arg[1];
	execvp(name, argv);
	(void)fprintf(stderr, "%s: Command not found.\n", name);
	exit(1);
}

/*
 * usage --
 *	print usage message and die
 */
usage()
{
	(void)fprintf(stderr,
	    "usage: man [-ac] [-M path] [-m path] [section] title ...\n");
	exit(1);
}
