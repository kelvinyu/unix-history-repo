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
static char sccsid[] = "@(#)u.c	5.3 (Berkeley) %G%";
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
 * This restores the buffer to the state it was in just before the
 * last buffer modifying command - the global commands (with command
 * list) are looked at as one buffer modifying command. Note: this
 * just manipulates the undo stack (u_stk); x-ref u_add_stk(),
 * u_clr_stk(), d_add(), and d_do().
 */
void
u(inputt, errnum)
	FILE *inputt;
	int *errnum;
{
	if (rol(inputt, errnum))
		return;

	if (u_stk == NULL) {
		*errnum = 1;
		return;
	}
	undo();
	*errnum = 1;
}


/* This function does the "real work" of the undo. */
void
undo()
{
	LINE *l_current, *l_bottom, *l_top;
	struct u_layer *l_old_u_stk, *l_temp;

	sigspecial++;
	/* This is done because undo can be undone. */
	l_current = u_current;
	l_top = u_top;
	l_bottom = u_bottom;

	u_current = current;
	u_top = top;
	u_bottom = bottom;

	l_old_u_stk = u_stk;
	u_stk = NULL;

	while (l_old_u_stk != NULL) {
		u_add_stk(l_old_u_stk->cell);
		(*(l_old_u_stk->cell)) = (l_old_u_stk->val);
		l_temp = l_old_u_stk;
		l_old_u_stk = l_old_u_stk->below;
		free(l_temp);
	}

	current = l_current;
	top = l_top;
	bottom = l_bottom;
	sigspecial--;
	if (sigint_flag && (!sigspecial))
		SIGINT_ACTION;
}

/*
 * This function should be called before u_add_stk is in each command
 * function, _except_ when the global flag is high (>0) -- otherwise,
 * we couldn't undo all of the global commands, only the last iteration
 * of the last command -- and the u command.
 * This is where we begin to dispose of ed's undo knowledge of a line.
 * The call to d_do() gets rid of the rest.
 */
void
u_clr_stk()
{
	register struct u_layer *l_temp;

	u_current = current;
	u_top = top;
	u_bottom = bottom;

	sigspecial++;
	/* Only if there is something to delete in the buffer. */
	if ((u_stk) && (d_stk))
		d_do();

	while (u_stk != NULL) {
		l_temp = u_stk;
		u_stk = u_stk->below;
		free(l_temp);
	}
	u_stk = NULL;		/* Just to sure. */
	sigspecial--;
	if (sigint_flag && (!sigspecial))
		SIGINT_ACTION;
}

/*
 * Place the addresses of and the pointer values of the LINE structures
 * that are being changed on the undo stack.  This is a quick, simple,
 * and effective way to preserve what could be be brought back on request
 * without keeping a copy of every bleep'n thing.
 */
void
u_add_stk(in)
	LINE **in;
{
	register struct u_layer *l_now;

	if (in == NULL)
		return;
	l_now = malloc(sizeof(struct u_layer));
	if (l_now == NULL) {
		strcpy(help_msg, "undo: out of memory error");
		return;
	}
	sigspecial++;
	if (u_stk == NULL)
		(l_now->below) = NULL;
	else
		(l_now->below) = u_stk;
	u_stk = l_now;
	(u_stk->cell) = in;
	(u_stk->val) = (*(u_stk->cell));
	sigspecial--;
	if (sigint_flag && (!sigspecial))
		SIGINT_ACTION;
}
