/*-
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	from: @(#)tty_subr.c	8.2 (Berkeley) 9/5/93
 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/tty.h>

char cwaiting;
struct cblock *cfree, *cfreelist;
int cfreecount, nclist;

void
clist_init()
{

	/*
	 * Body deleted.
	 */
	return;
}

getc(a1)
	struct clist *a1;
{

	/*
	 * Body deleted.
	 */
	return ((char)0);
}

q_to_b(a1, a2, a3)
	struct clist *a1;
	char *a2;
	int a3;
{

	/*
	 * Body deleted.
	 */
	return (0);
}

ndqb(a1, a2)
	struct clist *a1;
	int a2;
{

	/*
	 * Body deleted.
	 */
	return (0);
}

void
ndflush(a1, a2)
	struct clist *a1;
	int a2;
{

	/*
	 * Body deleted.
	 */
	return;
}

putc(a1, a2)
	char a1;
	struct clist *a2;
{

	/*
	 * Body deleted.
	 */
	return (0);
}

b_to_q(a1, a2, a3)
	char *a1;
	int a2;
	struct clist *a3;
{

	/*
	 * Body deleted.
	 */
	return (0);
}

char *
nextc(a1, a2, a3)
	struct clist *a1;
	char *a2;
	int *a3;
{

	/*
	 * Body deleted.
	 */
	return ((char *)0);
}

unputc(a1)
	struct clist *a1;
{

	/*
	 * Body deleted.
	 */
	return ((char)0);
}

void
catq(a1, a2)
	struct clist *a1, *a2;
{

	/*
	 * Body deleted.
	 */
	return;
}
