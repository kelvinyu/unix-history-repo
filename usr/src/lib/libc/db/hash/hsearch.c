/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)hsearch.c	5.1 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/file.h>
#include <sys/types.h>
#include <stdio.h>
#include <db.h>
#include "search.h"

static	DB	*dbp = NULL;
static	ENTRY	retval;

extern	int
hcreate ( nel )
unsigned	nel;
{
    int	status;
    HASHINFO	info;

    info.nelem = nel;
    info.bsize = 256;
    info.ffactor = 8;
    info.ncached = NULL;
    info.hash = NULL;
    info.lorder = 0;
    dbp = hash_open ( NULL, O_CREAT|O_RDWR, 0600, &info );
    return ( (int) dbp );
}


extern ENTRY	*
hsearch ( item, action )
ENTRY	item;
ACTION	action;
{
    int	status;
    DBT	key, val;

    if ( !dbp ) {
	return(NULL);
    }

    key.data = item.key;
    key.size = strlen(item.key) + 1;

    if ( action == ENTER ) {
	val.data = item.data;
	val.size = strlen(item.data) + 1;
	status = (dbp->put) ( dbp, &key, &val, R_NOOVERWRITE );
	if ( status ) {
	    return(NULL);
	} 
    } else {
	/* FIND */
	status = (dbp->get) ( dbp, &key, &val );
	if ( status ) {
	    return ( NULL );
	} else {
	    item.data = val.data;
	}
    }
    return ( &item );
}


extern void
hdestroy ()
{
    if (dbp) {
	(void)(dbp->close) (dbp);
	dbp = NULL;
    }
    return;
}


