/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)limits.h	7.3 (Berkeley) %G%
 */

#define	CHAR_BIT	8		/* number of bits in a char */
#define	CLK_TCK		60		/* ticks per second */
#define	MB_LEN_MAX	1		/* no multibyte characters */

#define	SCHAR_MAX	127		/* min value for a signed char */
#define	SCHAR_MIN	-128		/* max value for a signed char */

#define	UCHAR_MAX	255		/* max value for an unsigned char */
#define	CHAR_MAX	127		/* max value for a char */
#define	CHAR_MIN	128		/* min value for a char */

#define	USHRT_MAX	65535		/* max value for an unsigned short */
#define	SHRT_MAX	32767		/* max value for a short */
#define	SHRT_MIN	-32768		/* min value for a short */

#define	UINT_MAX	4294967295	/* max value for an unsigned int */
#define	INT_MAX		2147483647	/* max value for an int */
#define	INT_MIN		(-2147483647-1)	/* min value for an int */

#define	ULONG_MAX	4294967295	/* max value for an unsigned long */
#define	LONG_MAX	2147483647	/* max value for a long */
#define	LONG_MIN	(-2147483647-1)	/* min value for a long */
