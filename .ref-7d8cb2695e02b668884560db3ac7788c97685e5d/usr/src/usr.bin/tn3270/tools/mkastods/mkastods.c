/*-
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mkastods.c	4.2 (Berkeley) %G%";
#endif /* not lint */

#include <stdio.h>
#if	defined(unix)
#include <strings.h>
#else	/* defined(unix) */
#include <string.h>
#endif	/* defined(unix) */
#include <ctype.h>
#include "../api/asc_ebc.h"
#include "../api/ebc_disp.h"

int
main()
{
    int i;

    /* For each ascii code, find the display code that matches */

    printf("unsigned char asc_disp[256] = {");
    for (i = 0; i < NASCII; i++) {
	if ((i%8) == 0) {
	    printf("\n");
	}
	printf("\t0x%02x,", ebc_disp[asc_ebc[i]]);
    }
    for (i = sizeof disp_ebc; i < 256; i++) {
	if ((i%8) == 0) {
	    printf("\n");
	}
	printf("\t0x%02x,", 0);
    }
    printf("\n};\n");

    return 0;
}
