/*
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)pdma.h	7.1 (Berkeley) %G%
 */

struct pdma {
	dcregs	*p_addr;
	char	*p_mem;
	char	*p_end;
	int	p_arg;
	int	(*p_fcn)();
};
