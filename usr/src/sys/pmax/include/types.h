/*-
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)types.h	7.1 (Berkeley) %G%
 */

#ifndef	_MACHTYPES_H_
#define	_MACHTYPES_H_

typedef struct _physadr {
	int r[1];
} *physadr;

typedef struct label_t {
	int val[12];
} label_t;

typedef	u_long	vm_offset_t;
typedef	u_long	vm_size_t;

#endif	/* _MACHTYPES_H_ */
