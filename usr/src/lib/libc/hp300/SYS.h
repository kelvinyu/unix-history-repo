/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)SYS.h	5.5 (Berkeley) %G%
 */

#include <sys/syscall.h>

#ifdef PROF
#ifdef __GNUC__
#define	ENTRY(x)	.globl _/**/x; .even; _/**/x:; .data; PROF/**/x:; \
			.long 0; .text; link a6,#0; lea PROF/**/x,a0; \
			jbsr mcount; unlk a6
#else
#define	ENTRY(x)	.globl _/**/x; .even; _/**/x:; .data; PROF/**/x:; \
			.long 0; .text; lea PROF/**/x,a0; jbsr mcount
#endif
#else
#define	ENTRY(x)	.globl _/**/x; .even; _/**/x:
#endif PROF
#define	SYSCALL(x)	.even; err: jmp cerror; ENTRY(x); movl #SYS_/**/x,d0; \
			trap #0; jcs err
#define	RSYSCALL(x)	SYSCALL(x); rts
#define	PSEUDO(x,y)	ENTRY(x); movl #SYS_/**/y,d0; trap #0; rts

#define	ASMSTR		.asciz

	.globl	cerror
