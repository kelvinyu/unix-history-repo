/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)collect.c	8.19 (Berkeley) %G%";
#endif /* not lint */

# include <errno.h>
# include "sendmail.h"

/*
**  COLLECT -- read & parse message header & make temp file.
**
**	Creates a temporary file name and copies the standard
**	input to that file.  Leading UNIX-style "From" lines are
**	stripped off (after important information is extracted).
**
**	Parameters:
**		fp -- file to read.
**		from -- the person we think it may be from.  If
**			there is a "From" line, we will replace
**			the name of the person by this.  If NULL,
**			do no such replacement.
**
**	Returns:
**		Name of the "from" person extracted from the
**		arpanet header.
**
**	Side Effects:
**		Temp file is created and filled.
**		The from person may be set.
*/

char	*CollectErrorMessage;
bool	CollectErrno;

static jmp_buf	CtxCollectTimeout;
static int	collecttimeout();
static bool	CollectProgress;
static EVENT	*CollectTimeout;

/* values for input state machine */
#define IS_NORM		0	/* middle of line */
#define IS_BOL		1	/* beginning of line */
#define IS_DOT		2	/* read a dot at beginning of line */
#define IS_DOTCR	3	/* read ".\r" at beginning of line */
#define IS_CR		4	/* read a carriage return */

/* values for message state machine */
#define MS_UFROM	0	/* reading Unix from line */
#define MS_HEADER	1	/* reading message header */
#define MS_BODY		2	/* reading message body */


