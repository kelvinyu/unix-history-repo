/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)SEEK.c 1.2 %G%";

#include "h00vars.h"

/*
 * Random access routine
 */
SEEK(curfile, loc)

	register struct iorec	*curfile;
	long			loc;
{
	curfile->funit |= SYNC;
	if (fseek(curfile->fbuf, loc, 0) == -1) {
		PERROR("Could not reset ", curfile->pfname);
		return;
	}
}
