/*-
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Casey Leedom of Lawrence Livermore National Laboratory.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getcap.c	5.1 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <errno.h>	
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	BFRAG		1024
#define	BSIZE		80
#define	ESC		('[' & 037)	/* ASCII ESC */
#define	MAX_RECURSION	32		/* maximum getent recursion */
#define	SFRAG		100		/* cgetstr mallocs in SFRAG chunks */

static size_t	 topreclen;	/* toprec length */
static char	*toprec;	/* Additional record specified by cgetset() */

static int getent __P((char **, u_int *, char **, int, char *, int));

/*
 * Cgetset() allows the addition of a user specified buffer to be added
 * to the database array, in effect "pushing" the buffer on top of the
 * virtual database. 0 is returned on success, -1 on failure.
 */
int
cgetset(ent)
	char *ent;
{
	if (ent == NULL) {
		if (toprec)
			free(toprec);
                toprec = NULL;
                topreclen = 0;
                return (0);
        }
        topreclen = strlen(ent);
        if ((toprec = malloc (topreclen + 1)) == NULL) {
		errno = ENOMEM;
                return (-1);
	}
        (void)strcpy(toprec, ent);
        return (0);
}

/*
 * Cgetcap searches the capability record buf for the capability cap with
 * type `type'.  A pointer to the value of cap is returned on success, NULL
 * if the requested capability couldn't be found.
 *
 * Specifying a type of ':' means that nothing should follow cap (:cap:).
 * In this case a pointer to the terminating ':' or NUL will be returned if
 * cap is found.
 *
 * If (cap, '@') or (cap, terminator, '@') is found before (cap, terminator)
 * return NULL.
 */
char *
cgetcap(buf, cap, type)
	char *buf, *cap;
	int type;
{
	register char *bp, *cp;

	bp = buf;
	for (;;) {
		/*
		 * Skip past the current capability field - it's either the
		 * name field if this is the first time through the loop, or
		 * the remainder of a field whose name failed to match cap.
		 */
		for (;;)
			if (*bp == '\0')
				return (NULL);
			else
				if (*bp++ == ':')
					break;

		/*
		 * Try to match (cap, type) in buf.
		 */
		for (cp = cap; *cp == *bp && *bp != '\0'; cp++, bp++)
			continue;
		if (*cp != '\0')
			continue;
		if (*bp == '@')
			return (NULL);
		if (type == ':') {
			if (*bp != '\0' && *bp != ':')
				continue;
			return(bp);
		}
		if (*bp != type)
			continue;
		bp++;
		return (*bp == '@' ? NULL : bp);
	}
	/* NOTREACHED */
}

/*
 * Cgetent extracts the capability record name from the NULL terminated file
 * array db_array and returns a pointer to a malloc'd copy of it in buf.
 * Buf must be retained through all subsequent calls to cgetcap, cgetnum,
 * cgetflag, and cgetstr, but may then be free'd.  0 is returned on success,
 * -1 if the requested record couldn't be found, -2 if a system error was
 * encountered (couldn't open/read a file, etc.), and -3 if a potential
 * reference loop is detected.
 */
int
cgetent(buf, db_array, name)
	char **buf, **db_array, *name;
{
	u_int dummy;

	return (getent(buf, &dummy, db_array, -1, name, 0));
}

/*
 * Getent implements the functions of cgetent.  If fd is non-negative,
 * *db_array has already been opened and fd is the open file descriptor.  We
 * do this to save time and avoid using up file descriptors for tc=
 * recursions.
 *
 * Getent returns the same success/failure codes as cgetent.  On success, a
 * pointer to a malloc'ed capability record with all tc= capabilities fully
 * expanded and its length (not including trailing ASCII NUL) are left in
 * *cap and *len.
 *
 * Basic algorithm:
 *	+ Allocate memory incrementally as needed in chunks of size BFRAG
 *	  for capability buffer.
 *	+ Recurse for each tc=name and interpolate result.  Stop when all
 *	  names interpolated, a name can't be found, or depth exceeds
 *	  MAX_RECURSION.
 */
