/*
 * Copyright (c) 1992 Eric P. Allman.
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)map.c	6.13 (Berkeley) %G%";
#endif /* not lint */

#include "sendmail.h"

#ifdef DBM_MAP
#include <ndbm.h>
#endif
#if defined(HASH_MAP) || defined(BTREE_MAP)
#include <db.h>
#endif
#ifdef NIS_MAP
#include <rpcsvc/ypclnt.h>
#endif


#ifdef DBM_MAP

/*
**  DBM_MAP_INIT -- DBM-style map initialization
**
**	Parameters:
**		map -- the pointer to the actual map
**		mapname -- the name of the map (for error messages)
**		args -- a pointer to the config file line arguments
**
**	Returns:
**		TRUE -- if it could successfully open the map.
**		FALSE -- otherwise.
**
**	Side Effects:
**		Gives an error if it can't open the map.
*/

bool
dbm_map_init(map, mapname, args)
	MAP *map;
	char *mapname;
	char *args;
{
	DBM *dbm;

	map_parseargs(map, &args);
	if (map->map_file == NULL)
	{
		syserr("No file name for DBM map %s", mapname);
		return FALSE;
	}
	dbm = dbm_open(map->map_file, O_RDONLY, 0644);
	if (dbm == NULL)
	{
		if (!bitset(MF_OPTIONAL, map->map_flags))
			syserr("Cannot open DBM database %s", map->map_file);
		return FALSE;
	}
	map->map_db = (void *) dbm;
	return TRUE;
}
/*
**  DBM_MAP_LOOKUP -- look up a datum in a DBM-type map
**
**	Parameters:
**		map -- the map to look up in.
**		buf -- a pointer to to the buffer containing the key.
**			This is a null terminated string.
**		bufsiz -- the size of buf -- note that this is in general
**			larger that strlen(buf), and buf can be changed
**			in place if desired.
**		av -- arguments from the config file (can be interpolated
**			into the final result).
**		statp -- pointer to status word (out-parameter).
**
**	Returns:
**		A pointer to the rewritten result.
**		NULL if not found in the map.
*/

char *
dbm_map_lookup(map, buf, bufsiz, av, statp)
	MAP *map;
	char buf[];
	int bufsiz;
	char **av;
	int *statp;
{
	datum key, val;

	key.dptr = buf;
	key.dsize = strlen(buf);
	if (!bitset(MF_NOFOLDCASE, map->map_flags))
	{
		register char *p;

		for (p = buf; *p != '\0'; p++)
			if (isascii(*p) && isupper(*p))
				*p = tolower(*p);
	}
	if (bitset(MF_INCLNULL, map->map_flags))
		key.dsize++;
	(void) lockfile(dbm_dirfno((DBM *) map->map_db), map->map_file, LOCK_SH);
	val = dbm_fetch((DBM *) map->map_db, key);
	(void) lockfile(dbm_dirfno((DBM *) map->map_db), map->map_file, LOCK_UN);
	if (val.dptr == NULL)
		return NULL;
	if (!bitset(MF_MATCHONLY, map->map_flags))
		map_rewrite(val.dptr, val.dsize, buf, bufsiz, av);
	return buf;
}

#endif /* DBM_MAP */

#ifdef BTREE_MAP

/*
**  BTREE_MAP_INIT -- BTREE-style map initialization
**
**	Parameters:
**		map -- the pointer to the actual map
**		mapname -- the name of the map (for error messages)
**		args -- a pointer to the config file line arguments
**
**	Returns:
**		TRUE -- if it could successfully open the map.
**		FALSE -- otherwise.
**
**	Side Effects:
**		Gives an error if it can't open the map.
*/

bool
bt_map_init(map, mapname, args)
	MAP *map;
	char *mapname;
	char *args;
{
	DB *db;

	map_parseargs(map, &args);
	if (map->map_file == NULL)
	{
		syserr("No file name for BTREE map %s", mapname);
		return FALSE;
	}
	db = dbopen(map->map_file, O_RDONLY, 0644, DB_BTREE, NULL);
	if (db == NULL)
	{
		if (!bitset(MF_OPTIONAL, map->map_flags))
			syserr("Cannot open BTREE database %s", map->map_file);
		return FALSE;
	}
	map->map_db = (void *) db;
	return TRUE;
}

#endif /* BTREE_MAP */

#ifdef HASH_MAP

