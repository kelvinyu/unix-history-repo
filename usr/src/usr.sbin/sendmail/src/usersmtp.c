/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

# include "sendmail.h"

#ifndef lint
#ifdef SMTP
static char sccsid[] = "@(#)usersmtp.c	6.20 (Berkeley) %G% (with SMTP)";
#else
static char sccsid[] = "@(#)usersmtp.c	6.20 (Berkeley) %G% (without SMTP)";
#endif
#endif /* not lint */

# include <sysexits.h>
# include <errno.h>

# ifdef SMTP

/*
**  USERSMTP -- run SMTP protocol from the user end.
**
**	This protocol is described in RFC821.
*/

#define REPLYTYPE(r)	((r) / 100)		/* first digit of reply code */
#define REPLYCLASS(r)	(((r) / 10) % 10)	/* second digit of reply code */
#define SMTPCLOSING	421			/* "Service Shutting Down" */

char	SmtpMsgBuffer[MAXLINE];		/* buffer for commands */
char	SmtpReplyBuffer[MAXLINE];	/* buffer for replies */
char	SmtpError[MAXLINE] = "";	/* save failure error messages */
int	SmtpPid;			/* pid of mailer */

#ifdef __STDC__
extern	smtpmessage(char *f, MAILER *m, MCI *mci, ...);
#endif
/*
**  SMTPINIT -- initialize SMTP.
**
**	Opens the connection and sends the initial protocol.
**
**	Parameters:
**		m -- mailer to create connection to.
**		pvp -- pointer to parameter vector to pass to
**			the mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		creates connection and sends initial protocol.
*/

smtpinit(m, mci, e)
	struct mailer *m;
	register MCI *mci;
	ENVELOPE *e;
{
	register int r;
	EVENT *gte;
	extern STAB *stab();

	if (tTd(17, 1))
	{
		printf("smtpinit ");
		mci_dump(mci);
	}

	/*
	**  Open the connection to the mailer.
	*/

	SmtpError[0] = '\0';
	CurHostName = mci->mci_host;		/* XXX UGLY XXX */
	switch (mci->mci_state)
	{
	  case MCIS_ACTIVE:
		/* need to clear old information */
		smtprset(m, mci, e);
		/* fall through */

	  case MCIS_OPEN:
		return;

	  case MCIS_ERROR:
	  case MCIS_SSD:
		/* shouldn't happen */
		smtpquit(m, mci, e);
		/* fall through */

	  case MCIS_CLOSED:
		syserr("451 smtpinit: state CLOSED");
		return;

	  case MCIS_OPENING:
		break;
	}

	SmtpPhase = mci->mci_phase = "user open";
	mci->mci_state = MCIS_OPENING;

	/*
	**  Get the greeting message.
	**	This should appear spontaneously.  Give it five minutes to
	**	happen.
	*/

	SmtpPhase = mci->mci_phase = "greeting wait";
	setproctitle("%s %s: %s", e->e_id, CurHostName, mci->mci_phase);
	r = reply(m, mci, e, TimeOuts.to_initial);
	if (r < 0 || REPLYTYPE(r) != 2)
		goto tempfail1;

	/*
	**  Send the HELO command.
	**	My mother taught me to always introduce myself.
	*/

	smtpmessage("HELO %s", m, mci, MyHostName);
	SmtpPhase = mci->mci_phase = "HELO wait";
	setproctitle("%s %s: %s", e->e_id, CurHostName, mci->mci_phase);
	r = reply(m, mci, e, TimeOuts.to_helo);
	if (r < 0)
		goto tempfail1;
	else if (REPLYTYPE(r) == 5)
		goto unavailable;
	else if (REPLYTYPE(r) != 2)
		goto tempfail1;

	/*
	**  If this is expected to be another sendmail, send some internal
	**  commands.
	*/

	if (bitnset(M_INTERNAL, m->m_flags))
	{
		/* tell it to be verbose */
		smtpmessage("VERB", m, mci);
		r = reply(m, mci, e, TimeOuts.to_miscshort);
		if (r < 0)
			goto tempfail2;
	}

	mci->mci_state = MCIS_OPEN;
	return;

  tempfail1:
  tempfail2:
	mci->mci_exitstat = EX_TEMPFAIL;
	if (mci->mci_errno == 0)
		mci->mci_errno = errno;
	if (mci->mci_state != MCIS_CLOSED)
		smtpquit(m, mci, e);
	return;

  unavailable:
	mci->mci_exitstat = EX_UNAVAILABLE;
	mci->mci_errno = errno;
	smtpquit(m, mci, e);
	return;
}

