/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

# include "sendmail.h"
# include <pwd.h>

#ifndef lint
static char sccsid[] = "@(#)alias.c	8.37 (Berkeley) %G%";
#endif /* not lint */


MAP	*AliasDB[MAXALIASDB + 1];	/* actual database list */
int	NAliasDBs;			/* number of alias databases */
/*
**  ALIAS -- Compute aliases.
**
**	Scans the alias file for an alias for the given address.
**	If found, it arranges to deliver to the alias list instead.
**	Uses libdbm database if -DDBM.
**
**	Parameters:
**		a -- address to alias.
**		sendq -- a pointer to the head of the send queue
**			to put the aliases in.
**		aliaslevel -- the current alias nesting depth.
**		e -- the current envelope.
**
**	Returns:
**		none
**
**	Side Effects:
**		Aliases found are expanded.
**
**	Deficiencies:
**		It should complain about names that are aliased to
**			nothing.
*/

void
alias(a, sendq, aliaslevel, e)
	register ADDRESS *a;
	ADDRESS **sendq;
	int aliaslevel;
	register ENVELOPE *e;
{
	register char *p;
	int naliases;
	char *owner;
	char obuf[MAXNAME + 6];
	extern ADDRESS *sendto();
	extern char *aliaslookup();

	if (tTd(27, 1))
		printf("alias(%s)\n", a->q_user);

	/* don't realias already aliased names */
	if (bitset(QDONTSEND|QBADADDR|QVERIFIED, a->q_flags))
		return;

	if (NoAlias)
		return;

	e->e_to = a->q_paddr;

	/*
	**  Look up this name
	*/

	p = aliaslookup(a->q_user, e);
	if (p == NULL)
		return;

	/*
	**  Match on Alias.
	**	Deliver to the target list.
	*/

	if (tTd(27, 1))
		printf("%s (%s, %s) aliased to %s\n",
		    a->q_paddr, a->q_host, a->q_user, p);
	if (bitset(EF_VRFYONLY, e->e_flags))
	{
		a->q_flags |= QVERIFIED;
		e->e_nrcpts++;
		return;
	}
	message("aliased to %s", p);
#ifdef LOG
	if (LogLevel > 9)
		syslog(LOG_INFO, "%s: alias %s => %s",
			e->e_id == NULL ? "NOQUEUE" : e->e_id,
			a->q_paddr, p);
#endif
	a->q_flags &= ~QSELFREF;
	if (tTd(27, 5))
	{
		printf("alias: QDONTSEND ");
		printaddr(a, FALSE);
	}
	a->q_flags |= QDONTSEND;
	naliases = sendtolist(p, a, sendq, aliaslevel + 1, e);
	if (bitset(QSELFREF, a->q_flags))
		a->q_flags &= ~QDONTSEND;

	/*
	**  Look for owner of alias
	*/

	(void) strcpy(obuf, "owner-");
	if (strncmp(a->q_user, "owner-", 6) == 0)
		(void) strcat(obuf, "owner");
	else
		(void) strcat(obuf, a->q_user);
	if (!bitnset(M_USR_UPPER, a->q_mailer->m_flags))
		makelower(obuf);
	owner = aliaslookup(obuf, e);
	if (owner == NULL)
		return;

	/* reflect owner into envelope sender */
	if (strpbrk(owner, ",:/|\"") != NULL)
		owner = obuf;
	a->q_owner = newstr(owner);

	/* announce delivery to this alias; NORECEIPT bit set later */
	if (e->e_xfp != NULL)
		fprintf(e->e_xfp, "Message delivered to mailing list %s\n",
			a->q_paddr);
	e->e_flags |= EF_SENDRECEIPT;
	a->q_flags |= QREPORT;
}
/*
**  ALIASLOOKUP -- look up a name in the alias file.
**
**	Parameters:
**		name -- the name to look up.
**
**	Returns:
**		the value of name.
**		NULL if unknown.
**
**	Side Effects:
**		none.
**
**	Warnings:
**		The return value will be trashed across calls.
*/

