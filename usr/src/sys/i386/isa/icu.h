/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * %sccs.include.386.c%
 *
 *	@(#)icu.h	5.8 (Berkeley) %G%
 */

/*
 * AT/386 Interrupt Control constants
 * W. Jolitz 8/89
 */

#ifndef	__ICU__
#define	__ICU__

#ifndef	LOCORE

/*
 * Interrupt "level" mechanism variables, masks, and macros
 */
extern	unsigned short	imen;	/* interrupt mask enable */
extern	unsigned short	cpl;	/* current priority level mask */

extern	unsigned short highmask; /* group of interrupts masked with splhigh() */
extern	unsigned short ttymask; /* group of interrupts masked with spltty() */
extern	unsigned short biomask; /* group of interrupts masked with splbio() */
extern	unsigned short netmask; /* group of interrupts masked with splimp() */

#define	INTREN(s)	imen &= ~(s)
#define	INTRDIS(s)	imen |= (s)
#define	INTRMASK(msk,s)	msk |= (s)

#else

/*
 * Macro's for interrupt level priority masks (used in interrupt vector entry)
 */

/* Mask a group of interrupts atomically */
#define	INTR_HEAD(unit,mask,offst) \
	pushl	$ offst ; \
	pushl	$ T_ASTFLT ; \
	pushal ; \
	movb	$0x20,%al ; \

#define INTR_TAIL(unit,mask,offst) \
	outb	%al,$ IO_ICU1 ; \
	pushl	%ds ; \
	pushl	%es ; \
	movw	$0x10, %ax ; \
	movw	%ax, %ds ; \
	movw	%ax,%es ; \
	incl	_cnt+V_INTR ; \
	incl	_isa_intr + offst * 4 ; \
	movzwl	_cpl,%eax ; \
	pushl	%eax ; \
	pushl	$ unit ; \
	orw	mask ,%ax ; \
	movw	%ax,_cpl ; \
	orw	_imen,%ax ; \
	outb	%al,$ IO_ICU1+1 ; \
	movb	%ah,%al ; \
	outb	%al,$ IO_ICU2+1	; \
	sti ;

#define INTR1(unit,mask,offst) \
	INTR_HEAD(unit,mask,offst) \
	INTR_TAIL(unit,mask,offst)

#define INTR2(unit,mask,offst) \
	INTR_HEAD(unit,mask,offst) \
	outb	%al,$ IO_ICU2 ; \
	INTR_TAIL(unit,mask,offst)



/* Interrupt vector exit macros */

/* First eight interrupts (ICU1) */
#define	INTREXIT1	\
	jmp	doreti

/* Second eight interrupts (ICU2) */
#define	INTREXIT2	\
	jmp	doreti

#endif

/*
 * Interrupt enable bits -- in order of priority
 */
#define	IRQ0		0x0001		/* highest priority - timer */
#define	IRQ1		0x0002
#define	IRQ_SLAVE	0x0004
#define	IRQ8		0x0100
#define	IRQ9		0x0200
#define	IRQ2		IRQ9
#define	IRQ10		0x0400
#define	IRQ11		0x0800
#define	IRQ12		0x1000
#define	IRQ13		0x2000
#define	IRQ14		0x4000
#define	IRQ15		0x8000
#define	IRQ3		0x0008
#define	IRQ4		0x0010
#define	IRQ5		0x0020
#define	IRQ6		0x0040
#define	IRQ7		0x0080		/* lowest - parallel printer */

/*
 * Interrupt Control offset into Interrupt descriptor table (IDT)
 */
#define	ICU_OFFSET	32		/* 0-31 are processor exceptions */
#define	ICU_LEN		16		/* 32-47 are ISA interrupts */

#endif	__ICU__
