/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)print.c	5.8 (Berkeley) %G%";
#endif /* not lint */

/* debug print routines */

#include <sys/types.h>
#include <sys/socket.h>
#include <protocols/talkd.h>
#include <syslog.h>
#include <stdio.h>

static	char *types[] =
    { "leave_invite", "look_up", "delete", "announce" };
#define	NTYPES	(sizeof (types) / sizeof (types[0]))
static	char *answers[] = 
    { "success", "not_here", "failed", "machine_unknown", "permission_denied",
      "unknown_request", "badversion", "badaddr", "badctladdr" };
#define	NANSWERS	(sizeof (answers) / sizeof (answers[0]))

print_request(cp, mp)
	char *cp;
	register CTL_MSG *mp;
{
	char tbuf[80], *tp;
	
	if (mp->type > NTYPES) {
		(void)sprintf(tbuf, "type %d", mp->type);
		tp = tbuf;
	} else
		tp = types[mp->type];
	syslog(LOG_DEBUG, "%s: %s: id %d, l_user %s, r_user %s, r_tty %s",
	    cp, tp, mp->id_num, mp->l_name, mp->r_name, mp->r_tty);
}

print_response(cp, rp)
	char *cp;
	register CTL_RESPONSE *rp;
{
	char tbuf[80], *tp, abuf[80], *ap;
	
	if (rp->type > NTYPES) {
		(void)sprintf(tbuf, "type %d", rp->type);
		tp = tbuf;
	} else
		tp = types[rp->type];
	if (rp->answer > NANSWERS) {
		(void)sprintf(abuf, "answer %d", rp->answer);
		ap = abuf;
	} else
		ap = answers[rp->answer];
	syslog(LOG_DEBUG, "%s: %s: %s, id %d", cp, tp, ap, ntohl(rp->id_num));
}
