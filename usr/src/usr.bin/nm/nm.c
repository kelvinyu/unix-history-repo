/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hans Huebner.
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
char copyright[] =
"@(#) Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)nm.c	5.2 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <a.out.h>
#include <stab.h>
#include <ar.h>
#include <ranlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <strings.h>

int	ignore_bad_archive_entries = 1;
int	print_only_external_symbols;
int	print_only_undefined_symbols;
int	print_all_symbols;
int	print_file_each_line;
int	cmp_value(), cmp_name();
int	(*sort_func)() = cmp_name;

enum { FORWARD, BACKWARD } sort_direction = FORWARD;
int fcount;

/* some macros for symbol type (nlist.n_type) handling */
#define	IS_DEBUGGER_SYMBOL(x)	((x) & N_STAB)
#define	IS_EXTERNAL(x)		((x) & N_EXT)
#define	SYMBOL_TYPE(x)		((x) & (N_TYPE | N_STAB))

/*
 * main()
 *	parse command line, execute process_file() for each file
 *	specified on the command line.
 */
main(argc, argv)
	int argc;
	char **argv;
{
	extern int optind;
	int ch, errors;

	while ((ch = getopt(argc, argv, "agnopruw")) != EOF) {
		switch (ch) {
		case 'a':
			print_all_symbols = 1;
			break;
		case 'g':
			print_only_external_symbols = 1;
			break;
		case 'n':
			sort_func = cmp_value;
			break;
		case 'o':
			print_file_each_line = 1;
			break;
		case 'p':
			sort_func = NULL;
			break;
		case 'r':
			sort_direction = BACKWARD;
			break;
		case 'u':
			print_only_undefined_symbols = 1;
			break;
		case 'w':
			ignore_bad_archive_entries = 0;
			break;
		case '?':
		default:
			usage();
		}
	}
	fcount = argc - optind;
	argv += optind;

	if (!fcount)
		errors = process_file("a.out");
	else {
		errors = 0;
		do {
			errors |= process_file(*argv);
		} while (*++argv);
	}
	exit(errors);
}

/*
 * process_file()
 *	show symbols in the file given as an argument.  Accepts archive and
 *	object files as input.
 */
process_file(fname)
	char *fname;
{
	struct exec exec_head;
	FILE *fp;
	int retval;
	char magic[SARMAG];
    
	if (!(fp = fopen(fname, "r"))) {
		(void)fprintf(stderr, "nm: cannot read %s.\n", fname);
		return(1);
	}

	if (fcount > 1)
		(void)printf("\n%s:\n", fname);
    
	/*
	 * first check whether this is an object file - read a object
	 * header, and skip back to the beginning
	 */
	if (fread((char *)&exec_head, 1, sizeof(exec_head), fp) !=
	    sizeof(exec_head)) {
		(void)fprintf(stderr, "nm: %s: bad format.\n", fname);
		(void)fclose(fp);
		return(1);
	}
	rewind(fp);

	/* this could be an archive */
	if (N_BADMAG(exec_head)) {
		if (fread(magic, 1, sizeof(magic), fp) != sizeof(magic) ||
		    strncmp(magic, ARMAG, SARMAG)) {
			(void)fprintf(stderr,
			    "nm: %s: not object file or archive.\n", fname);
			(void)fclose(fp);
			return(1);
		}
		retval = show_archive(fname, fp);
	} else
		retval = show_objfile(fname, fp);
	(void)fclose(fp);
	return(retval);
}

/*
 * show_archive()
 *	show symbols in the given archive file
 */
show_archive(fname, fp)
	char *fname;
	FILE *fp;
{
	extern int errno;
	struct ar_hdr ar_head;
	struct exec exec_head;
	off_t esize;
	int i, last_ar_off, rval;
	char *p, *name, *emalloc();
	long atol();

	name = emalloc((u_int)(sizeof(ar_head.ar_name) + strlen(fname) + 3));

	rval = 0;

	/* while there are more entries in the archive */
	while (fread((char *)&ar_head, 1, sizeof(ar_head), fp) ==
	    sizeof(ar_head)) {
		/* bad archive entry - stop processing this archive */
		if (strncmp(ar_head.ar_fmag, ARFMAG, sizeof(ar_head.ar_fmag))) {
			(void)fprintf(stderr,
			    "nm: %s: bad format archive header", fname);
			(void)free(name);
			return(1);
		}

		/*
		 * construct a name of the form "archive.a:obj.o:" for the
		 * current archive entry if the object name is to be printed
		 * on each output line
		 */
		if (print_file_each_line) {
			(void)sprintf(name, "%s:", fname);
			p = name + strlen(name);
		} else
			p = name;
		for (i = 0; i < sizeof(ar_head.ar_name); ++i)
			if (ar_head.ar_name[i] && ar_head.ar_name[i] != ' ')
				*p++ = ar_head.ar_name[i];
		*p++ = '\0';

		/* remember start position of current archive object */
		last_ar_off = ftell(fp);

		/* get and check current object's header */
		if (fread((char *)&exec_head, 1, sizeof(exec_head), fp) !=
		    sizeof(exec_head)) {
			(void)fprintf(stderr, "nm: %s: premature EOF.\n", name);
			(void)free(name);
			return(1);
		}
		if (strcmp(name, RANLIBMAG))
			if (N_BADMAG(exec_head)) {
				if (!ignore_bad_archive_entries) {
					(void)fprintf(stderr,
					    "nm: %s: bad format.\n", name);
					rval = 1;
				}
			} else {
				(void)fseek(fp, (long)-sizeof(exec_head),
				    SEEK_CUR);
				if (!print_file_each_line)
					(void)printf("\n%s:\n", name);
				rval |= show_objfile(name, fp);
			}
		esize = atol(ar_head.ar_size);

		/*
		 * skip to next archive object - esize&1 is added to stay
	 	 * on even starting points relative to the start of the
		 * archive file
		 */
		if (fseek(fp, (long)(last_ar_off + esize + (esize&1)),
		    SEEK_SET)) {
			(void)fprintf(stderr,
			    "nm: %s: %s\n", fname, strerror(errno));
			(void)free(name);
			return(1);
		}
	}
	(void)free(name);
	return(rval);
}