char *
aliaslookup(name, e)
	char *name;
	ENVELOPE *e;
{
	register int dbno;
	register MAP *map;
	register char *p;

	for (dbno = 0; dbno < NAliasDBs; dbno++)
	{
		auto int stat;

		map = AliasDB[dbno];
		if (!bitset(MF_OPEN, map->map_mflags))
			continue;
		p = (*map->map_class->map_lookup)(map, name, NULL, &stat);
		if (p != NULL)
			return p;
	}
	return NULL;
}
/*
**  SETALIAS -- set up an alias map
**
**	Called when reading configuration file.
**
**	Parameters:
**		spec -- the alias specification
**
**	Returns:
**		none.
*/

void
setalias(spec)
	char *spec;
{
	register char *p;
	register MAP *map;
	char *class;
	STAB *s;
	static bool first_unqual = TRUE;

	if (tTd(27, 8))
		printf("setalias(%s)\n", spec);

	for (p = spec; p != NULL; )
	{
		while (isspace(*p))
			p++;
		if (*p == '\0')
			break;
		spec = p;

		/*
		**  Treat simple filename specially -- this is the file name
		**  for the files implementation, not necessarily in order.
		*/

		if (spec[0] == '/' && first_unqual)
		{
			s = stab("aliases.files", ST_MAP, ST_ENTER);
			map = &s->s_map;
			first_unqual = FALSE;
		}
		else
		{
			char aname[50];

			if (NAliasDBs >= MAXALIASDB)
			{
				syserr("Too many alias databases defined, %d max",
					MAXALIASDB);
				return;
			}
			(void) sprintf(aname, "Alias%d", NAliasDBs);
			s = stab(aname, ST_MAP, ST_ENTER);
			map = &s->s_map;
			AliasDB[NAliasDBs] = map;
		}
		bzero(map, sizeof *map);
		map->map_mname = s->s_name;

		p = strpbrk(p, " ,/:");
		if (p != NULL && *p == ':')
		{
			/* map name */
			*p++ = '\0';
			class = spec;
			spec = p;
		}
		else
		{
			class = "implicit";
			map->map_mflags = MF_OPTIONAL|MF_INCLNULL;
		}

		/* find end of spec */
		if (p != NULL)
			p = strchr(p, ',');
		if (p != NULL)
			*p++ = '\0';

		if (tTd(27, 20))
			printf("  map %s:%s %s\n", class, s->s_name, spec);

		/* look up class */
		s = stab(class, ST_MAPCLASS, ST_FIND);
		if (s == NULL)
		{
			if (tTd(27, 1))
				printf("Unknown alias class %s\n", class);
		}
		else if (!bitset(MCF_ALIASOK, s->s_mapclass.map_cflags))
		{
			syserr("setalias: map class %s can't handle aliases",
				class);
		}
		else
		{
			map->map_class = &s->s_mapclass;
			if (map->map_class->map_parse(map, spec))
			{
				map->map_mflags |= MF_VALID|MF_ALIAS;
				if (AliasDB[NAliasDBs] == map)
					NAliasDBs++;
			}
		}
	}
}
/*
**  ALIASWAIT -- wait for distinguished @:@ token to appear.
**
**	This can decide to reopen or rebuild the alias file
**
**	Parameters:
**		map -- a pointer to the map descriptor for this alias file.
**		ext -- the filename extension (e.g., ".db") for the
**			database file.
**		isopen -- if set, the database is already open, and we
**			should check for validity; otherwise, we are
**			just checking to see if it should be created.
**
**	Returns:
**		TRUE -- if the database is open when we return.
**		FALSE -- if the database is closed when we return.
*/

