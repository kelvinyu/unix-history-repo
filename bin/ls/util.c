/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Michael Fischbein.
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
 */

#ifndef lint
static char sccsid[] = "@(#)util.c	5.8 (Berkeley) 7/22/90";
static char rcsid[] = "$Header: /a/cvs/386BSD/src/bin/ls/util.c,v 1.2 1993/06/29 02:59:34 nate Exp $";
#endif /* not lint */

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>

prcopy(src, dest, len)
	register char *src, *dest;
	register int len;
{
	register unsigned char ch;

	while(len--) {
		ch = *src++;
		/* XXX:
		 * because *BSD don't have setlocale() (yet)
		 * here simple hack that allows ISO8859-x
		 * and koi8-r charsets in terminal mode.
		 * Note: range 0x80-0x9F skipped to avoid
		 * some kinda security hole on poor DEC VTs
		 */
		*dest++ = (ch >= 0xA0 || isprint(ch)) ? ch : '?';
	}
}

char
*emalloc(size)
	u_int size;
{
	char *retval, *malloc();

	if (!(retval = malloc(size)))
		nomem();
	return(retval);
}

nomem()
{
	(void)fprintf(stderr, "ls: out of memory.\n");
	exit(1);
}

usage()
{
	(void)fprintf(stderr, "usage: ls [-1ACFLRTacdfgiklqrstu] [file ...]\n");
	exit(1);
}
