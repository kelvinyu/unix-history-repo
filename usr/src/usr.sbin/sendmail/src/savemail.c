/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)savemail.c	8.59 (Berkeley) %G%";
#endif /* not lint */

# include "sendmail.h"
# include <pwd.h>

/*
**  SAVEMAIL -- Save mail on error
**
**	If mailing back errors, mail it back to the originator
**	together with an error message; otherwise, just put it in
**	dead.letter in the user's home directory (if he exists on
**	this machine).
**
**	Parameters:
**		e -- the envelope containing the message in error.
**		sendbody -- if TRUE, also send back the body of the
**			message; otherwise just send the header.
**
**	Returns:
**		none
**
**	Side Effects:
**		Saves the letter, by writing or mailing it back to the
**		sender, or by putting it in dead.letter in her home
**		directory.
*/

/* defines for state machine */
# define ESM_REPORT	0	/* report to sender's terminal */
# define ESM_MAIL	1	/* mail back to sender */
# define ESM_QUIET	2	/* messages have already been returned */
# define ESM_DEADLETTER	3	/* save in ~/dead.letter */
# define ESM_POSTMASTER	4	/* return to postmaster */
# define ESM_USRTMP	5	/* save in /usr/tmp/dead.letter */
# define ESM_PANIC	6	/* leave the locked queue/transcript files */
# define ESM_DONE	7	/* the message is successfully delivered */

# ifndef _PATH_VARTMP
#  define _PATH_VARTMP	"/usr/tmp/"
# endif


