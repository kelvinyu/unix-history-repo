/*
 * Copyright (c) 1994 Eric P. Allman
 * Copyright (c) 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

# include "sendmail.h"
# include <string.h>

#ifndef lint
static char sccsid[] = "@(#)mime.c	8.1 (Berkeley) %G%";
#endif /* not lint */

/*
**  MIME support.
**
**	I am indebted to John Beck of Hewlett-Packard, who contributed
**	his code to me for inclusion.  As it turns out, I did not use
**	his code since he used a "minimum change" approach that used
**	several temp files, and I wanted a "minimum impact" approach
**	that would avoid copying.  However, looking over his code
**	helped me cement my understanding of the problem.
**
**	I also looked at, but did not directly use, Nathaniel
**	Borenstein's "code.c" module.  Again, it functioned as
**	a file-to-file translator, which did not fit within my
**	design bounds, but it was a useful base for understanding
**	the problem.
*/


/* character set for hex and base64 encoding */
char	Base16Code[] =	"0123456789ABCDEF";
char	Base64Code[] =	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* types of MIME boundaries */
#define MBT_SYNTAX	0	/* syntax error */
#define MBT_NOTSEP	1	/* not a boundary */
#define MBT_INTERMED	2	/* intermediate boundary (no trailing --) */
#define MBT_FINAL	3	/* final boundary (trailing -- included) */
/*
**  MIME8TO7 -- output 8 bit body in 7 bit format
**
**	The header has already been output -- this has to do the
**	8 to 7 bit conversion.  It would be easy if we didn't have
**	to deal with nested formats (multipart/xxx and message/rfc822).
**
**	We won't be called if we don't have to do a conversion, and
**	appropriate MIME-Version: and Content-Type: fields have been
**	output.  Any Content-Transfer-Encoding: field has not been
**	output, and we can add it here.
**
**	Parameters:
**		mci -- mailer connection information.
**		header -- the header for this body part.
**		e -- envelope.
**		boundary -- the message boundary -- NULL if we are
**			processing the outer portion.
**
**	Returns:
**		An indicator of what terminated the message part:
**		  MBT_FINAL -- the final boundary
**		  MBT_INTERMED -- an intermediate boundary
**		  MBT_NOTSEP -- an end of file
*/