/*
**  HASH_MAP_INIT -- HASH-style map initialization
**
**	Parameters:
**		map -- the pointer to the actual map
**		mapname -- the name of the map (for error messages)
**		args -- a pointer to the config file line arguments
**
**	Returns:
**		TRUE -- if it could successfully open the map.
**		FALSE -- otherwise.
**
**	Side Effects:
**		Gives an error if it can't open the map.
*/

bool
hash_map_init(map, mapname, args)
	MAP *map;
	char *mapname;
	char *args;
{
	DB *db;

	map_parseargs(map, &args);
	if (map->map_file == NULL)
	{
		syserr("No file name for HASH map %s", mapname);
		return FALSE;
	}
	db = dbopen(map->map_file, O_RDONLY, 0644, DB_HASH, NULL);
	if (db == NULL)
	{
		if (!bitset(MF_OPTIONAL, map->map_flags))
			syserr("Cannot open HASH database %s", map->map_file);
		return FALSE;
	}
	map->map_db = (void *) db;
	return TRUE;
}

#endif /* HASH_MAP */

#if defined(BTREE_MAP) || defined(HASH_MAP)

/*
**  DB_MAP_LOOKUP -- look up a datum in a BTREE- or HASH-type map
**
**	Parameters:
**		map -- the map to look up in.
**		buf -- a pointer to to the buffer containing the key.
**			This is a null terminated string.
**		bufsiz -- the size of buf -- note that this is in general
**			larger that strlen(buf), and buf can be changed
**			in place if desired.
**		av -- arguments from the config file (can be interpolated
**			into the final result).
**		statp -- pointer to status word (out-parameter).
**
**	Returns:
**		A pointer to the rewritten result.
**		NULL if not found in the map.
*/

char *
db_map_lookup(map, buf, bufsiz, av, statp)
	MAP *map;
	char buf[];
	int bufsiz;
	char **av;
	int *statp;
{
	DBT key, val;

	key.data = buf;
	key.size = strlen(buf);
	if (!bitset(MF_NOFOLDCASE, map->map_flags))
	{
		register char *p;

		for (p = buf; *p != '\0'; p++)
			if (isascii(*p) && isupper(*p))
				*p = tolower(*p);
	}
	if (bitset(MF_INCLNULL, map->map_flags))
		key.size++;
	if (((DB *) map->map_db)->get((DB *) map->map_db, &key, &val, 0) != 0)
		return NULL;
	if (!bitset(MF_MATCHONLY, map->map_flags))
		map_rewrite(val.data, val.size, buf, bufsiz, av);
	return buf;
}

#endif /* BTREE_MAP || HASH_MAP */
/*
**  MAP_PARSEARGS -- parse config line arguments for database lookup
**
**	Parameters:
**		map -- the map being initialized.
**		pp -- an indirect pointer to the config line.  It will
**			be replaced with a pointer to the next field
**			on the line.
**
**	Returns:
**		none
**
**	Side Effects:
**		null terminates the filename; stores it in map
*/

map_parseargs(map, pp)
	MAP *map;
	char **pp;
{
	register char *p = *pp;

	for (;;)
	{
		while (isascii(*p) && isspace(*p))
			p++;
		if (*p != '-')
			break;
		switch (*++p)
		{
		  case 'N':
			map->map_flags |= MF_INCLNULL;
			break;

		  case 'o':
			map->map_flags |= MF_OPTIONAL;
			break;

		  case 'f':
			map->map_flags |= MF_NOFOLDCASE;
			break;

		  case 'm':
			map->map_flags |= MF_MATCHONLY;
			break;

		  case 'a':
			map->map_app = ++p;
			break;

		  case 'd':
			map->map_domain = ++p;
			break;
		}
		while (*p != '\0' && !(isascii(*p) && isspace(*p)))
			p++;
		if (*p != '\0')
			*p++ = '\0';
	}
	if (map->map_app != NULL)
		map->map_app = newstr(map->map_app);
	if (map->map_domain != NULL)
		map->map_domain = newstr(map->map_domain);

	if (*p != '\0')
	{
		map->map_file = p;
		while (*p != '\0' && !(isascii(*p) && isspace(*p)))
			p++;
		if (*p != '\0')
			*p++ = '\0';
		map->map_file = newstr(map->map_file);
	}

	while (*p != '\0' && isascii(*p) && isspace(*p))
		p++;
	*pp = p;
	if (*p != '\0')
		map->map_rebuild = newstr(p);
}

