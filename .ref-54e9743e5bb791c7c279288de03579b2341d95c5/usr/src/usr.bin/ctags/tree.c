/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)tree.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctags.h"

/*
 * pfnote --
 *	enter a new node in the tree
 */
pfnote(name,ln)
	char	*name;
	int	ln;
{
	extern NODE	*head;		/* head of the sorted binary tree */
	extern char	*curfile;	/* current input file name */
	register NODE	*np;
	register char	*fp;
	char	nbuf[MAXTOKEN];

	/*NOSTRICT*/
	if (!(np = (NODE *)malloc(sizeof(NODE)))) {
		fputs("ctags: too many entries to sort\n",stderr);
		put_entries(head);
		free_tree(head);
		/*NOSTRICT*/
		if (!(head = np = (NODE *)malloc(sizeof(NODE)))) {
			fputs("ctags: out of space.\n",stderr);
			exit(1);
		}
	}
	if (!xflag && !strcmp(name,"main")) {
		if (!(fp = rindex(curfile,'/')))
			fp = curfile;
		else
			++fp;
		(void)sprintf(nbuf,"M%s",fp);
		fp = rindex(nbuf,'.');
		if (fp && !fp[2])
			*fp = EOS;
		name = nbuf;
	}
	if (!(np->entry = strdup(name))) {
		(void)fprintf(stderr, "ctags: %s\n", strerror(errno));
		exit(1);
	}
	np->file = curfile;
	np->lno = ln;
	np->left = np->right = 0;
	if (!(np->pat = strdup(lbuf))) {
		(void)fprintf(stderr, "ctags: %s\n", strerror(errno));
		exit(1);
	}
	if (!head)
		head = np;
	else
		add_node(np,head);
}

add_node(node,cur_node)
	register NODE	*node,
			*cur_node;
{
	extern int	wflag;			/* -w: suppress warnings */
	register int	dif;

	dif = strcmp(node->entry,cur_node->entry);
	if (!dif) {
		if (node->file == cur_node->file) {
			if (!wflag)
				fprintf(stderr,"Duplicate entry in file %s, line %d: %s\nSecond entry ignored\n",node->file,lineno,node->entry);
			return;
		}
		if (!cur_node->been_warned)
			if (!wflag)
				fprintf(stderr,"Duplicate entry in files %s and %s: %s (Warning only)\n",node->file,cur_node->file,node->entry);
		cur_node->been_warned = YES;
	}
	else if (dif < 0)
		if (cur_node->left)
			add_node(node,cur_node->left);
		else
			cur_node->left = node;
	else if (cur_node->right)
		add_node(node,cur_node->right);
	else
		cur_node->right = node;
}

free_tree(node)
	register NODE	*node;
{
	while (node) {
		if (node->right)
			free_tree(node->right);
		free(node);
		node = node->left;
	}
}