static int
getent(cap, len, db_array, fd, name, depth)
	char **cap, **db_array, *name;
	u_int *len;
	int fd, depth;
{
	register char *r_end, *rp, **db_p;
	int myfd, eof, foundit;
	char *record;

	/*
	 * Return with ``loop detected'' error if we've recursed more than
	 * MAX_RECURSION times.
	 */
	if (depth > MAX_RECURSION)
		return (-3);

	/*
	 * Check if we have a top record from cgetset().
         */
	if (depth == 0 && toprec != NULL) {
		if ((record = malloc (topreclen + BFRAG)) == NULL) {
			errno = ENOMEM;
			return (-2);
		}
		(void)strcpy(record, toprec);
		myfd = 0;
		db_p = db_array;
		rp = record + topreclen + 1;
		r_end = rp + BFRAG;
		goto tc_exp;
	}
	/*
	 * Allocate first chunk of memory.
	 */
	if ((record = malloc(BFRAG)) == NULL) {
		errno = ENOMEM;
		return (-2);
	}
	r_end = record + BFRAG;
	foundit = 0;

	/*
	 * Loop through database array until finding the record.
	 */

	for (db_p = db_array; *db_p != NULL; db_p++) {
		eof = 0;

		/*
		 * Open database if not already open.
		 */
		if (fd >= 0) {
			(void)lseek(fd, 0L, L_SET);
			myfd = 0;
		} else {
			fd = open(*db_p, O_RDONLY, 0);
			if (fd < 0) {
				free(record);
				return (-2);
			}
			myfd = 1;
		}

		/*
		 * Find the requested capability record ...
		 */
		{
		char buf[BUFSIZ];
		register char *b_end, *bp;
		register int c;

		/*
		 * Loop invariants:
		 *	There is always room for one more character in record.
		 *	R_end always points just past end of record.
		 *	Rp always points just past last character in record.
		 *	B_end always points just past last character in buf.
		 *	Bp always points at next character in buf.
		 */
		b_end = buf;
		bp = buf;
		for (;;) {

			/*
			 * Read in a line implementing (\, newline)
			 * line continuation.
			 */
			rp = record;
			for (;;) {
				if (bp >= b_end) {
					int n;
		
					n = read(fd, buf, sizeof(buf));
					if (n <= 0) {
						if (myfd)
							(void)close(fd);
						if (n < 0) {
							free(record);
							return (-2);
						} else {
							fd = -1;
							eof = 1;
							break;
						}
					}
					b_end = buf+n;
					bp = buf;
				}
	
				c = *bp++;
				if (c == '\n') {
					if (rp > record && *(rp-1) == '\\') {
						rp--;
						continue;
					} else
						break;
				}
				*rp++ = c;

				/*
				 * Enforce loop invariant: if no room 
				 * left in record buffer, try to get
				 * some more.
				 */
				if (rp >= r_end) {
					u_int pos, newsize;

					pos = rp - record;
					newsize = r_end - record + BFRAG;
					record = realloc(record, newsize);
					if (record == NULL) {
						errno = ENOMEM;
						if (myfd)
							(void)close(fd);
						return (-2);
					}
					r_end = record + newsize;
					rp = record + pos;
				}
			}
				/* loop invariant let's us do this */
			*rp++ = '\0';

			/*
			 * If encountered eof check next file.
			 */
			if (eof)
				break;
				
			/*
			 * Toss blank lines and comments.
			 */
			if (*record == '\0' || *record == '#')
				continue;
	
			/*
			 * See if this is the record we want ...
			 */
			if (cgetmatch(record, name) == 0) {
				foundit = 1;
				break;	/* found it! */
			}
		}
		}
		if (foundit)
			break;
	}

	if (!foundit)
		return (-1);
	
	/*
	 * Got the capability record, but now we have to expand all tc=name
	 * references in it ...
	 */
tc_exp:	{
		register char *newicap, *s;
		register int newilen;
		u_int ilen;
		int diff, iret, tclen;
		char *icap, *scan, *tc, *tcstart, *tcend;

		/*
		 * Loop invariants:
		 *	There is room for one more character in record.
		 *	R_end points just past end of record.
		 *	Rp points just past last character in record.
		 *	Scan points at remainder of record that needs to be
		 *	scanned for tc=name constructs.
		 */
		scan = record;
		for (;;) {
			if ((tc = cgetcap(scan, "tc", '=')) == NULL)
				break;

			/*
			 * Find end of tc=name and stomp on the trailing `:'
			 * (if present) so we can use it to call ourselves.
			 */
			s = tc;
			for (;;)
				if (*s == '\0')
					break;
				else
					if (*s++ == ':') {
						*(s-1) = '\0';
						break;
					}
			tcstart = tc - 3;
			tclen = s - tcstart;
			tcend = s;

			iret = getent(&icap, &ilen, db_p, fd, tc, depth+1);
			newicap = icap;		/* Put into a register. */
			newilen = ilen;
			if (iret != 0) {
				/* an error or couldn't resolve tc= */
				if (myfd)
					(void)close(fd);
				free(record);
				return (iret);
			}

			/* not interested in name field of tc'ed record */
			s = newicap;
			for (;;)
				if (*s == '\0')
					break;
				else
					if (*s++ == ':')
						break;
			newilen -= s - newicap;
			newicap = s;

			/* make sure interpolated record is `:'-terminated */
			s += newilen;
			if (*(s-1) != ':') {
				*s = ':';	/* overwrite NUL with : */
				newilen++;
			}

			/*
			 * Make sure there's enough room to insert the
			 * new record.
			 */
			diff = newilen - tclen;
			if (diff >= r_end - rp) {
				u_int pos, newsize, tcpos, tcposend;

				pos = rp - record;
				newsize = r_end - record + diff + BFRAG;
				tcpos = tcstart - record;
				tcposend = tcend - record;
				record = realloc(record, newsize);
				if (record == NULL) {
					errno = ENOMEM;
					if (myfd)
						(void)close(fd);
					free(icap);
					return (-2);
				}
				r_end = record + newsize;
				rp = record + pos;
				tcstart = record + tcpos;
				tcend = record + tcposend;
			}

			/*
			 * Insert tc'ed record into our record.
			 */
			s = tcstart + newilen;
			bcopy(tcend, s, rp - tcend);
			bcopy(newicap, tcstart, newilen);
			rp += diff;
			free(icap);

			/*
			 * Start scan on `:' so next cgetcap works properly
			 * (cgetcap always skips first field).
			 */
			scan = s-1;
		}
	}

	/*
	 * Close file (if we opened it), give back any extra memory, and
	 * return capability, length and success.
	 */
	if (myfd)
		(void)close(fd);
	*len = rp - record - 1;	/* don't count NUL */
	if (r_end > rp)
		record = realloc(record, (u_int)(rp - record));
	*cap = record;
	return (0);
}

