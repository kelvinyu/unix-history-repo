/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)getNAME.c	5.6 (Berkeley) %G%";
#endif /* not lint */

/*
 * Get name sections from manual pages.
 *	-t	for building toc
 *	-i	for building intro entries
 *	other	apropos database
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tocrc;
int intro;
int typeflag;

void doname __P((char *));
void dorefname __P((char *));
void getfrom __P((char *));
void split __P((char *, char *));
void trimln __P((char *));
void usage __P((void));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern int optind;
	int ch;

	while ((ch = getopt(argc, argv, "itw")) != EOF)
		switch(ch) {
		case 'i':
			intro = 1;
			break;
		case 't':
			tocrc = 1;
			break;
		case 'w':
			typeflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (!*argv)
		usage();

	for (; *argv; ++argv)
		getfrom(*argv);
	exit(0);
}

void
getfrom(pathname)
	char *pathname;
{
	int i = 0;
	char *name, *loc;
	char headbuf[BUFSIZ];
	char linbuf[BUFSIZ];

	if (freopen(pathname, "r", stdin) == 0) {
		perror(pathname);
		return;
	}
	if (name = strrchr(pathname, '/'))
		name++;
	else
		name = pathname;
	for (;;) {
		if (fgets(headbuf, sizeof headbuf, stdin) == NULL) {
			if (typeflag)
				printf("%-60s	UNKNOWN\n", pathname);
			return;
		}
		if (headbuf[0] != '.')
			continue;
		if ((headbuf[1] == 'T' && headbuf[2] == 'H') ||
		    (headbuf[1] == 't' && headbuf[2] == 'h'))
			break;
		if (headbuf[1] == 'D' && headbuf[2] == 't') {
			if (typeflag) {
				printf("%-60s	NEW\n", pathname);
				return;
			}
			goto newman;
		}
	}
	if (typeflag) {
		printf("%-60s	OLD\n", pathname);
		return;
	}
	for (;;) {
		if (fgets(linbuf, sizeof linbuf, stdin) == NULL)
			return;
		if (linbuf[0] != '.')
			continue;
		if (linbuf[1] == 'S' && linbuf[2] == 'H')
			break;
		if (linbuf[1] == 's' && linbuf[2] == 'h')
			break;
	}
	trimln(headbuf);
	if (tocrc)
		doname(name);
	if (!intro)
		printf("%s\t", headbuf);
	linbuf[0] = '\0';
	for (;;) {
		if (fgets(headbuf, sizeof headbuf, stdin) == NULL)
			break;
		if (headbuf[0] == '.') {
			if (headbuf[1] == 'S' && headbuf[2] == 'H')
				break;
			if (headbuf[1] == 's' && headbuf[2] == 'h')
				break;
		}
		if (i != 0)
			strcat(linbuf, " ");
		i++;
		trimln(headbuf);
		strcat(linbuf, headbuf);
	}
	if (intro)
		split(linbuf, name);
	else
		printf("%s\n", linbuf);
	return;

newman:
	for (;;) {
		if (fgets(linbuf, sizeof linbuf, stdin) == NULL)
			return;
		if (linbuf[0] != '.')
			continue;
		if (linbuf[1] == 'S' && linbuf[2] == 'h')
			break;
	}
	trimln(headbuf);
	if (tocrc)
		doname(name);
	if (!tocrc && !intro)
		printf(".TH%s\t", &headbuf[3]);
	linbuf[0] = '\0';
	for (;;) {
		if (fgets(headbuf, sizeof headbuf, stdin) == NULL)
			break;
		if (headbuf[0] == '.') {
			if (headbuf[1] == 'S' && headbuf[2] == 'h')
				break;
		}
		if (i != 0)
			strcat(linbuf, " ");
		i++;
		trimln(headbuf);
		for (loc = headbuf; loc; loc = strchr(loc, ' '))
			if (loc[1] == ',')
				strcpy(loc, &loc[1]);
			else
				loc++;
		if (headbuf[0] != '.') {
			strcat(linbuf, headbuf);
		} else {
			if (headbuf[1] == 'N' && headbuf[2] == 'd')
				strcat(linbuf, "\\- ");
			strcat(linbuf, &headbuf[4]);
		}
	}
	if (intro)
		split(linbuf, name);
	else
		printf("%s\n", linbuf);
}

void
trimln(cp)
	register char *cp;
{

	while (*cp)
		cp++;
	if (*--cp == '\n')
		*cp = 0;
}

void
doname(name)
	char *name;
{
	register char *dp = name, *ep;

again:
	while (*dp && *dp != '.')
		putchar(*dp++);
	if (*dp)
		for (ep = dp+1; *ep; ep++)
			if (*ep == '.') {
				putchar(*dp++);
				goto again;
			}
	putchar('(');
	if (*dp)
		dp++;
	while (*dp)
		putchar (*dp++);
	putchar(')');
	putchar(' ');
}

void
split(line, name)
	char *line, *name;
{
	register char *cp, *dp;
	char *sp, *sep;

	cp = strchr(line, '-');
	if (cp == 0)
		return;
	sp = cp + 1;
	for (--cp; *cp == ' ' || *cp == '\t' || *cp == '\\'; cp--)
		;
	*++cp = '\0';
	while (*sp && (*sp == ' ' || *sp == '\t'))
		sp++;
	for (sep = "", dp = line; dp && *dp; dp = cp, sep = "\n") {
		cp = strchr(dp, ',');
		if (cp) {
			register char *tp;

			for (tp = cp - 1; *tp == ' ' || *tp == '\t'; tp--)
				;
			*++tp = '\0';
			for (++cp; *cp == ' ' || *cp == '\t'; cp++)
				;
		}
		printf("%s%s\t", sep, dp);
		dorefname(name);
		printf("\t%s", sp);
	}
}

void
dorefname(name)
	char *name;
{
	register char *dp = name, *ep;

again:
	while (*dp && *dp != '.')
		putchar(*dp++);
	if (*dp)
		for (ep = dp+1; *ep; ep++)
			if (*ep == '.') {
				putchar(*dp++);
				goto again;
			}
	putchar('.');
	if (*dp)
		dp++;
	while (*dp)
		putchar (*dp++);
}

void
usage()
{
	(void)fprintf(stderr, "usage: getNAME [-it] file ...\n");
	exit(1);
}
