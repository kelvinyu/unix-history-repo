/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * %sccs.include.redist.c%
 */

#include <sys/syscall.h>
#include <machine/reg.h>
#include "DEFS.h"

#if defined(LIBC_SCCS) && !defined(lint)
	ASMSTR("@(#)setjmp.s	5.3 (Berkeley) %G%")
#endif /* LIBC_SCCS and not lint */

/*
 * C library -- setjmp, longjmp
 *
 *	longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	setjmp(a)
 * by restoring registers from the stack,
 * and a struct sigcontext, see <signal.h>
 */

#define SETJMP_FRAME_SIZE	(STAND_FRAME_SIZE + 8)

NON_LEAF(setjmp, SETJMP_FRAME_SIZE, ra)
	subu	sp, sp, SETJMP_FRAME_SIZE	# allocate stack frame
	.mask	0x80000000, (STAND_RA_OFFSET - STAND_FRAME_SIZE)
	sw	ra, STAND_RA_OFFSET(sp)		# save state
	sw	a0, SETJMP_FRAME_SIZE(sp)
	move	a0, zero			# get current signal mask
	jal	sigblock
	lw	v1, SETJMP_FRAME_SIZE(sp)	# v1 = jmpbuf
	sw	v0, (1 * 4)(v1)			# save sc_mask = sigblock(0)
	move	a0, zero
	addu	a1, sp, STAND_FRAME_SIZE	# pointer to struct sigstack
	jal	sigstack
	lw	a0, SETJMP_FRAME_SIZE(sp)	# restore jmpbuf
	lw	v1, STAND_FRAME_SIZE+4(sp)	# get old ss_onstack
	sw	v1, 0(a0)			# save it in sc_onstack
	lw	ra, STAND_RA_OFFSET(sp)
	addu	sp, sp, SETJMP_FRAME_SIZE
	blt	v0, zero, botch			# check for sigstack() error
	sw	ra, (2 * 4)(a0)			# sc_pc = return address
	li	v0, 0xACEDBADE			# sigcontext magic number
	sw	v0, ((ZERO + 3) * 4)(a0)	#   saved in sc_regs[0]
	sw	s0, ((S0 + 3) * 4)(a0)
	sw	s1, ((S1 + 3) * 4)(a0)
	sw	s2, ((S2 + 3) * 4)(a0)
	sw	s3, ((S3 + 3) * 4)(a0)
	sw	s4, ((S4 + 3) * 4)(a0)
	sw	s5, ((S5 + 3) * 4)(a0)
	sw	s6, ((S6 + 3) * 4)(a0)
	sw	s7, ((S7 + 3) * 4)(a0)
	sw	gp, ((GP + 3) * 4)(a0)
	sw	sp, ((SP + 3) * 4)(a0)
	sw	s8, ((S8 + 3) * 4)(a0)
	li	v0, 1				# be nice if we could tell
	sw	v0, (37 * 4)(a0)		# sc_fpused = 1
	cfc1	v0, $31
	swc1	$f20, ((20 + 38) * 4)(a0)
	swc1	$f21, ((21 + 38) * 4)(a0)
	swc1	$f22, ((22 + 38) * 4)(a0)
	swc1	$f23, ((23 + 38) * 4)(a0)
	swc1	$f24, ((24 + 38) * 4)(a0)
	swc1	$f25, ((25 + 38) * 4)(a0)
	swc1	$f26, ((26 + 38) * 4)(a0)
	swc1	$f27, ((27 + 38) * 4)(a0)
	swc1	$f28, ((28 + 38) * 4)(a0)
	swc1	$f29, ((29 + 38) * 4)(a0)
	swc1	$f30, ((30 + 38) * 4)(a0)
	swc1	$f31, ((31 + 38) * 4)(a0)
	sw	v0, ((32 + 38) * 4)(a0)
	move	v0, zero
	j	ra
END(setjmp)

LEAF(longjmp)
	sw	a1, ((V0 + 3) * 4)(a0)	# save return value in sc_regs[V0]
	li	v0, SYS_sigreturn
	syscall
botch:
	jal	longjmperror
	jal	abort
END(longjmp)
