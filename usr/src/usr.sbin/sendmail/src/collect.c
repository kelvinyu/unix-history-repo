# include <stdio.h>
# include <ctype.h>
# include <errno.h>
# include "sendmail.h"

static char	SccsId[] = "@(#)collect.c	3.7	%G%";

/*
**  COLLECT -- read & parse message header & make temp file.
**
**	Creates a temporary file name and copies the standard
**	input to that file.  While it is doing it, it looks for
**	"From:" and "Sender:" fields to use as the from-person
**	(but only if the -a flag is specified).  It prefers to
**	to use the "Sender:" field.
**
**	MIT seems to like to produce "Sent-By:" fields instead
**	of "Sender:" fields.  We used to catch this, but it turns
**	out that the "Sent-By:" field doesn't always correspond
**	to someone real ("___057", for instance), as required by
**	the protocol.  So we limp by.....
**
**	Parameters:
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
**
**	Called By:
**		main
**
**	Notes:
**		This is broken off from main largely so that the
**		temp buffer can be deallocated.
*/

char	*MsgId;			/* message-id, determined or created */
long	MsgSize;		/* size of message in bytes */
char	*Date;			/* UNIX-style origination date */

char *
maketemp(from)
	char *from;
{
	register FILE *tf;
	char buf[MAXFIELD+1];
	register char *p;
	char c;
	extern int errno;
	register HDR *h;
	HDR **hp;
	extern bool isheader();
	extern char *newstr();
	extern char *xalloc();
	char *fname;
	char *fvalue;
	extern char *index(), *rindex();
	char *xfrom;
	extern char *hvalue();
	struct hdrinfo *hi;
	extern char *strcpy(), *strcat(), *mktemp();
	extern char *index();

	/*
	**  Create the temp file name and create the file.
	*/

	mktemp(InFileName);
	close(creat(InFileName, 0600));
	if ((tf = fopen(InFileName, "w")) == NULL)
	{
		syserr("Cannot create %s", InFileName);
		return (NULL);
	}

	/* try to read a UNIX-style From line */
	if (fgets(buf, sizeof buf, stdin) == NULL)
		return (NULL);
	if (strncmp(buf, "From ", 5) == 0)
	{
		eatfrom(buf);
		fgets(buf, sizeof buf, stdin);
	}

	/*
	**  Copy stdin to temp file & do message editting.
	**	To keep certain mailers from getting confused,
	**	and to keep the output clean, lines that look
	**	like UNIX "From" lines are deleted in the header,
	**	and prepended with ">" in the body.
	*/

	for (; !feof(stdin); !feof(stdin) && fgets(buf, sizeof buf, stdin))
	{
		/* see if the header is over */
		if (!isheader(buf))
			break;

		/* get the rest of this field */
		while ((c = getc(stdin)) == ' ' || c == '\t')
		{
			p = &buf[strlen(buf)];
			*p++ = c;
			if (fgets(p, sizeof buf - (p - buf), stdin) == NULL)
				break;
		}
		if (c != EOF)
			ungetc(c, stdin);

		MsgSize += strlen(buf);

		/*
		**  Snarf header away.
		*/

		/* strip off trailing newline */
		p = rindex(buf, '\n');
		if (p != NULL)
			*p = '\0';

		/* find canonical name */
		fname = buf;
		p = index(buf, ':');
		fvalue = &p[1];
		while (isspace(*--p))
			continue;
		*++p = '\0';
		makelower(fname);

		/* strip field value on front */
		if (*fvalue == ' ')
			fvalue++;

		/* search header list for this header */
		for (hp = &Header, h = Header; h != NULL; hp = &h->h_link, h = h->h_link)
		{
			if (strcmp(fname, h->h_field) == 0 && bitset(H_DEFAULT, h->h_flags))
				break;
		}

		/* see if it is a known type */
		for (hi = HdrInfo; hi->hi_field != NULL; hi++)
		{
			if (strcmp(hi->hi_field, fname) == 0)
				break;
		}

		/* if this means "end of header" quit now */
		if (bitset(H_EOH, hi->hi_flags))
			break;

		/* create/fill in a new node */
		if (h == NULL)
		{
			/* create a new node */
			*hp = h = (HDR *) xalloc(sizeof *h);
			h->h_field = newstr(fname);
			h->h_value = NULL;
			h->h_link = NULL;
			h->h_flags = hi->hi_flags;
		}
		if (h->h_value != NULL)
			free(h->h_value);
		h->h_value = newstr(fvalue);

		/* save the location of this field */
		if (hi->hi_pptr != NULL)
			*hi->hi_pptr = h->h_value;
	}

# ifdef DEBUG
	if (Debug)
		printf("EOH\n");
# endif DEBUG

	/* throw away a blank line */
	if (buf[0] == '\n')
		fgets(buf, sizeof buf, stdin);

	/*
	**  Collect the body of the message.
	*/

	for (; !feof(stdin); !feof(stdin) && fgets(buf, sizeof buf, stdin) != NULL)
	{
		/* check for end-of-message */
		if (!IgnrDot && buf[0] == '.' && (buf[1] == '\n' || buf[1] == '\0'))
			break;

		/* Hide UNIX-like From lines */
		if (strncmp(buf, "From ", 5) == 0)
		{
			fputs(">", tf);
			MsgSize++;
		}
		MsgSize += strlen(buf);
		fputs(buf, tf);
		if (ferror(tf))
		{
			if (errno == ENOSPC)
			{
				freopen(InFileName, "w", tf);
				fputs("\nMAIL DELETED BECAUSE OF LACK OF DISK SPACE\n\n", tf);
				syserr("Out of disk space for temp file");
			}
			else
				syserr("Cannot write %s", InFileName);
			freopen("/dev/null", "w", tf);
		}
	}
	fclose(tf);

	/*
	**  Find out some information from the headers.
	**	Examples are who is the from person & the date.  Some
	**	fields, e.g., Message-Id, may have been handled by
	**	the hi_pptr mechanism.
	*/

	/* from person */
	xfrom = hvalue("sender");
	if (xfrom == NULL)
		xfrom = hvalue("from");

	/* date message originated */
	/* we don't seem to have a good way to do canonical conversion ....
	p = hvalue("date");
	if (p != NULL)
		Date = newstr(arpatounix(p));
	.... so we will ignore the problem for the time being */
	if (Date == NULL)
	{
		extern char *ctime();

		Date = newstr(ctime(&CurTime));
	}

	if (freopen(InFileName, "r", stdin) == NULL)
		syserr("Cannot reopen %s", InFileName);

# ifdef DEBUG
	if (Debug)
	{
		printf("----- collected header -----\n");
		for (h = Header; h != NULL; h = h->h_link)
			printf("%s: %s\n", capitalize(h->h_field), h->h_value);
		printf("----------------------------\n");
	}
# endif DEBUG
	return (ArpaFmt ? xfrom : NULL);
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
**		such as the Date.
*/

char	*MonthList[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	NULL
};

eatfrom(fm)
	char *fm;
{
	register char *p;
	register char **dt;

	/* find the date part */
	p = fm;
	while (*p != '\0')
	{
		/* skip a word */
		while (*p != '\0' && *p != ' ')
			*p++;
		while (*p == ' ')
			*p++;
		if (!isupper(*p) || p[3] != ' ' || p[13] != ':' || p[16] != ':')
			continue;

		/* we have a possible date */
		for (dt = MonthList; *dt != NULL; dt++)
			if (strncmp(*dt, p, 3) == 0)
				break;

		if (*dt != NULL)
			break;
	}

	if (*p != NULL)
	{
		/* we have found a date */
		Date = xalloc(25);
		strncpy(Date, p, 25);
		Date[24] = '\0';
	}
}
/*
**  HVALUE -- return value of a header.
**
**	Parameters:
**		field -- the field name.
**
**	Returns:
**		pointer to the value part.
**		NULL if not found.
**
**	Side Effects:
**		sets the H_USED bit in the header if found.
*/

char *
hvalue(field)
	char *field;
{
	register HDR *h;

	for (h = Header; h != NULL; h = h->h_link)
	{
		if (strcmp(h->h_field, field) == 0)
		{
			h->h_flags |= H_USED;
			return (h->h_value);
		}
	}
	return (NULL);
}
/*
**  MAKEMSGID -- Compute a message id for this process.
**
**	This routine creates a message id for a message if
**	it did not have one already.  If the MESSAGEID compile
**	flag is set, the messageid will be added to any message
**	that does not already have one.  Currently it is more
**	of an artifact, but I suggest that if you are hacking,
**	you leave it in -- I may want to use it someday if
**	duplicate messages turn out to be a problem.
**
**	Parameters:
**		none.
**
**	Returns:
**		a message id.
**
**	Side Effects:
**		none.
*/

char *
makemsgid()
{
	static char buf[50];
	extern char *expand();

	expand("<$m>", buf, &buf[sizeof buf - 1]);
	return (buf);
}
/*
**  ISHEADER -- predicate telling if argument is a header.
**
**	Parameters:
**		s -- string to check for possible headerness.
**
**	Returns:
**		TRUE if s is a header.
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

bool
isheader(s)
	register char *s;
{
	if (!isalnum(*s))
		return (FALSE);
	while (!isspace(*s) && *s != ':')
		s++;
	while (isspace(*s))
		s++;
	return (*s == ':');
}
/*
**  SAVEDATE -- find and save date field from a "From" line
**
**	This will be used by putheader when a From line is created.
**
**	Parameters:
**		buf -- a "From" line.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Saves the "date" part (with newline) in SentDate.
*/

char	SentDate[30];

savedate(buf)
	char *buf;
{
	register char *p;

	for (p = buf; p != '\0'; p++)
	{
		if (*p != ' ')
			continue;
		if (strncmp(p, " Sun ", 5) == 0 ||
		    strncmp(p, " Mon ", 5) == 0 ||
		    strncmp(p, " Tue ", 5) == 0 ||
		    strncmp(p, " Wed ", 5) == 0 ||
		    strncmp(p, " Thu ", 5) == 0 ||
		    strncmp(p, " Fri ", 5) == 0 ||
		    strncmp(p, " Sat ", 5) == 0)
		{
			if (p[4] != ' ' || p[8] != ' ' || p[11] != ' ' ||
			    p[14] != ':' || p[17] != ':' || p[20] != ' ')
				continue;
			strncpy(SentDate, ++p, 25);
			SentDate[24] = '\n';
			SentDate[25] = '\0';
			return;
		}
	}
}
