/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms is permitted
 * provided that all copyright information, including this notice,
 * is retained in all such forms, and that any documentation,
 * advertising or other materials related to such distribution and
 * use acknowledge that the software was
 * developed by the University of California, Berkeley.  The name
 * of the University may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * kvm_mkdb -- Create kernel information database for running kernel.
 */

#ifndef lint
static char sccsid[] = "@(#)kvm_mkdb.c	5.1 (Berkeley) %G%";
#endif /* not lint */
#include <sys/param.h>
#include <sys/file.h>
#include <ndbm.h>
#include <a.out.h>
#include <kvm.h>
#include <paths.h>
#include <limits.h>
#include <errno.h>
#include <strings.h>
#include <stdio.h>

char *tmp;
#define basename(cp)	((tmp=rindex((cp), '/')) ? tmp+1 : (cp))
#define USAGE	"kvm_mkdb"
extern errno;

char *progname;

main(argc, argv)
	char *argv[];
{
	DBM *db;
	char *nlistpath, *nlistname;
	char dbtemp[MAXPATHLEN];
	char dbname[MAXPATHLEN];
	extern char *optarg;
	extern int optind;
	int ch;

	progname = argv[0];
	while ((ch = getopt(argc, argv, "")) != EOF)
		switch((char)ch) {
		case '?':
		default:
			fprintf(stderr, "usage: %s", progname, USAGE);
			exit(1);
		}
	argc -= optind;
	argv += optind;

	nlistpath = argc > 1 ? argv[0] : _PATH_UNIX;
	nlistname = basename(nlistpath);
	sprintf(dbtemp, "%s/kvm_tmp%s", KVMDBDIR, nlistname);
	sprintf(dbname, "%s/kvm_%s", KVMDBDIR, nlistname);
	rmdb(dbtemp);
	umask(0);
	if ((db = dbm_open(dbtemp, O_CREAT|O_WRONLY|O_EXCL, 0644)) == NULL)
		syserrexit("error opening dbmfile");
	if (create_knlist(nlistpath, db) == -1)
		errexit("error creating kernel namelist");
	if (create_devnames(db) == -1)
		errexit("error creating devnames");
	rmdb(dbname);
	mvdb(dbtemp, dbname);

	exit(0);
}

rmdb(file)
	char *file;
{
	int len = strlen(file);

	if (len > (MAXPATHLEN - 5))
		errexit("pathname too long: %s", file);
	strcpy(file+len, ".dir");
	if (unlink(file) < 0 && errno != ENOENT)
		syserrexit("can't unlink %s", file);
	strcpy(file+len, ".pag");
	if (unlink(file) < 0 && errno != ENOENT)
		syserrexit("can't unlink %s", file);
	*(file+len) = '\0';
}
	
mvdb(from, to)
	char *from;
	char *to;
{
	int flen = strlen(from);
	int tlen = strlen(to);

	if (flen > (MAXPATHLEN - 5) || tlen > (MAXPATHLEN - 5))
		errexit("pathname too long: %s or %s", from, to);
	strcpy(from+flen, ".dir");
	strcpy(to+tlen, ".dir");
	if (rename(from, to) == -1)
		syserrexit("rename %s to %s", from, to);
	strcpy(from+flen, ".pag");
	strcpy(to+tlen, ".pag");
	if (rename(from, to) == -1)
		syserrexit("rename %s to %s", from, to);
	*(from+flen) = *(to+tlen) = '\0';
}

/* from libc/nlist.c */
#include <unistd.h>

typedef struct nlist NLIST;
#define	_strx	n_un.n_strx
#define	_name	n_un.n_name
#define	ISVALID(p)	(p->_name && p->_name[0])
#define MAXSYMSIZE	256