/*
 * Cgetmatch will return 0 if name is one of the names of the capability
 * record buf, -1 if not.
 */
int
cgetmatch(buf, name)
	char *buf, *name;
{
	register char *np, *bp;

	/*
	 * Start search at beginning of record.
	 */
	bp = buf;
	for (;;) {
		/*
		 * Try to match a record name.
		 */
		np = name;
		for (;;)
			if (*np == '\0')
				if (*bp == '|' || *bp == ':' || *bp == '\0')
					return (0);
				else
					break;
			else
				if (*bp++ != *np++)
					break;

		/*
		 * Match failed, skip to next name in record.
		 */
		bp--;	/* a '|' or ':' may have stopped the match */
		for (;;)
			if (*bp == '\0' || *bp == ':')
				return (-1);	/* match failed totally */
			else
				if (*bp++ == '|')
					break;	/* found next name */
	}
}

int
cgetfirst(buf, db_array)
	char **buf, **db_array;
{
	(void)cgetclose();
	return (cgetnext(buf, db_array));
}

static FILE *pfp;
static int slash;
static char **dbp;

int
cgetclose()
{
	if (pfp != NULL) {
		(void)fclose(pfp);
		pfp = NULL;
	}
	dbp = NULL;
	slash = 0;
	return (0);
}

/*
 * Cgetnext() gets either the first or next entry in the logical database 
 * specified by db_array.  It returns 0 upon completion of the database, 1
 * upon returning an entry with more remaining, and -1 if an error occurs.
 */
