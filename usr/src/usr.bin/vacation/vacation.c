# include <pwd.h>
# include <stdio.h>
# include <sysexits.h>
# include "useful.h"
# include "userdbm.h"

static char	SccsId[] =	"@(#)vacation.c	3.2	%G%";

/*
**  VACATION -- return a message to the sender when on vacation.
**
**	This program could be invoked as a message receiver
**	when someone is on vacation.  It returns a message
**	specified by the user to whoever sent the mail, taking
**	care not to return a message too often to prevent
**	"I am on vacation" loops.
**
**	Positional Parameters:
**		the user to send to.
**
**	Flag Parameters:
**		-I	initialize the database.
**
**	Side Effects:
**		A message is sent back to the sender.
**
**	Author:
**		Eric Allman
**		UCB/INGRES
*/

# define MAXLINE	256	/* max size of a line */
# define MAXNAME	128	/* max size of one name */

struct dbrec
{
	long	sentdate;
};

main(argc, argv)
	char **argv;
{
	char *from;
	register char *p;
	struct passwd *pw;
	register char *homedir;
	char buf[MAXLINE];
	extern struct passwd *getpwnam();
	extern char *newstr();
	extern char *getfrom();
	extern bool knows();

	/* process arguments */
	while (--argc > 0 && (p = *++argv) != NULL && *p == '-')
	{
		switch (*++p)
		{
		  case 'I':	/* initialize */
			homedir = getenv("HOME");
			if (homedir == NULL)
				syserr("No home!");
			strcpy(buf, homedir);
			strcat(buf, "/.vacation.dir");
			if (close(creat(buf, 0644)) < 0)
				syserr("Cannot create %s", buf);
			strcpy(buf, homedir);
			strcat(buf, "/.vacation.pag");
			if (close(creat(buf, 0644)) < 0)
				syserr("Cannot create %s", buf);
			exit(EX_OK);

		  default:
			usrerr("Unknown flag -%s", p);
			exit(EX_USAGE);
		}
	}

	/* verify recipient argument */
	if (argc != 1)
	{
		usrerr("Usage: vacation username (or) vacation -I");
		exit(EX_USAGE);
	}

	/* find user's home directory */
	pw = getpwnam(p);
	if (pw == NULL)
	{
		usrerr("Unknown user %s", p);
		exit(EX_NOUSER);
	}
	homedir = newstr(pw->pw_dir);
	strcpy(buf, homedir);
	strcat(buf, "/.vacation");
	dbminit(buf);

	/* read message from standard input (just from line) */
	from = getfrom();

	/* check if this person is already informed */
	if (!knows(from))
	{
		/* mark this person as knowing */
		setknows(from, buf);

		/* send the message back */
		strcat(buf, ".msg");
		sendmessage(buf, from);
		/* never returns */
	}
	exit (EX_OK);
}
/*
**  GETFROM -- read message from standard input and return sender
**
**	Parameters:
**		none.
**
**	Returns:
**		pointer to the sender address.
**
**	Side Effects:
**		Reads first line from standard input.
*/

char *
getfrom()
{
	static char line[MAXLINE];
	register char *p;

	/* read the from line */
	if (fgets(line, sizeof line, stdin) == NULL ||
	    strncmp(line, "From ", 5) != NULL)
	{
		usrerr("No initial From line");
		exit(EX_USAGE);
	}

	/* find the end of the sender address and terminate it */
	p = index(&line[5], ' ');
	if (p == NULL)
	{
		usrerr("Funny From line '%s'", line);
		exit(EX_USAGE);
	}
	*p = '\0';

	/* return the sender address */
	return (&line[5]);
}
/*
**  KNOWS -- predicate telling if user has already been informed.
**
**	Parameters:
**		user -- the user who sent this message.
**
**	Returns:
**		TRUE if 'user' has already been informed that the
**			recipient is on vacation.
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

# define ONEWEEK	(60L*60L*24L*7L)

bool
knows(user)
	char *user;
{
	DATUM k, d;
	long now;

	time(&now);
	k.dptr = user;
	k.dsize = strlen(user) + 1;
	d = fetch(k);
	if (d.dptr == NULL || ((struct dbrec *) d.dptr)->sentdate + ONEWEEK < now)
		return (FALSE);
	return (TRUE);
}
/*
**  SETKNOWS -- set that this user knows about the vacation.
**
**	Parameters:
**		user -- the user who should be marked.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The dbm file is updated as appropriate.
*/

setknows(user)
	char *user;
{
	register int i;
	DATUM k, d;
	struct dbrec xrec;

	k.dptr = user;
	k.dsize = strlen(user) + 1;
	time(&xrec.sentdate);
	d.dptr = (char *) &xrec;
	d.dsize = sizeof xrec;
	store(k, d);
}
/*
**  SENDMESSAGE -- send a message to a particular user.
**
**	Parameters:
**		msgf -- filename containing the message.
**		user -- user who should receive it.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sends mail to 'user' using /usr/lib/sendmail.
*/

sendmessage(msgf, user)
	char *msgf;
	char *user;
{
	FILE *f;

	/* find the message to send */
	f = freopen(msgf, "r", stdin);
	if (f == NULL)
	{
		f = freopen("/usr/lib/vacation.def", "r", stdin);
		if (f == NULL)
			syserr("No message to send");
	}

	execl("/usr/lib/sendmail", "sendmail", "-n", user, NULL);
	syserr("Cannot exec /usr/lib/sendmail");
}
/*
**  USRERR -- print user error
**
**	Parameters:
**		f -- format.
**		p -- first parameter.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

usrerr(f, p)
	char *f;
	char *p;
{
	fprintf(stderr, "vacation: ");
	_doprnt(f, &p, stderr);
	fprintf(stderr, "\n");
}
/*
**  SYSERR -- print system error
**
**	Parameters:
**		f -- format.
**		p -- first parameter.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

syserr(f, p)
	char *f;
	char *p;
{
	fprintf(stderr, "vacation: ");
	_doprnt(f, &p, stderr);
	fprintf(stderr, "\n");
	exit(EX_USAGE);
}