maketemp(from)
	char *from;
{
	register FILE *tf;
	bool ignrdot = smtpmode ? FALSE : IgnrDot;
	time_t dbto = smtpmode ? TimeOuts.to_datablock : 0;
	register char *bp;
	register int c;
	bool inputerr = FALSE;
	bool headeronly = FALSE;
	char *buf;
	int buflen;
	int istate;
	int mstate;
	char *pbp;
	char peekbuf[8];
	char bufbuf[MAXLINE];
	extern char *hvalue();
	extern bool isheader();
	extern char *index();

	CollectErrorMessage = NULL;
	CollectErrno = 0;
	if (hdrp == NULL)
		hdrp = &e->e_header;
	else
		headeronly = TRUE;

	/*
	**  Create the temp file name and create the file.
	*/

	if (!headeronly)
	{
		e->e_df = queuename(e, 'd');
		e->e_df = newstr(e->e_df);
		if ((tf = dfopen(e->e_df, O_WRONLY|O_CREAT|O_TRUNC, FileMode)) == NULL)
		{
			syserr("Cannot create %s", e->e_df);
			e->e_flags |= EF_NORETURN;
			finis();
		}
		HasEightBits = FALSE;
	}

	/*
	**  Tell ARPANET to go ahead.
	*/

	if (smtpmode)
		message("354 Enter mail, end with \".\" on a line by itself");

	/*
	**  Read the message.
	**
	**	This is done using two interleaved state machines.
	**	The input state machine is looking for things like
	**	hidden dots; the message state machine is handling
	**	the larger picture (e.g., header versus body).
	*/

	buf = bp = bufbuf;
	buflen = sizeof bufbuf;
	pbp = peekbuf;
	istate = IS_BOL;
	mstate = SaveFrom ? MS_HEADER : MS_UFROM;
	CollectProgress = FALSE;

	/* if transmitting binary, don't map NL to EOL */
	if (e->e_bodytype != NULL && strcasecmp(e->e_bodytype, "8BITMIME") == 0)
		e->e_flags |= EF_NL_NOT_EOL;

	if (dbto != 0)
	{
		/* handle possible input timeout */
		if (setjmp(CtxCollectTimeout) != 0)
		{
#ifdef LOG
			syslog(LOG_NOTICE,
			    "timeout waiting for input from %s during message collect",
			    CurHostName ? CurHostName : "<local machine>");
#endif
			errno = 0;
			usrerr("451 timeout waiting for input during message collect");
			goto readerr;
		}
		CollectTimeout = setevent(dbto, collecttimeout, dbto);
	}

	for (;;)
	{
		if (tTd(30, 35))
			printf("top, istate=%d, mstate=%d\n", istate, mstate);
		for (;;)
		{
			if (pbp > peekbuf)
				c = *--pbp;
			else
			{
				while (!feof(InChannel) && !ferror(InChannel))
				{
					errno = 0;
					c = fgetc(InChannel);
					if (errno != EINTR)
						break;
					clearerr(InChannel);
				}
				CollectProgress = TRUE;
				if (TrafficLogFile != NULL)
				{
					if (istate == IS_BOL)
						fprintf(TrafficLogFile, "%05d <<< ",
							getpid());
					if (c == EOF)
						fprintf(TrafficLogFile, "[EOF]\n");
					else
						fputc(c, TrafficLogFile);
				}
				if (c == EOF)
					goto readerr;
				if (SevenBitInput)
					c &= 0x7f;
				else
					HasEightBits |= bitset(0x80, c);
				e->e_msgsize++;
			}
			if (tTd(30, 94))
				printf("istate=%d, c=%c (0x%x)\n",
					istate, c, c);
			switch (istate)
			{
			  case IS_BOL:
				if (c == '.')
				{
					istate = IS_DOT;
					continue;
				}
				break;

			  case IS_DOT:
				if (c == '\n' && !ignrdot &&
				    !bitset(EF_NL_NOT_EOL, e->e_flags))
					goto readerr;
				else if (c == '\r' &&
					 !bitset(EF_CRLF_NOT_EOL, e->e_flags))
				{
					istate = IS_DOTCR;
					continue;
				}
				else if (c != '.' ||
					 (OpMode != MD_SMTP &&
					  OpMode != MD_DAEMON &&
					  OpMode != MD_ARPAFTP))
				{
					*pbp++ = c;
					c = '.';
				}
				break;

			  case IS_DOTCR:
				if (c == '\n')
					goto readerr;
				else
				{
					/* push back the ".\rx" */
					*pbp++ = c;
					*pbp++ = '\r';
					c = '.';
				}
				break;

			  case IS_CR:
				if (c != '\n')
				{
					ungetc(c, InChannel);
					c = '\r';
				}
				else if (!bitset(EF_CRLF_NOT_EOL, e->e_flags))
					istate = IS_BOL;
				break;
			}

			if (c == '\r')
			{
				istate = IS_CR;
				continue;
			}
			else if (c == '\n' && !bitset(EF_NL_NOT_EOL, e->e_flags))
				istate = IS_BOL;
			else
				istate = IS_NORM;

			if (mstate == MS_BODY)
			{
				/* just put the character out */
				fputc(c, tf);
				continue;
			}

			/* header -- buffer up */
			if (bp >= &buf[buflen - 2])
			{
				char *obuf;

				if (mstate != MS_HEADER)
					break;

				/* out of space for header */
				obuf = buf;
				if (buflen < MEMCHUNKSIZE)
					buflen *= 2;
				else
					buflen += MEMCHUNKSIZE;
				buf = xalloc(buflen);
				bcopy(obuf, buf, bp - obuf);
				bp = &buf[bp - obuf];
				if (obuf != bufbuf)
					free(obuf);
			}
			*bp++ = c;
			if (istate == IS_BOL)
				break;
		}
		*bp = '\0';

nextstate:
		if (tTd(30, 35))
			printf("nextstate, istate=%d, mstate=%d, line = \"%s\"\n",
				istate, mstate, buf);
		switch (mstate)
		{
		  case MS_UFROM:
			mstate = MS_HEADER;
			if (strncmp(buf, "From ", 5) == 0)
			{
				eatfrom(buf, e);
				continue;
			}
			/* fall through */

		  case MS_HEADER:
			if (!isheader(buf))
			{
				mstate = MS_BODY;
				goto nextstate;
			}

			/* check for possible continuation line */
			do
			{
				clearerr(InChannel);
				errno = 0;
				c = fgetc(InChannel);
			} while (errno == EINTR);
			if (c != EOF)
				ungetc(c, InChannel);
			if (c == ' ' || c == '\t')
			{
				/* yep -- defer this */
				continue;
			}

			/* trim off trailing CRLF or NL */
			if (*--bp != '\n' || *--bp != '\r')
				bp++;
			*bp = '\0';
			if (bitset(H_EOH, chompheader(buf, FALSE, e)))
				mstate = MS_BODY;
			break;

		  case MS_BODY:
			if (tTd(30, 1))
				printf("EOH\n");
			if (headeronly)
				goto readerr;
			bp = buf;

			/* toss blank line */
			if ((!bitset(EF_CRLF_NOT_EOL, e->e_flags) &&
				bp[0] == '\r' && bp[1] == '\n') ||
			    (!bitset(EF_NL_NOT_EOL, e->e_flags) &&
				bp[0] == '\n'))
			{
				break;
			}

			/* if not a blank separator, write it out */
			while (*bp != '\0')
				fputc(*bp++, tf);
			break;
		}
		bp = buf;
	}

readerr:
	if ((feof(fp) && smtpmode) || ferror(fp))
	{
		if (tTd(30, 1))
			printf("collect: read error\n");
		inputerr = TRUE;
	}

	/* reset global timer */
	clrevent(CollectTimeout);

	if (headeronly)
		return;

	if (tf != NULL)
	{
		if (fflush(tf) != 0)
			tferror(tf, e);
		if (fsync(fileno(tf)) < 0 || fclose(tf) < 0)
		{
			tferror(tf, e);
			finis();
		}
	}

	if (CollectErrorMessage != NULL && Errors <= 0)
	{
		if (CollectErrno != 0)
		{
			errno = CollectErrno;
			syserr(CollectErrorMessage, e->e_df);
			finis();
		}
		usrerr(CollectErrorMessage);
	}
	else if (inputerr && (OpMode == MD_SMTP || OpMode == MD_DAEMON))
	{
		/* An EOF when running SMTP is an error */
		char *host;
		char *problem;

		host = RealHostName;
		if (host == NULL)
			host = "localhost";

		if (feof(fp))
			problem = "unexpected close";
		else if (ferror(fp))
			problem = "I/O error";
		else
			problem = "read timeout";
# ifdef LOG
		if (LogLevel > 0 && feof(fp))
			syslog(LOG_NOTICE,
			    "collect: %s on connection from %s, sender=%s: %s\n",
			    problem, host, e->e_from.q_paddr, errstring(errno));
# endif
		if (feof(fp))
			usrerr("451 collect: %s on connection from %s, from=%s",
				problem, host, e->e_from.q_paddr);
		else
			syserr("451 collect: %s on connection from %s, from=%s",
				problem, host, e->e_from.q_paddr);

		/* don't return an error indication */
		e->e_to = NULL;
		e->e_flags &= ~EF_FATALERRS;
		e->e_flags |= EF_CLRQUEUE;

		/* and don't try to deliver the partial message either */
		if (InChild)
			ExitStat = EX_QUIT;
		finis();
	}

	/*
	**  Find out some information from the headers.
	**	Examples are who is the from person & the date.
	*/

	eatheader(e, !requeueflag);

	/* collect statistics */
	if (OpMode != MD_VERIFY)
		markstats(e, (ADDRESS *) NULL);

	/*
	**  Add an Apparently-To: line if we have no recipient lines.
	*/

	if (hvalue("to", e->e_header) == NULL &&
	    hvalue("cc", e->e_header) == NULL &&
	    hvalue("bcc", e->e_header) == NULL &&
	    hvalue("apparently-to", e->e_header) == NULL)
	{
		register ADDRESS *q;

		/* create an Apparently-To: field */
		/*    that or reject the message.... */
		for (q = e->e_sendqueue; q != NULL; q = q->q_next)
		{
			if (q->q_alias != NULL)
				continue;
			if (tTd(30, 3))
				printf("Adding Apparently-To: %s\n", q->q_paddr);
			addheader("Apparently-To", q->q_paddr, &e->e_header);
		}
	}

	/* check for message too large */
	if (MaxMessageSize > 0 && e->e_msgsize > MaxMessageSize)
	{
		usrerr("552 Message exceeds maximum fixed size (%ld)",
			MaxMessageSize);
	}

	/* check for illegal 8-bit data */
	if (HasEightBits)
	{
		e->e_flags |= EF_HAS8BIT;
		if (bitset(MM_MIME8BIT, MimeMode))
		{
			/* convert it to MIME */
			if (hvalue("MIME-Version", e->e_header) == NULL)
			{
				char mimebuf[20];

				strcpy(mimebuf, "MIME-Version: 1.0");
				chompheader(mimebuf, FALSE, e);
			}
			if (e->e_bodytype == NULL)
				e->e_bodytype = "8BITMIME";
		}
		else if (!bitset(MM_PASS8BIT, MimeMode))
			usrerr("554 Eight bit data not allowed");
	}

	if ((e->e_dfp = fopen(e->e_df, "r")) == NULL)
	{
		/* we haven't acked receipt yet, so just chuck this */
		syserr("Cannot reopen %s", e->e_df);
		finis();
	}
}


