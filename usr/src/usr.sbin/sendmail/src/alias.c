# include <pwd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "sendmail.h"

# ifdef DBM
static char SccsId[] = "@(#)alias.c	3.24	%G%	(with DBM)";
# else DBM
static char SccsId[] = "@(#)alias.c	3.24	%G%	(without DBM)";
# endif DBM

/*
**  ALIAS -- Compute aliases.
**
**	Scans the file /usr/lib/aliases for a set of aliases.
**	If found, it arranges to deliver to them.  Uses libdbm
**	database if -DDBM.
**
**	Parameters:
**		a -- address to alias.
**
**	Returns:
**		none
**
**	Side Effects:
**		Aliases found are expanded.
**
**	Files:
**		/usr/lib/aliases -- the mail aliases.  The format is
**			a series of lines of the form:
**				alias:name1,name2,name3,...
**			where 'alias' expands to all of
**			'name[i]'.  Continuations begin with
**			space or tab.
**		/usr/lib/aliases.pag, /usr/lib/aliases.dir: libdbm version
**			of alias file.  Keys are aliases, datums
**			(data?) are name1,name2, ...
**
**	Notes:
**		If NoAlias (the "-n" flag) is set, no aliasing is
**			done.
**
**	Deficiencies:
**		It should complain about names that are aliased to
**			nothing.
**		It is unsophisticated about line overflows.
*/


#ifdef DBM
typedef struct
{
	char	*dptr;
	int	dsize;
} DATUM;
DATUM lhs, rhs;
extern DATUM fetch();
#endif DBM

alias(a)
	register ADDRESS *a;
{
	register char *p;
# ifndef DBM
	register STAB *s;
# endif DBM

	if (NoAlias)
		return;
# ifdef DEBUG
	if (Debug)
		printf("alias(%s)\n", a->q_paddr);
# endif

	/* don't realias already aliased names */
	if (bitset(QDONTSEND, a->q_flags))
		return;

	To = a->q_paddr;

	/*
	**  Look up this name
	*/

# ifdef DBM
	/* create a key for fetch */
	lhs.dptr = a->q_user;
	lhs.dsize = strlen(a->q_user) + 1;
	rhs = fetch(lhs);

	/* find this alias? */
	p = rhs.dptr;
	if (p == NULL)
		return;
# else DBM
	s = stab(a->q_user, ST_ALIAS, ST_FIND);
	if (s == NULL)
		return;
	p = s->s_alias;
# endif DBM

	/*
	**  Match on Alias.
	**	Deliver to the target list.
	*/

# ifdef DEBUG
	if (Debug)
		printf("%s (%s, %s) aliased to %s\n",
		    a->q_paddr, a->q_host, a->q_user, p);
# endif
	if (Verbose)
		message(Arpa_Info, "aliased to %s", p);
	AliasLevel++;
	sendto(p, 1, a);
	AliasLevel--;
}
/*
**  INITALIASES -- initialize for aliasing
**
**	Very different depending on whether we are running DBM or not.
**
**	Parameters:
**		aliasfile -- location of aliases.
**		init -- if set and if DBM, initialize the DBM files.
**
**	Returns:
**		none.
**
**	Side Effects:
**		initializes aliases:
**		if DBM:  opens the database.
**		if ~DBM: reads the aliases into the symbol table.
*/

# define DBMMODE	0666

initaliases(aliasfile, init)
	char *aliasfile;
	bool init;
{
	char buf[MAXNAME];
	struct stat stb;
	time_t modtime;

	/*
	**  See if the DBM version of the file is out of date with
	**  the text version.  If so, go into 'init' mode automatically.
	**	This only happens if our effective userid owns the DBM
	**	version or if the mode of the database is 666 -- this
	**	is an attempt to avoid protection problems.  Note the
	**	unpalatable hack to see if the stat succeeded.
	*/

	if (stat(aliasfile, &stb) < 0)
		return;
# ifdef DBM
	modtime = stb.st_mtime;
	(void) strcpy(buf, aliasfile);
	(void) strcat(buf, ".pag");
	stb.st_ino = 0;
	if ((stat(buf, &stb) < 0 || stb.st_mtime < modtime) && !init)
	{
		if (stb.st_ino != 0 &&
		    ((stb.st_mode & 0666) == 0666 || stb.st_uid == geteuid()))
		{
			init = TRUE;
			if (Verbose)
				message(Arpa_Info, "rebuilding alias database");
		}
		else
		{
			message(Arpa_Info, "Warning: alias database out of date");
		}
	}
# endif DBM

	/*
	**  If initializing, create the new files.
	**	We should lock the alias file here to prevent other
	**	instantiations of sendmail from reading an incomplete
	**	file -- or worse yet, doing a concurrent initialize.
	*/

# ifdef DBM
	if (init)
	{
		(void) strcpy(buf, aliasfile);
		(void) strcat(buf, ".dir");
		if (close(creat(buf, DBMMODE)) < 0)
		{
			syserr("cannot make %s", buf);
			return;
		}
		(void) strcpy(buf, aliasfile);
		(void) strcat(buf, ".pag");
		if (close(creat(buf, DBMMODE)) < 0)
		{
			syserr("cannot make %s", buf);
			return;
		}
	}

	/*
	**  Open and, if necessary, load the DBM file.
	**	If running without DBM, load the symbol table.
	*/

	dbminit(aliasfile);
	if (init)
		readaliases(aliasfile, TRUE);
# else DBM
	readaliases(aliasfile, init);
# endif DBM
}
/*
**  READALIASES -- read and process the alias file.
**
**	This routine implements the part of initaliases that occurs
**	when we are not going to use the DBM stuff.
**
**	Parameters:
**		aliasfile -- the pathname of the alias file master.
**		init -- if set, initialize the DBM stuff.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Reads aliasfile into the symbol table.
**		Optionally, builds the .dir & .pag files.
*/

