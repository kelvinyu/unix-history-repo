/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Hibler and Chris Torek.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)memset.c	5.8 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>

#include <limits.h>
#include <string.h>

#define	wsize	sizeof(u_int)
#define	wmask	(wsize - 1)

#ifdef BZERO
void
bzero(dst0, length)
	void *dst0;
	register size_t length;
#else
void *
memset(dst0, c0, length)
	void *dst0;
	register int c0;
	register size_t length;
#endif
{
	register size_t t;
	register u_int c;
	register u_char *dst;

	dst = dst0;
	/*
	 * If not enough words, just fill bytes.  A length >= 2 words
	 * guarantees that at least one of them is `complete' after
	 * any necessary alignment.  For instance:
	 *
	 *	|-----------|-----------|-----------|
	 *	|00|01|02|03|04|05|06|07|08|09|0A|00|
	 *	          ^---------------------^
	 *		 dst		 dst+length-1
	 *
	 * but we use a minimum of 3 here since the overhead of the code
	 * to do word writes is substantial.
	 */ 
	if (length < 3 * wsize) {
		while (length != 0) {
#ifdef BZERO
			*dst++ = 0;
#else
			*dst++ = c0;
#endif
			--length;
		}
#ifdef BZERO
		return;
#else
		return (dst0);
#endif
	}

#ifndef BZERO
	if ((c = (u_char)c0) != 0) {	/* Fill the word. */
		c = (c << 8) | c;	/* u_int is 16 bits. */
#if UINT_MAX > 0xffff
		c = (c << 16) | c;	/* u_int is 32 bits. */
#endif
#if UINT_MAX > 0xffffffff
		c = (c << 32) | c;	/* u_int is 64 bits. */
#endif
	}
#endif
	/* Align destination by filling in bytes. */
	if ((t = (int)dst & wmask) != 0) {
		t = wsize - t;
		length -= t;
		do {
#ifdef BZERO
			*dst++ = 0;
#else
			*dst++ = c0;
#endif
		} while (--t != 0);
	}

	/* Fill words.  Length was >= 2*words so we know t >= 1 here. */
	t = length / wsize;
	do {
#ifdef	BZERO
		*(u_int *)dst = 0;
#else
		*(u_int *)dst = c;
#endif
		dst += wsize;
	} while (--t != 0);

	/* Mop up trailing bytes, if any. */
	t = length & wmask;
	if (t != 0)
		do {
#ifdef BZERO
			*dst++ = 0;
#else
			*dst++ = c0;
#endif
		} while (--t != 0);
#ifdef BZERO
	return;
#else
	return (dst0);
#endif
}
