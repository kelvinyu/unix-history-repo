/*	kdb_pcs.c	7.1	86/11/20	*/

#include "../kdb/defs.h"

char	*NOBKPT;
char	*SZBKPT;
char	*EXBKPT;
char	*BADMOD;

/* breakpoints */
BKPTR	bkpthead;

char	*lp;
char	lastc;

long	loopcnt;

/* sub process control */

subpcs(modif)
{
	register check, runmode;
	register BKPTR bkptr;
	register char *comptr;

	loopcnt=cntval;
	switch (modif) {

		/* delete breakpoint */
	case 'd': case 'D':
		if (bkptr=scanbkpt(dot)) {
			bkptr->flag=0;
			return;
		}
		error(NOBKPT);

		/* set breakpoint */
	case 'b': case 'B':
		if (bkptr=scanbkpt(dot))
			bkptr->flag=0;
		for (bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt)
			if (bkptr->flag == 0)
				break;
		if (bkptr==0) {
			bkptr=(BKPTR)kdbmalloc(sizeof *bkptr);
			if (bkptr == (BKPTR)-1)
				error(SZBKPT);
			bkptr->nxtbkpt=bkpthead;
			bkpthead=bkptr;
		}
		bkptr->loc = dot;
		bkptr->initcnt = bkptr->count = cntval;
		bkptr->flag = BKPTSET;
		check=MAXCOM-1; comptr=bkptr->comm; rdc(); lp--;
		do
			*comptr++ = readchar();
		while (check-- && lastc!=EOR);
		*comptr=0; lp--;
		if (check)
			return;
		error(EXBKPT);

		/* single step */
	case 's': case 'S':
		runmode=SINGLE;
		break;

		/* continue */
	case 'c': case 'C':
		runmode=CONTIN;
		break;

	default:
		error(BADMOD);
	}
	if (loopcnt>0)
		runpcs(runmode, 0);
}
