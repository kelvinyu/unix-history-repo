/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)display.c	5.12 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hexdump.h"

enum _vflag vflag = FIRST;

static off_t address;			/* address/offset in stream */
static off_t eaddress;			/* end address */

#define PRINT { \
	switch(pr->flags) { \
	case F_ADDRESS: \
		(void)printf(pr->fmt, address); \
		break; \
	case F_BPAD: \
		(void)printf(pr->fmt, ""); \
		break; \
	case F_C: \
		conv_c(pr, bp); \
		break; \
	case F_CHAR: \
		(void)printf(pr->fmt, *bp); \
		break; \
	case F_DBL: { \
		double dval; \
		float fval; \
		switch(pr->bcnt) { \
		case 4: \
			bcopy((char *)bp, (char *)&fval, sizeof(fval)); \
			(void)printf(pr->fmt, fval); \
			break; \
		case 8: \
			bcopy((char *)bp, (char *)&dval, sizeof(dval)); \
			(void)printf(pr->fmt, dval); \
			break; \
		} \
		break; \
	} \
	case F_INT: { \
		int ival; \
		short sval; \
		switch(pr->bcnt) { \
		case 1: \
			(void)printf(pr->fmt, (int)*bp); \
			break; \
		case 2: \
			bcopy((char *)bp, (char *)&sval, sizeof(sval)); \
			(void)printf(pr->fmt, (int)sval); \
			break; \
		case 4: \
			bcopy((char *)bp, (char *)&ival, sizeof(ival)); \
			(void)printf(pr->fmt, ival); \
			break; \
		} \
		break; \
	} \
	case F_P: \
		(void)printf(pr->fmt, isprint(*bp) ? *bp : '.'); \
		break; \
	case F_STR: \
		(void)printf(pr->fmt, (char *)bp); \
		break; \
	case F_TEXT: \
		(void)printf(pr->fmt); \
		break; \
	case F_U: \
		conv_u(pr, bp); \
		break; \
	case F_UINT: { \
		u_int ival; \
		u_short sval; \
		switch(pr->bcnt) { \
		case 1: \
			(void)printf(pr->fmt, (u_int)*bp); \
			break; \
		case 2: \
			bcopy((char *)bp, (char *)&sval, sizeof(sval)); \
			(void)printf(pr->fmt, (u_int)sval); \
			break; \
		case 4: \
			bcopy((char *)bp, (char *)&ival, sizeof(ival)); \
			(void)printf(pr->fmt, ival); \
			break; \
		} \
		break; \
	} \
	} \
}

display()
{
	extern FU *endfu;
	register FS *fs;
	register FU *fu;
	register PR *pr;
	register int cnt;
	register u_char *bp;
	off_t saveaddress;
	u_char savech, *savebp, *get();

	while (bp = get())
	    for (fs = fshead, savebp = bp, saveaddress = address; fs;
		fs = fs->nextfs, bp = savebp, address = saveaddress)
		    for (fu = fs->nextfu; fu; fu = fu->nextfu) {
			if (fu->flags&F_IGNORE)
				break;
			for (cnt = fu->reps; cnt; --cnt)
			    for (pr = fu->nextpr; pr; address += pr->bcnt,
				bp += pr->bcnt, pr = pr->nextpr) {
				    if (eaddress && address >= eaddress &&
					!(pr->flags&(F_TEXT|F_BPAD)))
					    bpad(pr);
				    if (cnt == 1 && pr->nospace) {
					savech = *pr->nospace;
					*pr->nospace = '\0';
				    }
				    PRINT;
				    if (cnt == 1 && pr->nospace)
					*pr->nospace = savech;
			    }
		    }
	if (endfu) {
		/*
		 * if eaddress not set, error or file size was multiple of
		 * blocksize, and no partial block ever found.
		 */
		if (!eaddress) {
			if (!address)
				return;
			eaddress = address;
		}
		for (pr = endfu->nextpr; pr; pr = pr->nextpr)
			switch(pr->flags) {
			case F_ADDRESS:
				(void)printf(pr->fmt, eaddress);
				break;
			case F_TEXT:
				(void)printf(pr->fmt);
				break;
			}
	}
}

