/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)praliases.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include <ndbm.h>
#include <sendmail.h>
#ifdef NEWDB
#include <db.h>
#endif

int
main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	DBM *dbp;
	datum content, key;
	char *filename;
	int ch;
#ifdef NEWDB
	const DB *db;
	DBT newdbkey, newdbcontent;
	char buf[MAXNAME];
#endif

	filename = "/etc/aliases";
	while ((ch = getopt(argc, argv, "f:")) != EOF)
		switch((char)ch) {
		case 'f':
			filename = optarg;
			break;
		case '?':
		default:
			(void)fprintf(stderr, "usage: praliases [-f file]\n");
			exit(EX_USAGE);
		}
	argc -= optind;
	argv += optind;

#ifdef NEWDB
	(void) strcpy(buf, filename);
	(void) strcat(buf, ".db");
	if (db = dbopen(buf, O_RDONLY, 0444 , DB_HASH, NULL)) {
		if (!argc) {
			while(!db->seq(db, &newdbkey, &newdbcontent, R_NEXT))
				printf("%s:%s\n", newdbkey.data,
						newdbcontent.data);
		}
		else for (; *argv; ++argv) {
			newdbkey.data = *argv;
			newdbkey.size = strlen(*argv) + 1;
			if ( !db->get(db, &newdbkey, &newdbcontent, 0) )
				printf("%s:%s\n", newdbkey.data,
					newdbcontent.data);
			else
				printf("%s: No such key\n",
					newdbkey.data);
		}
	}
	else {
#endif
		if ((dbp = dbm_open(filename, O_RDONLY, 0)) == NULL) {
			(void)fprintf(stderr,
			    "praliases: %s: %s\n", filename, strerror(errno));
			exit(EX_OSFILE);
		}
		if (!argc)
			for (key = dbm_nextkey(dbp);
			    key.dptr != NULL; key = dbm_nextkey(dbp)) {
				content = dbm_fetch(dbp, key);
				(void)printf("%s:%s\n", key.dptr, content.dptr);
			}
		else for (; *argv; ++argv) {
			key.dptr = *argv;
			key.dsize = strlen(*argv) + 1;
			content = dbm_fetch(dbp, key);
			if (!content.dptr)
				(void)printf("%s: No such key\n", key.dptr);
			else
				(void)printf("%s:%s\n", key.dptr, content.dptr);
		}
#ifdef NEWDB
	}
#endif
	exit(EX_OK);
}