smtpmailfrom(m, mci, e)
	struct mailer *m;
	MCI *mci;
	ENVELOPE *e;
{
	int r;
	char buf[MAXNAME];

	if (tTd(17, 2))
		printf("smtpmailfrom: CurHost=%s\n", CurHostName);

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

	mci->mci_state = MCIS_ACTIVE;

	if (bitset(EF_RESPONSE, e->e_flags))
		(void) strcpy(buf, "");
	else
		expand("\201g", buf, &buf[sizeof buf - 1], e);
	if (e->e_from.q_mailer == LocalMailer ||
	    !bitnset(M_FROMPATH, m->m_flags))
	{
		smtpmessage("MAIL From:<%s>", m, mci, buf);
	}
	else
	{
		smtpmessage("MAIL From:<@%s%c%s>", m, mci, MyHostName,
			buf[0] == '@' ? ',' : ':', buf);
	}
	SmtpPhase = mci->mci_phase = "MAIL wait";
	setproctitle("%s %s: %s", e->e_id, CurHostName, mci->mci_phase);
	r = reply(m, mci, e, TimeOuts.to_mail);
	if (r < 0 || REPLYTYPE(r) == 4)
	{
		mci->mci_exitstat = EX_TEMPFAIL;
		mci->mci_errno = errno;
		smtpquit(m, mci, e);
		return EX_TEMPFAIL;
	}
	else if (r == 250)
	{
		mci->mci_exitstat = EX_OK;
		return EX_OK;
	}
	else if (r == 552)
	{
		/* signal service unavailable */
		mci->mci_exitstat = EX_UNAVAILABLE;
		smtpquit(m, mci, e);
		return EX_UNAVAILABLE;
	}

#ifdef LOG
	if (LogLevel > 1)
	{
		syslog(LOG_CRIT, "%s: SMTP MAIL protocol error: %s",
			e->e_id, SmtpReplyBuffer);
	}
#endif

	/* protocol error -- close up */
	smtpquit(m, mci, e);
	mci->mci_exitstat = EX_PROTOCOL;
	return EX_PROTOCOL;
}
/*
**  SMTPRCPT -- designate recipient.
**
**	Parameters:
**		to -- address of recipient.
**		m -- the mailer we are sending to.
**		mci -- the connection info for this transaction.
**		e -- the envelope for this transaction.
**
**	Returns:
**		exit status corresponding to recipient status.
**
**	Side Effects:
**		Sends the mail via SMTP.
*/

smtprcpt(to, m, mci, e)
	ADDRESS *to;
	register MAILER *m;
	MCI *mci;
	ENVELOPE *e;
{
	register int r;

	smtpmessage("RCPT To:<%s>", m, mci, to->q_user);

	SmtpPhase = mci->mci_phase = "RCPT wait";
	setproctitle("%s %s: %s", e->e_id, CurHostName, mci->mci_phase);
	r = reply(m, mci, e, TimeOuts.to_rcpt);
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (REPLYTYPE(r) == 2)
		return (EX_OK);
	else if (r == 550 || r == 551 || r == 553)
		return (EX_NOUSER);
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);

#ifdef LOG
	if (LogLevel > 1)
	{
		syslog(LOG_CRIT, "%s: SMTP RCPT protocol error: %s",
			e->e_id, SmtpReplyBuffer);
	}