bool
aliaswait(map, ext, isopen)
	MAP *map;
	char *ext;
	int isopen;
{
	bool attimeout = FALSE;
	time_t mtime;
	struct stat stb;
	char buf[MAXNAME];

	if (tTd(27, 3))
		printf("aliaswait(%s:%s)\n",
			map->map_class->map_cname, map->map_file);
	if (bitset(MF_ALIASWAIT, map->map_mflags))
		return isopen;
	map->map_mflags |= MF_ALIASWAIT;

	if (SafeAlias > 0)
	{
		auto int st;
		time_t toolong = curtime() + SafeAlias;
		unsigned int sleeptime = 2;

		while (isopen &&
		       map->map_class->map_lookup(map, "@", NULL, &st) == NULL)
		{
			if (curtime() > toolong)
			{
				/* we timed out */
				attimeout = TRUE;
				break;
			}

			/*
			**  Close and re-open the alias database in case
			**  the one is mv'ed instead of cp'ed in.
			*/

			if (tTd(27, 2))
				printf("aliaswait: sleeping for %d seconds\n",
					sleeptime);

			map->map_class->map_close(map);
			sleep(sleeptime);
			sleeptime *= 2;
			if (sleeptime > 60)
				sleeptime = 60;
			isopen = map->map_class->map_open(map, O_RDONLY);
		}
	}

	/* see if we need to go into auto-rebuild mode */
	if (!bitset(MCF_REBUILDABLE, map->map_class->map_cflags))
	{
		if (tTd(27, 3))
			printf("aliaswait: not rebuildable\n");
		map->map_mflags &= ~MF_ALIASWAIT;
		return isopen;
	}
	if (stat(map->map_file, &stb) < 0)
	{
		if (tTd(27, 3))
			printf("aliaswait: no source file\n");
		map->map_mflags &= ~MF_ALIASWAIT;
		return isopen;
	}
	mtime = stb.st_mtime;
	(void) strcpy(buf, map->map_file);
	if (ext != NULL)
		(void) strcat(buf, ext);
	if (stat(buf, &stb) < 0 || stb.st_mtime < mtime || attimeout)
	{
		/* database is out of date */
		if (AutoRebuild && stb.st_ino != 0 && stb.st_uid == geteuid())
		{
			bool oldSuprErrs;

			message("auto-rebuilding alias database %s", buf);
			oldSuprErrs = SuprErrs;
			SuprErrs = TRUE;
			if (isopen)
				map->map_class->map_close(map);
			rebuildaliases(map, TRUE);
			isopen = map->map_class->map_open(map, O_RDONLY);
			SuprErrs = oldSuprErrs;
		}
		else
		{
#ifdef LOG
			if (LogLevel > 3)
				syslog(LOG_INFO, "alias database %s out of date",
					buf);
#endif /* LOG */
			message("Warning: alias database %s out of date", buf);
		}
	}
	map->map_mflags &= ~MF_ALIASWAIT;
	return isopen;
}
/*
**  REBUILDALIASES -- rebuild the alias database.
**
**	Parameters:
**		map -- the database to rebuild.
**		automatic -- set if this was automatically generated.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Reads the text version of the database, builds the
**		DBM or DB version.
*/

