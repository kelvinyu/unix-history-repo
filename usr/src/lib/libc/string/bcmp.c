/*
 * Copyright (c) 1987 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)bcmp.c	5.3 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <string.h>

/*
 * bcmp -- vax cmpc3 instruction
 */
bcmp(b1, b2, length)
	register char *b1, *b2;
	register size_t length;
{

	if (length == 0)
		return(0);
	do
		if (*b1++ != *b2++)
			break;
	while (--length);
	return(length);
}