bpad(pr)
	PR *pr;
{
	static char *spec = " -0+#";
	register char *p1, *p2;

	/*
	 * remove all conversion flags; '-' is the only one valid
	 * with %s, and it's not useful here.
	 */
	pr->flags = F_BPAD;
	*pr->cchar = 's';
	for (p1 = pr->fmt; *p1 != '%'; ++p1);
	for (p2 = ++p1; *p1 && index(spec, *p1); ++p1);
	while (*p2++ = *p1++);
}

static char **_argv;

u_char *
get()
{
	extern enum _vflag vflag;
	extern int length;
	static int ateof = 1;
	static u_char *curp, *savp;
	register int n;
	int need, nread;
	u_char *tmpp;

	if (!curp) {
		curp = (u_char *)emalloc(blocksize);
		savp = (u_char *)emalloc(blocksize);
	} else {
		tmpp = curp;
		curp = savp;
		savp = tmpp;
		address += blocksize;
	}
	for (need = blocksize, nread = 0;;) {
		/*
		 * if read the right number of bytes, or at EOF for one file,
		 * and no other files are available, zero-pad the rest of the
		 * block and set the end flag.
		 */
		if (!length || ateof && !next((char **)NULL)) {
			if (need == blocksize)
				return((u_char *)NULL);
			if (vflag != ALL && !bcmp(curp, savp, nread)) {
				if (vflag != DUP)
					(void)printf("*\n");
				return((u_char *)NULL);
			}
			bzero((char *)curp + nread, need);
			eaddress = address + nread;
			return(curp);
		}
		n = fread((char *)curp + nread, sizeof(u_char),
		    length == -1 ? need : MIN(length, need), stdin);
		if (!n) {
			if (ferror(stdin))
				(void)fprintf(stderr, "hexdump: %s: %s\n",
				    _argv[-1], strerror(errno));
			ateof = 1;
			continue;
		}
		ateof = 0;
		if (length != -1)
			length -= n;
		if (!(need -= n)) {
			if (vflag == ALL || vflag == FIRST ||
			    bcmp(curp, savp, blocksize)) {
				if (vflag == DUP || vflag == FIRST)
					vflag = WAIT;
				return(curp);
			}
			if (vflag == WAIT)
				(void)printf("*\n");
			vflag = DUP;
			address += blocksize;
			need = blocksize;
			nread = 0;
		}
		else
			nread += n;
	}
}

extern off_t skip;			/* bytes to skip */

next(argv)
	char **argv;
{
	extern int errno, exitval;
	static int done;
	int statok;

	if (argv) {
		_argv = argv;
		return(1);
	}
	for (;;) {
		if (*_argv) {
			if (!(freopen(*_argv, "r", stdin))) {
				(void)fprintf(stderr, "hexdump: %s: %s\n",
				    *_argv, strerror(errno));
				exitval = 1;
				++_argv;
				continue;
			}
			statok = done = 1;
		} else {
			if (done++)
				return(0);
			statok = 0;
		}
		if (skip)
			doskip(statok ? *_argv : "stdin", statok);
		if (*_argv)
			++_argv;
		if (!skip)
			return(1);
	}
	/* NOTREACHED */
}

doskip(fname, statok)
	char *fname;
	int statok;
{
	register int cnt;
	struct stat sb;

	if (statok) {
		if (fstat(fileno(stdin), &sb)) {
			(void)fprintf(stderr, "hexdump: %s: %s.\n",
			    fname, strerror(errno));
			exit(1);
		}
		if (S_ISREG(sb.st_mode) && skip >= sb.st_size) {
			address += sb.st_size;
			skip -= sb.st_size;
			return;
		}
	}
	if (S_ISREG(sb.st_mode)) {
		if (fseek(stdin, skip, SEEK_SET)) {
			(void)fprintf(stderr, "hexdump: %s: %s.\n",
			    fname, strerror(errno));
			exit(1);
		}
		address += skip;
		skip = 0;
	} else {
		for (cnt = 0; cnt < skip; ++cnt)
			if (getchar() == EOF)
				break;
		address += cnt;
		skip -= cnt;
	}
}

char *
emalloc(size)
	int size;
{
	char *p;

	if (!(p = malloc((u_int)size)))
		nomem();
	bzero(p, size);
	return(p);
}

nomem()
{
	extern int errno;

	(void)fprintf(stderr, "hexdump: %s.\n", strerror(errno));
	exit(1);
}
