/*-
 * Copyright (c) 1982, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)PFCLOSE.c	8.1 (Berkeley) %G%";
#endif /* not lint */

/*
 * Close a Pascal file deallocating resources as appropriate.
 */

#include "h00vars.h"
#include "libpc.h"

struct iorec *
PFCLOSE(filep, lastuse)
	register struct iorec *filep;
	bool lastuse;
{
	if ((filep->funit & FDEF) == 0 && filep->fbuf != NULL) {
		/*
		 * Have a previous buffer, close associated file.
		 */
		if (filep->fblk > PREDEF) {
			fflush(filep->fbuf);
			setbuf(filep->fbuf, NULL);
		}
		fclose(filep->fbuf);
		if (ferror(filep->fbuf)) {
			ERROR("%s: Close failed\n", filep->pfname);
			return;
		}
		/*
		 * Temporary files are discarded.
		 */
		if ((filep->funit & TEMP) != 0 && lastuse &&
		    unlink(filep->pfname)) {
			PERROR("Could not remove ", filep->pfname);
			return;
		}
	}
	_actfile[filep->fblk] = FILNIL;
	return (filep->fchain);
}
