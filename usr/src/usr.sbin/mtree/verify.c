/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)verify.c	5.2 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fts.h>
#include <errno.h>
#include <stdio.h>
#include "mtree.h"

extern ENTRY *root;

static char path[MAXPATHLEN];

verify()
{
	vwalk();
	miss(root, path);
}

vwalk()
{
	extern int ftsoptions, dflag, eflag, rflag;
	register FTS *t;
	register FTSENT *p;
	register ENTRY *ep, *level;

	if (!(t = ftsopen(".", ftsoptions, (int (*)())NULL))) {
		(void)fprintf(stderr,
		    "mtree: ftsopen: %s.\n", strerror(errno));
		exit(1);
	}
	level = root;
	while (p = ftsread(t)) {
		switch(p->fts_info) {
		case FTS_D:
			if (!strcmp(p->fts_name, "."))
				continue;
			break;
		case FTS_DC:
			(void)fprintf(stderr,
			    "mtree: directory cycle: %s.\n", RP(p));
			continue;
		case FTS_DNR:
			(void)fprintf(stderr,
			    "mtree: %s: unable to read.\n", RP(p));
			continue;
		case FTS_DNX:
			(void)fprintf(stderr,
			    "mtree: %s: unable to search.\n", RP(p));
			continue;
		case FTS_DP:
			for (level = level->parent; level->prev;
			    level = level->prev);
			continue;
		case FTS_ERR:
			(void)fprintf(stderr, "mtree: %s: %s.\n",
			    RP(p), strerror(errno));
			continue;
		case FTS_NS:
			(void)fprintf(stderr,
			    "mtree: can't stat: %s.\n", RP(p));
			continue;
		default:
			if (dflag)
				continue;
		}

		for (ep = level; ep; ep = ep->next)
			if (!strcmp(ep->name, p->fts_name)) {
				ep->flags |= F_VISIT;
				if (ep->info.flags&F_IGN) {
					(void)ftsset(t, p, FTS_SKIP);
					continue;
				}
				compare(ep->name, &ep->info, p);
				if (ep->child && ep->info.type == F_DIR &&
				    p->fts_info == FTS_D)
					level = ep->child;
				break;
			}
		if (ep)
			continue;
		if (!eflag) {
			(void)printf("extra: %s", RP(p));
			if (rflag) {
				if (unlink(p->fts_accpath)) {
					(void)printf(", not removed: %s",
					    strerror(errno));
				} else
					(void)printf(", removed");
			}
			(void)putchar('\n');
		}
		(void)ftsset(t, p, FTS_SKIP);
	}
}

miss(p, tail)
	register ENTRY *p;
	register char *tail;
{
	extern int dflag, uflag;
	register int create;
	register char *tp;

	for (; p; p = p->next) {
		if (p->info.type != F_DIR && (dflag || p->flags&F_VISIT))
			continue;
		(void)strcpy(tail, p->name);
		if (!(p->flags&F_VISIT))
			(void)printf("missing: %s", path);
		if (p->info.type != F_DIR) {
			putchar('\n');
			continue;
		}

		create = 0;
		if (!(p->flags&F_VISIT) && uflag)
#define	MINBITS	(F_GROUP|F_MODE|F_OWNER)
			if ((p->info.flags & MINBITS) != MINBITS)
				(void)printf(" (not created -- group, mode or owner not specified)");
			else if (mkdir(path, S_IRWXU))
				(void)printf(" (not created: %s)",
				    strerror(errno));
			else {
				create = 1;
				(void)printf(" (created)");
			}

		if (!(p->flags&F_VISIT))
			(void)putchar('\n');

		for (tp = tail; *tp; ++tp);
		*tp = '/';
		miss(p->child, tp + 1);
		*tp = '\0';

		if (!create)
			continue;
		if (chown(path, p->info.st_uid, p->info.st_gid)) {
			(void)printf("%s: owner/group/mode not modified: %s\n",
			    path, strerror(errno));
			continue;
		}
		if (chmod(path, p->info.st_mode))
			(void)printf("%s: permissions not set: %s\n",
			    path, strerror(errno));
	}
}
