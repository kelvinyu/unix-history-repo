# include <pwd.h>
# include "sendmail.h"

SCCSID(@(#)savemail.c	3.49		%G%);

/*
**  SAVEMAIL -- Save mail on error
**
**	If the MailBack flag is set, mail it back to the originator
**	together with an error message; otherwise, just put it in
**	dead.letter in the user's home directory (if he exists on
**	this machine).
**
**	Parameters:
**		e -- the envelope containing the message in error.
**
**	Returns:
**		none
**
**	Side Effects:
**		Saves the letter, by writing or mailing it back to the
**		sender, or by putting it in dead.letter in her home
**		directory.
*/

savemail(e)
	register ENVELOPE *e;
{
	register struct passwd *pw;
	register FILE *xfile;
	char buf[MAXLINE+1];
	extern struct passwd *getpwnam();
	register char *p;
	extern char *ttypath();
	static int exclusive;
	typedef int (*fnptr)();

# ifdef DEBUG
	if (tTd(6, 1))
		printf("\nsavemail: exclusive %d\n", exclusive);
# endif DEBUG

	if (exclusive++)
		return;
	if (e->e_class < 0)
	{
		message(Arpa_Info, "Dumping junk mail");
		return;
	}
	ForceMail = TRUE;
	e->e_flags &= ~EF_FATALERRS;

	/*
	**  In the unhappy event we don't know who to return the mail
	**  to, make someone up.
	*/

	if (CurEnv->e_returnto == NULL)
	{
		CurEnv->e_returnto = parse("root", (ADDRESS *) NULL, 0);
		if (CurEnv->e_returnto == NULL)
		{
			syserr("Cannot parse root!");
			ExitStat = EX_SOFTWARE;
			finis();
		}
	}
	e->e_to = NULL;

	/*
	**  If called from Eric Schmidt's network, do special mailback.
	**	Fundamentally, this is the mailback case except that
	**	it returns an OK exit status (assuming the return
	**	worked).
	**  Also, if the from address is not local, mail it back.
	*/

	if (BerkNet)
	{
		ExitStat = EX_OK;
		MailBack = TRUE;
	}
	if (!bitset(M_LOCAL, CurEnv->e_returnto->q_mailer->m_flags))
		MailBack = TRUE;

	/*
	**  If writing back, do it.
	**	If the user is still logged in on the same terminal,
	**	then write the error messages back to hir (sic).
	**	If not, set the MailBack flag so that it will get
	**	mailed back instead.
	*/

	if (WriteBack)
	{
		p = ttypath();
		if (p == NULL || freopen(p, "w", stdout) == NULL)
		{
			MailBack = TRUE;
			errno = 0;
		}
		else
		{
			if (Xscript != NULL)
				(void) fflush(Xscript);
			xfile = fopen(queuename(e, 'x'), "r");
			if (xfile == NULL)
				syserr("Cannot open %s", queuename(e, 'x'));
			expand("$n", buf, &buf[sizeof buf - 1], e);
			printf("\r\nMessage from %s...\r\n", buf);
			printf("Errors occurred while sending mail; transcript follows:\r\n");
			while (fgets(buf, sizeof buf, xfile) != NULL && !ferror(stdout))
				fputs(buf, stdout);
			if (ferror(stdout))
				(void) syserr("savemail: stdout: write err");
			(void) fclose(xfile);
		}
	}

	/*
	**  If mailing back, do it.
	**	Throw away all further output.  Don't do aliases, since
	**	this could cause loops, e.g., if joe mails to x:joe,
	**	and for some reason the network for x: is down, then
	**	the response gets sent to x:joe, which gives a
	**	response, etc.  Also force the mail to be delivered
	**	even if a version of it has already been sent to the
	**	sender.
	*/

	if (MailBack)
	{
		if (returntosender("Unable to deliver mail", CurEnv->e_returnto, TRUE) == 0)
			return;
	}

	/*
	**  Save the message in dead.letter.
	**	If we weren't mailing back, and the user is local, we
	**	should save the message in dead.letter so that the
	**	poor person doesn't have to type it over again --
	**	and we all know what poor typists programmers are.
	**	However, if we are running a "smart" protocol, we don't
	**	bother to return the message, since the other end is
	**	expected to handle that.
	*/

	if (OpMode == MD_ARPAFTP || OpMode == MD_SMTP)
		return;
	p = NULL;
	if (CurEnv->e_returnto->q_mailer == LocalMailer)
	{
		if (CurEnv->e_returnto->q_home != NULL)
			p = CurEnv->e_returnto->q_home;
		else if ((pw = getpwnam(CurEnv->e_returnto->q_user)) != NULL)
			p = pw->pw_dir;
	}
	if (p == NULL)
	{
		syserr("Can't return mail to %s", CurEnv->e_returnto->q_paddr);
# ifdef DEBUG
		p = "/usr/tmp";
# endif
	}
	if (p != NULL && TempFile != NULL)
	{
		auto ADDRESS *q;
		bool oldverb = Verbose;

		/* we have a home directory; open dead.letter */
		Verbose = TRUE;
		message(Arpa_Info, "Saving message in dead.letter");
		Verbose = oldverb;
		define('z', p);
		expand("$z/dead.letter", buf, &buf[sizeof buf - 1], e);
		e->e_to = buf;
		q = NULL;
		sendto(buf, (ADDRESS *) NULL, &q);
		(void) deliver(q);
	}

	/* add terminator to writeback message */
	if (WriteBack)
		printf("-----\r\n");
}
/*
**  RETURNTOSENDER -- return a message to the sender with an error.
**
**	Parameters:
**		msg -- the explanatory message.
**		returnto -- the queue of people to send the message to.
**		sendbody -- if TRUE, also send back the body of the
**			message; otherwise just send the header.
**
**	Returns:
**		zero -- if everything went ok.
**		else -- some error.
**
**	Side Effects:
**		Returns the current message to the sender via
**		mail.
*/

static bool	SendBody;

#define MAXRETURNS	6	/* max depth of returning messages */

returntosender(msg, returnto, sendbody)
	char *msg;
	ADDRESS *returnto;
	bool sendbody;
{
	char buf[MAXNAME];
	extern putheader(), errbody();
	register ENVELOPE *ee;
	extern ENVELOPE *newenvelope();
	ENVELOPE errenvelope;
	static int returndepth;

# ifdef DEBUG
	if (tTd(6, 1))
	{
		printf("Return To Sender: msg=\"%s\", depth=%d, CurEnv=%x,\n",
		       msg, returndepth, CurEnv);
		printf("\treturnto=");
		printaddr(returnto, FALSE);
	}
# endif DEBUG

	if (++returndepth >= MAXRETURNS)
	{
		if (returndepth != MAXRETURNS)
			syserr("returntosender: infinite recursion on %s", returnto->q_paddr);
		/* don't "unrecurse" and fake a clean exit */
		/* returndepth--; */
		return (0);
	}

	NoAlias = TRUE;
	SendBody = sendbody;
	define('g', "$f");
	ee = newenvelope(&errenvelope);
	ee->e_puthdr = putheader;
	ee->e_putbody = errbody;
	(void) queuename(ee, '\0');
	addheader("to", returnto->q_paddr, ee);
	addheader("subject", msg, ee);

	/* fake up an address header for the from person */
	expand("$n", buf, &buf[sizeof buf - 1], CurEnv);
	if (parse(buf, &ee->e_from, -1) == NULL)
	{
		syserr("Can't parse myself!");
		ExitStat = EX_SOFTWARE;
		returndepth--;
		return (-1);
	}
	ee->e_sendqueue = returnto;

	/* push state into submessage */
	CurEnv = ee;
	define('f', "$n");
	define('x', "Mail Delivery Subsystem");

	/* actually deliver the error message */
	sendall(ee, SendMode);

	/* do any closing error processing */
	checkerrors(ee);

	/* restore state */
	dropenvelope(ee);
	CurEnv = CurEnv->e_parent;
	returndepth--;

	/* should check for delivery errors here */
	return (0);
}
/*
**  ERRBODY -- output the body of an error message.
**
**	Typically this is a copy of the transcript plus a copy of the
**	original offending message.
**
**	Parameters:
**		xfile -- the transcript file.
**		fp -- the output file.
**		xdot -- if set, use the SMTP hidden dot algorithm.
**
**	Returns:
**		none
**
**	Side Effects:
**		Outputs the body of an error message.
*/

errbody(fp, m, xdot)
	register FILE *fp;
	register struct mailer *m;
	bool xdot;
{
	register FILE *xfile;
	char buf[MAXLINE];
	bool fullsmtp = bitset(M_FULLSMTP, m->m_flags);
	char *p;

	/*
	**  Output transcript of errors
	*/

	(void) fflush(stdout);
	p = queuename(CurEnv->e_parent, 'x');
	if ((xfile = fopen(p, "r")) == NULL)
	{
		syserr("Cannot open %s", p);
		fprintf(fp, "  ----- Transcript of session is unavailable -----\n");
	}
	else
	{
		fprintf(fp, "   ----- Transcript of session follows -----\n");
		if (Xscript != NULL)
			(void) fflush(Xscript);
		while (fgets(buf, sizeof buf, xfile) != NULL)
			putline(buf, fp, fullsmtp);
		(void) fclose(xfile);
	}
	errno = 0;

	/*
	**  Output text of original message
	*/

	if (NoReturn)
		fprintf(fp, "\n   ----- Return message suppressed -----\n\n");
	else if (TempFile != NULL)
	{
		if (SendBody)
		{
			fprintf(fp, "\n   ----- Unsent message follows -----\n");
			(void) fflush(fp);
			putheader(fp, m, CurEnv->e_parent);
			fprintf(fp, "\n");
			putbody(fp, m, xdot);
		}
		else
		{
			fprintf(fp, "\n  ----- Message header follows -----\n");
			(void) fflush(fp);
			putheader(fp, m, CurEnv);
		}
	}
	else
		fprintf(fp, "\n  ----- No message was collected -----\n\n");

	/*
	**  Cleanup and exit
	*/

	if (errno != 0)
		syserr("errbody: I/O error");
}
