/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)pstatus.c	8.1 (Berkeley) %G%";
#endif /* not lint */

/*
 * process status routines
 */

#include "defs.h"
#include <signal.h>
#include "process.h"
#include "machine.h"
#include "breakpoint.h"
#include "source.h"
#include "object.h"
#include "process.rep"

/*
 * Print the status of the process.
 * This routine does not return.
 */

printstatus()
{
    if (process->signo == SIGINT) {
	isstopped = TRUE;
	printerror();
    }
    if (isbperr() && isstopped) {
	skimsource(srcfilename(pc));
	printf("stopped at ");
	printwhere(curline, cursource);
	putchar('\n');
	if (curline > 0) {
	    printlines(curline, curline);
	} else {
	    printinst(pc, pc);
	}
	erecover();
    } else {
	isstopped = FALSE;
	fixbps();
	fixintr();
	if (process->status == FINISHED) {
	    quit(0);
	} else {
	    printerror();
	}
    }
}


/*
 * Print out the "line N [in file F]" information that accompanies
 * messages in various places.
 */

printwhere(lineno, filename)
LINENO lineno;
char *filename;
{
    if (lineno > 0) {
	printf("line %d", lineno);
	if (nlhdr.nfiles > 1) {
	    printf(" in file %s", filename);
	}
    } else {
	    printf("location %d\n", pc);
    }
}

/*
 * Return TRUE if the process is finished.
 */

BOOLEAN isfinished(p)
PROCESS *p;
{
    return(p->status == FINISHED);
}
