/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mailstats.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include <sys/file.h>
#include <sendmail.h>
#include <mailstats.h>
#include <pathnames.h>

#define MNAMELEN	20	/* max length of mailer name */

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	struct statistics stat;
	register int i;
	int mno;
	int ch, fd;
	char *sfile;
	char *cfile;
	FILE *cfp;
	bool mnames;
	char mtable[MAXMAILERS][MNAMELEN+1];
	char sfilebuf[100];
	char buf[MAXLINE];
	extern char *ctime();

	cfile = _PATH_SENDMAILCF;
	sfile = NULL;
	mnames = TRUE;
	while ((ch = getopt(argc, argv, "C:f:o")) != EOF)
	{
		switch (ch)
		{
		  case 'C':
			cfile = optarg;
			break;

		  case 'f':
			sfile = optarg;
			break;

		  case 'o':
			mnames = FALSE;
			break;

		  case '?':
		  default:
  usage:
			fputs("usage: mailstats [-C cffile] [-f stfile]\n", stderr);
			exit(EX_USAGE);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		goto usage;

	if ((cfp = fopen(cfile, "r")) == NULL)
	{
		fprintf(stderr, "mailstats: ");
		perror(cfile);
		exit(EX_NOINPUT);
	}

	mno = 0;
	(void) strcpy(mtable[mno++], "prog");
	(void) strcpy(mtable[mno++], "*file*");
	(void) strcpy(mtable[mno++], "*include*");

	while (fgets(buf, sizeof(buf), cfp) != NULL)
	{
		register char *b;
		char *s;
		register char *m;

		b = buf;
		switch (*b++)
		{
		  case 'M':		/* mailer definition */
			break;

		  case 'O':		/* option -- see if .st file */
			if (*b++ != 'S')
				continue;

			/* yep -- save this */
			strcpy(sfilebuf, b);
			b = strchr(sfilebuf, '\n');
			if (b != NULL)
				*b = '\0';
			if (sfile == NULL)
				sfile = sfilebuf;

		  default:
			continue;
		}

		if (mno >= MAXMAILERS)
		{
			fprintf(stderr,
				"Too many mailers defined, %d max.\n",
				MAXMAILERS);
			exit(EX_SOFTWARE);
		}
		m = mtable[mno];
		s = m + MNAMELEN;		/* is [MNAMELEN+1] */
		while (*b != ',' && !isspace(*b) && *b != '\0' && m < s)
			*m++ = *b++;
		*m = '\0';
		for (i = 0; i < mno; i++)
		{
			if (strcmp(mtable[i], mtable[mno]) == 0)
				break;
		}
		if (i == mno)
			mno++;
	}
	(void) fclose(cfp);
	for (; mno < MAXMAILERS; mno++)
		mtable[mno][0]='\0';

	if (sfile == NULL)
	{
		fprintf(stderr, "mailstats: no statistics file located\n");
		exit (EX_OSFILE);
	}

	if ((fd = open(sfile, O_RDONLY)) < 0) {
		fputs("mailstats: ", stderr);
		perror(sfile);
		exit(EX_NOINPUT);
	}
	if (read(fd, &stat, sizeof(stat)) != sizeof(stat) ||
	    stat.stat_size != sizeof(stat))
	{
		fputs("mailstats: file size changed.\n", stderr);
		exit(EX_OSERR);
	}

	printf("Statistics from %s", ctime(&stat.stat_itime));
	printf(" M msgsfr bytes_from  msgsto   bytes_to%s\n",
		mnames ? "  Mailer" : "");
	for (i = 0; i < MAXMAILERS; i++)
	{
		if (stat.stat_nf[i] || stat.stat_nt[i])
		{
			printf("%2d %6ld %10ldK %6ld %10ldK", i,
			    stat.stat_nf[i], stat.stat_bf[i],
			    stat.stat_nt[i], stat.stat_bt[i]);
			if (mnames)
				printf("  %s", mtable[i]);
			printf("\n");
		}
	}
	exit(EX_OK);
}
