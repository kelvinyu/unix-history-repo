/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)pdma.h	8.1 (Berkeley) %G%
 */

struct pdma {
	void	*p_addr;
	char	*p_mem;
	char	*p_end;
	int	p_arg;
	void	(*p_fcn)();
};
