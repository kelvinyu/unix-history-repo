/*
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)conf.c	7.1 (Berkeley) %G%
 */

#include "saio.h"
#include "../include/machMon.h"

devread(io)
	register struct iob *io;
{

	if (lseek(io->i_unit, (io->i_bn + io->i_boff) * DEV_BSIZE, 0) < 0)
		return (-1);
	return (read(io->i_unit, io->i_ma, io->i_cc));
}

#ifndef SMALL
devwrite(io)
	register struct iob *io;
{

	if (lseek(io->i_unit, (io->i_bn + io->i_boff) * DEV_BSIZE, 0) < 0)
		return (-1);
	return (write(io->i_unit, io->i_ma, io->i_cc));
}
#endif

struct devsw devsw[] = {
	"rz",
	"tz",
	0,
};
