/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)users.c	5.4 (Berkeley) %G%";
#endif not lint

/*
 * users
 */
#include <sys/types.h>
#include <utmp.h>
#include <stdio.h>

#define ERREXIT		1
#define OKEXIT		0
#define NMAX		sizeof(utmp.ut_name)
#define MAXUSERS	200
#define UTMP_FILE	"/etc/utmp"

static struct utmp	utmp;		/* read structure */
static int	ncnt;			/* count of names */
static char	*names[MAXUSERS];	/* names table */

main()
{
	register FILE	*fp;		/* file pointer */

	if (!(fp = fopen(UTMP_FILE,"r"))) {
		perror(UTMP_FILE);
		exit(ERREXIT);
	}
	while (fread((char *)&utmp,sizeof(utmp),1,fp) == 1)
		if (*utmp.ut_name) {
			if (++ncnt > MAXUSERS) {
				ncnt = MAXUSERS;
				fputs("users: too many users.\n",stderr);
				break;
			}
			nsave();
		}
	summary();
	exit(OKEXIT);
}

nsave()
{
	static char	**namp = names;		/* pointer to names table */
	char	*calloc();

	if (!(*namp = calloc((u_int)(NMAX + 1),sizeof(char)))) {
		fputs("users: malloc error.\n",stderr);
		exit(ERREXIT);
	}
	bcopy(utmp.ut_name,*namp++,NMAX);
}

summary()
{
	register char	**p;
	int	scmp();

	if (!ncnt)
		return;
	qsort((char *)names,ncnt,sizeof(names[0]),scmp);
	fputs(names[0],stdout);
	for (p = &names[1];--ncnt;++p) {
		putchar(' ');
		fputs(*p,stdout);
	}
	putchar('\n');
}

scmp(p,q)
char	**p,
	**q;
{
	return(strcmp(*p,*q));
}
