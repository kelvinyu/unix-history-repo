/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratories.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)fpu_arith.h	7.2 (Berkeley) %G%
 *
 * from: $Header: fpu_arith.h,v 1.2 92/06/17 05:41:28 torek Exp $
 */

/*
 * Extended-precision arithmetic.
 *
 * We hold the notion of a `carry register', which may or may not be a
 * machine carry bit or register.  On the SPARC, it is just the machine's
 * carry bit.
 *
 * In the worst case, you can compute the carry from x+y as
 *	(unsigned)(x + y) < (unsigned)x
 * and from x+y+c as
 *	((unsigned)(x + y + c) <= (unsigned)x && (y|c) != 0)
 * for example.
 */

/* set up for extended-precision arithemtic */
#define	FPU_DECL_CARRY

/*
 * We have three kinds of add:
 *	add with carry:					  r = x + y + c
 *	add (ignoring current carry) and set carry:	c'r = x + y + 0
 *	add with carry and set carry:			c'r = x + y + c
 * The macros use `C' for `use carry' and `S' for `set carry'.
 * Note that the state of the carry is undefined after ADDC and SUBC,
 * so if all you have for these is `add with carry and set carry',
 * that is OK.
 *
 * The same goes for subtract, except that we compute x - y - c.
 *
 * Finally, we have a way to get the carry into a `regular' variable,
 * or set it from a value.  SET_CARRY turns 0 into no-carry, nonzero
 * into carry; GET_CARRY sets its argument to 0 or 1.
 */
#define	FPU_ADDC(r, x, y) \
	asm volatile("addx %1,%2,%0" : "=r"(r) : "r"(x), "r"(y))
#define	FPU_ADDS(r, x, y) \
	asm volatile("addcc %1,%2,%0" : "=r"(r) : "r"(x), "r"(y))
#define	FPU_ADDCS(r, x, y) \
	asm volatile("addxcc %1,%2,%0" : "=r"(r) : "r"(x), "r"(y))
#define	FPU_SUBC(r, x, y) \
	asm volatile("subx %1,%2,%0" : "=r"(r) : "r"(x), "r"(y))
#define	FPU_SUBS(r, x, y) \
	asm volatile("subcc %1,%2,%0" : "=r"(r) : "r"(x), "r"(y))
#define	FPU_SUBCS(r, x, y) \
	asm volatile("subxcc %1,%2,%0" : "=r"(r) : "r"(x), "r"(y))

#define	FPU_GET_CARRY(r) asm volatile("addx %%g0,%%g0,%0" : "=r"(r))
#define	FPU_SET_CARRY(v) asm volatile("addcc %0,-1,%%g0" : : "r"(v))

#define	FPU_SHL1_BY_ADD	/* shift left 1 faster by ADDC than (a<<1)|(b>>31) */
