#ifndef lint
static	char *sccsid = "@(#)main.c	4.4 (Berkeley) 83/10/10";
#endif

#include "defs.h"

/*
 * Remote distribution program.
 */

char	*distfile = "distfile";
char	tmpfile[] = "/tmp/rdistAXXXXXX";
char	*tmpname = &tmpfile[5];
char	*tmpinc = &tmpfile[10];

int	debug;		/* debugging flag */
int	nflag;		/* NOP flag, just print commands without executing */
int	qflag;		/* Quiet. Don't print messages */
int	vflag;		/* verify only */
int	yflag;		/* update iff remote younger than master */
int	iamremote;	/* act as remote server for transfering files */

int	filec;		/* number of files to update */
char	**filev;	/* list of files/directories to update */
FILE	*fin = NULL;	/* input file pointer */
int	rem = 0;	/* file descriptor to remote source/sink process */
char	host[32];	/* host name */
int	errs;		/* number of errors while sending/receiving */
char	user[10];	/* user's name */
char	homedir[128];	/* user's home directory */
int	userid;		/* user's user ID */
int	groupid;	/* user's group ID */

int	cleanup();
int	lostconn();

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *arg;
	register struct	passwd *pw;

	pw = getpwuid(userid = getuid());
	if (pw == NULL) {
		fprintf(stderr, "%s: Who are you?\n", argv[0]);
		exit(1);
	}
	strcpy(user, pw->pw_name);
	strcpy(homedir, pw->pw_dir);
	groupid = pw->pw_gid;
	gethostname(host, sizeof(host));

	while (--argc > 0) {
		if ((arg = *++argv)[0] != '-')
			break;
		if (!strcmp(arg, "-Server"))
			iamremote++;
		else while (*++arg)
			switch (*arg) {
			case 'f':
				if (--argc <= 0)
					usage();
				distfile = *++argv;
				if (distfile[0] == '-' && distfile[1] == '\0')
					fin = stdin;
				break;

			case 'd':
				if (--argc <= 0)
					usage();
				define(*++argv);
				break;

			case 'D':
				debug++;
				break;

			case 'n':
				nflag++;
				break;

			case 'q':
				qflag++;
				break;

			case 'v':
				vflag++;
				break;

			case 'y':
				yflag++;
				break;

			default:
				usage();
			}
	}

	mktemp(tmpfile);
	signal(SIGPIPE, lostconn);
	if (iamremote) {
		server();
		exit(errs);
	}

	signal(SIGHUP, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGTERM, cleanup);

	filec = argc;
	filev = argv;
	if (fin == NULL && (fin = fopen(distfile, "r")) == NULL) {
		perror(distfile);
		exit(1);
	}
	yyparse();

	exit(errs);
}

usage()
{
	printf("Usage: rdist [-f distfile] [-d var=value] [-nqyD] [file ...]\n");
	exit(1);
}

/*
 * Remove temporary files and do any cleanup operations before exiting.
 */
cleanup()
{
	(void) unlink(tmpfile);
	exit(1);
}

/*
 * Print a list of NAME blocks (mostly for debugging).
 */
prnames(bp)
	register struct block *bp;
{
	printf("( ");
	while (bp != NULL) {
		printf("%s ", bp->b_name);
		bp = bp->b_next;
	}
	printf(")\n");
}

/*VARARGS*/
warn(fmt, a1, a2,a3)
	char *fmt;
{
	extern int yylineno;

	fprintf(stderr, "rdist: line %d: Warning: ", yylineno);
	fprintf(stderr, fmt, a1, a2, a3);
	fputc('\n', stderr);
}
