/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)conv.c	5.1 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <ctype.h>
#include "hexdump.h"

conv_c(pr, p)
	PR *pr;
	u_char *p;
{
	char *str;

	switch(*p) {
	case '\0':
		str = "\\0";
		goto strpr;
	/* case '\a': */
	case '\007':
		str = "\\a";
		goto strpr;
	case '\b':
		str = "\\b";
		goto strpr;
	case '\f':
		str = "\\f";
		goto strpr;
	case '\n':
		str = "\\n";
		goto strpr;
	case '\r':
		str = "\\r";
		goto strpr;
	case '\t':
		str = "\\t";
		goto strpr;
	case '\v':
		str = "\\v";
strpr:		*pr->cchar = 's';
		(void)printf(pr->fmt, str);
		break;
	default:
		if (isprint(*p)) {
			*pr->cchar = 'c';
			(void)printf(pr->fmt, *p);
		} else {
			*pr->cchar = 'x';
			(void)printf(pr->fmt, (int)*p);
		}
	}
}

conv_u(pr, p)
	PR *pr;
	u_char *p;
{
	static char *list[] = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
		 "bs",  "ht",  "lf",  "vt",  "ff",  "cr",  "so",  "si",
		"dle", "dcl", "dc2", "dc3", "dc4", "nak", "syn", "etc",
		"can",  "em", "sub", "esc",  "fs",  "gs",  "rs",  "us",
	};

	if (*p <= 0x1f) {
		*pr->cchar = 's';
		(void)printf(pr->fmt, list[*p]);
	} else if (*p == 0xff) {
		*pr->cchar = 's';
		(void)printf(pr->fmt, "del");
	} else if (isprint(*p)) {
		*pr->cchar = 'c';
		(void)printf(pr->fmt, *p);
	} else {
		*pr->cchar = 'x';
		(void)printf(pr->fmt, (int)*p);
	}
}