#endif

	return (EX_PROTOCOL);
}
/*
**  SMTPDATA -- send the data and clean up the transaction.
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

smtpdata(m, mci, e)
	struct mailer *m;
	register MCI *mci;
	register ENVELOPE *e;
{
	register int r;

	/*
	**  Send the data.
	**	First send the command and check that it is ok.
	**	Then send the data.
	**	Follow it up with a dot to terminate.
	**	Finally get the results of the transaction.
	*/

	/* send the command and check ok to proceed */
	smtpmessage("DATA", m, mci);
	SmtpPhase = mci->mci_phase = "DATA wait";
	setproctitle("%s %s: %s", e->e_id, CurHostName, mci->mci_phase);
	r = reply(m, mci, e, TimeOuts.to_datainit);
	if (r < 0 || REPLYTYPE(r) == 4)
	{
		smtpquit(m, mci, e);
		return (EX_TEMPFAIL);
	}
	else if (r == 554)
	{
		smtprset(m, mci, e);
		return (EX_UNAVAILABLE);
	}
	else if (r != 354)
	{
#ifdef LOG
		if (LogLevel > 1)
		{
			syslog(LOG_CRIT, "%s: SMTP DATA-1 protocol error: %s",
				e->e_id, SmtpReplyBuffer);
		}
#endif
		smtprset(m, mci, e);
		return (EX_PROTOCOL);
	}

	/* now output the actual message */
	(*e->e_puthdr)(mci->mci_out, m, e);
	putline("\n", mci->mci_out, m);
	(*e->e_putbody)(mci->mci_out, m, e);

	/* terminate the message */
	fprintf(mci->mci_out, ".%s", m->m_eol);
	if (Verbose)
		nmessage(">>> .");

	/* check for the results of the transaction */
	SmtpPhase = mci->mci_phase = "result wait";
	setproctitle("%s %s: %s", e->e_id, CurHostName, mci->mci_phase);
	r = reply(m, mci, e, TimeOuts.to_datafinal);
	if (r < 0)
	{
		smtpquit(m, mci, e);
		return (EX_TEMPFAIL);
	}
	mci->mci_state = MCIS_OPEN;
	e->e_message = newstr(&SmtpReplyBuffer[4]);
	if (REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (r == 250)
		return (EX_OK);
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);
#ifdef LOG
	if (LogLevel > 1)
	{
		syslog(LOG_CRIT, "%s: SMTP DATA-2 protocol error: %s",
			e->e_id, SmtpReplyBuffer);
	}
#endif
	return (EX_PROTOCOL);
}
/*
**  SMTPQUIT -- close the SMTP connection.
**
**	Parameters:
**		m -- a pointer to the mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sends the final protocol and closes the connection.
*/

smtpquit(m, mci, e)
	register MAILER *m;
	register MCI *mci;
	ENVELOPE *e;
{
	int i;

	/* send the quit message if we haven't gotten I/O error */
	if (mci->mci_state != MCIS_ERROR)
	{
		smtpmessage("QUIT", m, mci);
		(void) reply(m, mci, e, TimeOuts.to_quit);
		if (mci->mci_state == MCIS_CLOSED)
			return;
	}

	/* now actually close the connection and pick up the zombie */
	i = endmailer(mci, e, m->m_argv);
	if (i != EX_OK)
		syserr("451 smtpquit %s: stat %d", m->m_argv[0], i);
}
/*
**  SMTPRSET -- send a RSET (reset) command
*/

smtprset(m, mci, e)
	register MAILER *m;
	register MCI *mci;
	ENVELOPE *e;
{
	int r;

	smtpmessage("RSET", m, mci);
	r = reply(m, mci, e, TimeOuts.to_rset);
	if (r < 0)
		mci->mci_state = MCIS_ERROR;
	else if (REPLYTYPE(r) == 2)
	{
		mci->mci_state = MCIS_OPEN;
		return;
	}
	smtpquit(m, mci, e);
}
/*
**  SMTPPROBE -- check the connection state
*/

smtpprobe(mci)
	register MCI *mci;
{
	int r;
	MAILER *m = mci->mci_mailer;
	extern ENVELOPE BlankEnvelope;
	ENVELOPE *e = &BlankEnvelope;

	smtpmessage("RSET", m, mci);
	r = reply(m, mci, e, TimeOuts.to_miscshort);
	if (r < 0 || REPLYTYPE(r) != 2)
		smtpquit(m, mci, e);
	return r;
}
/*
**  REPLY -- read arpanet reply
**
**	Parameters:
**		m -- the mailer we are reading the reply from.
**		mci -- the mailer connection info structure.
**		e -- the current envelope.
**		timeout -- the timeout for reads.
**
**	Returns:
**		reply code it reads.
**
**	Side Effects:
**		flushes the mail file.
*/

