/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * %sccs.include.redist.c%
 */

#include "SYS.h"

#if defined(LIBC_SCCS) && !defined(lint)
	ASMSTR("@(#)sigreturn.s	5.2 (Berkeley) %G%")
#endif /* LIBC_SCCS and not lint */

/*
 * We must preserve the state of the registers as the user has set them up.
 */

RSYSCALL(sigreturn)
