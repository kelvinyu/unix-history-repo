/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 *
 * from: Utah $Hdr: cons.c 1.4 88/12/03$
 *
 *	@(#)cons.c	7.1 (Berkeley) %G%
 */

#include "param.h"
#include "user.h"
#include "systm.h"
#include "buf.h"
#include "ioctl.h"
#include "tty.h"
#include "file.h"
#include "conf.h"

#include "cons.h"

/* XXX - all this could be autoconfig()ed */
#include "ite.h"
#if NITE > 0
int itecnprobe(), itecninit(), itecngetc(), itecnputc();
#endif
#include "dca.h"
#if NDCA > 0
int dcacnprobe(), dcacninit(), dcacngetc(), dcacnputc();
#endif

struct	consdev constab[] = {
#if NITE > 0
	{ itecnprobe,	itecninit,	itecngetc,	itecnputc },
#endif
#if NDCA > 0
	{ dcacnprobe,	dcacninit,	dcacngetc,	dcacnputc },
#endif
	{ 0 },
};
/* end XXX */

extern	struct consdev constab[];

struct	tty *constty = 0;	/* virtual console output device */
struct	consdev *cn_tab;	/* physical console device info */
struct	tty *cn_tty;		/* XXX: console tty struct for tprintf */

cninit()
{
	register struct consdev *cp;

	/*
	 * Collect information about all possible consoles
	 * and find the one with highest priority
	 */
	for (cp = constab; cp->cn_probe; cp++) {
		(*cp->cn_probe)(cp);
		if (cp->cn_pri > CN_DEAD &&
		    (cn_tab == NULL || cp->cn_pri > cn_tab->cn_pri))
			cn_tab = cp;
	}
	/*
	 * No console, we can handle it
	 */
	if ((cp = cn_tab) == NULL)
		return;
	/*
	 * Turn on console
	 */
	cn_tty = cp->cn_tp;
	(*cp->cn_init)(cp);
}

cnopen(dev, flag)
	dev_t dev;
{
	if (cn_tab == NULL)
		return(0);
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_open)(dev, flag));
}
 
cnclose(dev, flag)
	dev_t dev;
{
	if (cn_tab == NULL)
		return(0);
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_close)(dev, flag));
}
 
cnread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	if (cn_tab == NULL)
		return(0);
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_read)(dev, uio));
}
 
cnwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	if (cn_tab == NULL)
		return(0);
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_write)(dev, uio));
}
 
cnioctl(dev, cmd, data, flag)
	dev_t dev;
	caddr_t data;
{
	int error;

	if (cn_tab == NULL)
		return(0);
	/*
	 * Superuser can always use this to wrest control of console
	 * output from the "virtual" console.
	 */
	if (cmd == TIOCCONS && constty) {
		error = suser(u.u_cred, &u.u_acflag);
		if (error)
			return (error);
		constty = NULL;
		return (0);
	}
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_ioctl)(dev, cmd, data, flag));
}

/*ARGSUSED*/
cnselect(dev, rw)
	dev_t dev;
	int rw;
{
	if (cn_tab == NULL)
		return(1);
	return(ttselect(cn_tab->cn_dev, rw));
}

cngetc()
{
	if (cn_tab == NULL)
		return(0);
	return((*cn_tab->cn_getc)(cn_tab->cn_dev));
}

cnputc(c)
	register int c;
{
	if (cn_tab == NULL)
		return;
	if (c) {
		(*cn_tab->cn_putc)(cn_tab->cn_dev, c);
		if (c == '\n')
			(*cn_tab->cn_putc)(cn_tab->cn_dev, '\r');
	}
}