void
rebuildaliases(map, automatic)
	register MAP *map;
	bool automatic;
{
	FILE *af;
	bool nolock = FALSE;
	sigfunc_t oldsigint, oldsigquit;
#ifdef SIGTSTP
	sigfunc_t oldsigtstp;
#endif

	if (!bitset(MCF_REBUILDABLE, map->map_class->map_cflags))
		return;

	/* try to lock the source file */
	if ((af = fopen(map->map_file, "r+")) == NULL)
	{
		if ((errno != EACCES && errno != EROFS) || automatic ||
		    (af = fopen(map->map_file, "r")) == NULL)
		{
			int saveerr = errno;

			if (tTd(27, 1))
				printf("Can't open %s: %s\n",
					map->map_file, errstring(saveerr));
			if (!automatic && !bitset(MF_OPTIONAL, map->map_mflags))
				message("newaliases: cannot open %s: %s",
					map->map_file, errstring(saveerr));
			errno = 0;
			return;
		}
		nolock = TRUE;
		message("warning: cannot lock %s: %s",
			map->map_file, errstring(errno));
	}

	/* see if someone else is rebuilding the alias file */
	if (!nolock &&
	    !lockfile(fileno(af), map->map_file, NULL, LOCK_EX|LOCK_NB))
	{
		/* yes, they are -- wait until done */
		message("Alias file %s is already being rebuilt",
			map->map_file);
		if (OpMode != MD_INITALIAS)
		{
			/* wait for other rebuild to complete */
			(void) lockfile(fileno(af), map->map_file, NULL,
					LOCK_EX);
		}
		(void) xfclose(af, "rebuildaliases1", map->map_file);
		errno = 0;
		return;
	}

	/* avoid denial-of-service attacks */
	resetlimits();
	oldsigint = setsignal(SIGINT, SIG_IGN);
	oldsigquit = setsignal(SIGQUIT, SIG_IGN);
#ifdef SIGTSTP
	oldsigtstp = setsignal(SIGTSTP, SIG_IGN);
#endif

	if (map->map_class->map_open(map, O_RDWR))
	{
#ifdef LOG
		if (LogLevel > 7)
		{
			syslog(LOG_NOTICE, "alias database %s %srebuilt by %s",
				map->map_file, automatic ? "auto" : "",
				username());
		}
#endif /* LOG */
		map->map_mflags |= MF_OPEN|MF_WRITABLE;
		readaliases(map, af, !automatic, TRUE);
	}
	else
	{
		if (tTd(27, 1))
			printf("Can't create database for %s: %s\n",
				map->map_file, errstring(errno));
		if (!automatic)
			syserr("Cannot create database for alias file %s",
				map->map_file);
	}

	/* close the file, thus releasing locks */
	xfclose(af, "rebuildaliases2", map->map_file);

	/* add distinguished entries and close the database */
	if (bitset(MF_OPEN, map->map_mflags))
		map->map_class->map_close(map);

	/* restore the old signals */
	(void) setsignal(SIGINT, oldsigint);
	(void) setsignal(SIGQUIT, oldsigquit);
#ifdef SIGTSTP
	(void) setsignal(SIGTSTP, oldsigtstp);
#endif
}
/*
**  READALIASES -- read and process the alias file.
**
**	This routine implements the part of initaliases that occurs
**	when we are not going to use the DBM stuff.
**
**	Parameters:
**		map -- the alias database descriptor.
**		af -- file to read the aliases from.
**		announcestats -- anounce statistics regarding number of
**			aliases, longest alias, etc.
**		logstats -- lot the same info.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Reads aliasfile into the symbol table.
**		Optionally, builds the .dir & .pag files.
*/

