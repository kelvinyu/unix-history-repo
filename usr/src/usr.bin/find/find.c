/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Cimarron D. Taylor of the University of California, Berkeley.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1990 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)find.c	4.36 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <time.h>
#include <fts.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "find.h"

FTS *tree;			/* pointer to top of FTS hierarchy */
time_t now;			/* time find was run */
				/* options for the ftsopen(3) call */
int ftsoptions = FTS_NOSTAT|FTS_PHYSICAL;
int isdeprecated;		/* using deprecated syntax */
int isdepth;			/* do directories on post-order visit */
int isoutput;			/* user specified output operator */
int isrelative;			/* can do -exec/ok on relative path */

main(argc, argv)
	int argc;
	char **argv;
{
	PLAN *plan;
	char **p, **paths;
	PLAN *find_formplan();
	time_t time();
	void newsyntax(), oldsyntax();
    
	(void)time(&now);			/* initialize the time-of-day */

	if (argc < 2)
		usage();

	paths = argv;

	/*
	 * if arguments start with an option, treat it like new syntax;
	 * otherwise, if has a "-option" anywhere (which isn't an argument
	 * to another command) treat it as old syntax.
	 */
	if (argv[1][0] != '-')
		for (p = argv + 1; *p; ++p) {
			if (!strcmp(*p, "exec") || !strcmp(*p, "ok")) {
				while (p[1] && strcmp(*++p, ";"));
				continue;
			}
			if (**p == '-') {
				isdeprecated = 1;
				oldsyntax(&argv);
				break;
			}
		}
	if (!isdeprecated)
		newsyntax(argc, &argv);
    
	plan = find_formplan(argv);		/* execution plan */
	find_execute(plan, paths);
}

/*
 * find_formplan --
 *	process the command line and create a "plan" corresponding to the
 *	command arguments.
 */
PLAN *
find_formplan(argv)
	char **argv;
{
	PLAN *plan, *tail, *new;
	PLAN *c_print(), *find_create(), *not_squish(), *or_squish();
	PLAN *paren_squish();

	/*
	 * for each argument in the command line, determine what kind of node
	 * it is, create the appropriate node type and add the new plan node
	 * to the end of the existing plan.  The resulting plan is a linked
	 * list of plan nodes.  For example, the string:
	 *
	 *	% find . -name foo -newer bar -print
	 *
	 * results in the plan:
	 *
	 *	[-name foo]--> [-newer bar]--> [-print]
	 *
	 * in this diagram, `[-name foo]' represents the plan node generated
	 * by c_name() with an argument of foo and `-->' represents the
	 * plan->next pointer.
	 */
	for (plan = NULL; *argv;) {
		if (!(new = find_create(&argv)))
			continue;
		if (plan == NULL)
			tail = plan = new;
		else {
			tail->next = new;
			tail = new;
		}
	}
    
	/*
	 * if the user didn't specify one of -print, -ok or -exec, then -print
	 * is assumed so we add a -print node on the end.  It is possible that
	 * the user might want the -print someplace else on the command line,
	 * but there's no way to know that.
	 */
	if (!isoutput) {
		new = c_print();
		if (plan == NULL)
			tail = plan = new;
		else {
			tail->next = new;
			tail = new;
		}
	}
    
	/*
	 * the command line has been completely processed into a search plan
	 * except for the (, ), !, and -o operators.  Rearrange the plan so
	 * that the portions of the plan which are affected by the operators
	 * are moved into operator nodes themselves.  For example:
	 *
	 *	[!]--> [-name foo]--> [-print]
	 *
	 * becomes
	 *
	 *	[! [-name foo] ]--> [-print]
	 *
	 * and
	 *
	 *	[(]--> [-depth]--> [-name foo]--> [)]--> [-print]
	 *
	 * becomes
	 *
	 *	[expr [-depth]-->[-name foo] ]--> [-print]
	 *
	 * operators are handled in order of precedence.
	 */

	plan = paren_squish(plan);		/* ()'s */
	plan = not_squish(plan);		/* !'s */
	plan = or_squish(plan);			/* -o's */
	return(plan);
}
 
/*
 * find_execute --
 *	take a search plan and an array of search paths and executes the plan
 *	over all FTSENT's returned for the given search paths.
 */
find_execute(plan, paths)
	PLAN *plan;		/* search plan */
	char **paths;		/* array of pathnames to traverse */
{
	register FTSENT *entry;
	PLAN *p;
    
	if (!(tree = fts_open(paths, ftsoptions, (int (*)())NULL))) {
		error("ftsopen", errno);
		exit(1);
	}

	while (entry = fts_read(tree)) {
		switch(entry->fts_info) {
		case FTS_D:
			if (isdepth)
				continue;
			break;
		case FTS_DP:
			if (!isdepth)
				continue;
			break;
		case FTS_DNR:
		case FTS_ERR:
		case FTS_NS:
			error(entry->fts_path, errno);
			continue;
		}

		/*
		 * call all the functions in the execution plan until one is
		 * false or all have been executed.  This is where we do all
		 * the work specified by the user on the command line.
		 */
		for (p = plan; p && (p->eval)(p, entry); p = p->next);
	}
	(void)fts_close(tree);
}