int
cgetnext(bp, db_array)
        register char **bp;
	char **db_array;
{
	size_t len;
	int status;
	char *cp, *line, *rp, buf[BSIZE];

	if (dbp == NULL)
		dbp = db_array;

	if (pfp == NULL && (pfp = fopen(*dbp, "r")) == NULL)
		return (-1);

	for(;;) {
		line = fgetline(pfp, &len);
		if (line == NULL) {
			(void)fclose(pfp);
			if (ferror(pfp)) {
				pfp = NULL;
				dbp = NULL;
				slash = 0;
				return (-1);
			} else {
				dbp++;
				if (*dbp == NULL) {
					pfp = NULL;
					dbp = NULL;
					slash = 0;
					return (0);
				} else if ((pfp = fopen(*dbp, "r")) == NULL) {
					pfp = NULL;
					dbp = NULL;
					slash = 0;
					return (-1);
				} else
					continue;
			}
		}
		if (isspace(*line) || *line == ':' || *line == '#' 
		    || len == 0 || slash) {
			if (len > 0 && line[len - 1] == '\\')
				slash = 1;
			else
				slash = 0;
			continue;
		}
		if (len > 0 && line[len - 1] == '\\')
			slash = 1;
		else
			slash = 0;

		/* line points to a name line */

		rp = buf;
		for(cp = line; *cp != NULL; cp++)
			if (*cp == '|' || *cp == ':')
				break;
			else
				*rp++ = *cp;

		*rp = '\0';
		status = cgetent(bp, db_array, &buf[0]);
		if (status == 0)
			return (1);
		if (status == -2 || status == -3) {
			pfp = NULL;
			dbp = NULL;
			slash = 0;
			return (status + 1);
		}
	}
	/* NOTREACHED */
}

/*
 * Cgetstr retrieves the value of the string capability cap from the
 * capability record pointed to by buf.  A pointer to a decoded, NUL
 * terminated, malloc'd copy of the string is returned in the char *
 * pointed to by str.  The length of the string not including the trailing
 * NUL is returned on success, -1 if the requested string capability
 * couldn't be found, -2 if a system error was encountered (storage
 * allocation failure).
 */
int
cgetstr(buf, cap, str)
	char *buf, *cap;
	char **str;
{
	register u_int m_room;
	register char *bp, *mp;
	int len;
	char *mem;

	/*
	 * Find string capability cap
	 */
	bp = cgetcap(buf, cap, '=');
	if (bp == NULL)
		return (-1);

	/*
	 * Conversion / storage allocation loop ...  Allocate memory in
	 * chunks SFRAG in size.
	 */
	if ((mem = malloc(SFRAG)) == NULL) {
		errno = ENOMEM;
		return (-2);	/* couldn't even allocate the first fragment */
	}
	m_room = SFRAG;
	mp = mem;

	while (*bp != ':' && *bp != '\0') {
		/*
		 * Loop invariants:
		 *	There is always room for one more character in mem.
		 *	Mp always points just past last character in mem.
		 *	Bp always points at next character in buf.
		 */
		if (*bp == '^') {
			bp++;
			if (*bp == ':' || *bp == '\0')
				break;	/* drop unfinished escape */
			*mp++ = *bp++ & 037;
		} else if (*bp == '\\') {
			bp++;
			if (*bp == ':' || *bp == '\0')
				break;	/* drop unfinished escape */
			if ('0' <= *bp && *bp <= '7') {
				register int n, i;

				n = 0;
				i = 3;	/* maximum of three octal digits */
				do {
					n = n * 8 + (*bp++ - '0');
				} while (--i && '0' <= *bp && *bp <= '7');
				*mp++ = n;
			}
			else switch (*bp++) {
				case 'b': case 'B':
					*mp++ = '\b';
					break;
				case 't': case 'T':
					*mp++ = '\t';
					break;
				case 'n': case 'N':
					*mp++ = '\n';
					break;
				case 'f': case 'F':
					*mp++ = '\f';
					break;
				case 'r': case 'R':
					*mp++ = '\r';
					break;
				case 'e': case 'E':
					*mp++ = ESC;
					break;
				case 'c': case 'C':
					*mp++ = ':';
					break;
				default:
					/*
					 * Catches '\', '^', and
					 *  everything else.
					 */
					*mp++ = *(bp-1);
					break;
			}
		} else
			*mp++ = *bp++;
		m_room--;

		/*
		 * Enforce loop invariant: if no room left in current
		 * buffer, try to get some more.
		 */
		if (m_room == 0) {
			u_int size = mp - mem;

			if ((mem = realloc(mem, size + SFRAG)) == NULL)
				return (-2);
			m_room = SFRAG;
			mp = mem + size;
		}
	}
	*mp++ = '\0';	/* loop invariant let's us do this */
	m_room--;
	len = mp - mem - 1;

	/*
	 * Give back any extra memory and return value and success.
	 */
	if (m_room != 0)
		mem = realloc(mem, (u_int)(mp - mem));
	*str = mem;
	return (len);
}

