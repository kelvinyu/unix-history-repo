/*-
 * Copyright (c) 1982, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)DFDISPOSE.c	8.1 (Berkeley) %G%";
#endif /* not lint */

/*
 * Close all active files within a dynamic record,
 * then dispose of the record.
 */

#include "h00vars.h"
#include "libpc.h"

DFDISPOSE(var, size)
	char	**var;	/* pointer to pointer being deallocated */
	long	size;	/* sizeof(bletch) */
{
	register struct iorec	*next, *prev;
	struct iorec *start, *end;

	start = (struct iorec *)(*var);
	end = (struct iorec *)(*var + size);
	prev = (struct iorec *)(&_fchain);
	next = _fchain.fchain;
	while(next != FILNIL && (next->flev < GLVL || next < start)) {
		prev = next;
		next = next->fchain;
	}
	while(next != FILNIL && start <= next && next < end)
		next = PFCLOSE(next, TRUE);
	prev->fchain = next;
	DISPOSE(var, size);
}
