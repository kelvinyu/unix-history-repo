/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)extern.h	5.1 (Berkeley) %G%
 */

typedef struct {
	char *p_end;			/* pointer to NULL at end of path */
	char p_path[MAXPATHLEN + 1];	/* pointer to the start of a path */
} PATH_T;

extern char *progname;			/* program name */

#include <sys/cdefs.h>

__BEGIN_DECLS
int	 path_set(PATH_T *, char *);
char	*path_append(PATH_T *, char *, int);
char	*path_basename(PATH_T *);
void	 path_restore(PATH_T *, char *);
__END_DECLS