static
collecttimeout(timeout)
	time_t timeout;
{
	/* if no progress was made, die now */
	if (!CollectProgress)
		longjmp(CtxCollectTimeout, 1);

	/* otherwise reset the timeout */
	CollectTimeout = setevent(timeout, collecttimeout, timeout);
	CollectProgress = FALSE;
}
/*
**  TFERROR -- signal error on writing the temporary file.
**
**	Parameters:
**		tf -- the file pointer for the temporary file.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Gives an error message.
**		Arranges for following output to go elsewhere.
*/

tferror(tf, e)
	FILE *tf;
	register ENVELOPE *e;
{
	CollectErrno = errno;
	if (errno == ENOSPC)
	{
		struct stat st;
		long avail;
		long bsize;

		e->e_flags |= EF_NORETURN;
		if (fstat(fileno(tf), &st) < 0)
			st.st_size = 0;
		(void) freopen(e->e_df, "w", tf);
		if (st.st_size <= 0)
			fprintf(tf, "\n*** Mail could not be accepted");
		else if (sizeof st.st_size > sizeof (long))
			fprintf(tf, "\n*** Mail of at least %qd bytes could not be accepted\n",
				st.st_size);
		else
			fprintf(tf, "\n*** Mail of at least %ld bytes could not be accepted\n",
				st.st_size);
		fprintf(tf, "*** at %s due to lack of disk space for temp file.\n",
			MyHostName);
		avail = freespace(QueueDir, &bsize);
		if (avail > 0)
		{
			if (bsize > 1024)
				avail *= bsize / 1024;
			else if (bsize < 1024)
				avail /= 1024 / bsize;
			fprintf(tf, "*** Currently, %ld kilobytes are available for mail temp files.\n",
				avail);
		}
		CollectErrorMessage = "452 Out of disk space for temp file";
	}
	else
	{
		CollectErrorMessage = "cannot write message body to disk (%s)";
	}
	(void) freopen("/dev/null", "w", tf);
}
/*
**  EATFROM -- chew up a UNIX style from line and process
**
**	This does indeed make some assumptions about the format
**	of UNIX messages.
**
**	Parameters:
**		fm -- the from line.
**
**	Returns:
**		none.
**
**	Side Effects:
**		extracts what information it can from the header,
**		such as the date.
*/

