/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
 *
 * %sccs.include.redist.c%
 *
 * from: Utah $Hdr: mem.c 1.14 90/10/12$
 *
 *	@(#)mem.c	8.3 (Berkeley) %G%
 */

/*
 * Memory special file
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/systm.h>
#include <sys/malloc.h>

#include <machine/cpu.h>

#include <vm/vm.h>

/*ARGSUSED*/
mmrw(dev, uio, flags)
	dev_t dev;
	struct uio *uio;
	int flags;
{
	register u_long v;
	register u_int c;
	register struct iovec *iov;
	int error = 0;
	caddr_t zbuf = NULL;
	extern vm_offset_t avail_end;

	while (uio->uio_resid > 0 && error == 0) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("mmrw");
			continue;
		}
		switch (minor(dev)) {

/* minor device 0 is physical memory */
		case 0:
			v = (u_long)uio->uio_offset;
			c = iov->iov_len;
			if (v + c <= btoc(physmem))
				v += MACH_CACHED_MEMORY_ADDR;
			else
				return (EFAULT);
			error = uiomove((caddr_t)v, (int)c, uio);
			continue;

/* minor device 1 is kernel memory */
		case 1:
			v = (u_long)uio->uio_offset;
			if (v < MACH_CACHED_MEMORY_ADDR)
				return (EFAULT);
			c = iov->iov_len;
			if (v + c <= MACH_PHYS_TO_CACHED(avail_end) ||
			    v >= MACH_KSEG2_ADDR && kernacc((caddr_t)v, c,
			    uio->uio_rw == UIO_READ ? B_READ : B_WRITE)) {
				error = uiomove((caddr_t)v, (int)c, uio);
				continue;
			}
			return (EFAULT);

/* minor device 2 is EOF/RATHOLE */
		case 2:
			if (uio->uio_rw == UIO_WRITE)
				uio->uio_resid = 0;
			return (0);

/* minor device 12 (/dev/zero) is source of nulls on read, rathole on write */
		case 12:
			if (uio->uio_rw == UIO_WRITE) {
				c = iov->iov_len;
				break;
			}
			if (zbuf == NULL) {
				zbuf = (caddr_t)
				    malloc(CLBYTES, M_TEMP, M_WAITOK);
				bzero(zbuf, CLBYTES);
			}
			c = min(iov->iov_len, CLBYTES);
			error = uiomove(zbuf, (int)c, uio);
			continue;

		default:
			return (ENXIO);
		}
		if (error)
			break;
		iov->iov_base += c;
		iov->iov_len -= c;
		uio->uio_offset += c;
		uio->uio_resid -= c;
	}
	if (zbuf)
		free(zbuf, M_TEMP);
	return (error);
}
