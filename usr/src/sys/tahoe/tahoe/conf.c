/*	conf.c	1.8	87/03/26	*/

#include "param.h"
#include "systm.h"
#include "buf.h"
#include "ioctl.h"
#include "tty.h"
#include "conf.h"

int	nulldev();
int	nodev();

#include "dk.h"
#if NVD > 0
int	vdopen(),vdclose(),vdstrategy(),vdread(),vdwrite(),vdioctl();
int	vddump(),vdsize();
#else
#define	vdopen		nodev
#define	vdclose		nodev
#define	vdstrategy	nodev
#define	vdread		nodev
#define	vdwrite		nodev
#define	vdioctl		nodev
#define	vddump		nodev
#define	vdsize		0
#endif

#include "yc.h"
#if NCY > 0
int	cyopen(),cyclose(),cystrategy(),cyread(),cywrite(),cydump(),cyioctl(),cyreset();
#else
#define	cyopen		nodev
#define	cyclose		nodev
#define	cystrategy	nodev
#define	cyread		nodev
#define	cywrite		nodev
#define	cydump		nodev
#define	cyioctl		nodev
#define	cyreset		nulldev
#endif

int	swstrategy(),swread(),swwrite();

struct bdevsw	bdevsw[] =
{
	{ nodev,	nulldev,	nodev,		nodev,		/*0*/
	  nodev,	0,		0 },
	{ vdopen,	vdclose,	vdstrategy,	vdioctl,	/*1*/
	  vddump,	vdsize,		0 },
	{ nodev,	nulldev,	nodev,		nodev,		/*2*/
	  nodev,	0,		0 },
	{ cyopen,	cyclose,	cystrategy,	cyioctl,	/*3*/
	  cydump,	0,		B_TAPE },
	{ nodev,	nodev,		swstrategy,	nodev,		/*4*/
	  nodev,	0,		0 },
};
int	nblkdev = sizeof (bdevsw) / sizeof (bdevsw[0]);

int	cnopen(),cnclose(),cnread(),cnwrite(),cnioctl();
extern	struct tty cons;

#include "vx.h"
#if NVX == 0
#define	vxopen	nodev
#define	vxclose	nodev
#define	vxread	nodev
#define	vxwrite	nodev
#define	vxioctl	nodev
#define	vxstop	nodev
#define	vxreset	nulldev
#define	vx_tty	0
#else
int	vxopen(),vxclose(),vxread(),vxwrite(),vxioctl(),vxstop(),vxreset();
struct	tty vx_tty[];
#endif

int	syopen(),syread(),sywrite(),syioctl(),syselect();

int 	mmread(),mmwrite();
#define	mmselect	seltrue

#include "pty.h"
#if NPTY > 0
int	ptsopen(),ptsclose(),ptsread(),ptswrite(),ptsstop();
int	ptcopen(),ptcclose(),ptcread(),ptcwrite(),ptcselect();
int	ptyioctl();
struct	tty pt_tty[];
#else
#define ptsopen		nodev
#define ptsclose	nodev
#define ptsread		nodev
#define ptswrite	nodev
#define ptcopen		nodev
#define ptcclose	nodev
#define ptcread		nodev
#define ptcwrite	nodev
#define ptyioctl	nodev
#define	pt_tty		0
#define	ptcselect	nodev
#define	ptsstop		nulldev
#endif

#include "vbsc.h"
#if NVBSC > 0
int	bscopen(), bscclose(), bscread(), bscwrite(), bscioctl();
int	bsmopen(),bsmclose(),bsmread(),bsmwrite(),bsmioctl();
int	bstopen(),bstclose(),bstread(),bstioctl();
#else
#define bscopen		nodev
#define bscclose	nodev
#define bscread		nodev
#define bscwrite	nodev
#define bscioctl	nodev
#define bsmopen		nodev
#define bsmclose	nodev
#define bsmread		nodev
#define bsmwrite	nodev
#define bsmioctl	nodev
#define bstopen		nodev
#define bstclose	nodev
#define bstread		nodev
#define bstwrite	nodev
#define bstioctl	nodev
#endif

#if NII > 0
int	iiioctl(), iiclose(), iiopen();
#else
#define	iiopen	nodev
#define	iiclose	nodev
#define	iiioctl	nodev
#endif

#include "enp.h"
#if NENP > 0
int	enpr_open(), enpr_close(), enpr_read(), enpr_write(), enpr_ioctl();
#else
#define enpr_open	nodev
#define enpr_close	nodev
#define enpr_read	nodev
#define enpr_write	nodev
#define enpr_ioctl	nodev
#endif