savemail(e, sendbody)
	register ENVELOPE *e;
	bool sendbody;
{
	register struct passwd *pw;
	register FILE *fp;
	int state;
	auto ADDRESS *q = NULL;
	register char *p;
	MCI mcibuf;
	char buf[MAXLINE+1];
	extern struct passwd *getpwnam();
	extern char *ttypath();
	typedef int (*fnptr)();
	extern bool writable();

	if (tTd(6, 1))
	{
		printf("\nsavemail, errormode = %c, id = %s, ExitStat = %d\n  e_from=",
			e->e_errormode, e->e_id == NULL ? "NONE" : e->e_id,
			ExitStat);
		printaddr(&e->e_from, FALSE);
	}

	if (e->e_id == NULL)
	{
		/* can't return a message with no id */
		return;
	}

	/*
	**  In the unhappy event we don't know who to return the mail
	**  to, make someone up.
	*/

	if (CurEnv->e_returnto == NULL)
	{
		CurEnv->e_returnto = parse("root", (ADDRESS *) NULL, 0);
		if (CurEnv->e_returnto == NULL)
		{
			syserr("553 Cannot parse Postmaster!");
			ExitStat = EX_SOFTWARE;
			finis();
		}
	}
	e->e_to = NULL;

	/*
	**  Basic state machine.
	**
	**	This machine runs through the following states:
	**
	**	ESM_QUIET	Errors have already been printed iff the
	**			sender is local.
	**	ESM_REPORT	Report directly to the sender's terminal.
	**	ESM_MAIL	Mail response to the sender.
	**	ESM_DEADLETTER	Save response in ~/dead.letter.
	**	ESM_POSTMASTER	Mail response to the postmaster.
	**	ESM_PANIC	Save response anywhere possible.
	*/

	/* determine starting state */
	switch (e->e_errormode)
	{
	  case EM_WRITE:
		state = ESM_REPORT;
		break;

	  case EM_BERKNET:
		/* mail back, but return o.k. exit status */
		ExitStat = EX_OK;

		/* fall through.... */

	  case EM_MAIL:
		state = ESM_MAIL;
		break;

	  case EM_PRINT:
	  case '\0':
		state = ESM_QUIET;
		break;

	  case EM_QUIET:
		/* no need to return anything at all */
		return;

	  default:
		syserr("554 savemail: bogus errormode x%x\n", e->e_errormode);
		state = ESM_MAIL;
		break;
	}

	/* if this is already an error response, send to postmaster */
	if (bitset(EF_RESPONSE, e->e_flags))
	{
		if (e->e_parent != NULL &&
		    bitset(EF_RESPONSE, e->e_parent->e_flags))
		{
			/* got an error sending a response -- can it */
			return;
		}
		state = ESM_POSTMASTER;
	}

	while (state != ESM_DONE)
	{
		if (tTd(6, 5))
			printf("  state %d\n", state);

		switch (state)
		{
		  case ESM_QUIET:
			if (bitnset(M_LOCALMAILER, e->e_from.q_mailer->m_flags))
				state = ESM_DEADLETTER;
			else
				state = ESM_MAIL;
			break;

		  case ESM_REPORT:

			/*
			**  If the user is still logged in on the same terminal,
			**  then write the error messages back to hir (sic).
			*/

			p = ttypath();
			if (p == NULL || freopen(p, "w", stdout) == NULL)
			{
				state = ESM_MAIL;
				break;
			}

			expand("\201n", buf, sizeof buf, e);
			printf("\r\nMessage from %s...\r\n", buf);
			printf("Errors occurred while sending mail.\r\n");
			if (e->e_xfp != NULL)
			{
				(void) fflush(e->e_xfp);
				fp = fopen(queuename(e, 'x'), "r");
			}
			else
				fp = NULL;
			if (fp == NULL)
			{
				syserr("Cannot open %s", queuename(e, 'x'));
				printf("Transcript of session is unavailable.\r\n");
			}
			else
			{
				printf("Transcript follows:\r\n");
				while (fgets(buf, sizeof buf, fp) != NULL &&
				       !ferror(stdout))
					fputs(buf, stdout);
				(void) xfclose(fp, "savemail transcript", e->e_id);
			}
			printf("Original message will be saved in dead.letter.\r\n");
			state = ESM_DEADLETTER;
			break;

		  case ESM_MAIL:
			/*
			**  If mailing back, do it.
			**	Throw away all further output.  Don't alias,
			**	since this could cause loops, e.g., if joe
			**	mails to joe@x, and for some reason the network
			**	for @x is down, then the response gets sent to
			**	joe@x, which gives a response, etc.  Also force
			**	the mail to be delivered even if a version of
			**	it has already been sent to the sender.
			**
			**  If this is a configuration or local software
			**	error, send to the local postmaster as well,
			**	since the originator can't do anything
			**	about it anyway.  Note that this is a full
			**	copy of the message (intentionally) so that
			**	the Postmaster can forward things along.
			*/

			if (ExitStat == EX_CONFIG || ExitStat == EX_SOFTWARE)
			{
				(void) sendtolist("postmaster",
					  NULLADDR, &e->e_errorqueue, 0, e);
			}
			if (!emptyaddr(&e->e_from))
			{
				(void) sendtolist(e->e_from.q_paddr,
					  NULLADDR, &e->e_errorqueue, 0, e);
			}

			/*
			**  Deliver a non-delivery report to the
			**  Postmaster-designate (not necessarily
			**  Postmaster).  This does not include the
			**  body of the message, for privacy reasons.
			**  You really shouldn't need this.
			*/

			e->e_flags |= EF_PM_NOTIFY;

			/* check to see if there are any good addresses */
			for (q = e->e_errorqueue; q != NULL; q = q->q_next)
				if (!bitset(QBADADDR|QDONTSEND, q->q_flags))
					break;
			if (q == NULL)
			{
				/* this is an error-error */
				state = ESM_POSTMASTER;
				break;
			}
			if (returntosender(e->e_message, e->e_errorqueue,
					   sendbody, e) == 0)
			{
				state = ESM_DONE;
				break;
			}

			/* didn't work -- return to postmaster */
			state = ESM_POSTMASTER;
			break;

		  case ESM_POSTMASTER:
			/*
			**  Similar to previous case, but to system postmaster.
			*/

			q = NULL;
			if (sendtolist("postmaster", NULL, &q, 0, e) <= 0)
			{
				syserr("553 cannot parse postmaster!");
				ExitStat = EX_SOFTWARE;
				state = ESM_USRTMP;
				break;
			}
			if (returntosender(e->e_message, q, sendbody, e) == 0)
			{
				state = ESM_DONE;
				break;
			}

			/* didn't work -- last resort */
			state = ESM_USRTMP;
			break;

		  case ESM_DEADLETTER:
			/*
			**  Save the message in dead.letter.
			**	If we weren't mailing back, and the user is
			**	local, we should save the message in
			**	~/dead.letter so that the poor person doesn't
			**	have to type it over again -- and we all know
			**	what poor typists UNIX users are.
			*/

			p = NULL;
			if (bitnset(M_HASPWENT, e->e_from.q_mailer->m_flags))
			{
				if (e->e_from.q_home != NULL)
					p = e->e_from.q_home;
				else if ((pw = getpwnam(e->e_from.q_user)) != NULL)
					p = pw->pw_dir;
			}
			if (p == NULL)
			{
				/* no local directory */
				state = ESM_MAIL;
				break;
			}
			if (e->e_dfp != NULL)
			{
				bool oldverb = Verbose;

				/* we have a home directory; open dead.letter */
				define('z', p, e);
				expand("\201z/dead.letter", buf, sizeof buf, e);
				Verbose = TRUE;
				message("Saving message in %s", buf);
				Verbose = oldverb;
				e->e_to = buf;
				q = NULL;
				(void) sendtolist(buf, &e->e_from, &q, 0, e);
				if (q != NULL &&
				    !bitset(QBADADDR, q->q_flags) &&
				    deliver(e, q) == 0)
					state = ESM_DONE;
				else
					state = ESM_MAIL;
			}
			else
			{
				/* no data file -- try mailing back */
				state = ESM_MAIL;
			}
			break;

		  case ESM_USRTMP:
			/*
			**  Log the mail in /usr/tmp/dead.letter.
			*/

			if (e->e_class < 0)
			{
				state = ESM_DONE;
				break;
			}

			if (SafeFileEnv != NULL && SafeFileEnv[0] != '\0')
			{
				state = ESM_PANIC;
				break;
			}

			strcpy(buf, _PATH_VARTMP);
			strcat(buf, "dead.letter");
			if (!writable(buf, NULLADDR, SFF_NOSLINK|SFF_CREAT))
			{
				state = ESM_PANIC;
				break;
			}
			fp = safefopen(buf, O_WRONLY|O_CREAT|O_APPEND,
					FileMode, SFF_NOSLINK|SFF_REGONLY);
			if (fp == NULL)
			{
				state = ESM_PANIC;
				break;
			}

			bzero(&mcibuf, sizeof mcibuf);
			mcibuf.mci_out = fp;
			mcibuf.mci_mailer = FileMailer;
			if (bitnset(M_7BITS, FileMailer->m_flags))
				mcibuf.mci_flags |= MCIF_7BIT;

			putfromline(&mcibuf, e);
			(*e->e_puthdr)(&mcibuf, e->e_header, e);
			(*e->e_putbody)(&mcibuf, e, NULL);
			putline("\n", &mcibuf);
			(void) fflush(fp);
			state = ferror(fp) ? ESM_PANIC : ESM_DONE;
			(void) xfclose(fp, "savemail", buf);
			break;

		  default:
			syserr("554 savemail: unknown state %d", state);

			/* fall through ... */

		  case ESM_PANIC:
			/* leave the locked queue & transcript files around */
			loseqfile(e, "savemail panic");
			syserr("!554 savemail: cannot save rejected email anywhere");
		}
	}
}
/*
**  RETURNTOSENDER -- return a message to the sender with an error.
**
**	Parameters:
**		msg -- the explanatory message.
**		returnq -- the queue of people to send the message to.
**		sendbody -- if TRUE, also send back the body of the
**			message; otherwise just send the header.
**		e -- the current envelope.
**
**	Returns:
**		zero -- if everything went ok.
**		else -- some error.
**
**	Side Effects:
**		Returns the current message to the sender via
**		mail.
*/

