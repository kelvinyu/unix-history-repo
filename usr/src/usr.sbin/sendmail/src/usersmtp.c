# include <ctype.h>
# include <sysexits.h>
# include "sendmail.h"

# ifndef SMTP
SCCSID(@(#)usersmtp.c	3.12.1.1		%G%	(no SMTP));
# else SMTP

SCCSID(@(#)usersmtp.c	3.12.1.1		%G%);

/*
**  SMTPINIT -- initialize SMTP.
**
**	Opens the connection and sends the initial protocol.
**
**	Parameters:
**		m -- mailer to create connection to.
**		pvp -- pointer to parameter vector to pass to
**			the mailer.
**		ctladdr -- controlling address for this mailer.
**
**	Returns:
**		appropriate exit status -- EX_OK on success.
**
**	Side Effects:
**		creates connection and sends initial protocol.
*/

# define REPLYTYPE(r)	((r) / 100)

static FILE	*SmtpOut;	/* output file */
static FILE	*SmtpIn;	/* input file */
static int	SmtpPid;	/* pid of mailer */
static int	SmtpErrstat;	/* error status if open fails */

smtpinit(m, pvp, ctladdr)
	struct mailer *m;
	char **pvp;
	ADDRESS *ctladdr;
{
	register int r;
	char buf[MAXNAME];

	/*
	**  Open the connection to the mailer.
	*/

	SmtpIn = SmtpOut = NULL;
	SmtpPid = openmailer(m, pvp, ctladdr, TRUE, &SmtpOut, &SmtpIn);
	if (SmtpPid < 0)
	{
		SmtpErrstat = ExitStat;
# ifdef DEBUG
		if (Debug > 0)
			printf("smtpinit: cannot open: Errstat %d errno %d\n",
			   SmtpErrstat, errno);
# endif DEBUG
		return (ExitStat);
	}

	/*
	**  Get the greeting message.
	**	This should appear spontaneously.
	*/

	r = reply();
	if (REPLYTYPE(r) != 2)
		return (EX_TEMPFAIL);

	/*
	**  Send the HELO command.
	**	My mother taught me to always introduce myself, even
	**	if it is useless.
	*/

	smtpmessage("HELO %s", HostName);
	r = reply();
	if (REPLYTYPE(r) == 5)
		return (EX_UNAVAILABLE);
	if (REPLYTYPE(r) != 2)
		return (EX_TEMPFAIL);

	/*
	**  Send the HOPS command.
	**	This is non-standard and may give an "unknown command".
	**		This is not an error.
	**	It can give a "bad hop count" error if the hop
	**		count is exceeded.
	*/

	/*
	**  Send the MAIL command.
	**	Designates the sender.
	*/

	expand("$g", buf, &buf[sizeof buf - 1], CurEnv);
	smtpmessage("MAIL From:<%s>", buf);
	r = reply();
	if (REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	if (r != 250)
		return (EX_SOFTWARE);
	return (EX_OK);
}
/*
**  SMTPRCPT -- designate recipient.
**
**	Parameters:
**		to -- address of recipient.
**
**	Returns:
**		exit status corresponding to recipient status.
**
**	Side Effects:
**		Sends the mail via SMTP.
*/

smtprcpt(to)
	ADDRESS *to;
{
	register int r;

	if (SmtpPid < 0)
		return (SmtpErrstat);

	smtpmessage("RCPT To:<%s>", to->q_user);

	r = reply();
	if (REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	if (r != 250)
		return (EX_NOUSER);

	return (EX_OK);
}
/*
**  SMTPFINISH -- finish up sending all the SMTP protocol.
**
**	Parameters:
**		m -- mailer being sent to.
**		e -- the envelope for this message.
**
**	Returns:
**		exit status corresponding to DATA command.
**
**	Side Effects:
**		none.
*/

smtpfinish(m, e)
	struct mailer *m;
	register ENVELOPE *e;
{
	register int r;

	if (SmtpPid < 0)
		return (SmtpErrstat);

	/*
	**  Send the data.
	**	Dot hiding is done here.
	*/

	smtpmessage("DATA");
	r = reply();
	if (REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	if (r != 354)
		return (EX_SOFTWARE);
	(*e->e_puthdr)(SmtpOut, m, CurEnv);
	fprintf(SmtpOut, "\n");
	(*e->e_putbody)(SmtpOut, m, TRUE);
	smtpmessage(".");
	r = reply();
	if (REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	if (r != 250)
		return (EX_SOFTWARE);
	return (EX_OK);
}
/*
**  SMTPQUIT -- close the SMTP connection.
**
**	Parameters:
**		name -- name of mailer we are quitting.
**		showresp -- if set, give a response message.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sends the final protocol and closes the connection.
*/

smtpquit(name, showresp)
	char *name;
	bool showresp;
{
	register int i;

	if (SmtpPid < 0)
		return;
	smtpmessage("QUIT");
	(void) reply();
	(void) fclose(SmtpIn);
	(void) fclose(SmtpOut);
	i = endmailer(SmtpPid, name);
	if (showresp)
		giveresponse(i, TRUE, LocalMailer);
}
/*
**  REPLY -- read arpanet reply
**
**	Parameters:
**		none.
**
**	Returns:
**		reply code it reads.
**
**	Side Effects:
**		flushes the mail file.
*/

reply()
{
	(void) fflush(SmtpOut);

	if (Debug)
		printf("reply\n");

	/* read the input line */
	for (;;)
	{
		char buf[MAXLINE];
		register int r;

		if (fgets(buf, sizeof buf, SmtpIn) == NULL)
			return (-1);
		if (Verbose && !HoldErrs)
			fputs(buf, stdout);
		fputs(buf, Xscript);
		if (buf[3] == '-' || !isdigit(buf[0]))
			continue;
		r = atoi(buf);
		if (r < 100)
			continue;
		return (r);
	}
}
/*
**  SMTPMESSAGE -- send message to server
**
**	Parameters:
**		f -- format
**		a, b, c -- parameters
**
**	Returns:
**		none.
**
**	Side Effects:
**		writes message to SmtpOut.
*/

/*VARARGS1*/
smtpmessage(f, a, b, c)
	char *f;
{
	char buf[100];

	(void) sprintf(buf, f, a, b, c);
	if (Debug || (Verbose && !HoldErrs))
		printf(">>> %s\n", buf);
	fprintf(Xscript, ">>> %s\n", buf);
	fprintf(SmtpOut, "%s\r\n", buf);
}

# endif SMTP
