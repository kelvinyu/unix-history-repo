/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)sysctl.h	7.12 (Berkeley) %G%
 */

/*
 * Get kernel info
 */
#define ki_op(x)		((x)&0x0000ffff)
#define ki_type(x)		((x)&0x0000ff00)

/* 
 * proc info 
 */
#define	KINFO_PROC		(0<<8)
#define KINFO_PROC_ALL	(KINFO_PROC|0)	/* everything */
#define	KINFO_PROC_PID	(KINFO_PROC|1)	/* by process id */
#define	KINFO_PROC_PGRP	(KINFO_PROC|2)	/* by process group id */
#define	KINFO_PROC_SESSION (KINFO_PROC|3)	/* by session of pid */
#define	KINFO_PROC_TTY	(KINFO_PROC|4)	/* by controlling tty */
#define	KINFO_PROC_UID	(KINFO_PROC|5)	/* by effective uid */
#define	KINFO_PROC_RUID	(KINFO_PROC|6)	/* by real uid */

/*
 * Routing table
 */
#define ki_af(x)		(((x)&0x00ff0000) >> 16)
#define KINFO_RT		(1<<8)
#define KINFO_RT_DUMP	(KINFO_RT|1)	/* dump; may limit to a.f. */
#define KINFO_RT_FLAGS	(KINFO_RT|2)	/* by flags, e.g. RESOLVING */
#define KINFO_RT_IFLIST (KINFO_RT|3)	/* survey interface list */

/*
 * vnodes
 */
#define	KINFO_VNODE		(2<<8)

/*
 * file structures
 */
#define KINFO_FILE		(3<<8)

/*
 * vmtotal structure
 */
#define KINFO_METER		(4<<8)

/*
 * load average structure
 */
#define KINFO_LOADAVG		(5<<8)

/*
 * clock rate structure
 */
#define KINFO_CLOCKRATE		(6<<8)

/*
 * Locking and stats
 */

struct kinfo_lock {
	int	kl_lock;
	int	kl_want;
	int	kl_locked;
};

#ifdef KERNEL
extern struct kinfo_lock kinfo_lock;
#else

#include <sys/cdefs.h>

__BEGIN_DECLS
int	getkerninfo __P((int, void *, int *, int));
__END_DECLS
#endif