# ifndef NOTUNIX

char	*DowList[] =
{
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
};

char	*MonthList[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	NULL
};

eatfrom(fm, e)
	char *fm;
	register ENVELOPE *e;
{
	register char *p;
	register char **dt;

	if (tTd(30, 2))
		printf("eatfrom(%s)\n", fm);

	/* find the date part */
	p = fm;
	while (*p != '\0')
	{
		/* skip a word */
		while (*p != '\0' && *p != ' ')
			p++;
		while (*p == ' ')
			p++;
		if (!(isascii(*p) && isupper(*p)) ||
		    p[3] != ' ' || p[13] != ':' || p[16] != ':')
			continue;

		/* we have a possible date */
		for (dt = DowList; *dt != NULL; dt++)
			if (strncmp(*dt, p, 3) == 0)
				break;
		if (*dt == NULL)
			continue;

		for (dt = MonthList; *dt != NULL; dt++)
			if (strncmp(*dt, &p[4], 3) == 0)
				break;
		if (*dt != NULL)
			break;
	}

	if (*p != '\0')
	{
		char *q;
		extern char *arpadate();

		/* we have found a date */
		q = xalloc(25);
		(void) strncpy(q, p, 25);
		q[24] = '\0';
		q = arpadate(q);
		define('a', newstr(q), e);
	}
}

# endif /* NOTUNIX */
