/*-
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#include <stdio.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/kinfo.h>
#include <sys/kinfo_proc.h>

md_core(kd, fd, ki)
	kvm_t *kd;
	int fd;
	struct kinfo_proc *ki;
{
	/* Don't need to fix anything for this architecture. */
	return (0);
}