reply(m, mci, e, timeout)
	MAILER *m;
	MCI *mci;
	ENVELOPE *e;
{
	if (mci->mci_out != NULL)
		(void) fflush(mci->mci_out);

	if (tTd(18, 1))
		printf("reply\n");

	/*
	**  Read the input line, being careful not to hang.
	*/

	for (;;)
	{
		register int r;
		register char *p;
		extern time_t curtime();

		/* actually do the read */
		if (e->e_xfp != NULL)
			(void) fflush(e->e_xfp);	/* for debugging */

		/* if we are in the process of closing just give the code */
		if (mci->mci_state == MCIS_CLOSED)
			return (SMTPCLOSING);

		if (mci->mci_out != NULL)
			fflush(mci->mci_out);

		/* get the line from the other side */
		p = sfgets(SmtpReplyBuffer, sizeof SmtpReplyBuffer, mci->mci_in,
			   timeout);
		mci->mci_lastuse = curtime();

		if (p == NULL)
		{
			extern char MsgBuf[];		/* err.c */

			/* if the remote end closed early, fake an error */
			if (errno == 0)
# ifdef ECONNRESET
				errno = ECONNRESET;
# else /* ECONNRESET */
				errno = EPIPE;
# endif /* ECONNRESET */

			mci->mci_errno = errno;
			mci->mci_exitstat = EX_TEMPFAIL;
			message("451 %s: reply: read error from %s",
				e->e_id == NULL ? "NOQUEUE" : e->e_id,
				mci->mci_host);
			/* if debugging, pause so we can see state */
			if (tTd(18, 100))
				pause();
# ifdef LOG
			if (LogLevel > 1)
				syslog(LOG_INFO, "%s", &MsgBuf[4]);
# endif /* LOG */
			mci->mci_state = MCIS_ERROR;
			smtpquit(m, mci, e);
			return (-1);
		}
		fixcrlf(SmtpReplyBuffer, TRUE);

		if (e->e_xfp != NULL && strchr("45", SmtpReplyBuffer[0]) != NULL)
		{
			/* serious error -- log the previous command */
			if (SmtpMsgBuffer[0] != '\0')
				fprintf(e->e_xfp, ">>> %s\n", SmtpMsgBuffer);
			SmtpMsgBuffer[0] = '\0';

			/* now log the message as from the other side */
			fprintf(e->e_xfp, "<<< %s\n", SmtpReplyBuffer);
		}

		/* display the input for verbose mode */
		if (Verbose)
			nmessage("%s", SmtpReplyBuffer);

		/* if continuation is required, we can go on */
		if (SmtpReplyBuffer[3] == '-' ||
		    !(isascii(SmtpReplyBuffer[0]) && isdigit(SmtpReplyBuffer[0])))
			continue;

		/* decode the reply code */
		r = atoi(SmtpReplyBuffer);

		/* extra semantics: 0xx codes are "informational" */
		if (r < 100)
			continue;

		/* save temporary failure messages for posterity */
		if (SmtpReplyBuffer[0] == '4' && SmtpError[0] == '\0')
			(void) strcpy(SmtpError, SmtpReplyBuffer);

		/* reply code 421 is "Service Shutting Down" */
		if (r == SMTPCLOSING && mci->mci_state != MCIS_SSD)
		{
			/* send the quit protocol */
			mci->mci_state = MCIS_SSD;
			smtpquit(m, mci, e);
		}

		return (r);
	}
}
/*
**  SMTPMESSAGE -- send message to server
**
**	Parameters:
**		f -- format
**		m -- the mailer to control formatting.
**		a, b, c -- parameters
**
**	Returns:
**		none.
**
**	Side Effects:
**		writes message to mci->mci_out.
*/

/*VARARGS1*/
#ifdef __STDC__
smtpmessage(char *f, MAILER *m, MCI *mci, ...)
#else
smtpmessage(f, m, mci, va_alist)
	char *f;
	MAILER *m;
	MCI *mci;
	va_dcl
#endif
{
	VA_LOCAL_DECL

	VA_START(mci);
	(void) vsprintf(SmtpMsgBuffer, f, ap);
	VA_END;

	if (tTd(18, 1) || Verbose)
		nmessage(">>> %s", SmtpMsgBuffer);
	if (mci->mci_out != NULL)
	{
		fprintf(mci->mci_out, "%s%s", SmtpMsgBuffer,
			m == NULL ? "\r\n" : m->m_eol);
	}
	else
	{
		syserr("smtpmessage: NULL mci_out");
	}
}

# endif /* SMTP */
