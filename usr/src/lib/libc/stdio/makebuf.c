/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)makebuf.c	5.1 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "local.h"

/*
 * Allocate a file buffer, or switch to unbuffered I/O.
 * Per the ANSI C standard, ALL tty devices default to line buffered.
 *
 * As a side effect, we set __SOPT or __SNPT (en/dis-able fseek
 * optimisation) right after the fstat() that finds the buffer size.
 */
void
__smakebuf(fp)
	register FILE *fp;
{
	register size_t size, couldbetty;
	register void *p;
	struct stat st;

	if (fp->_flags & __SNBF) {
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
		return;
	}
	if (fp->_file < 0 || fstat(fp->_file, &st) < 0) {
		couldbetty = 0;
		size = BUFSIZ;
		/* do not try to optimise fseek() */
		fp->_flags |= __SNPT;
	} else {
		couldbetty = (st.st_mode & S_IFMT) == S_IFCHR;
		size = st.st_blksize <= 0 ? BUFSIZ : st.st_blksize;
		/*
		 * Optimise fseek() only if it is a regular file.
		 * (The test for __sseek is mainly paranoia.)
		 */
		if ((st.st_mode & S_IFMT) == S_IFREG &&
		    fp->_seek == __sseek) {
			fp->_flags |= __SOPT;
			fp->_blksize = st.st_blksize;
		} else
			fp->_flags |= __SNPT;
	}
	if ((p = malloc(size)) == NULL) {
		fp->_flags |= __SNBF;
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
	} else {
		__cleanup = _cleanup;
		fp->_flags |= __SMBF;
		fp->_bf._base = fp->_p = p;
		fp->_bf._size = size;
		if (couldbetty && isatty(fp->_file))
			fp->_flags |= __SLBF;
	}
}
