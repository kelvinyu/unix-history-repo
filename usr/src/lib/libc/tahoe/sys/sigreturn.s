/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid:.asciz	"@(#)sigreturn.s	5.1 (Berkeley) %G%"
#endif SYSLIBC_SCCS

#include "SYS.h"

/*
 * We must preserve the state of the registers as the user has set them up.
 */
#ifdef PROF
#define	POPR(r)	movl (sp)+,r
#undef ENTRY
#define	ENTRY(x) \
	.globl _/**/x; .align 2; _/**/x: .word 0; \
	pushl r0; pushl r1; pushl r2; pushl r3; pushl r4; pushl r5; \
	.data; 1:; .long 0; .text; pushl $1b; callf $8,mcount; \
	POPR(r5); POPR(r4); POPR(r3); POPR(r2); POPR(r1); POPR(r0);
#endif PROF

SYSCALL(sigreturn)
	ret
