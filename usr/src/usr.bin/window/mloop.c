#ifndef lint
static char sccsid[] = "@(#)mloop.c	3.6 %G%";
#endif

#include "defs.h"

mloop()
{
	wwrint();		/* catch typeahead before we set ASYNC */
	while (!quit) {
		if (incmd) {
			docmd();
		} else if (wwcurwin->ww_state != WWS_HASPROC) {
			setcmd(1);
			if (wwpeekc() == escapec)
				(void) wwgetc();
			error("Process died.");
		} else {
			register char *p;
			register n;

			wwiomux();
			if (wwibp < wwibq) {
				for (p = wwibp; p < wwibq && *p != escapec;
				     p++)
					;
				if ((n = p - wwibp) > 0) {
					(void) write(wwcurwin->ww_pty,
						wwibp, n);
					wwibp = p;
				}
				if (wwpeekc() == escapec) {
					(void) wwgetc();
					setcmd(1);
				}
			}
		}
	}
}