create_knlist(name, db)
	char *name;
	DBM *db;
{
	register NLIST *p, *s;
	struct exec ebuf;
	FILE *fstr, *fsym;
	NLIST nbuf;
	off_t strings_offset, symbol_offset, symbol_size, lseek();
	char sbuf[MAXSYMSIZE+1];
	register char *bp;
	register int c, len;
	datum key, data;

	if (!(fsym = fopen(name, "r")))
		syserrexit("can't open %s", name);
	if (fread((char *)&ebuf, sizeof(struct exec), 1, fsym) != 1 ||
	    N_BADMAG(ebuf))
		syserrexit("can't read exec");

	symbol_offset = N_SYMOFF(ebuf);
	symbol_size = ebuf.a_syms;
	strings_offset = symbol_offset + symbol_size;

	if (fseek(fsym, symbol_offset, SEEK_SET) == -1)
		syserrexit("can't seek symbol table: %x", symbol_offset);
	if ((fstr = fopen(name, "r")) == NULL)
		syserrexit("can't open %s", name);

	sbuf[0] = KVMDB_NLIST;
	key.dptr = sbuf;
	data.dptr = (char *)&nbuf;
	data.dsize = sizeof (NLIST);

	for (s = &nbuf; symbol_size; symbol_size -= sizeof (NLIST)) {
		if (fread((char *)s, sizeof (NLIST), 1, fsym) != 1)
			syserrexit("can't read nlist entry");
		if (!s->_strx || s->n_type&N_STAB)
			continue;
		if (fseek(fstr, strings_offset + s->_strx, SEEK_SET) == -1)
			syserrexit("can't seek string: %x", 
				strings_offset + s->_strx);
		/*
		 * read string
		 */
		bp = sbuf + 1;
		len = 0;
		while ((c = fgetc(fstr)) != EOF && c != '\0') {
			if (++len == MAXSYMSIZE)
				errexit("string too long");
			*bp++ = c;
		}
		*bp = '\0';
		/*
		 * and store it
		 */
		key.dsize = bp - sbuf;
		if (dbm_store(db, key, data, DBM_INSERT) < 0)
			syserrexit("dbm_store");
		if (strcmp(sbuf+1, "_version") == 0) {
			/* 
			 * store the value of version in VERSION
			 */
			datum vers;
			char versbuf[LINE_MAX];
			long versoff;
			long reloffset;

			/*
			 * Offset relative to start of text image in VM.
			 * On tahoe, first 0x800 is reserved for
			 * communication with the console processor.
			 */
#ifdef tahoe
			reloffset = ((s->n_value & ~KERNBASE) - 0x800);
#endif
#ifdef vax
			reloffset = (s->n_value & ~KERNBASE);
#endif
			/*
			 * When loaded, data is rounded
			 * to next 1024 after text, but not in file.
			 */
			reloffset -= 1024 - (ebuf.a_text % 1024);
			versoff = N_TXTOFF(ebuf) + reloffset;
			if (fseek(fstr, versoff, SEEK_SET) == -1)
				syserrexit("seek (version): %x", s->n_value);
			/*
			 * Just read version string up to, and
			 * including newline.
			 */
			if (fgets(versbuf, LINE_MAX, fstr) == NULL)
				syserrexit("can't read version");
			strcpy(sbuf+1, "VERSION");
			key.dsize = (sizeof ("VERSION") - 1) + 1;
			vers.dptr = versbuf;
			vers.dsize = strlen(versbuf);
			if (dbm_store(db, key, vers, DBM_INSERT) < 0)
				syserrexit("dbm_store: can't store VERSION");
		}
	}
	(void)fclose(fstr);
	(void)fclose(fsym);
	return (0);
}

create_devnames() {}

#include <varargs.h>

warning(va_alist)
	va_dcl
{
	char *fmt;
	va_list ap;

	fprintf(stderr, "%s: warning: ", progname);
	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}


errexit(va_alist)
	va_dcl
{
	char *fmt;
	va_list ap;

	fprintf(stderr, "%s: ", progname);
	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(1);
}


syserrexit(va_alist)
	va_dcl
{
	char *fmt;
	va_list ap;

	fprintf(stderr, "%s: ", progname);
	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, ": %s\n", strerror(errno));
	exit(1);
}
