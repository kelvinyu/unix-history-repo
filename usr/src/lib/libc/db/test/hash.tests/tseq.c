/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1991 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)tseq.c	5.3 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <db.h>

#define INITIAL	25000
#define MAXWORDS    25000	       /* # of elements in search table */


char	wp[8192];
char	cp[8192];
main(argc, argv)
char **argv;
{
	DBT item, key, res;
	DB	*dbp;
	FILE *fp;
	int	stat;

	if (!(dbp = hash_open( "hashtest", O_RDONLY, 0400, NULL))) {
		/* create table */
		fprintf(stderr, "cannot open: hash table\n" );
		exit(1);
	}

/*
* put info in structure, and structure in the item
*/
	for ( stat = (dbp->seq) (dbp, &res, &item, 1 ); 
	      stat == 0;
	      stat = (dbp->seq) (dbp, &res, &item, 0 ) ) {

	      bcopy ( res.data, wp, res.size );
	      wp[res.size] = 0;
	      bcopy ( item.data, cp, item.size );
	      cp[item.size] = 0;

	      printf ( "%s %s\n", wp, cp );
	}
	(dbp->close)(dbp);
	exit(0);
}