void
readaliases(map, af, announcestats, logstats)
	register MAP *map;
	FILE *af;
	bool announcestats;
	bool logstats;
{
	register char *p;
	char *lhs;
	char *rhs;
	bool skipping;
	long naliases, bytes, longest;
	ADDRESS al, bl;
	char line[BUFSIZ];

	/*
	**  Read and interpret lines
	*/

	FileName = map->map_file;
	LineNumber = 0;
	naliases = bytes = longest = 0;
	skipping = FALSE;
	while (fgets(line, sizeof (line), af) != NULL)
	{
		int lhssize, rhssize;

		LineNumber++;
		p = strchr(line, '\n');
		if (p != NULL)
			*p = '\0';
		switch (line[0])
		{
		  case '#':
		  case '\0':
			skipping = FALSE;
			continue;

		  case ' ':
		  case '\t':
			if (!skipping)
				syserr("554 Non-continuation line starts with space");
			skipping = TRUE;
			continue;
		}
		skipping = FALSE;

		/*
		**  Process the LHS
		**
		**	Find the colon separator, and parse the address.
		**	It should resolve to a local name.
		**
		**	Alternatively, it can be "@hostname" for host
		**	aliases -- all we do here is map case.  Hostname
		**	need not be a single token.
		*/

		for (p = line; *p != '\0' && *p != ':' && *p != '\n'; p++)
			continue;
		if (*p != ':')
		{
			syserr("%s, line %d: syntax error", aliasfile, lineno);
			continue;
		}
		*p++ = '\0';
		if (line[0] == '@')
		{
			/* a host alias */
			makelower(line);
			lhs = line;
		}
		else
		{
			/* a user alias */
			if (parseaddr(line, &al, 1, ':') == NULL)
			{
				syserr("illegal alias name");
				continue;
			}
			loweraddr(&al);
			if (al.q_mailer != LocalMailer)
			{
				syserr("cannot alias non-local names");
				continue;
			}
			lhs = al.q_user;
		}

		/*
		**  Process the RHS.
		**	'al' is the internal form of the LHS address.
		**	'p' points to the text of the RHS.
		**		'p' may begin with a colon (i.e., the
		**		separator was "::") which will use the
		**		first address as the person to send
		**		errors to -- i.e., designates the
		**		list maintainer.
		*/

		while (isascii(*p) && isspace(*p))
			p++;
		if (*p == ':')
		{
			ADDRESS *maint;

			while (isspace(*++p))
				continue;
			rhs = p;
			while (*p != '\0' && *p != ',')
				p++;
			if (*p != ',')
				goto syntaxerr;
			*p++ = '\0';
			maint = parse(p, (ADDRESS *) NULL, 1);
			if (maint == NULL)
				syserr("Illegal list maintainer for list %s", al.q_user);
			else if (CurEnv->e_returnto == &CurEnv->e_from)
			{
				CurEnv->e_returnto = maint;
				MailBack++;
			}
		}
			
		rhs = p;
		for (;;)
		{
			register char c;
			register char *nlp;

			nlp = &p[strlen(p)];
			if (nlp[-1] == '\n')
				*--nlp = '\0';

			if (CheckAliases)
			{
				/* do parsing & compression of addresses */
				while (*p != '\0')
				{
					auto char *delimptr;

					while ((isascii(*p) && isspace(*p)) ||
								*p == ',')
						p++;
					if (*p == '\0')
						break;
					if (parseaddr(p, &bl, RF_COPYNONE, ',',
						      &delimptr, CurEnv) == NULL)
						usrerr("553 %s... bad address", p);
					p = delimptr;
				}
			}
			else
			{
				p = nlp;
			}

			/* see if there should be a continuation line */
			c = fgetc(af);
			if (!feof(af))
				(void) ungetc(c, af);
			if (c != ' ' && c != '\t')
				break;

			/* read continuation line */
			if (fgets(p, sizeof line - (p - line), af) == NULL)
				break;
			LineNumber++;

			/* check for line overflow */
			if (strchr(p, '\n') == NULL)
			{
				usrerr("554 alias too long");
				break;
			}
		}

		/*
		**  Insert alias into symbol table or DBM file
		*/

		lhssize = strlen(lhs) + 1;

		lhssize = strlen(al.q_user);
		rhssize = strlen(rhs);
		map->map_class->map_store(map, al.q_user, rhs);

		if (al.q_paddr != NULL)
			free(al.q_paddr);
		if (al.q_host != NULL)
			free(al.q_host);
		if (al.q_user != NULL)
			free(al.q_user);

		/* statistics */
		naliases++;
		bytes += lhssize + rhssize;
		if (rhssize > longest)
			longest = rhssize;
	}

	CurEnv->e_to = NULL;
	FileName = NULL;
	if (Verbose || announcestats)
		message("%s: %d aliases, longest %d bytes, %d bytes total",
			map->map_file, naliases, longest, bytes);
# ifdef LOG
	if (LogLevel > 7 && logstats)
		syslog(LOG_INFO, "%s: %d aliases, longest %d bytes, %d bytes total",
			map->map_file, naliases, longest, bytes);
# endif /* LOG */
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
**		sendq -- a pointer to the head of the send queue to
**			put this user's aliases in.
**		aliaslevel -- the current alias nesting depth.
**		e -- the current envelope.
**
**	Returns:
**		none.
**
**	Side Effects:
**		New names are added to send queues.
*/

void
forward(user, sendq, aliaslevel, e)
	ADDRESS *user;
	ADDRESS **sendq;
	int aliaslevel;
	register ENVELOPE *e;
{
	char *pp;
	char *ep;

	if (tTd(27, 1))
		printf("forward(%s)\n", user->q_paddr);

	if (!bitnset(M_HASPWENT, user->q_mailer->m_flags) ||
	    bitset(QBADADDR, user->q_flags))
		return;
	if (user->q_home == NULL)
	{
		syserr("554 forward: no home");
		user->q_home = "/nosuchdirectory";
	}

	/* good address -- look for .forward file in home */
	define('z', user->q_home, e);
	define('u', user->q_user, e);
	define('h', user->q_host, e);
	if (ForwardPath == NULL)
		ForwardPath = newstr("\201z/.forward");

	for (pp = ForwardPath; pp != NULL; pp = ep)
	{
		int err;
		char buf[MAXPATHLEN+1];
		extern int include();

		ep = strchr(pp, ':');
		if (ep != NULL)
			*ep = '\0';
		expand(pp, buf, &buf[sizeof buf - 1], e);
		if (ep != NULL)
			*ep++ = ':';
		if (tTd(27, 3))
			printf("forward: trying %s\n", buf);

		err = include(buf, TRUE, user, sendq, aliaslevel, e);
		if (err == 0)
			break;
		else if (transienterror(err))
		{
			/* we have to suspend this message */
			if (tTd(27, 2))
				printf("forward: transient error on %s\n", buf);
#ifdef LOG
			if (LogLevel > 2)
				syslog(LOG_ERR, "%s: forward %s: transient error: %s",
					e->e_id == NULL ? "NOQUEUE" : e->e_id,
					buf, errstring(err));
#endif
			message("%s: %s: message queued", buf, errstring(err));
			user->q_flags |= QQUEUEUP;
			return;
		}
	}
}
/*
**  MAPHOST -- given a host description, produce a mapping.
**
**	This is done by looking up the name in the alias file,
**	preceeded by an "@".  This can be used for UUCP mapping.
**	For example, a call with {blia, ., UUCP} as arguments
**	might return {ucsfcgl, !, blia, ., UUCP} as the result.
**
**	We first break the input into three parts -- before the
**	lookup, the lookup itself, and after the lookup.  We
**	then do the lookup, concatenate them together, and rescan
**	the result.
**
**	Parameters:
**		pvp -- the parameter vector to map.
**
**	Returns:
**		The result of the mapping.  If nothing found, it
**		should just concatenate the three parts together and
**		return that.
**
**	Side Effects:
**		none.
*/

char **
maphost(pvp)
	char **pvp;
{
	register char **avp;
	register char **bvp;
	char *p;
	char buf1[MAXNAME];
	char buf2[MAXNAME];
	char buf3[MAXNAME];
	extern char **prescan();

	/*
	**  Extract the three parts of the input as strings.
	*/

	/* find the part before the lookup */
	for (bvp = pvp; *bvp != NULL && **bvp != MATCHLOOKUP; bvp++)
		continue;
	if (*bvp == NULL)
		return (pvp);
	p = *bvp;
	*bvp = NULL;
	cataddr(pvp, buf1, sizeof buf1);
	*bvp++ = p;

	/* find the rest of the lookup */
	for (avp = bvp; *pvp != NULL && **bvp != MATCHELOOKUP; bvp++)
		continue;
	if (*bvp == NULL)
		return (pvp);
	p = *bvp;
	*bvp = NULL;
	cataddr(avp, buf2, sizeof buf2);
	*bvp++ = p;

	/* save the part after the lookup */
	cataddr(bvp, buf3, sizeof buf3);

	/*
	**  Now look up the middle part.
	*/

	p = aliaslookup(buf2);
	if (p != NULL)
		strcpy(buf2, p);

	/*
	**  Put the three parts back together and break into tokens.
	*/

	strcat(buf1, buf2);
	strcat(buf1, buf3);
	avp = prescan(buf1, '\0');

	/* return this mapping */
	return (avp);
}
