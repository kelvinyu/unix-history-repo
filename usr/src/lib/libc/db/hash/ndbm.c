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
static char sccsid[] = "@(#)ndbm.c	5.1 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

/*
    This package provides a dbm compatible interface to the new hashing
    package described in db(3)
*/

#include <sys/param.h>
#include <ndbm.h>
#include <stddef.h>
#include "hash.h"

/*
    return 	*DBM on success
		NULL on failure
*/
extern DBM *
dbm_open( file, flags, mode )
char 	*file;
int	flags;
int	mode;
{
    HASHINFO	info;

    info.bsize = 1024;
    info.ffactor = 5;
    info.nelem = 1;
    info.ncached = NULL;
    info.hash = NULL;
    info.lorder = 0;
    return( hash_open ( file, flags, mode, &info ) );
}

extern void 
dbm_close(db)
DBM	*db;
{
    (void)(db->close) (db);
}

/*
    Returns 	DATUM on success
		NULL on failure
*/
extern datum 
dbm_fetch( db, key )
DBM 	*db;
datum	key;
{
    int	status;
    datum	retval;

    status = (db->get) ( db, (DBT *)&key, (DBT *)&retval );
    if ( status ) {
	retval.dptr = NULL;
    }
    return(retval);
}

/*
    Returns 	DATUM on success
		NULL on failure
*/
extern datum 
dbm_firstkey(db)
DBM 	*db;
{
    int	status;
    datum	retkey;
    datum	retdata;

    status = (db->seq) ( db, (DBT *)&retkey, (DBT *)&retdata, R_FIRST );
    if ( status ) {
	retkey.dptr = NULL;
    }
    return(retkey);
}
/*
    Returns 	DATUM on success
		NULL on failure
*/
extern datum 
dbm_nextkey(db)
DBM 	*db;
{
    int	status;
    datum	retkey;
    datum	retdata;

    status = (db->seq) ( db, (DBT *)&retkey, (DBT *)&retdata, R_NEXT );
    if ( status ) {
	retkey.dptr = NULL;
    }
    return(retkey);
}

/*
    0 on success
    <0 failure
*/
extern int 
dbm_delete(db, key)
DBM 	*db;
datum	key;
{
    int	status;

    status = (db->delete)( db, (DBT *)&key );
    if ( status ) {
	return(-1);
    } else {
	return(0);
    }
}

/*
    0 on success
    <0 failure
    1 if DBM_INSERT and entry exists
*/
extern int 
dbm_store(db, key, content, flags)
DBM 	*db;
datum	key;
datum	content;
int	flags;
{
    return ((db->put)( db, (DBT *)&key, (DBT *)&content, 
			(flags == DBM_INSERT) ? R_NOOVERWRITE : 0 ));
}

extern int
dbm_error(db)
DBM	*db;
{
    HTAB	*hp;

    hp = (HTAB *)db->internal;
    return ( hp->errno );
}

extern int
dbm_clearerr(db)
DBM	*db;
{
    HTAB	*hp;

    hp = (HTAB *)db->internal;
    hp->errno = 0;
    return ( 0 );
}
