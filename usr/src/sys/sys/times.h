/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)times.h	7.7 (Berkeley) %G%
 */

#include <machine/ansi.h>

#ifdef	_BSD_CLOCK_T_
typedef	_BSD_CLOCK_T_	clock_t;
#undef	_BSD_CLOCK_T_
#endif

struct tms {
	clock_t tms_utime;	/* User CPU time */
	clock_t tms_stime;	/* System CPU time */
	clock_t tms_cutime;	/* User CPU time of terminated child procs */
	clock_t tms_cstime;	/* System CPU time of terminated child procs */
};

#ifndef KERNEL
#include <sys/cdefs.h>

__BEGIN_DECLS
clock_t	times __P((struct tms *));
__END_DECLS
#endif
