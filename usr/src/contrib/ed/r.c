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
static char sccsid[] = "@(#)r.c	5.7 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>

#include <limits.h>
#include <a.out.h>
#include <errno.h>
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
 * This sets up things for the central input routine to place the
 * incoming text at the proper place in the buffer.
 */
void
r(inputt, errnum)
	FILE *inputt;
	int *errnum;
{
	FILE *l_fp;
	long l_num;
	int l_bang_flag=0;
	char *l_filename_read=NULL, *l_temp=NULL;

	if (filename_flag == 1) {
		sigspecial++;
		l_filename_read = filename_current;
		l_temp = filename_current;
		sigspecial--;
		if (sigint_flag && (!sigspecial))
			SIGINT_ACTION;
	} else {
		l_temp = filename(inputt, errnum);
		if (*errnum == 1)
			l_filename_read = l_temp;
		else
			if (*errnum == -2) {
				while (((ss = getc(inputt)) != '\n') ||
				    (ss == EOF));
				l_filename_read = filename_current;
			} else
				if (*errnum < 0) {
					filename_flag = 0;
					return;
				}
		*errnum = 0;
	}

	if (filename_current == NULL) {
		if (l_filename_read == NULL) {
			strcpy(help_msg, "no filename given");
			*errnum = -1;
			filename_flag = 0;
			if (ss)
				ungetc('\n', inputt);
			return;
		} else
			filename_current = l_filename_read;
	}

	/*
	 * Determine if the file can be read.  If not set the help message to
	 * something descriptive that the user should understand.
	 * We're now allowing ed to read directory and executable files
	 * for as much as it can, if the last character in the file
	 * isn't a '\n' then one will be added and a warning given - the
	 * user (for now) has to figure out how to remove it if they want.
	 * Ed accepts the NUL character now.
	 */
	if (l_temp && l_temp[FILENAME_LEN+1]) { /* bang flag */
		FILE *popen();

		if (l_temp[0] == '\0') {
			strcpy(help_msg, "no command given");
			*errnum = -1;
			return;
		}
		if ((l_fp = popen(l_temp, "r")) == NULL) {
			strcpy(help_msg, "error executing command");
			*errnum = -1;
			filename_flag = 0;
			if (l_fp != NULL)
				pclose(l_fp);
			return;
		}
		if (filename_flag == 1)
			filename_current = NULL;
		l_bang_flag = 1;
	}
	else if ((l_fp = fopen(l_filename_read, "r")) == NULL) {
		strcpy(help_msg, "permission lacking to read file");
		printf("?%s\n", l_filename_read);
		filename_flag = 0;
		*errnum = 0;
		return;
	}
	filename_flag = 0;
	if (!l_bang_flag)
		fseek(l_fp, (off_t)0, 0);
	if (g_flag == 0)
		u_clr_stk();
	l_num = input_lines(l_fp, errnum);
	if (*errnum < 0)
		return;
	*errnum = 0;

	if (explain_flag > 0)
		printf("%ld\n", l_num);
	if (l_filename_read != filename_current)
		free(l_filename_read);

	if (l_bang_flag)
		pclose(l_fp);
	else
		fclose(l_fp);
	change_flag = 1;
	if (sigint_flag)
		SIGINT_ACTION;
	*errnum = 1;
}
