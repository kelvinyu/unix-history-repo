/*
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)symorder.c	5.5 (Berkeley) %G%";
#endif /* not lint */

/*
 * symorder - reorder symbol table
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <a.out.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPACE 500

struct	nlist order[SPACE];

struct	exec exec;
struct	stat stb;
struct	nlist *newtab, *symtab;
off_t	sa;
int	nsym, strtabsize, symfound; 
char	*kfile, *newstrings, *strings, asym[BUFSIZ];

main(argc, argv)
	int argc;
	char **argv;
{
	register struct nlist *p, *symp;
	register FILE *f;
	register int i;
	register char *start, *t;
	int n, o;

	if (argc != 3) {
		(void)fprintf(stderr, "usage: symorder orderlist file\n");
		exit(1);
	}
	if ((f = fopen(argv[1], "r")) == NULL)
		error(argv[1]);

	for (p = order; fgets(asym, sizeof(asym), f) != NULL;) {
		for (t = asym; isspace(*t); ++t);
		if (!*(start = t))
			continue;
		while (*++t);
		if (*--t == '\n')
			*t = '\0';
		p->n_un.n_name = strdup(start);
		++p;
		++nsym;
	}
	(void)fclose(f);

	kfile = argv[2];
	if ((f = fopen(kfile, "r")) == NULL)
		error(kfile);
	if ((o = open(kfile, O_WRONLY)) < 0)
		error(kfile);

	/* read exec header */
	if ((fread(&exec, sizeof(exec), 1, f)) != 1)
		badfmt("no exec header");
	if (N_BADMAG(exec))
		badfmt("bad magic number");
	if (exec.a_syms == 0)
		badfmt("stripped");
	(void)fstat(fileno(f), &stb);
	if (stb.st_size < N_STROFF(exec) + sizeof(off_t))
		badfmt("no string table");

	/* seek to and read the symbol table */
	sa = N_SYMOFF(exec);
	(void)fseek(f, sa, SEEK_SET);
	n = exec.a_syms;
	if (!(symtab = (struct nlist *)malloc(n)))
		error((char *)NULL);
	if (fread((char *)symtab, 1, n, f) != n)
		badfmt("corrupted symbol table");

	/* read string table size and string table */
	if (fread((char *)&strtabsize, sizeof(int), 1, f) != 1 ||
	    strtabsize <= 0)
		badfmt("corrupted string table");
	strings = malloc(strtabsize);
	if (strings == (char *)NULL)
		error((char *)NULL);
	/*
	 * Subtract four from strtabsize since strtabsize includes itself,
	 * and we've already read it.
	 */
	if (fread(strings, 1, strtabsize - sizeof(int), f) !=
	    strtabsize - sizeof(int))
		badfmt("corrupted string table");

	newtab = (struct nlist *)malloc(n);
	if (newtab == (struct nlist *)NULL)
		error((char *)NULL);

	i = n / sizeof(struct nlist);
	reorder(symtab, newtab, i);
	free((char *)symtab);
	symtab = newtab;

	newstrings = malloc(strtabsize);
	if (newstrings == (char *)NULL)
		error((char *)NULL);
	t = newstrings;
	for (symp = symtab; --i >= 0; symp++) {
		if (symp->n_un.n_strx == 0)
			continue;
		symp->n_un.n_strx -= sizeof(int);
		(void)strcpy(t, &strings[symp->n_un.n_strx]);
		symp->n_un.n_strx = (t - newstrings) + sizeof(int);
		t += strlen(t) + 1;
	}

	(void)lseek(o, sa, SEEK_SET);
	if (write(o, (char *)symtab, n) != n)
		error(kfile);
	if (write(o, (char *)&strtabsize, sizeof(int)) != sizeof(int)) 
		error(kfile);
	if (write(o, newstrings, strtabsize - sizeof(int)) !=
	    strtabsize - sizeof(int))
		error(kfile);
	if ((i = nsym - symfound) > 0) {
		(void)printf("symorder: %d symbol%s not found:\n",
		    i, i == 1 ? "" : "s");
		for (i = 0; i < nsym; i++)
			if (order[i].n_value == 0)
				printf("%s\n", order[i].n_un.n_name);
	}
	exit(0);
}

reorder(st1, st2, entries)
	register struct nlist *st1, *st2;
	int entries;
{
	register struct nlist *p;
	register int i, n;

	for (p = st1, n = entries; --n >= 0; ++p)
		if (inlist(p) != -1)
			++symfound; 
	for (p = st2 + symfound, n = entries; --n >= 0; ++st1) {
		i = inlist(st1);
		if (i == -1)
			*p++ = *st1;
		else
			st2[i] = *st1;
	}
}

inlist(p)
	register struct nlist *p;
{
	register char *nam;
	register struct nlist *op;

	if (p->n_type & N_STAB)
		return (-1);
	if (p->n_un.n_strx == 0)
		return (-1);

	if (p->n_un.n_strx >= strtabsize)
		badfmt("corrupted symbol table");

	nam = &strings[p->n_un.n_strx - sizeof(int)];
	for (op = &order[nsym]; --op >= order; ) {
		if (strcmp(op->n_un.n_name, nam) != 0)
			continue;
		op->n_value = 1;
		return (op - order);
	}
	return (-1);
}

badfmt(why)
	char *why;
{
	(void)fprintf(stderr,
	    "symorder: %s: %s: %s\n", kfile, why, strerror(EFTYPE));
	exit(1);
}

error(n)
	char *n;
{
	int sverr;

	sverr = errno;
	(void)fprintf(stderr, "symorder: ");
	if (n)
		(void)fprintf(stderr, "%s: ", n);
	(void)fprintf(stderr, "%s\n", strerror(sverr));
	exit(1);
}