/*
 * show_objfile()
 *	show symbols from the object file pointed to by fp.  The current
 *	file pointer for fp is expected to be at the beginning of an a.out
 *	header.
 */
show_objfile(objname, fp)
	char *objname;
	FILE *fp;
{
	register struct nlist *names;
	register int i, nnames, nrawnames;
	struct exec head;
	long stabsize;
	char *stab, *emalloc();

	/* read a.out header */
	if (fread((char *)&head, sizeof(head), 1, fp) != 1) {
		(void)fprintf(stderr,
		    "nm: %s: cannot read header.\n", objname);
		return(1);
	}

	/*
	 * skip back to the header - the N_-macros return values relative
	 * to the beginning of the a.out header
	 */
	if (fseek(fp, (long)-sizeof(head), SEEK_CUR)) {
		(void)fprintf(stderr,
		    "nm: %s: %s\n", objname, strerror(errno));
		return(1);
	}

	/* stop if this is no valid object file */
	if (N_BADMAG(head)) {
		(void)fprintf(stderr,
		    "nm: %s: bad format.\n", objname);
		return(1);
	}

	/* stop if the object file contains no symbol table */
	if (!head.a_syms) {
		(void)fprintf(stderr,
		    "nm: %s: no name list.\n", objname);
		return(1);
	}

	if (fseek(fp, (long)N_SYMOFF(head), SEEK_CUR)) {
		(void)fprintf(stderr,
		    "nm: %s: %s\n", objname, strerror(errno));
		return(1);
	}

	/* get memory for the symbol table */
	names = (struct nlist *)emalloc((u_int)head.a_syms);
	nrawnames = head.a_syms / sizeof(*names);
	if (fread((char *)names, 1, (int)head.a_syms, fp) != head.a_syms) {
		(void)fprintf(stderr,
		    "nm: %s: cannot read symbol table.\n", objname);
		(void)free((char *)names);
		return(1);
	}

	/*
	 * Following the symbol table comes the string table.  The first
	 * 4-byte-integer gives the total size of the string table
	 * _including_ the size specification itself.
	 */
	if (fread((char *)&stabsize, sizeof(stabsize), 1, fp) != 1) {
		(void)fprintf(stderr,
		    "nm: %s: cannot read stab size.\n", objname);
		(void)free((char *)names);
		return(1);
	}
	stab = emalloc((u_int)stabsize);

	/*
	 * read the string table offset by 4 - all indices into the string
	 * table include the size specification.
	 */
	stabsize -= 4;		/* we already have the size */
	if (fread(stab + 4, 1, (int)stabsize, fp) != stabsize) {
		(void)fprintf(stderr,
		    "nm: %s: stab truncated..\n", objname);
		(void)free((char *)names);
		(void)free(stab);
		return(1);
	}

	/*
	 * fix up the symbol table and filter out unwanted entries
	 *
	 * common symbols are characterized by a n_type of N_UNDF and a
	 * non-zero n_value -- change n_type to N_COMM for all such
	 * symbols to make life easier later.
	 *
	 * filter out all entries which we don't want to print anyway
	 */
	for (i = nnames = 0; i < nrawnames; ++i) {
		if (SYMBOL_TYPE(names[i].n_type) == N_UNDF && names[i].n_value)
			names[i].n_type = N_COMM | (names[i].n_type & N_EXT);
		if (!print_all_symbols && IS_DEBUGGER_SYMBOL(names[i].n_type))
			continue;
		if (print_only_external_symbols &&
		    !IS_EXTERNAL(names[i].n_type))
			continue;
		if (print_only_undefined_symbols &&
		    (SYMBOL_TYPE(names[i].n_type) != N_UNDF))
			continue;

		/*
		 * make n_un.n_name a character pointer by adding the string
		 * table's base to n_un.n_strx
		 *
		 * don't mess with null offsets
		 */
		if (names[i].n_un.n_name)
			names[i].n_un.n_name = stab + names[i].n_un.n_strx;
		else
			names[i].n_un.n_name = "";
		if (nnames != i)
			names[nnames] = names[i];
		++nnames;
	}

	/* sort the symbol table if applicable */
	if (sort_func)
		qsort((char *)names, (int)nnames, sizeof(*names), sort_func);

	/* print out symbols */
	for (i = 0; i < nnames; ++i)
		print_symbol(objname, &names[i]);

	(void)free((char *)names);
	(void)free(stab);
	return(0);
}

