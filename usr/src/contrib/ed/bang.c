/*-
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rodney Ruddock of the University of Guelph.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)bang.c	5.5 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>

#include <limits.h>
#include <regex.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DBI
#include <db.h>
#endif

#include "ed.h"
#include "extern.h"

/*
 * Execute a command in sh (and always sh). For those wondering the
 * proper name for '!' _is_ bang.
 */

void
bang(inputt, errnum)
	FILE *inputt;
	int *errnum;
{
	static int l_cnt_last_pos;		/* "!!", l_shellcmd offset */
	static char l_shellcmd[FILENAME_LEN];	/* "!!" */
	int l_cnt = 0, l_esc = 0;

	for (;;) {
		ss = getchar();
		if ((ss == '\\') && (l_esc == 0)) {
			ss = getchar();
			l_esc = 1;
		} else
			l_esc = 0;
		if ((ss == '\n') || (ss == EOF)) {
			if (l_cnt == 0) {
				strcpy(help_msg, "no shell command given");
				*errnum = -1;
				ungetc('\n', inputt);
				return;
			}
			l_shellcmd[l_cnt] = '\0';
			break;
		} else
			if ((ss == '!') && (l_esc == 0)) {
				l_cnt = l_cnt_last_pos;
				printf("%s\n", l_shellcmd);
			}
			else
				if ((ss == '%') && (l_esc == 0)) {
					l_shellcmd[l_cnt] = '\0';
					if (filename_current) {
						strcat(l_shellcmd,
						    filename_current);
						l_cnt =
						    l_cnt +
						    strlen(filename_current);
					}
				} else
					l_shellcmd[l_cnt++] = ss;
		if (l_cnt >= FILENAME_LEN) {
			strcpy(help_msg, "shell command too long");
			*errnum = -1;
			ungetc('\n', inputt);
			return;
		}
	}

	system(l_shellcmd);
	if (explain_flag > 0)	/* for the -s option */
		printf("!\n");
	l_cnt_last_pos = l_cnt;
	*errnum = 0;
}
