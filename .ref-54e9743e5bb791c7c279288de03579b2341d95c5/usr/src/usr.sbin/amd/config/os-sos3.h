/*
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
 *	@(#)os-sos3.h	8.1 (Berkeley) %G%
 *
 * $Id: os-sos3.h,v 5.2.2.1 1992/02/09 15:10:39 jsp beta $
 *
 * SunOS 3.2 definitions for Amd (automounter)
 */

/*
 * Does the compiler grok void *
 */
#define	VOIDP

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
 * Byte ordering
 */
#undef ARCH_ENDIAN
#if defined(mc68010) || defined(mc68020) || defined(sparc)
#define	ARCH_ENDIAN	"big"
#endif
#if defined(i386)
#define ARCH_ENDIAN	"little"
#endif

/*
 * Name of filesystem types
 */
#define	MOUNT_TYPE_UFS	MOUNT_UFS
#define MOUNT_TYPE_NFS	MOUNT_NFS
