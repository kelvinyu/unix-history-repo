/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getnetbyname.c	5.2 (Berkeley) %G%";
#endif LIBC_SCCS and not lint

#include <netdb.h>

struct netent *
getnetbyname(name)
	register char *name;
{
	register struct netent *p;
	register char **cp;

	setnetent(0);
	while (p = getnetent()) {
		if (strcmp(p->n_name, name) == 0)
			break;
		for (cp = p->n_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	endnetent();
	return (p);
}
