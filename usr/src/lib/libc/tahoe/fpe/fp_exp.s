/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Computer Consoles Inc.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.asciz "@(#)fp_exp.s	1.2 (Berkeley) %G%"
#endif /* SYSLIBC_SCCS and not lint */

#include <tahoemath/fp.h>
#include "DEFS.h"

/*
 * Reserved floating point operand.
 */
ASENTRY(fpresop, 0)
	movl	$0xaaaaaaaa,r0
	movl	$0xaaaaaaaa,r1
	ret

/*
 * Floating point overflow.
 */
ASENTRY(fpover, 0)
	movl	$HUGE0,r0
	movl	$HUGE1,r1
	ret

/*
 * Floating point underflow.
 */
ASENTRY(fpunder, 0)
	clrl	r0
	clrl	r1
	ret

/*
 * Floating point division by zero.
 */
ASENTRY(fpzdiv, 0)
	divl2	$0,r1		# force division by zero.
	ret
