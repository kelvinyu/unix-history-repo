/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
_sccsid:.asciz	"@(#)ffs.s	5.4 (Berkeley) %G%"
#endif /* SYSLIBC_SCCS and not lint */

/* bit = ffs(value) */

#include "DEFS.h"

ENTRY(ffs, 0)
	ffs	$0,$32,4(ap),r0
	bneq	1f
	mnegl	$1,r0
1:
	incl	r0
	ret