static
readaliases(aliasfile, init)
	char *aliasfile;
	bool init;
{
	char line[BUFSIZ];
	register char *p;
	char *p2;
	char *rhs;
	bool skipping;
	ADDRESS al, bl;
	FILE *af;
	int lineno;
	register STAB *s;
	int naliases, bytes, longest;

	if ((af = fopen(aliasfile, "r")) == NULL)
	{
# ifdef DEBUG
		if (Debug)
			printf("Can't open %s\n", aliasfile);
# endif
		errno = 0;
		NoAlias++;
		return;
	}

	/*
	**  Read and interpret lines
	*/

	lineno = 0;
	naliases = bytes = longest = 0;
	skipping = FALSE;
	while (fgets(line, sizeof (line), af) != NULL)
	{
		int lhssize, rhssize;

		lineno++;
		switch (line[0])
		{
		  case '#':
		  case '\n':
		  case '\0':
			skipping = FALSE;
			continue;

		  case ' ':
		  case '\t':
			if (!skipping)
				syserr("aliases: %d: Non-continuation line starts with space", lineno);
			skipping = TRUE;
			continue;
		}
		skipping = FALSE;

		/*
		**  Process the LHS
		**	Find the final colon, and parse the address.
		**	It should resolve to a local name -- this will
		**	be checked later (we want to optionally do
		**	parsing of the RHS first to maximize error
		**	detection).
		*/

		for (p = line; *p != '\0' && *p != ':' && *p != '\n'; p++)
			continue;
		if (*p == '\0' || *p == '\n')
		{
		 syntaxerr:
			syserr("aliases: %d: missing colon", lineno);
			continue;
		}
		*p++ = '\0';
		if (parse(line, &al, 1) == NULL)
		{
			*--p = ':';
			goto syntaxerr;
		}

		/*
		**  Process the RHS.
		**	'al' is the internal form of the LHS address.
		**	'p' points to the text of the RHS.
		*/

		rhs = p;
		for (;;)
		{
			register char c;

			if (init)
			{
				/* do parsing & compression of addresses */
				c = *p;
				while (c != '\0')
				{
					p2 = p;
					while (*p != '\n' && *p != ',' && *p != '\0')
						p++;
					c = *p;
					*p++ = '\0';
					if (c == '\n')
						c = '\0';
					if (*p2 == '\0')
					{
						p[-1] = c;
						continue;
					}
					(void) parse(p2, &bl, -1);
					p[-1] = c;
					while (isspace(*p))
						p++;
				}
			}
			else
				p = &p[strlen(p)];

			/* see if there should be a continuation line */
			c = fgetc(af);
			if (!feof(af))
				(void) ungetc(c, af);
			if (c != ' ' && c != '\t')
				break;

			/* read continuation line */
			p--;
			if (fgets(p, sizeof line - (p - line), af) == NULL)
				break;
			lineno++;
		}
		if (al.q_mailer != MN_LOCAL)
		{
			syserr("aliases: %d: cannot alias non-local names", lineno);
			continue;
		}

		/*
		**  Insert alias into symbol table or DBM file
		*/

		lhssize = strlen(al.q_user) + 1;
		rhssize = strlen(rhs) + 1;

# ifdef DBM
		if (init)
		{
			DATUM key, content;

			key.dsize = lhssize;
			key.dptr = al.q_user;
			content.dsize = rhssize;
			content.dptr = rhs;
			store(key, content);
		}
		else
# endif DBM
		{
			s = stab(al.q_user, ST_ALIAS, ST_ENTER);
			s->s_alias = newstr(rhs);
		}

		/* statistics */
		naliases++;
		bytes += lhssize + rhssize;
		if (rhssize > longest)
			longest = rhssize;
	}
	(void) fclose(af);
	To = NULL;
	if (Verbose)
		message(Arpa_Info, "%d aliases, longest %d bytes, %d bytes total",
			naliases, longest, bytes);
}
/*
**  FORWARD -- Try to forward mail
**
**	This is similar but not identical to aliasing.
**
**	Parameters:
**		user -- the name of the user who's mail we would like
**			to forward to.  It must have been verified --
**			i.e., the q_home field must have been filled
**			in.
**
**	Returns:
**		none.
**
**	Side Effects:
**		New names are added to send queues.
*/

forward(user)
	ADDRESS *user;
{
	char buf[60];
	struct stat stbuf;

# ifdef DEBUG
	if (Debug)
		printf("forward(%s)\n", user->q_paddr);
# endif DEBUG

	if (user->q_mailer != MN_LOCAL || bitset(QBADADDR, user->q_flags))
		return;
# ifdef DEBUG
	if (user->q_home == NULL)
		syserr("forward: no home");
# endif DEBUG

	/* good address -- look for .forward file in home */
	define('z', user->q_home);
	(void) expand("$z/.forward", buf, &buf[sizeof buf - 1]);
	if (stat(buf, &stbuf) < 0 || stbuf.st_uid != user->q_uid ||
	    !bitset(S_IREAD, stbuf.st_mode))
		return;

	/* we do have an address to forward to -- do it */
	include(buf, "forwarding", user);
}