# ifdef NIS_MAP

/*
**  NIS_MAP_INIT -- initialize DBM map
**
**	Parameters:
**		map -- the pointer to the actual map.
**		mapname -- the name of the map (for error messages).
**		args -- a pointer to the config file line arguments.
**
**	Returns:
**		TRUE -- if it could successfully open the map.
**		FALSE -- otherwise.
**
**	Side Effects:
**		Prints an error if it can't open the map.
*/

bool
nis_map_init(map, mapname, args)
	MAP *map;
	char *mapname;
	char *args;
{
	int yperr;
	char *master;

	/* parse arguments */
	map_parseargs(map, &args);
	if (map->map_file == NULL)
	{
		syserr("No NIS map name for map %s", mapname);
		return FALSE;
	}
	if (map->map_domain == NULL)
		yp_get_default_domain(&map->map_domain);

	/* check to see if this map actually exists */
	yperr = yp_master(map->map_domain, map->map_file, &master);
	if (yperr == 0)
		return TRUE;
	if (!bitset(MF_OPTIONAL, map->map_flags))
		syserr("Cannot bind to domain %s: %s", map->map_domain,
			yperr_string(yperr));
	return FALSE;
}
/*
**  NIS_MAP_LOOKUP -- look up a datum in a NIS map
**
**	Parameters:
**		map -- the map to look up in.
**		buf -- a pointer to to the buffer containing the key.
**			This is a null terminated string.
**		bufsiz -- the size of buf -- note that this is in general
**			larger that strlen(buf), and buf can be changed
**			in place if desired.
**		av -- arguments from the config file (can be interpolated
**			into the final result).
**		statp -- pointer to status word (out-parameter).
**
**	Returns:
**		A pointer to the rewritten result.
**		NULL if not found in the map.
*/

char *
nis_map_lookup(map, buf, bufsiz, av, statp)
	MAP *map;
	char buf[];
	int bufsiz;
	char **av;
	int *statp;
{
	char *vp;
	auto int vsize;
	int buflen;

	if (!bitset(MF_NOFOLDCASE, map->map_flags))
	{
		register char *p;

		for (p = buf; *p != '\0'; p++)
			if (isascii(*p) && isupper(*p))
				*p = tolower(*p);
	}
	buflen = strlen(buf);
	if (bitset(MF_INCLNULL, map->map_flags))
		buflen++;
	if (yp_match(map->map_domain, map->map_file, buf, buflen,
		     &vp, &vsize) != 0)
		return NULL;
	if (!bitset(MF_MATCHONLY, map->map_flags))
		map_rewrite(vp, vsize, buf, bufsiz, av);
	return buf;
}

#endif /* NIS_MAP */
/*
**  MAP_REWRITE -- rewrite a database key, interpolating %n indications.
**
**	Parameters:
**		s -- the string to rewrite, NOT necessarily null terminated.
**		slen -- the length of s.
**		buf -- the place to write it.
**		buflen -- the length of buf.
**		av -- arguments to interpolate into buf.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

map_rewrite(s, slen, buf, buflen, av)
	register char *s;
	int slen;
	char buf[];
	int buflen;
	char **av;
{
	register char *bp;
	char *buflim;
	register char c;
	char **avp;
	register char *ap;

	if (tTd(23, 1))
	{
		printf("map_rewrite(%.*s), av =\n", slen, s);
		for (avp = av; *avp != NULL; avp++)
			printf("\t%s\n", *avp);
	}

	bp = buf;
	buflim = &buf[buflen - 2];
	while (--slen >= 0 && (c = *s++) != '\0')
	{
		if (c != '%')
		{
  pushc:
			if (bp < buflim)
				*bp++ = c;
			continue;
		}
		if (--slen < 0 || (c = *s++) == '\0')
			c = '%';
		if (c == '%')
			goto pushc;
		if (!(isascii(c) && isdigit(c)))
		{
			*bp++ = '%';
			goto pushc;
		}
		c -= '0';
		for (avp = av; --c >= 0 && *avp != NULL; avp++)
			continue;
		if (*avp == NULL)
			continue;

		/* transliterate argument into output string */
		for (ap = *avp; (c = *ap++) != '\0'; )
		{
			if (bp < buflim)
				*bp++ = c;
		}
	}
	*bp++ = '\0';
	if (tTd(23, 1))
		printf("map_rewrite => %s\n", buf);
}
