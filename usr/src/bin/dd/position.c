/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego and Lance
 * Visser of Convex Computer Corporation.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)position.c	5.3 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "dd.h"
#include "extern.h"

/*
 * Position input/output data streams before starting the copy.  Device type
 * dependent.  Seekable devices use lseek, and the rest position by reading.
 * Seeking past the end of file can cause null blocks to be written to the
 * output.
 */
void
pos_in()
{
	register int bcnt, cnt, nr, warned;

	/* If not a character, pipe or tape device, try to seek on it. */
	if (!(in.flags & (ISCHR|ISPIPE|ISTAPE))) {
		if (lseek(in.fd, (off_t)(in.offset * in.dbsz), SEEK_CUR) == -1)
			err("%s: %s", in.name, strerror(errno));
		return;
	}

	/*
	 * Read the data.  If a pipe, read until satisfy the number of bytes
	 * being skipped.  No differentiation for reading complete and partial
	 * blocks for other devices.
	 */
	for (bcnt = in.dbsz, cnt = in.offset, warned = 0; cnt;) {
		if ((nr = read(in.fd, in.db, bcnt)) > 0) {
			if (in.flags & ISPIPE) {
				if (!(bcnt -= nr)) {
					bcnt = in.dbsz;
					--cnt;
				}
			} else
				--cnt;
			continue;
		}

		if (nr == 0) {
			if (files_cnt > 1) {
				--files_cnt;
				continue;
			}
			err("skip reached end of input");
		}

		/*
		 * Input error -- either EOF with no more files, or I/O error.
		 * If noerror not set die.  POSIX requires that the warning
		 * message be followed by an I/O display.
		 */
		if (ddflags & C_NOERROR) {
			if (!warned) {
				warn("%s: %s", in.name, strerror(errno));
				warned = 1;
				summary(0);
			}
			continue;
		}
		err("%s: %s", in.name, strerror(errno));
	}
}

void
pos_out()
{
	register int cnt, n;
	struct mtop t_op;

	/*
	 * If not a tape, try seeking on the file.  Seeking on a pipe is
	 * going to fail, but don't protect the user -- they shouldn't
	 * have specified the seek operand.
	 */
	if (!(out.flags & ISTAPE)) {
		if (lseek(out.fd,
		    (off_t)out.offset * out.dbsz, SEEK_SET) == -1)
			err("%s: %s", out.name, strerror(errno));
		return;
	}

	/* If no read access, try using mtio. */
	if (out.flags & NOREAD) {
		t_op.mt_op = MTFSR;
		t_op.mt_count = out.offset;

		if (ioctl(out.fd, MTIOCTOP, &t_op) < 0)
			err("%s: %s", out.name, strerror(errno));
		return;
	}

	/* Read it. */
	for (cnt = 0; cnt < out.offset; ++cnt) {
		if ((n = read(out.fd, out.db, out.dbsz)) > 0)
			continue;

		if (n < 0)
			err("%s: %s", out.name, strerror(errno));

		/*
		 * If reach EOF, fill with NUL characters; first, back up over
		 * the EOF mark.  Note, cnt has not yet been incremented, so
		 * the EOF read does not count as a seek'd block.
		 */
		t_op.mt_op = MTBSR;
		t_op.mt_count = 1;
		if (ioctl(out.fd, MTIOCTOP, &t_op) == -1)
			err("%s: %s", out.name, strerror(errno));

		while (cnt++ < out.offset)
			if ((n = write(out.fd, out.db, out.dbsz)) != out.dbsz)
				err("%s: %s", out.name, strerror(errno));
		break;
	}
}
