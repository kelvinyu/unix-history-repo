# include <stdio.h>
# include <sgtty.h>
# include <signal.h>

static char	SccsId[] =	"@(#)cpr.c	1.5		%G%";

/*
**  CPR -- print on concept 108
**
**	This filter arranges to output onto a printer connected
**	to a Concept 108 terminal.  It probably works on other
**	models in the Concept 100 series also.
**
**	Usage:
**		cpr [-f] [file ...]
**
**	Flags:
**		-f	form feed following to print.
*/

typedef char	bool;
#define TRUE	1
#define FALSE	0

struct sgttyb	tbuf;
int		SysLine;
bool		FormFeedFollowing;

main(argc, argv)
	int argc;
	char **argv;
{
	register char *p;
	extern cleanterm();
	extern char *getenv();

	/* arrange to stop the sysline program during printing */
	p = getenv("SYSLINE");
	if (p != NULL)
		SysLine = atoi(p);

	/* process arguments */
	while (--argc > 0)
	{
		p = *++argv;
		if (*p != '-')
			break;
		switch (*++p)
		{
		  case 'f':
			FormFeedFollowing = TRUE;
			break;
		}
	}

	/* be nice on interrupts, etc. */
	signal(SIGINT, cleanterm);

	/* set the terminal to output to printer */
	setupterm();

	/* print the appropriate files */
	if (argc < 1)
		copyfile();
	else
	{
		while (argc-- > 0)
		{
			p = *argv++;
			if (freopen(p, "r", stdin) == NULL)
				perror(p);
			else
				copyfile();
		}
	}

	/* reset terminal to a nice state */
	cleanterm();
}

copyfile()
{
	char buf[200];
	register char *p;
	extern char *index();

	while (fgets(buf, sizeof buf, stdin) != NULL)
	{
		p = index(buf, '\n');
		if (p == NULL)
			continue;
		*p = '\r';
		printf("\033 5%s\033|", buf);
		if (getack() < 0)
		{
			fprintf(stderr, "Lost printer\n");
			cleanterm();
		}
		fputs("\n", stdout);
	}
}

setupterm()
{
	int oldflags;

	if (gtty(1, &tbuf) < 0)
	{
		perror("cpr: stdout");
		exit(1);
	}
	oldflags = tbuf.sg_flags;
	tbuf.sg_flags &= ~ECHO;
	tbuf.sg_flags |= CBREAK;
	stty(1, &tbuf);
	tbuf.sg_flags = oldflags;
	if (SysLine > 0)
		kill(SysLine, SIGSTOP);
}

cleanterm()
{
	/* output trailing formfeed */
	if (FormFeedFollowing)
		fputs("\r\f", stdout);

	/* disconnect printer */
	resetmodes();
	exit(0);
}

getack()
{
	char buf[1];

	fflush(stdout);
	if (read(2, buf, 1) <= 0 || buf[0] != '\006')
		return (-1);
	return (0);
}

resetmodes()
{
	stty(1, &tbuf);
	if (SysLine > 0)
		kill(SysLine, SIGCONT);
}