/*
 * print_symbol()
 *	show one symbol
 */
print_symbol(objname, sym)
	char *objname;
	struct nlist *sym;
{
	char *typestring(), typeletter();

	if (print_file_each_line)
		printf("%s:", objname);

	/*
	 * handle undefined-only format seperately (no space is
	 * left for symbol values, no type field is printed)
	 */
	if (print_only_undefined_symbols) {
		printf("%s\n", sym->n_un.n_name);
		return;
	}

	/* print symbol's value */
	if (SYMBOL_TYPE(sym->n_type) == N_UNDF)
		(void)printf("        ");
	else
		(void)printf("%08lx", sym->n_value);

	/* print type information */
	if (IS_DEBUGGER_SYMBOL(sym->n_type))
		(void)printf(" - %02x %04x %5s ", sym->n_other,
		    sym->n_desc&0xffff, typestring(sym->n_type));
	else
		(void)printf(" %c ", typeletter(sym->n_type));

	/* print the symbol's name */
	(void)printf("%s\n", sym->n_un.n_name ? sym->n_un.n_name : "");
}

/*
 * typestring()
 *	return the a description string for an STAB entry
 */
char *
typestring(type)
	register u_char type;
{
	switch(type) {
	case N_BCOMM:
		return("BCOMM");
	case N_ECOML:
		return("ECOML");
	case N_ECOMM:
		return("ECOMM");
	case N_ENTRY:
		return("ENTRY");
	case N_FNAME:
		return("FNAME");
	case N_FUN:
		return("FUN");
	case N_GSYM:
		return("GSYM");
	case N_LBRAC:
		return("LBRAC");
	case N_LCSYM:
		return("LCSYM");
	case N_LENG:
		return("LENG");
	case N_LSYM:
		return("LSYM");
	case N_PC:
		return("PC");
	case N_PSYM:
		return("PSYM");
	case N_RBRAC:
		return("RBRAC");
	case N_RSYM:
		return("RSYM");
	case N_SLINE:
		return("SLINE");
	case N_SO:
		return("SO");
	case N_SOL:
		return("SOL");
	case N_SSYM:
		return("SSYM");
	case N_STSYM:
		return("STSYM");
	}
	return("???");
}

/*
 * typeletter()
 *	return a description letter for the given basic type code of an
 *	symbol table entry.  The return value will be upper case for
 *	external, lower case for internal symbols.
 */
char
typeletter(type)
	u_char type;
{
	switch(SYMBOL_TYPE(type)) {
	case N_ABS:
		return(IS_EXTERNAL(type) ? 'A' : 'a');
	case N_BSS:
		return(IS_EXTERNAL(type) ? 'B' : 'b');
	case N_COMM:
		return(IS_EXTERNAL(type) ? 'C' : 'c');
	case N_DATA:
		return(IS_EXTERNAL(type) ? 'D' : 'd');
	case N_FN:
		return(IS_EXTERNAL(type) ? 'F' : 'f');
	case N_TEXT:
		return(IS_EXTERNAL(type) ? 'T' : 't');
	case N_UNDF:
		return(IS_EXTERNAL(type) ? 'U' : 'u');
	}
	return('?');
}

/*
 * cmp_name()
 *	compare two symbols by their names
 */
cmp_name(a, b)
	struct nlist *a, *b;
{
	return(sort_direction == FORWARD ?
	    strcmp(a->n_un.n_name, b->n_un.n_name) :
	    strcmp(b->n_un.n_name, a->n_un.n_name));
}

/*
 * cmp_value()
 *	compare two symbols by their values
 */
cmp_value(a, b)
	struct nlist *a, *b;
{
	if (SYMBOL_TYPE(a->n_type) == N_UNDF)
		if (SYMBOL_TYPE(b->n_type) == N_UNDF)
			return(0);
		else
			return(-1);
	else if (SYMBOL_TYPE(b->n_type) == N_UNDF)
		return(1);
	if (a->n_value == b->n_value)
		return(cmp_name(a, b));
	return(sort_direction == FORWARD ? a->n_value > b->n_value :
	    a->n_value < b->n_value);
}

char *
emalloc(size)
	u_int size;
{
	char *p, *malloc();

	/* NOSTRICT */
	if (!(p = malloc(size))) {
		(void)fprintf(stderr, "nm: no more memory.\n");
		exit(1);
	}
	return(p);
}

usage()
{
	(void)fprintf(stderr, "usage: nm [-agnopruw] [file ...]");
	exit(1);
}
