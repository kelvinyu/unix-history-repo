# include "sendmail.h"

static char	SccsId[] =	"@(#)srvrsmtp.c	3.10	%G%";

/*
**  SMTP -- run the SMTP protocol.
**
**	Parameters:
**		none.
**
**	Returns:
**		never.
**
**	Side Effects:
**		Reads commands from the input channel and processes
**			them.
*/

struct cmd
{
	char	*cmdname;	/* command name */
	int	cmdcode;	/* internal code, see below */
};

/* values for cmdcode */
# define CMDERROR	0	/* bad command */
# define CMDMAIL	1	/* mail -- designate sender */
# define CMDRCPT	2	/* rcpt -- designate recipient */
# define CMDDATA	3	/* data -- send message text */
# define CMDRSET	5	/* rset -- reset state */
# define CMDVRFY	6	/* vrfy -- verify address */
# define CMDHELP	7	/* help -- give usage info */
# define CMDNOOP	8	/* noop -- do nothing */
# define CMDQUIT	9	/* quit -- close connection and die */
# define CMDMRSQ	10	/* mrsq -- for old mtp compat only */
# define CMDHELO	11	/* helo -- be polite */
# define CMDDBGSHOWQ	12	/* showq -- show send queue (DEBUG) */

static struct cmd	CmdTab[] =
{
	"mail",		CMDMAIL,
	"rcpt",		CMDRCPT,
	"mrcp",		CMDRCPT,	/* for old MTP compatability */
	"data",		CMDDATA,
	"rset",		CMDRSET,
	"vrfy",		CMDVRFY,
	"expn",		CMDVRFY,
	"help",		CMDHELP,
	"noop",		CMDNOOP,
	"quit",		CMDQUIT,
	"mrsq",		CMDMRSQ,
	"helo",		CMDHELO,
# ifdef DEBUG
	"showq",	CMDDBGSHOWQ,
# endif DEBUG
	NULL,		CMDERROR,
};

smtp()
{
	char inp[MAXLINE];
	register char *p;
	struct cmd *c;
	char *cmd;
	extern char *skipword();
	extern bool sameword();
	bool hasmail;			/* mail command received */
	int rcps;			/* number of recipients */
	extern ADDRESS *sendto();
	ADDRESS *a;

	hasmail = FALSE;
	rcps = 0;
	message("220", "%s Sendmail at your service", HostName);
	for (;;)
	{
		To = NULL;
		Errors = 0;
		if (fgets(inp, sizeof inp, InChannel) == NULL)
		{
			/* end of file, just die */
			message("421", "%s Lost input channel", HostName);
			finis();
		}

		/* clean up end of line */
		fixcrlf(inp, TRUE);

		/* echo command to transcript */
		fprintf(Xscript, "*** %s\n", inp);

		/* break off command */
		for (p = inp; isspace(*p); p++)
			continue;
		cmd = p;
		while (*++p != '\0' && !isspace(*p))
			continue;
		if (*p != '\0')
			*p++ = '\0';

		/* decode command */
		for (c = CmdTab; c->cmdname != NULL; c++)
		{
			if (sameword(c->cmdname, cmd))
				break;
		}

		/* process command */
		switch (c->cmdcode)
		{
		  case CMDHELO:		/* hello -- introduce yourself */
			message("250", "%s Hello %s, pleased to meet you", HostName, p);
			break;

		  case CMDMAIL:		/* mail -- designate sender */
			if (hasmail)
			{
				message("503", "Sender already specified");
				break;
			}
			p = skipword(p, "from");
			if (p == NULL)
				break;
			if (index(p, ',') != NULL)
			{
				message("501", "Source routing not implemented");
				Errors++;
				break;
			}
			setsender(p);
			if (Errors == 0)
			{
				message("250", "Sender ok");
				hasmail = TRUE;
			}
			break;

		  case CMDRCPT:		/* rcpt -- designate recipient */
			p = skipword(p, "to");
			if (p == NULL)
				break;
			if (index(p, ',') != NULL)
			{
				message("501", "Source routing not implemented");
				Errors++;
				break;
			}
			a = sendto(p, 1, (ADDRESS *) NULL, 0);
# ifdef DEBUG
			if (Debug > 1)
				printaddr(a, TRUE);
# endif DEBUG
			if (Errors == 0)
			{
				message("250", "Recipient ok");
				rcps++;
			}
			break;

		  case CMDDATA:		/* data -- text of mail */
			if (!hasmail)
			{
				message("503", "Need MAIL command");
				break;
			}
			else if (rcps <= 0)
			{
				message("503", "Need RCPT (recipient)");
				break;
			}

			/* collect the text of the message */
			collect(TRUE);
			if (Errors != 0)
				break;

			/* if sending to multiple people, mail back errors */
			if (rcps != 1)
				HoldErrs = MailBack = TRUE;

			/* send to all recipients */
			sendall(FALSE);

			/* reset strange modes */
			HoldErrs = FALSE;
			To = NULL;

			/* issue success if appropriate */
			if (Errors == 0 || rcps != 1)
				message("250", "Sent");
			break;

		  case CMDRSET:		/* rset -- reset state */
			message("250", "Reset state");
			finis();

		  case CMDVRFY:		/* vrfy -- verify address */
				paddrtree(a);
			break;

		  case CMDHELP:		/* help -- give user info */
			if (*p == '\0')
				p = "SMTP";
			help(p);
			break;

		  case CMDNOOP:		/* noop -- do nothing */
			message("200", "OK");
			break;

		  case CMDQUIT:		/* quit -- leave mail */
			message("221", "%s closing connection", HostName);
			finis();

		  case CMDMRSQ:		/* mrsq -- negotiate protocol */
			if (*p == 'R' || *p == 'T')
			{
				/* recipients first or text first */
				message("200", "%c ok, please continue", *p);
			}
			else if (*p == '?')
			{
				/* what do I prefer?  anything, anytime */
				message("215", "R Recipients first is my choice");
			}
			else if (*p == '\0')
			{
				/* no meaningful scheme */
				message("200", "okey dokie boobie");
			}
			else
			{
				/* bad argument */
				message("504", "Scheme unknown");
			}
			break;

# ifdef DEBUG
		  case CMDDBGSHOWQ:	/* show queues */
			printf("SendQueue=");
			printaddr(SendQueue, TRUE);
			break;
# endif DEBUG

		  case CMDERROR:	/* unknown command */
			message("500", "Command unrecognized");
			break;

		  default:
			syserr("smtp: unknown code %d", c->cmdcode);
			break;
		}
	}
}
/*
**  SKIPWORD -- skip a fixed word.
**
**	Parameters:
**		p -- place to start looking.
**		w -- word to skip.
**
**	Returns:
**		p following w.
**		NULL on error.
**
**	Side Effects:
**		clobbers the p data area.
*/

