/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)syscall.s	8.1 (Berkeley) %G%"
#endif /* LIBC_SCCS and not lint */

#include "SYS.h"

ENTRY(syscall)
	clrl	d0
	trap	#0
	jcs	err
	rts
err:
	jmp	cerror
