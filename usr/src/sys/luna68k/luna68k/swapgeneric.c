/*
 * Copyright (c) 1992 OMRON Corporation.
 * Copyright (c) 1982, 1986, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 * from: hp300/hp300/swapgeneric.c	7.8 (Berkeley) 10/11/92
 *
 *	@(#)swapgeneric.c	8.1 (Berkeley) %G%
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/systm.h>
#include <sys/reboot.h>

#include <luna68k/dev/device.h>

/*
 * Generic configuration;  all in one
 */

dev_t	rootdev = NODEV;
dev_t	argdev  = NODEV;
dev_t	dumpdev = NODEV;

int	nswap;

struct	swdevt swdevt[] = {
	{ -1,	1,	0 },
	{ NODEV,0,	0 },
};

int	dmmin, dmmax, dmtext;

extern	struct driver sddriver;
extern struct hp_ctlr hp_cinit[];
extern struct hp_device hp_dinit[];

struct	genericconf {
	caddr_t	gc_driver;
	char	*gc_name;
	dev_t	gc_root;
} genericconf[] = {
	{ (caddr_t)&sddriver,	"sd",	makedev(4, 0) },
	{ 0 },
};

setconf()
{
	register struct hp_ctlr *hc;
	register struct hp_device *hd;
	register struct genericconf *gc;
	register char *cp;
	int unit, part, swaponroot = 0;

	if (rootdev != NODEV)
		goto doswap;
	unit = 0;
	part = 0;
	if (boothowto & RB_ASKNAME) {
		char name[128];
retry:
		printf("root device? ");
		gets(name);
		for (gc = genericconf; gc->gc_driver; gc++)
			if (gc->gc_name[0] == name[0] &&
			    gc->gc_name[1] == name[1])
				goto gotit;
		printf("use one of:");
		for (gc = genericconf; gc->gc_driver; gc++)
			printf(" %s%%d", gc->gc_name);
		printf("\n");
		goto retry;
gotit:
		cp = &name[2];
		if (*cp < '0' || *cp > '9') {
			printf("bad/missing unit number\n");
			goto retry;
		}
		while (*cp >= '0' && *cp <= '9')
			unit = 10 * unit + *cp++ - '0';
		if (*cp < 'a' || *cp > 'h') {
			printf("bad/missing partiiton number\n");
			goto retry;
		}
		part = *cp++ - 'a';
/*
		if (*cp == '*')
			swaponroot++;
 */
		goto found;
	}

	for (gc = genericconf; gc->gc_driver; gc++) {
		for (hd = hp_dinit; hd->hp_driver; hd++) {
			if (hd->hp_alive == 0)
				continue;
			if (hd->hp_unit == 0 && hd->hp_driver ==
			    (struct driver *)gc->gc_driver) {
				printf("root on %s0\n", hd->hp_driver->d_name);
				goto found;
			}
		}
	}
	printf("no suitable root\n");
	asm("stop #0x2700");
found:
	gc->gc_root = makedev(major(gc->gc_root), (unit * 8) + part );
	rootdev = gc->gc_root;
/*
	printf("using root device: %s%d%c\n", gc->gc_name, unit, 'a' + part);
 */
doswap:
	swdevt[0].sw_dev = argdev = dumpdev =
	    makedev(major(rootdev), (minor(rootdev) & ~0x7) + 1);
/*
	printf("using swap device: %s%d%c\n",
	       gc->gc_name, unit, 'a' + (minor(swdevt[0].sw_dev) & 0x7));
 */
	/* swap size and dumplo set during autoconfigure */
	if (swaponroot)
		rootdev = dumpdev;
}

gets(cp)
	char *cp;
{
	register char *lp;
	register c;

	lp = cp;
	for (;;) {
		cnputc(c = cngetc());
		switch (c) {
		case '\n':
		case '\r':
			*lp++ = '\0';
			return;
		case '\b':
		case '\177':
			if (lp > cp) {
				lp--;
				cnputc(' ');
				cnputc('\b');
			}
			continue;
		case '#':
			lp--;
			if (lp < cp)
				lp = cp;
			continue;
		case '@':
		case 'u'&037:
			lp = cp;
			cnputc('\n');
			continue;
		default:
			*lp++ = c;
		}
	}
}
