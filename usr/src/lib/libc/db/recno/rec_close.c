/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rec_close.c	5.11 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include <db.h>
#include "recno.h"

/*
 * __REC_CLOSE -- Close a recno tree.
 *
 * Parameters:
 *	dbp:	pointer to access method
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
__rec_close(dbp)
	DB *dbp;
{
	BTREE *t;
	int rval;

	if (__rec_sync(dbp, 0) == RET_ERROR)
		return (RET_ERROR);

	/* Committed to closing. */
	t = dbp->internal;

	rval = RET_SUCCESS;
	if (ISSET(t, R_MEMMAPPED) && munmap(t->bt_smap, t->bt_msize))
		rval = RET_ERROR;

	if (!ISSET(t, R_INMEM))
		if (ISSET(t, R_CLOSEFP)) {
			if (fclose(t->bt_rfp))
				rval = RET_ERROR;
		} else
			if (close(t->bt_rfd))
				rval = RET_ERROR;

	if (__bt_close(dbp) == RET_ERROR)
		rval = RET_ERROR;

	return (rval);
}

/*
 * __REC_SYNC -- sync the recno tree to disk.
 *
 * Parameters:
 *	dbp:	pointer to access method
 *
 * Returns:
 *	RET_SUCCESS, RET_ERROR.
 */
int
__rec_sync(dbp, flags)
	const DB *dbp;
	u_int flags;
{
	struct iovec iov[2];
	BTREE *t;
	DBT data, key;
	off_t off;
	recno_t scursor, trec;
	int status;

	t = dbp->internal;

	if (flags == R_RECNOSYNC)
		return (__bt_sync(dbp, 0));

	if (ISSET(t, R_RDONLY | R_INMEM) || !ISSET(t, R_MODIFIED))
		return (RET_SUCCESS);

	/* Read any remaining records into the tree. */
	if (!ISSET(t, R_EOF) && t->bt_irec(t, MAX_REC_NUMBER) == RET_ERROR)
		return (RET_ERROR);

	/* Rewind the file descriptor. */
	if (lseek(t->bt_rfd, (off_t)0, SEEK_SET) != 0)
		return (RET_ERROR);

	iov[1].iov_base = "\n";
	iov[1].iov_len = 1;
	scursor = t->bt_rcursor;

	key.size = sizeof(recno_t);
	key.data = &trec;

	status = (dbp->seq)(dbp, &key, &data, R_FIRST);
        while (status == RET_SUCCESS) {
		iov[0].iov_base = data.data;
		iov[0].iov_len = data.size;
		if (writev(t->bt_rfd, iov, 2) != data.size + 1)
			return (RET_ERROR);
                status = (dbp->seq)(dbp, &key, &data, R_NEXT);
        }
	t->bt_rcursor = scursor;
	if (status == RET_ERROR)
		return (RET_ERROR);
	if ((off = lseek(t->bt_rfd, (off_t)0, SEEK_CUR)) == -1)
		return (RET_ERROR);
	if (ftruncate(t->bt_rfd, off))
		return (RET_ERROR);
	CLR(t, R_MODIFIED);
	return (RET_SUCCESS);
}