/*
 * Cgetustr retrieves the value of the string capability cap from the
 * capability record pointed to by buf.  The difference between cgetustr()
 * and cgetstr() is that cgetustr does not decode escapes but rather treats
 * all characters literally.  A pointer to a  NUL terminated malloc'd 
 * copy of the string is returned in the char pointed to by str.  The 
 * length of the string not including the trailing NUL is returned on success,
 * -1 if the requested string capability couldn't be found, -2 if a system 
 * error was encountered (storage allocation failure).
 */
int
cgetustr(buf, cap, str)
	char *buf, *cap, **str;
{
	register u_int m_room;
	register char *bp, *mp;
	int len;
	char *mem;

	/*
	 * Find string capability cap
	 */
	if ((bp = cgetcap(buf, cap, '=')) == NULL)
		return (-1);

	/*
	 * Conversion / storage allocation loop ...  Allocate memory in
	 * chunks SFRAG in size.
	 */
	if ((mem = malloc(SFRAG)) == NULL) {
		errno = ENOMEM;
		return (-2);	/* couldn't even allocate the first fragment */
	}
	m_room = SFRAG;
	mp = mem;

	while (*bp != ':' && *bp != '\0') {
		/*
		 * Loop invariants:
		 *	There is always room for one more character in mem.
		 *	Mp always points just past last character in mem.
		 *	Bp always points at next character in buf.
		 */
		*mp++ = *bp++;
		m_room--;

		/*
		 * Enforce loop invariant: if no room left in current
		 * buffer, try to get some more.
		 */
		if (m_room == 0) {
			u_int size = mp - mem;

			if ((mem = realloc(mem, size + SFRAG)) == NULL)
				return (-2);
			m_room = SFRAG;
			mp = mem + size;
		}
	}
	*mp++ = '\0';	/* loop invariant let's us do this */
	m_room--;
	len = mp - mem - 1;

	/*
	 * Give back any extra memory and return value and success.
	 */
	if (m_room != 0)
		mem = realloc(mem, (u_int)(mp - mem));
	*str = mem;
	return (len);
}

/*
 * Cgetnum retrieves the value of the numeric capability cap from the
 * capability record pointed to by buf.  The numeric value is returned in
 * the long pointed to by num.  0 is returned on success, -1 if the requested
 * numeric capability couldn't be found.
 */
int
cgetnum(buf, cap, num)
	char *buf, *cap;
	long *num;
{
	register long n;
	register int base, digit;
	register char *bp;

	/*
	 * Find numeric capability cap
	 */
	bp = cgetcap(buf, cap, '#');
	if (bp == NULL)
		return (-1);

	/*
	 * Look at value and determine numeric base:
	 *	0x... or 0X...	hexadecimal,
	 * else	0...		octal,
	 * else			decimal.
	 */
	if (*bp == '0') {
		bp++;
		if (*bp == 'x' || *bp == 'X') {
			bp++;
			base = 16;
		} else
			base = 8;
	} else
		base = 10;

	/*
	 * Conversion loop ...
	 */
	n = 0;
	for (;;) {
		if ('0' <= *bp && *bp <= '9')
			digit = *bp - '0';
		else
			if (   ('a' <= *bp && *bp <= 'f')
			    || ('A' <= *bp && *bp <= 'F'))
				digit = 10 + *bp - 'a';
			else
				break;

		if (digit >= base)
			break;

		n = n * base + digit;
		bp++;
	}

	/*
	 * Return value and success.
	 */
	*num = n;
	return (0);
}