static char *
skipword(p, w)
	register char *p;
	char *w;
{
	register char *q;
	extern bool sameword();

	/* find beginning of word */
	while (isspace(*p))
		p++;
	q = p;

	/* find end of word */
	while (*p != '\0' && *p != ':' && !isspace(*p))
		p++;
	while (isspace(*p))
		*p++ = '\0';
	if (*p != ':')
	{
	  syntax:
		message("501", "Syntax error");
		Errors++;
		return (NULL);
	}
	*p++ = '\0';
	while (isspace(*p))
		p++;

	/* see if the input word matches desired word */
	if (!sameword(q, w))
		goto syntax;

	return (p);
}
/*
**  HELP -- implement the HELP command.
**
**	Parameters:
**		topic -- the topic we want help for.
**
**	Returns:
**		none.
**
**	Side Effects:
**		outputs the help file to message output.
*/

help(topic)
	char *topic;
{
	register FILE *hf;
	int len;
	char buf[MAXLINE];
	bool noinfo;
	extern char *HelpFile;

	hf = fopen(HelpFile, "r");
	if (hf == NULL)
	{
		/* no help */
		message("502", "HELP not implemented");
		return;
	}

	len = strlen(topic);
	makelower(topic);
	noinfo = TRUE;

	while (fgets(buf, sizeof buf, hf) != NULL)
	{
		if (strncmp(buf, topic, len) == 0)
		{
			register char *p;

			p = index(buf, '\t');
			if (p == NULL)
				p = buf;
			else
				p++;
			fixcrlf(p, TRUE);
			message("214-", p);
			noinfo = FALSE;
		}
	}

	if (noinfo)
		message("504", "HELP topic unknown");
	else
		message("214", "End of HELP info");
	(void) fclose(hf);
}
/*
**  PADDRTREE -- print address tree
**
**	Used by VRFY and EXPD to dump the tree of addresses produced.
**
**	Parameters:
**		a -- address of root.
**
**	Returns:
**		none.
**
**	Side Effects:
**		prints the tree in a nice order.
*/

paddrtree(a)
	register ADDRESS *a;
{
	static ADDRESS *prev;
	static int lev;

	if (a == NULL)
		return;
	lev++;
	if (!bitset(QDONTSEND, a->q_flags))
	{
		if (prev != NULL)
		{
			if (prev->q_fullname != NULL)
				message("250-", "%s <%s>", prev->q_fullname, prev->q_paddr);
			else
				message("250-", "<%s>", prev->q_paddr);
		}
		prev = a;
	}
	paddrtree(a->q_child);
	paddrtree(a->q_sibling);
	if (--lev <= 0)
	{
		if (prev != NULL)
		{
			/* last one */
			if (prev->q_fullname != NULL)
				message("250", "%s <%s>", prev->q_fullname, prev->q_paddr);
			else
				message("250", "<%s>", prev->q_paddr);
			prev = NULL;
		}
		else
			message("550", "User unknown");
	}
}
