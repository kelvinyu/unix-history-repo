/* $Id: os-utx32.h,v 5.2.1.1 90/10/21 22:31:11 jsp Exp $ */

/*
 * Gould UTX/32 definitions for Amd (automounter)
 *
 * Copyright (c) 1989 Jan-Simon Pendry
 * Copyright (c) 1989 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)os-utx32.h	8.1 (Berkeley) %G%
 */

/*
 * Does the compiler grok void *
 */
#ifdef __GNUC__
#define	VOIDP
#endif

/*
 * Which version of the Sun RPC library we are using
 * This is the implementation release number, not
 * the protocol revision number.
 */
#define	RPC_3

/*
 * Which version of the NFS interface are we using.
 * This is the implementation release number, not
 * the protocol revision number.
 */
#define	NFS_3

/*
 * Does this OS have NDBM support?
 */
#define OS_HAS_NDBM

/*
 * Byte ordering
 */
#undef ARCH_ENDIAN
#if defined(gould) || defined(GOULD_PN)
#define	ARCH_ENDIAN	"big"
#endif

/*
 * Name of filesystem types
 */
#define	MOUNT_TYPE_NFS	MOUNT_NFS
#define	MOUNT_TYPE_UFS	MOUNT_UFS
#undef MTAB_TYPE_UFS
#define	MTAB_TYPE_UFS	"4.3"
