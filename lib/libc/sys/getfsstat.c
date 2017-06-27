/*-
 * Copyright (c) 2017 M. Warner Losh <imp@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include "compat-ino64.h"
#include <sys/errno.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <unistd.h>

#include "libc_private.h"

int
getfsstat(struct statfs *buf, long bufsize, int flags)
{
	struct freebsd11_statfs *statfs11 = NULL;
	ssize_t len = 0;
	int rv, i;

	if (__getosreldate() >= INO64_FIRST)
		return (__sys_getfsstat(buf, bufsize, flags));
	if (buf != NULL) {
		len = sizeof(struct freebsd11_statfs) *	/* Round down on purpose to avoid */
		    (bufsize / sizeof(struct statfs));	/* overflow on translation.	  */
		statfs11 = malloc(len);
		if (statfs11 == NULL) {
			errno = ENOMEM;
			return (-1);
		}
	}
	rv = syscall(SYS_freebsd11_getfsstat, statfs11, len, flags);
	if (rv != -1 && buf != NULL) {
		for (i = 0; i < rv; i++)
			__statfs11_to_statfs(&statfs11[i], &buf[i]);
	}
	free(statfs11);
	return (rv);
}