#define MAXRETURNS	6	/* max depth of returning messages */
#define ERRORFUDGE	100	/* nominal size of error message text */

returntosender(msg, returnq, sendbody, e)
	char *msg;
	ADDRESS *returnq;
	bool sendbody;
	register ENVELOPE *e;
{
	char buf[MAXNAME + 1];
	extern putheader(), errbody();
	register ENVELOPE *ee;
	ENVELOPE *oldcur = CurEnv;
	ENVELOPE errenvelope;
	static int returndepth;
	register ADDRESS *q;
	char *p;

	if (returnq == NULL)
		return (-1);

	if (msg == NULL)
		msg = "Unable to deliver mail";

	if (tTd(6, 1))
	{
		printf("\n*** Return To Sender: msg=\"%s\", depth=%d, e=%x, returnq=",
		       msg, returndepth, e);
		printaddr(returnq, TRUE);
		if (tTd(6, 20))
		{
			printf("Sendq=");
			printaddr(e->e_sendqueue, TRUE);
		}
	}

	if (++returndepth >= MAXRETURNS)
	{
		if (returndepth != MAXRETURNS)
			syserr("554 returntosender: infinite recursion on %s", returnq->q_paddr);
		/* don't "unrecurse" and fake a clean exit */
		/* returndepth--; */
		return (0);
	}

	define('g', e->e_from.q_paddr, e);
	define('u', NULL, e);

	/* initialize error envelope */
	ee = newenvelope(&errenvelope, e);
	define('a', "\201b", ee);
	define('r', "internal", ee);
	define('s', "localhost", ee);
	define('_', "localhost", ee);
	ee->e_puthdr = putheader;
	ee->e_putbody = errbody;
	ee->e_flags |= EF_RESPONSE|EF_METOO;
	if (!bitset(EF_OLDSTYLE, e->e_flags))
		ee->e_flags &= ~EF_OLDSTYLE;
	ee->e_sendqueue = returnq;
	ee->e_msgsize = ERRORFUDGE;
	if (sendbody)
		ee->e_msgsize += e->e_msgsize;
	initsys(ee);
	for (q = returnq; q != NULL; q = q->q_next)
	{
		if (bitset(QBADADDR, q->q_flags))
			continue;

		if (!DontPruneRoutes && pruneroute(q->q_paddr))
		{
			register ADDRESS *p;

			parseaddr(q->q_paddr, q, RF_COPYPARSE, '\0', NULL, e);
			for (p = returnq; p != NULL; p = p->q_next)
			{
				if (p != q && sameaddr(p, q))
					q->q_flags |= QDONTSEND;
			}
		}

		if (!bitset(QDONTSEND, q->q_flags))
			ee->e_nrcpts++;

		if (q->q_alias == NULL)
			addheader("To", q->q_paddr, &ee->e_header);
	}

# ifdef LOG
	if (LogLevel > 5)
		syslog(LOG_INFO, "%s: %s: returntosender: %s",
			e->e_id, ee->e_id, msg);
# endif

	if (strncasecmp(msg, "warning:", 8) != 0)
	{
		(void) sprintf(buf, "Returned mail: %s", msg);
		msg = buf;
	}
	addheader("Subject", msg, ee);

	/* fake up an address header for the from person */
	expand("\201n", buf, sizeof buf, e);
	if (parseaddr(buf, &ee->e_from, RF_COPYALL|RF_SENDERADDR, '\0', NULL, e) == NULL)
	{
		syserr("553 Can't parse myself!");
		ExitStat = EX_SOFTWARE;
		returndepth--;
		return (-1);
	}
	ee->e_sender = ee->e_from.q_paddr;

	/* push state into submessage */
	CurEnv = ee;
	define('f', "\201n", ee);
	define('x', "Mail Delivery Subsystem", ee);
	eatheader(ee, TRUE);

	/* mark statistics */
	markstats(ee, NULLADDR);

	/* actually deliver the error message */
	sendall(ee, SM_DEFAULT);

	/* restore state */
	dropenvelope(ee);
	CurEnv = oldcur;
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
**		mci -- the mailer connection information.
**		e -- the envelope we are working in.
**		separator -- any possible MIME separator.
**		flags -- to modify the behaviour.
**
**	Returns:
**		none
**
**	Side Effects:
**		Outputs the body of an error message.
*/

errbody(mci, e, separator)
	register MCI *mci;
	register ENVELOPE *e;
	char *separator;
{
	register FILE *xfile;
	char *p;
	register ADDRESS *q;
	bool printheader;
	bool sendbody;
	char buf[MAXLINE];
	extern char *xtextify();

	if (bitset(MCIF_INHEADER, mci->mci_flags))
	{
		putline("", mci);
		mci->mci_flags &= ~MCIF_INHEADER;
	}
	if (e->e_parent == NULL)
	{
		syserr("errbody: null parent");
		putline("   ----- Original message lost -----\n", mci);
		return;
	}

	/*
	**  Output MIME header.
	*/

	if (e->e_msgboundary != NULL)
	{
		putline("This is a MIME-encapsulated message", mci);
		putline("", mci);
		(void) sprintf(buf, "--%s", e->e_msgboundary);
		putline(buf, mci);
		putline("", mci);
	}

	/*
	**  Output introductory information.
	*/

	for (q = e->e_parent->e_sendqueue; q != NULL; q = q->q_next)
		if (bitset(QBADADDR, q->q_flags))
			break;
	if (q == NULL &&
	    !bitset(EF_FATALERRS|EF_SENDRECEIPT, e->e_parent->e_flags))
	{
		putline("    **********************************************",
			mci);
		putline("    **      THIS IS A WARNING MESSAGE ONLY      **",
			mci);
		putline("    **  YOU DO NOT NEED TO RESEND YOUR MESSAGE  **",
			mci);
		putline("    **********************************************",
			mci);
		putline("", mci);
	}
	sprintf(buf, "The original message was received at %s",
		arpadate(ctime(&e->e_parent->e_ctime)));
	putline(buf, mci);
	expand("from \201_", buf, sizeof buf, e->e_parent);
	putline(buf, mci);
	putline("", mci);

	/*
	**  Output error message header (if specified and available).
	*/

	if (ErrMsgFile != NULL && !bitset(EF_SENDRECEIPT, e->e_parent->e_flags))
	{
		if (*ErrMsgFile == '/')
		{
			xfile = fopen(ErrMsgFile, "r");
			if (xfile != NULL)
			{
				while (fgets(buf, sizeof buf, xfile) != NULL)
				{
					expand(buf, buf, sizeof buf, e);
					putline(buf, mci);
				}
				(void) fclose(xfile);
				putline("\n", mci);
			}
		}
		else
		{
			expand(ErrMsgFile, buf, sizeof buf, e);
			putline(buf, mci);
			putline("", mci);
		}
	}

	/*
	**  Output message introduction
	*/

	printheader = TRUE;
	for (q = e->e_parent->e_sendqueue; q != NULL; q = q->q_next)
	{
		if (bitset(QBADADDR|QREPORT|QRELAYED, q->q_flags))
		{
			if (printheader)
			{
				putline("   ----- The following addresses have delivery notifications -----",
					mci);
				printheader = FALSE;
			}
			strcpy(buf, q->q_paddr);
			if (bitset(QBADADDR, q->q_flags))
				strcat(buf, "  (unrecoverable error)");
			else if (bitset(QRELAYED, q->q_flags))
				strcat(buf, "  (relayed to non-DSN-aware mailer)");
			else if (bitset(QSENT, q->q_flags))
				strcat(buf, "  (successfully delivered)");
			else
				strcat(buf, "  (transient failure)");
			putline(buf, mci);
			if (q->q_alias != NULL)
			{
				strcpy(buf, "    (expanded from: ");
				strcat(buf, q->q_alias->q_paddr);
				strcat(buf, ")");
				putline(buf, mci);
			}
		}
	}
	if (!printheader)
		putline("\n", mci);

	/*
	**  Output transcript of errors
	*/

	(void) fflush(stdout);
	p = queuename(e->e_parent, 'x');
	if ((xfile = fopen(p, "r")) == NULL)
	{
		syserr("Cannot open %s", p);
		putline("   ----- Transcript of session is unavailable -----\n", mci);
	}
	else
	{
		putline("   ----- Transcript of session follows -----\n", mci);
		if (e->e_xfp != NULL)
			(void) fflush(e->e_xfp);
		while (fgets(buf, sizeof buf, xfile) != NULL)
			putline(buf, mci);
		(void) xfclose(xfile, "errbody xscript", p);
	}
	errno = 0;

#ifdef DSN
	/*
	**  Output machine-readable version.
	*/

	if (e->e_msgboundary != NULL)
	{
		putline("", mci);
		(void) sprintf(buf, "--%s", e->e_msgboundary);
		putline(buf, mci);
		putline("Content-Type: message/X-delivery-status-2 (Draft of 20 January 1995)", mci);
		putline("", mci);

		/*
		**  Output per-message information.
		*/

		/* original envelope id from MAIL FROM: line */
		if (e->e_parent->e_envid != NULL)
		{
			(void) sprintf(buf, "Original-Envelope-Id: %s",
				xtextify(e->e_parent->e_envid));
			putline(buf, mci);
		}

		/* Reporting-MTA: is us (required) */
		p = e->e_parent->e_from.q_mailer->m_mtatype;
		if (p == NULL)
			p = "dns";
		(void) sprintf(buf, "Reporting-MTA: %s; %s", p,
			xtextify(MyHostName));
		putline(buf, mci);

		/* Received-From-MTA: shows where we got this message from */
		if (RealHostName != NULL)
		{
			/* XXX use $s for type? */
			p = e->e_parent->e_from.q_mailer->m_mtatype;
			if (p == NULL)
				p = "dns";
			(void) sprintf(buf, "Received-From-MTA: %s; %s",
				p, xtextify(RealHostName));
			putline(buf, mci);
		}

		/* Arrival-Date: -- when it arrived here */
		(void) sprintf(buf, "Arrival-Date: %s",
			arpadate(ctime(&e->e_parent->e_ctime)));
		putline(buf, mci);

		/*
		**  Output per-address information.
		*/

		for (q = e->e_parent->e_sendqueue; q != NULL; q = q->q_next)
		{
			register ADDRESS *r;

			if (!bitset(QBADADDR|QREPORT|QRELAYED, q->q_flags))
				continue;
			putline("", mci);

			/* Original-Recipient: -- passed from on high */
			if (q->q_orcpt != NULL)
			{
				(void) sprintf(buf, "Original-Recipient: %s",
					xtextify(q->q_orcpt));
				putline(buf, mci);
			}

			/* Final-Recipient: -- the name from the RCPT command */
			p = e->e_parent->e_from.q_mailer->m_addrtype;
			if (p == NULL)
				p = "rfc822";
			for (r = q; r->q_alias != NULL; r = r->q_alias)
				continue;
			if (strchr(r->q_user, '@') == NULL)
			{
				(void) sprintf(buf, "Final-Recipient: %s; %s@",
					p, xtextify(r->q_user));
				strcat(buf, xtextify(MyHostName));
			}
			else
			{
				(void) sprintf(buf, "Final-Recipient: %s; %s",
					p, xtextify(r->q_user));
			}
			putline(buf, mci);

			/* Action: -- what happened? */
			if (bitset(QBADADDR, q->q_flags))
				putline("Action: failure", mci);
			else if (bitset(QQUEUEUP, q->q_flags))
				putline("Action: delayed", mci);
			else if (bitset(QRELAYED, q->q_flags))
				putline("Action: relayed", mci);
			else
				putline("Action: delivered", mci);

			/* Status: -- what _really_ happened? */
			strcpy(buf, "Status: ");
			if (q->q_status != NULL)
				strcat(buf, q->q_status);
			else if (bitset(QBADADDR, q->q_flags))
				strcat(buf, "5.0.0");
			else if (bitset(QQUEUEUP, q->q_flags))
				strcat(buf, "4.0.0");
			else if (bitset(QRELAYED, q->q_flags))
				strcat(buf, "6.0.1");
			else
				strcat(buf, "2.0.0");
			putline(buf, mci);

			/* Remote-MTA: -- who was I talking to? */
			p = q->q_mailer->m_mtatype;
			if (p == NULL)
				p = "dns";
			(void) sprintf(buf, "Remote-MTA: %s; ", p);
			if (q->q_statmta != NULL)
				p = q->q_statmta;
			else if (q->q_host != NULL)
				p = q->q_host;
			else
				p = NULL;
			if (p != NULL)
			{
				strcat(buf, p);
				p = &buf[strlen(buf) - 1];
				if (*p == '.')
					*p = '\0';
				putline(buf, mci);
			}

			/* Diagnostic-Code: -- actual result from other end */
			if (q->q_rstatus != NULL)
			{
				p = q->q_mailer->m_diagtype;
				if (p == NULL)
					p = "smtp";
				(void) sprintf(buf, "Diagnostic-Code: %s; %s",
					p, xtextify(q->q_rstatus));
				putline(buf, mci);
			}

			/* Last-Attempt-Date: -- fine granularity */
			if (q->q_statdate == (time_t) 0L)
				q->q_statdate = curtime();
			(void) sprintf(buf, "Last-Attempt-Date: %s",
				arpadate(ctime(&q->q_statdate)));
			putline(buf, mci);

			/* Expiry-Date: -- for delayed messages only */
			if (bitset(QQUEUEUP, q->q_flags) &&
			    !bitset(QBADADDR, q->q_flags))
			{
				time_t xdate;

				xdate = e->e_ctime + TimeOuts.to_q_return[e->e_timeoutclass];
				sprintf(buf, "Expiry-Date: %s",
					arpadate(ctime(&xdate)));
				putline(buf, mci);
			}
		}
	}
#endif

	/*
	**  Output text of original message
	*/

	putline("", mci);
	if (bitset(EF_HAS_DF, e->e_parent->e_flags))
	{
		sendbody = !bitset(EF_NO_BODY_RETN, e->e_parent->e_flags);

		if (e->e_msgboundary == NULL)
		{
			if (sendbody)
				putline("   ----- Original message follows -----\n", mci);
			else
				putline("   ----- Message header follows -----\n", mci);
			(void) fflush(mci->mci_out);
		}
		else
		{
			(void) sprintf(buf, "--%s", e->e_msgboundary);
			putline(buf, mci);
			(void) sprintf(buf, "Content-Type: message/rfc822%s",
				mci, sendbody ? "" : "-headers");
			putline(buf, mci);
		}
		putline("", mci);
		putheader(mci, e->e_parent->e_header, e->e_parent);
		if (sendbody)
			putbody(mci, e->e_parent, e->e_msgboundary);
		else if (e->e_msgboundary == NULL)
		{
			putline("", mci);
			putline("   ----- Message body suppressed -----", mci);
		}
	}
	else if (e->e_msgboundary == NULL)
	{
		putline("  ----- No message was collected -----\n", mci);
	}

	if (e->e_msgboundary != NULL)
	{
		putline("", mci);
		(void) sprintf(buf, "--%s--", e->e_msgboundary);
		putline(buf, mci);
	}
	putline("", mci);

	/*
	**  Cleanup and exit
	*/

	if (errno != 0)
		syserr("errbody: I/O error");
}
/*
**  SMTPTODSN -- convert SMTP to DSN status code
**
**	Parameters:
**		smtpstat -- the smtp status code (e.g., 550).
**
**	Returns:
**		The DSN version of the status code.
*/

char *
smtptodsn(smtpstat)
	int smtpstat;
{
	switch (smtpstat)
	{
	  case 450:	/* Req mail action not taken: mailbox unavailable */
		return "4.2.0";

	  case 451:	/* Req action aborted: local error in processing */
		return "4.3.0";

	  case 452:	/* Req action not taken: insufficient sys storage */
		return "4.3.1";

	  case 500:	/* Syntax error, command unrecognized */
		return "5.5.2";

	  case 501:	/* Syntax error in parameters or arguments */
		return "5.5.4";

	  case 502:	/* Command not implemented */
		return "5.5.1";

	  case 503:	/* Bad sequence of commands */
		return "5.5.1";

	  case 504:	/* Command parameter not implemented */
		return "5.5.4";

	  case 550:	/* Req mail action not taken: mailbox unavailable */
		return "5.2.0";

	  case 551:	/* User not local; please try <...> */
		return "5.1.6";

	  case 552:	/* Req mail action aborted: exceeded storage alloc */
		return "5.2.2";

	  case 553:	/* Req action not taken: mailbox name not allowed */
		return "5.1.3";

	  case 554:	/* Transaction failed */
		return "5.0.0";
	}

	if ((smtpstat / 100) == 2)
		return "2.0.0";
	if ((smtpstat / 100) == 4)
		return "4.0.0";
	return "5.0.0";
}
/*
**  XTEXTIFY -- take regular text and turn it into DSN-style xtext
**
**	Parameters:
**		t -- the text to convert.
**
**	Returns:
**		The xtext-ified version of the same string.
*/

char *
xtextify(t)
	register char *t;
{
	register char *p;
	int l;
	int nbogus;
	static char *bp = NULL;
	static int bplen = 0;

	/* figure out how long this xtext will have to be */
	nbogus = l = 0;
	for (p = t; *p != '\0'; p++)
	{
		register int c = (*p & 0xff);

		/* ASCII dependence here -- this is the way the spec words it */
		if (c < '!' || c > '~' || c == '+' || c == '\\' || c == '(')
			nbogus++;
		l++;
	}
	if (nbogus == 0)
		return t;
	l += nbogus * 2 + 1;

	/* now allocate space if necessary for the new string */
	if (l > bplen)
	{
		if (bp != NULL)
			free(bp);
		bp = xalloc(l);
		bplen = l;
	}

	/* ok, copy the text with byte expansion */
	for (p = bp; *t != '\0'; )
	{
		register int c = (*t++ & 0xff);

		/* ASCII dependence here -- this is the way the spec words it */
		if (c < '!' || c > '~' || c == '+' || c == '\\' || c == '(')
		{
			*p++ = '+';
			*p++ = "0123456789abcdef"[c >> 4];
			*p++ = "0123456789abcdef"[c & 0xf];
		}
		else
			*p++ = c;
	}
	*p = '\0';
	return bp;
}
/*
**  XTEXTOK -- check if a string is legal xtext
**
**	Xtext is used in Delivery Status Notifications.  The spec was
**	taken from draft-ietf-notary-mime-delivery-04.txt.
**
**	Parameters:
**		s -- the string to check.
**
**	Returns:
**		TRUE -- if 's' is legal xtext.
**		FALSE -- if it has any illegal characters in it.
*/

bool
xtextok(s)
	char *s;
{
	int c;

	while ((c = *s++) != '\0')
	{
		if (c == '+')
		{
			c = *s++;
			if (!isascii(c) || !isxdigit(c))
				return FALSE;
			c = *s++;
			if (!isascii(c) || !isxdigit(c))
				return FALSE;
		}
		else if (c < '!' || c > '~' || c == '\\' || c == '(')
			return FALSE;
	}
	return TRUE;
}
/*
**  PRUNEROUTE -- prune an RFC-822 source route
** 
**	Trims down a source route to the last internet-registered hop.
**	This is encouraged by RFC 1123 section 5.3.3.
** 
**	Parameters:
**		addr -- the address
** 
**	Returns:
**		TRUE -- address was modified
**		FALSE -- address could not be pruned
** 
**	Side Effects:
**		modifies addr in-place
*/

pruneroute(addr)
	char *addr;
{
#if NAMED_BIND
	char *start, *at, *comma;
	char c;
	int rcode;
	char hostbuf[BUFSIZ];
	char *mxhosts[MAXMXHOSTS + 1];

	/* check to see if this is really a route-addr */
	if (*addr != '<' || addr[1] != '@' || addr[strlen(addr) - 1] != '>')
		return FALSE;
	start = strchr(addr, ':');
	at = strrchr(addr, '@');
	if (start == NULL || at == NULL || at < start)
		return FALSE;

	/* slice off the angle brackets */
	strcpy(hostbuf, at + 1);
	hostbuf[strlen(hostbuf) - 1] = '\0';

	while (start)
	{
		if (getmxrr(hostbuf, mxhosts, FALSE, &rcode) > 0)
		{
			strcpy(addr + 1, start + 1);
			return TRUE;
		}
		c = *start;
		*start = '\0';
		comma = strrchr(addr, ',');
		if (comma && comma[1] == '@')
			strcpy(hostbuf, comma + 2);
		else
			comma = 0;
		*start = c;
		start = comma;
	}
#endif
	return FALSE;
}