#include "dr.h"
#if NDR > 0
int     dropen(),drclose(),drread(),drwrite(),drioctl(),drreset();
#else
#define dropen nodev
#define drclose nodev
#define drread nodev
#define drwrite nodev
#define drioctl nodev
#define drreset nodev
#endif

#include "ik.h"
#if NIK > 0
int     ikopen(),ikclose(),ikread(),ikwrite(),ikioctl();
#else
#define ikopen nodev
#define ikclose nodev
#define ikread nodev
#define ikwrite nodev
#define ikioctl nodev
#endif

int	logopen(),logclose(),logread(),logioctl(),logselect();

int	ttselect(), seltrue();

struct cdevsw	cdevsw[] =
{
	cnopen,		cnclose,	cnread,		cnwrite,	/*0*/
	cnioctl,	nulldev,	nulldev,	&cons,
	ttselect,	nodev,
	vxopen,		vxclose,	vxread,		vxwrite,	/*1*/
	vxioctl,	vxstop,		vxreset,	vx_tty,
	ttselect,	nodev,
	syopen,		nulldev,	syread,		sywrite,	/*2*/
	syioctl,	nulldev,	nulldev,	0,
	syselect,	nodev,
	nulldev,	nulldev,	mmread,		mmwrite,	/*3*/
	nodev,		nulldev,	nulldev,	0,
	mmselect,	nodev,
	nodev,		nulldev,	nodev,		nodev,		/*4*/
	nodev,		nodev,		nulldev,	0,
	seltrue,	nodev,
	vdopen,		nulldev,	vdread,		vdwrite,	/*5*/
	vdioctl,	nodev,		nulldev,	0,
	seltrue,	nodev,
	nodev,		nulldev,	nodev,		nodev,		/*6*/
	nodev,		nodev,		nulldev,	0,
	seltrue,	nodev,
	cyopen,		cyclose,	cyread,		cywrite,	/*7*/
	cyioctl,	nodev,		cyreset,	0,
	seltrue,	nodev,
	nulldev,	nulldev,	swread,		swwrite,	/*8*/
	nodev,		nodev,		nulldev,	0,
	nodev,		nodev,
	ptsopen,	ptsclose,	ptsread,	ptswrite,	/*9*/
	ptyioctl,	ptsstop,	nodev,		pt_tty,
	ttselect,	nodev,
	ptcopen,	ptcclose,	ptcread,	ptcwrite,	/*10*/
	ptyioctl,	nulldev,	nodev,		pt_tty,
	ptcselect,	nodev,
	bscopen,	bscclose,	bscread,	bscwrite,	/*11*/
	bscioctl,	nodev,		nulldev,	0,
	nodev,		nodev,
	bsmopen,	bsmclose,	bsmread,	bsmwrite,	/*12*/
	bsmioctl,	nodev,		nulldev,	0,
	nodev,		nodev,
	bstopen,	bstclose,	bstread,	nodev,		/*13*/
	bstioctl,	nodev,		nulldev,	0,
	nodev,		nodev,
	iiopen,		iiclose,	nulldev,	nulldev,	/*14*/
	iiioctl,	nulldev,	nulldev,	0,
	seltrue,	nodev,
	logopen,	logclose,	logread,	nodev,		/*15*/
	logioctl,	nodev,		nulldev,	0,
	logselect,	nodev,
	enpr_open,	enpr_close,	enpr_read,	enpr_write,	/*16*/
	enpr_ioctl,	nodev,		nulldev,	0,
	nodev,		nodev,		
	nodev,		nodev,		nodev,		nodev,		/*17*/
	nodev,		nodev,		nulldev,	0,
	nodev,		nodev,
	dropen,		drclose,	drread,		drwrite,	/*18*/
	drioctl,	nodev,		drreset,	0,
	nodev,		nodev,
	nodev,		nodev,		nodev,		nodev,		/*19*/
	nodev,		nodev,		nulldev,	0,
	nodev,		nodev,
/* 20-30 are reserved for local use */
	ikopen,		ikclose,	ikread,		ikwrite,	/*20*/
	ikioctl,	nodev,		nulldev,	0,
	nodev,		nodev,
};
int	nchrdev = sizeof (cdevsw) / sizeof (cdevsw[0]);

int	mem_no = 3; 	/* major device number of memory special file */

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines. Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(4, 0);
