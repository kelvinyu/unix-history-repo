/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * %sccs.include.redist.c%
 *
 * from: $Header: htons.s,v 1.1 92/06/25 12:47:05 torek Exp $
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)htons.s	5.1 (Berkeley) %G%"
#endif /* LIBC_SCCS and not lint */

/* netorder = htons(hostorder) */

#include "DEFS.h"

ENTRY(htons)
	sethi	%hi(0xffff0000), %o1
	retl
	 andn	%o0, %o1, %o0
