# include <pwd.h>
# include "sendmail.h"

static char	SccsId[] = "@(#)savemail.c	3.16	%G%";

/*
**  SAVEMAIL -- Save mail on error
**
**	If the MailBack flag is set, mail it back to the originator
**	together with an error message; otherwise, just put it in
**	dead.letter in the user's home directory (if he exists on
**	this machine).
**
**	Parameters:
**		none
**
**	Returns:
**		none
**
**	Side Effects:
**		Saves the letter, by writing or mailing it back to the
**		sender, or by putting it in dead.letter in her home
**		directory.
**
**		WARNING: the user id is reset to the original user.
*/

savemail()
{
	register struct passwd *pw;
	register FILE *xfile;
	char buf[MAXLINE+1];
	extern errhdr();
	auto ADDRESS to_addr;
	extern struct passwd *getpwnam();
	register char *p;
	register int i;
	extern char *ttypath();
	static int exclusive;

	if (exclusive++)
		return;

	/*
	**  In the unhappy event we don't know who to return the mail
	**  to, make someone up.
	*/

	if (From.q_paddr == NULL)
	{
		if (parse("root", &From, 0) == NULL)
		{
			syserr("Cannot parse root!");
			ExitStat = EX_SOFTWARE;
			finis();
		}
	}
	To = NULL;

	/*
	**  If called from Eric Schmidt's network, do special mailback.
	**	Fundamentally, this is the mailback case except that
	**	it returns an OK exit status (assuming the return
	**	worked).
	*/

	if (BerkNet)
	{
		ExitStat = EX_OK;
		MailBack++;
	}

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
			MailBack++;
			errno = 0;
		}
		else
		{
			xfile = fopen(Transcript, "r");
			if (xfile == NULL)
				syserr("Cannot open %s", Transcript);
			(void) expand("$n", buf, &buf[sizeof buf - 1]);
			printf("\r\nMessage from %s...\r\n", buf);
			printf("Errors occurred while sending mail, transcript follows:\r\n");
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
		(void) freopen("/dev/null", "w", stdout);
		NoAlias++;
		ForceMail++;

		/* fake up an address header for the from person */
		bmove((char *) &From, (char *) &to_addr, sizeof to_addr);
		(void) expand("$n", buf, &buf[sizeof buf - 1]);
		if (parse(buf, &From, -1) == NULL)
		{
			syserr("Can't parse myself!");
			ExitStat = EX_SOFTWARE;
			finis();
		}
		i = deliver(&to_addr, errhdr);
		bmove((char *) &to_addr, (char *) &From, sizeof From);
		if (i != 0)
			syserr("Can't return mail to %s", p);
		else
			return;
	}

	/*
	**  Save the message in dead.letter.
	**	If we weren't mailing back, and the user is local, we
	**	should save the message in dead.letter so that the
	**	poor person doesn't have to type it over again --
	**	and we all know what poor typists programmers are.
	*/

	if (ArpaMode != ARPA_NONE)
		return;
	p = NULL;
	if (From.q_mailer == MN_LOCAL)
	{
		if (From.q_home != NULL)
			p = From.q_home;
		else if ((pw = getpwnam(From.q_user)) != NULL)
			p = pw->pw_dir;
	}
	if (p == NULL)
	{
		syserr("Can't return mail to %s", From.q_paddr);
# ifdef DEBUG
		p = "/usr/tmp";
# else
		p = NULL;
# endif
	}
	if (p != NULL && TempFile != NULL)
	{
		/* we have a home directory; open dead.letter */
		message(Arpa_Info, "Saving message in dead.letter");
		define('z', p);
		(void) expand("$z/dead.letter", buf, &buf[sizeof buf - 1]);
		To = buf;
		i = mailfile(buf);
		giveresponse(i, TRUE, Mailer[MN_LOCAL]);
	}

	/* add terminator to writeback message */
	if (WriteBack)
		printf("-----\r\n");
}
/*
**  ERRHDR -- Output the header for error mail.
**
**	This is the edit filter to error mailbacks.
**
**	Algorithm:
**		Output fixed header.
**		Output the transcript part.
**		Output the original message.
**
**	Parameters:
**		xfile -- the transcript file.
**		fp -- the output file.
**
**	Returns:
**		none
**
**	Side Effects:
**		input from xfile
**		output to fp
**
**	Called By:
**		deliver
*/


errhdr(fp)
	register FILE *fp;
{
	char buf[MAXLINE];
	register FILE *xfile;

	(void) fflush(stdout);
	if ((xfile = fopen(Transcript, "r")) == NULL)
		syserr("Cannot open %s", Transcript);
	errno = 0;
	fprintf(fp, "To: %s\n", To);
	fprintf(fp, "Subject: Unable to deliver mail\n");
	fprintf(fp, "\n   ----- Transcript of session follows -----\n");
	while (fgets(buf, sizeof buf, xfile) != NULL)
		fputs(buf, fp);
	if (NoReturn)
		fprintf(fp, "\n   ----- Return message suppressed -----\n\n");
	else if (TempFile != NULL)
	{
		fprintf(fp, "\n   ----- Unsent message follows -----\n");
		(void) fflush(fp);
		putmessage(fp, Mailer[1]);
	}
	else
		fprintf(fp, "\n  ----- No message was collected -----\n\n");
	(void) fclose(xfile);
	if (errno != 0)
		syserr("errhdr: I/O error");
}
