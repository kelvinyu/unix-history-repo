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
static char sccsid[] = "@(#)input_lines.c	5.6 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>

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
 * This central function gets text from some file, which can (and most
 * oft is) stdin. This flexability allows any text inputing function
 * to use it.
 */
long
input_lines(fp, errnum)
	FILE *fp;
	int *errnum;
{
	register long l_nn = 0;
	register int l_ss = ss;
	register char *l_text = text;
	LINE *l_temp_line, *l_temp1;
	long l_ttl = 0;
	int l_nn_max = nn_max, l_jmp_flag;
	char *l_text2;

	if (End_default)
		start = current;
	else
		start = End;
	start_default = End_default = 0;

	sigspecial++;
	/* start == NULL means line 0 which is legal for this function only. */
	nn_max_end = l_temp_line = start;
	if (start == NULL) {
		l_temp1 = top;
		u_add_stk(&(top->above));
	} else {
		u_add_stk(&(start->below));
		l_temp1 = start->below;
	}
	sigspecial--;
	if (sigint_flag && (!sigspecial))
		SIGINT_ACTION;

	sigspecial++;
        if (l_jmp_flag = setjmp(ctrl_position3))
                goto point;

	for (;;) {
		if (sigint_flag && (!sigspecial))
			goto point;
        	sigspecial3 = 1;
		l_ss = getc(fp);
        	sigspecial3 = 0;
		if (l_ss == EOF) {
			if (add_flag) {
				l_text[l_nn++] = '\0';
				goto eof_mk;
			}
			break;
		}
		if (!l_ss) /* 8-bit okay, but NULL not */
			continue;
		l_text[l_nn++] = (char)l_ss;
		if (l_ss == '\n') {
			if (sigint_flag && (!sigspecial))
				goto point;
			l_text[l_nn - 1] = '\0';
			if ((l_nn == 2) && (l_text[0] == '.') && add_flag)
				break;
eof_mk:
			nn_max_end = (LINE *)malloc(sizeof(LINE));
			if (nn_max_end == NULL) {
				*errnum = -1;
				strcpy(help_msg, "out of memory error");
				return (0L);
			}
			(nn_max_end->len) = l_nn - 1;
			(nn_max_end->handle) = add_line(l_text, l_nn - 1);
			(nn_max_end->above) = l_temp_line;
			(nn_max_end->below) = NULL;
			if (l_temp_line)
				(l_temp_line->below) = nn_max_end;
			else
				top = nn_max_end;
			l_temp_line = nn_max_end;

			l_ttl += l_nn;
			l_nn = 0;
			if (l_ss == EOF)
				break;
		} else
			if (l_nn > l_nn_max) {
				l_nn_max += 512;
				nn_max = l_nn_max;
				l_text2 = l_text;
				l_text = text =
				    calloc(l_nn_max + 2, sizeof(char));
				if (text == NULL) {
					*errnum = -1;
					strcpy(help_msg, "out of memory error");
					return (0L);
				}
				bcopy(l_text2, l_text, l_nn);
				free(l_text2);
			}
	}

point:	current = nn_max_end;
	if (current == NULL)
		current = top;
	if (l_temp1 == NULL)
		bottom = nn_max_end;
	else
		if (nn_max_end != NULL) {
			(nn_max_end->below) = l_temp1;
			u_add_stk(&(l_temp1->above));
			(l_temp1->above) = nn_max_end;
		}
	change_flag = 1;
	sigspecial--;
	sigspecial3 = 0;
	if (sigint_flag && (!sigspecial))
		SIGINT_ACTION;
	*errnum = 1;
	ss = l_ss;
	return (l_ttl);
}
