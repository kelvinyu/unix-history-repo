/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rec_utils.c	5.3 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <db.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "recno.h"

/*
 * __REC_RET -- Build return data as a result of search or scan.
 *
 * Parameters:
 *	t:	tree
 *	d:	LEAF to be returned to the user.
 *	data:	user's data structure
 *
 * Returns:
 *	RET_SUCCESS, RET_ERROR.
 */
int
__rec_ret(t, e, data)
	BTREE *t;
	EPG *e;
	DBT *data;
{
	register RLEAF *rl;
	register char *p;

	rl = GETRLEAF(e->page, e->index);
	if (rl->flags & P_BIGDATA) {
		if (__ovfl_get(t, rl->bytes,
		    &data->size, &t->bt_dbuf, &t->bt_dbufsz))
			return (RET_ERROR);
	} else {
		/* Use +1 in case the first record retrieved is 0 length. */
		if (rl->dsize + 1 > t->bt_dbufsz) {
			if ((p = realloc(t->bt_dbuf, rl->dsize + 1)) == NULL)
				return (RET_ERROR);
			t->bt_dbuf = p;
			t->bt_dbufsz = rl->dsize + 1;
		}
		bcopy(rl->bytes, t->bt_dbuf, t->bt_dbufsz);
		data->size = rl->dsize;
	}
	data->data = t->bt_dbuf;

	return (RET_SUCCESS);
}
