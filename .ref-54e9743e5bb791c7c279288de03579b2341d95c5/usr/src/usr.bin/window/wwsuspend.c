/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Edward Wang at The University of California, Berkeley.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)wwsuspend.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include "ww.h"
#include "tt.h"
#include <sys/signal.h>

wwsuspend()
{
	sig_t oldsig;

	oldsig = signal(SIGTSTP, SIG_IGN);
	wwend(0);
	(void) signal(SIGTSTP, SIG_DFL);
	(void) kill(0, SIGTSTP);
	(void) signal(SIGTSTP, SIG_IGN);
	wwstart();
	(void) signal(SIGTSTP, oldsig);
}
