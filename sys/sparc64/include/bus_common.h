/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratory.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	form: @(#)sbusreg.h	8.1 (Berkeley) 6/11/93
 *	from: NetBSD: iommureg.h,v 1.6 2001/07/20 00:07:13 eeh Exp
 *
 * $FreeBSD$
 */

#ifndef _MACHINE_BUS_COMMON_H_
#define _MACHINE_BUS_COMMON_H_

#define INTMAP_V		0x080000000LL	/* Interrupt valid (enabled) */
#define INTMAP_TID_MASK		0x07c000000LL	/* UPA target ID */
#define INTMAP_TID_SHIFT	26
#define INTMAP_IGN_MASK		0x0000007c0LL	/* Interrupt group no. */
#define INTMAP_IGN_SHIFT	6
#define INTMAP_INO_MASK		0x00000003fLL	/* Interrupt number */
#define INTMAP_INR_MASK		(INTMAP_IGN_MASK | INTMAP_INO_MASK)
#define INTMAP_SBUSSLOT_MASK	0x000000018LL	/* SBUS slot # */
#define INTMAP_PCIBUS_MASK	0x000000010LL	/* PCI bus number (A or B) */
#define INTMAP_PCISLOT_MASK	0x00000000cLL	/* PCI slot # */
#define INTMAP_PCIINT_MASK	0x000000003LL	/* PCI interrupt #A,#B,#C,#D */
#define INTMAP_OBIO_MASK	0x000000020LL	/* Onboard device */
#define INTVEC(x)		((x) & INTMAP_INR_MASK)
#define INTSLOT(x)		(((x) >> 3) & 0x7)
#define	INTPRI(x)		((x) & 0x7)
#define	INTINO(x)		((x) & INTMAP_INO_MASK)
#define	INTMAP_ENABLE(mr, mid) \
	(((mr) & ~INTMAP_TID_MASK) | ((mid) << INTMAP_TID_SHIFT) | INTMAP_V)

/* counter-timer support. */
void sparc64_counter_init(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset);

#endif	/* !_MACHINE_BUS_COMMON_H_ */
