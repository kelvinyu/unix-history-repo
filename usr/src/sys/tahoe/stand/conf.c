/*	conf.c	1.1	86/01/12	*/
/*	conf.c	6.1	83/07/29	*/

#include "../machine/pte.h"

#include "param.h"
#include "inode.h"
#include "fs.h"


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

	(*devsw[io->i_ino.i_dev].dv_open)(io);
}

devclose(io)
	register struct iob *io;
{

	(*devsw[io->i_ino.i_dev].dv_close)(io);
}


/*ARGSUSED*/
nullsys(io)
	struct iob *io;
{

	;
}


int	nullsys();


#define wchstrategy 	nullsys	
#define wchopen 	nullsys 
#define ctstrategy 	nullsys	
#define ctopen 		nullsys		
#define ctclose 	nullsys

int	udstrategy(),	udopen();
int	vdstrategy(),	vdopen();
int	cystrategy(),	cyopen(),	cyclose();

struct devsw devsw[] = {
	"fsd",  vdstrategy,	vdopen,		nullsys,
	"smd",  vdstrategy,	vdopen,		nullsys,
	"xfd",  vdstrategy,	vdopen,		nullsys,
	"fuj",  vdstrategy,	vdopen,		nullsys,
	"xsd",  vdstrategy,	vdopen,		nullsys,

	"xmd",	udstrategy,	udopen,		nullsys,
	"flp",	udstrategy,	udopen,		nullsys,
	"cyp",	cystrategy,	cyopen,		cyclose,
	"wch",	wchstrategy,	wchopen,	nullsys,
	"ctp",	ctstrategy,	ctopen,		ctclose,
	0,	0,		0,		0
};

