/*	conf.c	4.4	%G%	*/

#include "../h/param.h"
#include "../h/inode.h"
#include "../h/pte.h"
#include "../h/mba.h"
#include "saio.h"

devread(io)
register struct iob *io;
{

	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io,READ) );
}

devwrite(io)
register struct iob *io;
{
	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io, WRITE) );
}

devopen(io)
register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_open)(io);
}

devclose(io)
register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_close)(io);
}

nullsys()
{ ; }

int	nullsys();
#if VAX==780
int	hpstrategy(), hpopen();
int	htstrategy(), htopen(), htclose();
#endif
int	upstrategy(), upopen();
int	tmstrategy(), tmopen(), tmclose();
int	rkopen(),rkstrategy();

struct devsw devsw[] = {
#if VAX==780
	"hp",	hpstrategy,	hpopen,		nullsys,
	"ht",	htstrategy,	htopen,		htclose,
#endif
	"up",	upstrategy,	upopen,		nullsys,
	"tm",	tmstrategy,	tmopen,		tmclose,
	"rk",	rkstrategy,	rkopen,		ullsys,
	0,0,0,0
};

#if VAX==780
int mbanum[] = {	/* mba number of major device */
	0,		/* disk */
	1,		/* tape */
	-1,		/* unused */
};

int *mbaloc[] = { 	/* physical location of mba */
	(int *)PHYSMBA0,
	(int *)PHYSMBA1,
};
#endif
