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
static char sccsid[] = "@(#)conv.c	5.3 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <string.h>
#include "dd.h"
#include "extern.h"

/*
 * def --
 * Copy input to output.  Input is buffered until reaches obs, and then
 * output until less than obs remains.  Only a single buffer is used.
 * Worst case buffer calculations are as follows:
 *
 * if (ibs > obs)			# read ibs, output just over half
 *	max = ibs - 1 - ibs / 2 + ibs;
 * else if (ibs < obs)			# obs - 1 a multiple of ibs
 *	max = obs + ibs - 1;
 * else					# input == output
 *	max = ibs;
 */
void
def()
{
	register int cnt;
	register u_char *inp, *t;

	if (t = ctab)
		for (inp = in.dbp - (cnt = in.dbrcnt); cnt--; ++inp)
			*inp = t[*inp];

	/* Make the output buffer look right. */
	out.dbp = in.dbp;
	out.dbcnt = in.dbcnt;

	if (in.dbcnt >= out.dbsz) {
		/* If the output buffer is full, write it. */
		dd_out(0);

		/*
		 * Ddout copies the leftover output to the beginning of
		 * the buffer and resets the output buffer.  Reset the
		 * input buffer to match it.
	 	 */
		in.dbp = out.dbp;
		in.dbcnt = out.dbcnt;
	}
}

void
def_close()
{
	/* Just update the count, everything is already in the buffer. */
	if (in.dbcnt)
		out.dbcnt = in.dbcnt;
}

/*
 * Copy variable length newline terminated records with a max size cbsz
 * bytes to output.  Records less than cbs are padded with spaces.
 *
 * max in buffer:  MAX(ibs, cbsz)
 * max out buffer: obs + cbsz
 */

void
block()
{
	static int intrunc;
	register int ch, cnt;
	register u_char *inp, *outp, *t;
	int maxlen;

	/*
	 * Record truncation can cross block boundaries.  If currently in a
	 * truncation state, keep tossing characters until reach a newline.
	 * Start at the beginning of the buffer, as the input buffer is always
	 * left empty.
	 */
	if (intrunc) {
		for (inp = in.db, cnt = in.dbrcnt;
		    cnt && *inp++ != '\n'; --cnt);
		if (!cnt) {
			in.dbcnt = 0;
			in.dbp = in.db;
			return;
		}
		intrunc = 0;
		/* Adjust the input buffer numbers. */
		in.dbcnt = cnt - 1;
		in.dbp = inp + cnt - 1;
	}

	/*
	 * Copy records (max cbsz size chunks) into the output buffer.  The
	 * translation is done as we copy into the output buffer.
	 */
	for (inp = in.dbp - in.dbcnt, outp = out.dbp; in.dbcnt;) {
		maxlen = MIN(cbsz, in.dbcnt);
		if (t = ctab)
			for (cnt = 0;
			    cnt < maxlen && (ch = *inp++) != '\n'; ++cnt)
				*outp++ = t[ch];
		else
			for (cnt = 0;
			    cnt < maxlen && (ch = *inp++) != '\n'; ++cnt)
				*outp++ = ch;
		/*
		 * Check for short record without a newline.  Reassemble the
		 * input block.
		 */
		if (ch != '\n' && in.dbcnt < cbsz) {
			bcopy(in.dbp - in.dbcnt, in.db, in.dbcnt);
			break;
		}

		/* Adjust the input buffer numbers. */
		in.dbcnt -= cnt;
		if (ch == '\n')
			--in.dbcnt;

		/* Pad short records with "spaces". */
		if (cnt < cbsz)
			(void)memset(outp, ctab ? ctab[' '] : ' ', cbsz - cnt);
		else {
			++st.trunc;
			/* Toss characters to a newline. */
			for (; in.dbcnt && *inp++ != '\n'; --in.dbcnt);
			if (!in.dbcnt)
				intrunc = 1;
			else
				--in.dbcnt;
		}

		/* Adjust output buffer numbers. */
		out.dbp += cbsz;
		if ((out.dbcnt += cbsz) >= out.dbsz)
			dd_out(0);
		outp = out.dbp;
	}
	in.dbp = in.db + in.dbcnt;
}

void
block_close()
{
	/*
	 * Copy any remaining data into the output buffer and pad to a record.
	 * Don't worry about truncation or translation, the input buffer is
	 * always empty when truncating, and no characters have been added for
	 * translation.  The bottom line is that anything left in the input
	 * buffer is a truncated record.  Anything left in the output buffer
	 * just wasn't big enough.
	 */
	if (in.dbcnt) {
		++st.trunc;
		bcopy(in.dbp - in.dbcnt, out.dbp, in.dbcnt);
		(void)memset(out.dbp + in.dbcnt,
		    ctab ? ctab[' '] : ' ', cbsz - in.dbcnt);
		out.dbcnt += cbsz;
	}
}

/*
 * Convert fixed length (cbsz) records to variable length.  Deletes any
 * trailing blanks and appends a newline.
 *
 * max in buffer:  MAX(ibs, cbsz) + cbsz
 * max out buffer: obs + cbsz
 */
void
unblock()
{
	register int cnt;
	register u_char *inp, *t;

	/* Translation and case conversion. */
	if (t = ctab)
		for (cnt = in.dbrcnt, inp = in.dbp; cnt--;)
			*--inp = t[*inp];
	/*
	 * Copy records (max cbsz size chunks) into the output buffer.  The
	 * translation has to already be done or we might not recognize the
	 * spaces.
	 */
	for (inp = in.db; in.dbcnt >= cbsz; inp += cbsz, in.dbcnt -= cbsz) {
		for (t = inp + cbsz - 1; t >= inp && *t == ' '; --t);
		if (t >= inp) {
			cnt = t - inp + 1;
			bcopy(inp, out.dbp, cnt);
			out.dbp += cnt;
			out.dbcnt += cnt;
		}
		++out.dbcnt;
		*out.dbp++ = '\n';
		if (out.dbcnt >= out.dbsz)
			dd_out(0);
	}
	if (in.dbcnt)
		bcopy(in.dbp - in.dbcnt, in.db, in.dbcnt);
	in.dbp = in.db + in.dbcnt;
}

void
unblock_close()
{
	register int cnt;
	register u_char *t;

	if (in.dbcnt) {
		warn("%s: short input record", in.name);
		for (t = in.db + in.dbcnt - 1; t >= in.db && *t == ' '; --t);
		if (t >= in.db) {
			cnt = t - in.db + 1;
			bcopy(in.db, out.dbp, cnt);
			out.dbp += cnt;
			out.dbcnt += cnt;
		}
		++out.dbcnt;
		*out.dbp++ = '\n';
	}
}