int
mime8to7(mci, header, e, boundary)
	register MCI *mci;
	HDR *header;
	register ENVELOPE *e;
	char *boundary;
{
	register char *p;
	int linelen;
	int bt;
	off_t offset;
	size_t sectionsize, sectionhighbits;
	char bbuf[128];
	char buf[MAXLINE];
	extern char *hvalue();

	if (tTd(43, 1))
	{
		printf("mime8to7: boundary=%s\n",
			boundary == NULL ? "<none>" : boundary);
	}
	p = hvalue("Content-Type", header);
	if (p != NULL && strncasecmp(p, "multipart/", 10) == 0)
	{
		register char *q;

		/* oh dear -- this part is hard */
		p = strstr(p, "boundary=");		/*XXX*/
		if (p == NULL)
		{
			syserr("mime8to7: Content-Type: %s missing boundary", p);
			p = "---";
		}
		else
			p += 9;
		if (*p == '"')
			q = strchr(p, '"');
		else
			q = strchr(p, ',');
		if (q == NULL)
			q = p + strlen(p);
		if (q - p > sizeof bbuf - 1)
		{
			syserr("mime8to7: multipart boundary \"%.*s\" too long",
				q - p, p);
			q = p + sizeof bbuf - 1;
		}
		strncpy(bbuf, p, q - p);
		bbuf[q - p] = '\0';
		if (tTd(43, 1))
		{
			printf("mime8to7: multipart boundary \"%s\"\n", bbuf);
		}

		/* skip the early "comment" prologue */
		bt = MBT_FINAL;
		while (fgets(buf, sizeof buf, e->e_dfp) != NULL)
		{
			bt = mimeboundary(buf, bbuf);
			if (bt != MBT_NOTSEP)
				break;
			putline(buf, mci);
		}
		while (bt != MBT_FINAL)
		{
			auto HDR *hdr = NULL;

			sprintf(buf, "--%s", bbuf);
			putline(buf, mci);
			collect(e->e_dfp, FALSE, FALSE, &hdr, e);
			putheader(mci, hdr, e);
			bt = mime8to7(mci, hdr, e, bbuf);
		}
		sprintf(buf, "--%s--", bbuf);
		putline(buf, mci);

		/* skip the late "comment" epilogue */
		while (fgets(buf, sizeof buf, e->e_dfp) != NULL)
		{
			putline(buf, mci);
			bt = mimeboundary(buf, boundary);
			if (bt != MBT_NOTSEP)
				break;
		}
		return bt;
	}

	/*
	**  Non-compound body type
	**
	**	Compute the ratio of seven to eight bit characters;
	**	use that as a heuristic to decide how to do the
	**	encoding.
	*/

	/* remember where we were */
	offset = ftell(e->e_dfp);
	if (offset == -1)
		syserr("mime8to7: cannot ftell on %s", e->e_df);

	/* do a scan of this body type to count character types */
	sectionsize = sectionhighbits = 0;
	while (fgets(buf, sizeof buf, e->e_dfp) != NULL)
	{
		bt = mimeboundary(buf, boundary);
		if (bt != MBT_NOTSEP)
			break;
		for (p = buf; *p != '\0'; p++)
		{
			sectionsize++;
			if (bitset(0200, *p))
				sectionhighbits++;
		}
	}
	if (feof(e->e_dfp))
		bt = MBT_FINAL;

	/* return to the original offset for processing */
	if (fseek(e->e_dfp, offset, SEEK_SET) < 0)
		syserr("mime8to7: cannot fseek on %s", e->e_df);

	/* heuristically determine encoding method */
	if (tTd(43, 8))
	{
		printf("mime8to7: %ld high bits in %ld bytes\n",
			sectionhighbits, sectionsize);
	}
	if (sectionsize / 8 < sectionhighbits)
	{
		/* use base64 encoding */
		int c1, c2;

		putline("Content-Transfer-Encoding: base64", mci);
		putline("", mci);
		mci->mci_flags &= ~MCIF_INHEADER;
		linelen = 0;
		while ((c1 = mime_getchar(e->e_dfp, boundary)) != EOF)
		{
			if (linelen > 71)
			{
				fputs(mci->mci_mailer->m_eol, mci->mci_out);
				linelen = 0;
			}
			linelen += 4;
			fputc(Base64Code[c1 >> 2], mci->mci_out);
			c1 = (c1 & 0x03) << 4;
			c2 = mime_getchar(e->e_dfp, boundary);
			if (c2 == EOF)
			{
				fputc(Base64Code[c1], mci->mci_out);
				fputc('=', mci->mci_out);
				fputc('=', mci->mci_out);
				break;
			}
			c1 |= (c2 >> 4) & 0x0f;
			fputc(Base64Code[c1], mci->mci_out);
			c1 = (c2 & 0x0f) << 2;
			c2 = mime_getchar(e->e_dfp, boundary);
			if (c2 == EOF)
			{
				fputc(Base64Code[c1], mci->mci_out);
				fputc('=', mci->mci_out);
				break;
			}
			c1 |= (c2 >> 6) & 0x03;
			fputc(Base64Code[c1], mci->mci_out);
			fputc(Base64Code[c2 & 0x3f], mci->mci_out);
		}
	}
	else
	{
		/* use quoted-printable encoding */
		int c1, c2;

		putline("Content-Transfer-Encoding: quoted-printable", mci);
		putline("", mci);
		mci->mci_flags &= ~MCIF_INHEADER;
		linelen = 0;
		c2 = EOF;
		while ((c1 = mime_getchar(e->e_dfp, boundary)) != EOF)
		{
			if (c1 == '\n')
			{
				if (c2 == ' ' || c2 == '\t')
				{
					fputc('=', mci->mci_out);
					fputs(mci->mci_mailer->m_eol, mci->mci_out);
				}
				fputs(mci->mci_mailer->m_eol, mci->mci_out);
				linelen = 0;
				c2 = c1;
				continue;
			}
			if (linelen > 72)
			{
				fputc('=', mci->mci_out);
				fputs(mci->mci_mailer->m_eol, mci->mci_out);
				linelen = 0;
			}
			if ((c1 < 0x20 && c1 != '\t') || c2 >= 0xff || c2 == '=')
			{
				fputc('=', mci->mci_out);
				fputc(Base16Code[(c1 >> 4) & 0x0f], mci->mci_out);
				fputc(Base16Code[c1 & 0x0f], mci->mci_out);
				linelen += 3;
			}
			else
			{
				fputc(c1, mci->mci_out);
				linelen++;
			}
			c2 = c1;
		}
	}
	if (linelen > 0)
		fputs(mci->mci_mailer->m_eol, mci->mci_out);
	return bt;
}


