/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)conf.c	7.3 (Berkeley) %G%
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/inode.h"
#include "../h/fs.h"

#include "../vaxmba/mbareg.h"

#include "saio.h"

devread(io)
	register struct iob *io;
{
	int cc;

	io->i_flgs |= F_RDDATA;
	io->i_error = 0;
	cc = (*devsw[io->i_ino.i_dev].dv_strategy)(io, READ);
	io->i_flgs &= ~F_TYPEMASK;
	return (cc);
}

devwrite(io)
	register struct iob *io;
{
	int cc;

	io->i_flgs |= F_WRDATA;
	io->i_error = 0;
	cc = (*devsw[io->i_ino.i_dev].dv_strategy)(io, WRITE);
	io->i_flgs &= ~F_TYPEMASK;
	return (cc);
}

devopen(io)
	register struct iob *io;
{

	return (*devsw[io->i_ino.i_dev].dv_open)(io);
}

devclose(io)
	register struct iob *io;
{

	(*devsw[io->i_ino.i_dev].dv_close)(io);
}

devioctl(io, cmd, arg)
	register struct iob *io;
	int cmd;
	caddr_t arg;
{

	return ((*devsw[io->i_ino.i_dev].dv_ioctl)(io, cmd, arg));
}

/*ARGSUSED*/
nullsys(io)
	struct iob *io;
{

	;
}

/*ARGSUSED*/
nodev(io)
	struct iob *io;
{

	errno = EBADF;
}

/*ARGSUSED*/
noioctl(io, cmd, arg)
	struct iob *io;
	int cmd;
	caddr_t arg;
{

	return (ECMD);
}

#if defined(VAX780) || defined(VAX750) || defined(VAX8600)
#define	HP		"hp"
int	hpstrategy(), hpopen(), hpioctl();
#else
#define	HP		""
#define	hpstrategy	nodev
#define	hpopen		nodev
#define	hpioctl		noioctl
#endif
int	upstrategy(), upopen(), upioctl();
int	rkstrategy(), rkopen(), rkioctl();
int	rastrategy(), raopen(), raioctl();
#if defined(VAX730)
#define	RB		"rb"
int	idcstrategy(), idcopen(), idcioctl();
#else
#define	RB		""
#define	idcstrategy	nodev
#define	idcopen		nodev
#define	idcioctl	noioctl
#endif
int	rlstrategy(), rlopen(), rlioctl();

#ifndef BOOT
#define	TM		"tm"
int	tmstrategy(), tmopen(), tmclose();
#define	TS		"ts"
int	tsstrategy(), tsopen(), tsclose();
#if defined(VAX780) || defined(VAX750) || defined(VAX8600)
#define	HT		"ht"
int	htstrategy(), htopen(), htclose();
#define	MT		"mt"
int	mtstrategy(), mtopen(), mtclose();

#else massbus vax
#define	HT		""
#define	htstrategy	nodev
#define	htopen		nodev
#define	htclose		nodev
#define	MT		""
#define	mtstrategy	nodev
#define	mtopen		nodev
#define	mtclose		nodev
#endif massbus vax

#define	UT		"ut"
int	utstrategy(), utopen(), utclose();
#else BOOT
#define	TM		""
#define	tmstrategy	nodev
#define	tmopen		nodev
#define	tmclose		nodev
#define	TS		""
#define	tsstrategy	nodev
#define	tsopen		nodev
#define	tsclose		nodev
#define	HT		""
#define	htstrategy	nodev
#define	htopen		nodev
#define	htclose		nodev
#define	MT		""
#define	mtstrategy	nodev
#define	mtopen		nodev
#define	mtclose		nodev
#define	UT		""
#define	utstrategy	nodev
#define	utopen		nodev
#define	utclose		nodev
#endif

struct devsw devsw[] = {
	{ HP,	hpstrategy,	hpopen,	nullsys,	hpioctl }, /* 0 = hp */
	{ HT,	htstrategy,	htopen,	htclose,	noioctl }, /* 1 = ht */
	{ "up",	upstrategy,	upopen,	nullsys,	upioctl }, /* 2 = up */
	{ "hk",	rkstrategy,	rkopen,	nullsys,	rkioctl }, /* 3 = hk */
	{ "",	nodev,		nodev,	nullsys,	noioctl }, /* 4 = sw */
	{ TM,	tmstrategy,	tmopen,	tmclose,	noioctl }, /* 5 = tm */
	{ TS,	tsstrategy,	tsopen,	tsclose,	noioctl }, /* 6 = ts */
	{ MT,	mtstrategy,	mtopen,	mtclose,	noioctl }, /* 7 = mt */
	{ "",	nodev,		nodev,	nullsys,	noioctl }, /* 8 = tu */
	{ "ra",	rastrategy,	raopen,	nullsys,	raioctl }, /* 9 = ra */
	{ UT,	utstrategy,	utopen,	utclose,	noioctl }, /* 10 = ut */
	{ RB,	idcstrategy,	idcopen,nullsys,	idcioctl },/* 11 = rb */
	{ "",	nodev,		nodev,	nullsys,	noioctl }, /* 12 = uu */
	{ "",	nodev,		nodev,	nullsys,	noioctl }, /* 13 = rx */
	{ "rl",	rlstrategy,	rlopen,	nullsys,	rlioctl }, /* 14 = rl */
	{ 0, 0, 0, 0, 0 },
};

int	ndevs = (sizeof(devsw) / sizeof(devsw[0]) - 1);
