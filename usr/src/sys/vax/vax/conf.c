/*	conf.c	3.9	%G%	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/tty.h"
#include "../h/conf.h"
#include "../h/text.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/acct.h"
#include "../h/mba.h"

int	nulldev();
int	nodev();

int	hpstrategy(),hpread(),hpwrite(),hpintr();
struct	buf	hptab;
 
int	htopen(),htclose(),htstrategy(),htread(),htwrite();
struct	buf	httab;

#ifdef ERNIE
int	upstrategy(),upread(),upwrite(),upreset();
struct	buf	uptab;
#endif

int	swstrategy(),swread(),swwrite();

struct bdevsw	bdevsw[] =
{
/* 0 */	nulldev,	nulldev,	hpstrategy,	&hptab,
/* 1 */	htopen,		htclose,	htstrategy,	&httab,
#ifdef ERNIE
/* 2 */	nulldev,	nulldev,	upstrategy,	&uptab,
#endif
/* 3 */	nodev,		nodev,		nodev,		0,
/* 4 */	nodev,		nodev,		swstrategy,	0,
	0,
};

int	cnopen(),cnclose(),cnread(),cnwrite(),cnioctl();

#ifdef ERNIE
int	dhopen(),dhclose(),dhread(),dhwrite(),dhioctl(),dhstop(),dhreset();
struct	tty dh11[];
#endif

int	flopen(),flclose(),flread(),flwrite();

int	dzopen(),dzclose(),dzread(),dzwrite(),dzioctl(),dzstop(),dzreset();
struct	tty dz_tty[];

int	syopen(),syread(),sywrite(),syioctl();

int 	mmread(),mmwrite();

#ifdef ERNIE
int	vpopen(),vpclose(),vpwrite(),vpioctl(),vpreset();
int	vaopen(),vaclose(),vawrite(),vaioctl(),vareset();
#endif

int	mxopen(),mxclose(),mxread(),mxwrite(),mxioctl();
int	mcread();
char	*mcwrite();


struct cdevsw	cdevsw[] =
{
/*0*/	cnopen,		cnclose,	cnread,		cnwrite,
	cnioctl,	nulldev,	nulldev,	0,
/*1*/	dzopen,		dzclose,	dzread,		dzwrite,
	dzioctl,	dzstop,		dzreset,	dz_tty,
/*2*/	syopen,		nulldev,	syread,		sywrite,
	syioctl,	nulldev,	nulldev,	0,
/*3*/	nulldev,	nulldev,	mmread,		mmwrite,
	nodev,		nulldev,	nulldev,	0,
/*4*/	nulldev,	nulldev,	hpread,		hpwrite,
	nodev,		nodev,		nulldev,	0,
/*5*/	htopen,		htclose,	htread,		htwrite,
	nodev,		nodev,		nulldev,	0,
#ifdef ERNIE
/*6*/	vpopen,		vpclose,	nodev,		vpwrite,
	vpioctl,	nulldev,	vpreset,	0,
#else
/*6*/	nodev,		nodev,		nodev,		nodev,
	nodev,		nodev,		nulldev,	0,
#endif
/*7*/	nulldev,	nulldev,	swread,		swwrite,
	nodev,		nodev,		nulldev,	0,
/*8*/	flopen,		flclose,	flread,		flwrite,
	nodev,		nodev,		nulldev,	0,
/*9*/	mxopen,		mxclose,	mxread,		mxwrite,
	mxioctl,	nulldev,	nulldev,	0,
#ifdef ERNIE
/*10*/	vaopen,		vaclose,	nodev,		vawrite,
	vaioctl,	nulldev,	vareset,	0,
/*11*/	nodev,		nodev,		nodev,		nodev,
	nodev,		nodev,		nulldev,	0,
/*12*/	dhopen,		dhclose,	dhread,		dhwrite,
	dhioctl,	dhstop,		dhreset,	dh11,
/*13*/	nulldev,	nulldev,	upread,		upwrite,
	nodev,		nodev,		upreset,	0,
#endif
	0,	
};

int	ttyopen(),ttread(),nullioctl(),ttstart();
char	*ttwrite();
int	ttyinput(),ttyrend();
int	bkopen(),bkclose(),bkread(),bkinput(),bkioctl();
int	ntyopen(),ntyclose(),ntread();
int	okopen(),okread(),okioctl(),okinput();
char	*ntwrite();
int	ntyinput(),ntyrend();
 
struct	linesw linesw[] =
{
	ttyopen, nulldev, ttread, ttwrite, nullioctl,
	ttyinput, ttyrend, nulldev, nulldev, nulldev,	/* 0 */
	bkopen, bkclose, bkread, ttwrite, bkioctl,
	bkinput, nodev, nulldev, ttstart, nulldev,	/* 1 */
	ntyopen, ntyclose, ntread, ntwrite, nullioctl,
	ntyinput, ntyrend, nulldev, ttstart, nulldev,	/* 2 */
	okopen, nulldev, okread, ttwrite, okioctl,
	okinput, nodev, nulldev, ttstart, nulldev,	/* 3 */
	mxopen, mxclose, mcread, mcwrite, mxioctl,
	nulldev, nulldev, nulldev, nulldev, nulldev,	/* 4 */
	0
};
 
int	nldisp = 4;
 
struct	buf	buf[NBUF];
struct	file	file[NFILE];
struct	inode	inode[NINODE];
struct	text	text[NTEXT];
struct	proc	proc[NPROC];
struct	buf	bfreelist;
struct	buf	bswlist;	/* free list of swap headers */
struct	buf	*bclnlist;	/* header for list of cleaned pages */
struct	acct	acctbuf;
struct	inode	*acctp;
 
int	mem_no = 3; 	/* major device number of memory special file */

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines as well as the placement
 * of the block on the d_tab chains.  Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(4, 0);


extern struct user u;

int mbanum[] = {	/* mba number of major device */
	0,		/* disk */
	1,		/* tape */
	9999999,	/* unused */
	9999999,	/* unused */
	0,		/* disk, raw */
	1,		/* tape, raw */
};

int *mbaloc[] = { 	/* virtual location of mba */
	(int *)MBA0,
	(int *)MBA1,
};