int
mime_getchar(fp, boundary)
	register FILE *fp;
	char *boundary;
{
	int c;
	static char *bp = NULL;
	static int buflen = 0;
	static bool atbol = TRUE;	/* at beginning of line */
	static char buf[128];		/* need not be a full line */

	if (buflen > 0)
	{
		buflen--;
		return *bp++;
	}
	c = fgetc(fp);
	if (atbol && c == '-' && boundary != NULL)
	{
		/* check for a message boundary */
		bp = buf;
		c = fgetc(fp);
		if (c != '-')
		{
			if (c != EOF)
			{
				*bp = c;
				buflen++;
			}
			return '-';
		}

		/* got "--", now check for rest of separator */
		*bp++ = '-';
		*bp++ = '-';
		while (bp < &buf[sizeof buf - 1] &&
		       (c = fgetc(fp)) != EOF && c != '\n')
		{
			*bp++ = c;
		}
		*bp = '\0';
		switch (mimeboundary(buf, boundary))
		{
		  case MBT_FINAL:
		  case MBT_INTERMED:
			/* we have a message boundary */
			buflen = 0;
			return EOF;
		}

		atbol = c == '\n';
		if (c != EOF)
			*bp++ = c;
		buflen = bp - buf - 1;
		bp = buf;
		return *bp++;
	}
	else if (atbol && c == '.')
	{
		/* implement hidden dot algorithm */
		bp = buf;
		*bp = c;
		buflen = 1;
		c = fgetc(fp);
		if (c != '\n')
			return '.';
		atbol = TRUE;
		buf[0] = '.';
		buf[1] = '\n';
		buflen = 2;
		return '.';
	}

	atbol = c == '\n';
	return c;
}
/*
**  MIMEBOUNDARY -- determine if this line is a MIME boundary & its type
**
**	Parameters:
**		line -- the input line.
**		boundary -- the expected boundary.
**
**	Returns:
**		MBT_NOTSEP -- if this is not a separator line
**		MBT_INTERMED -- if this is an intermediate separator
**		MBT_FINAL -- if this is a final boundary
**		MBT_SYNTAX -- if this is a boundary for the wrong
**			enclosure -- i.e., a syntax error.
*/

int
mimeboundary(line, boundary)
	register char *line;
	char *boundary;
{
	int type;
	int i;

	if (line[0] != '-' || line[1] != '-' || boundary == NULL)
		return MBT_NOTSEP;
	if (tTd(43, 5))
		printf("mimeboundary: bound=\"%s\", line=\"%s\"... ",
			boundary, line);
	i = strlen(line);
	if (line[i - 1] == '\n')
		i--;
	if (i > 2 && strncmp(&line[i - 2], "--", 2) == 0)
	{
		type = MBT_FINAL;
		i -= 2;
	}
	else
		type = MBT_INTERMED;

	/* XXX should check for improper nesting here */
	if (strncmp(boundary, &line[2], i - 2) != 0 ||
	    strlen(boundary) != i - 2)
		type = MBT_NOTSEP;
	if (tTd(43, 5))
		printf("%d\n", type);
	return type;
}
