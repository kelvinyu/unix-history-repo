/*	hpmaptype.c	4.1	83/02/18	*/

/*
 * RP??/RM?? drive type mapping routine.
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/inode.h"
#include "../h/fs.h"

#include "../vaxmba/hpreg.h"
#include "../vaxmba/mbareg.h"

#include "saio.h"
#include "savax.h"

/* THIS SHOULD BE READ IN OFF THE PACK, PER DRIVE */
short	hp6_off[8] =	{ 0, 38, 0, -1, -1, -1, 118, -1 };
short	rm3_off[8] =	{ 0, 100, 0, -1, -1, -1, 310, -1 };
short	rm5_off[8] =	{ 0, 27, 0, 562, 589, 681, 562, 82 };
short	rm80_off[8] =	{ 0, 37, 0, -1, -1, -1, 115, 305 };
short	hp7_off[8] = 	{ 0, 10, 0, 330, 340, 500, 330, 50 };
short	ml_off[8] =	{ 0, -1, -1, -1, -1, -1, -1, -1 };
short	si9775_off[8] =	{ 0, 13, 0, -1, -1, -1, 40, 441 };
short	si9730_off[8] = { 0, 50, 0, -1, -1, -1, -1, 155 };
short	hpam_off[8] =	{ 0, 32, 0, 668, 723, 778, 668, 98 };
short	hpfj_off[8] =	{ 0, 19, 0, -1, -1, -1, 398, 59 };
/* END SHOULD BE READ IN */

struct st hpst[] = {
	32,	5,	32*5,	823,	rm3_off,	/* RM03 */
	32,	19,	32*19,	823,	rm5_off,	/* RM05 */
	22,	19,	22*19,	815,	hp6_off,	/* RP06 */
	31,	14, 	31*14,	559,	rm80_off,	/* RM80 */
	22,	19,	22*19,	411,	hp6_off,	/* RP06 */
	50,	32,	50*32,	630,	hp7_off,	/* RP07 */
	1,	1,	1,	1,	ml_off,		/* ML11A */
	1,	1,	1,	1,	ml_off,		/* ML11B */
	32,	40,	32*40,	843,	si9775_off,	/* 9775 */
	32,	10,	32*10,	823,	si9730_off,	/* 9730 */
	32,	16,	32*16,	1024,	hpam_off,	/* Ampex capricorn */
	43,	20,	43*20,	842,	hpfj_off,	/* Fuji Eagle */
	48,	20,	48*20,	842,	hpfj_off,	/* 48 sector Eagle */
	1,	1,	1,	1,	0,		/* rm02 - not used */
	32,	19,	32*19,	815,	rm5_off,	/* Ampex 9300 */

};

#define	MASKREG(reg)	((reg)&0xffff)

hpmaptype(hpaddr, type, unit)
	register struct hpdevice *hpaddr;
	unsigned type;
	int unit;
{
	int ntracks, hpsn;

printf("hpmaptype: type=%d\n", type);
	/*
	 * Handle SI model byte stuff when
	 * we think it's an RM03 or RM05.
	 */
	if (type == 0 || type == 1) {
		hpsn = hpaddr->hpsn;
printf("hpsn=%x\n", hpsn&0xffff);
		if ((hpsn & SIMB_LU) != unit)
			return (type);
		switch ((hpsn & SIMB_MB) &~ (SIMB_S6|SIRM03|SIRM05)) {

		case SI9775D:
			return (8);

		case SI9730D:
			return (9);

		/*
		 * Beware, since the only have SI controller we
		 * have has a 9300 instead of a 9766, we map the
		 * drive type into the 9300.  This means that
		 * on a 9766 you lose the last 8 cylinders (argh).
		 */
		case SI9766:
			return (14);

		case SI9762:
			return (0);

		case SICAPD:
			return (10);

		case SI9751D:
			return (12);
		}
		return (type);
	}
	/*
	 * RM03: EMULEX controller.  Map to correct
	 * drive type by checking the holding
	 * register for the disk geometry.
	 */
	if (type == 13) {
		hpaddr->hpcs1 = HP_NOP;
		hpaddr->hphr = HPHR_MAXTRAK;
		ntracks = MASKREG(hpaddr->hphr) + 1;
		if (ntracks == 16)
			return (10);	/* AMPEX capricorn */
		hpaddr->hphr = HPHR_MAXSECT;
		ntracks = MASKREG(hpaddr->hphr) + 1;
		if (ntracks == 48)
			return (12);	/* 48 sector Eagle */
		if (ntracks == 43)
			return (11);	/* 43 sector Eagle */
		printf("RM02 with %d sectors/track?\n", ntracks);
		return (type);
	}
	/*
	 * ML11's all map to the same type.
	 */
	if (type == 6 || type == 7)
		return (6);
	return (type);
}
