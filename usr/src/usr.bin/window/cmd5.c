#ifndef lint
static	char *sccsid = "@(#)cmd5.c	2.1 83/07/30";
#endif

#include "defs.h"

struct ww *doopen();
struct ww *idtowin();

int lineno;				/* current line number in source file */
static char *argv[100];			/* one line broken up into words */
static int argc;
static char insource;

int s_window();
int s_select();
int s_escape();
int s_label();
int s_terse();
int s_refresh();
int s_source();
int s_write();

struct scmd {
	char *s_name;			/* name of command */
	int s_lmin;			/* minimum length to check */
	int s_lmax;			/* maximum length to check */
	int s_amin;			/* minimum argument */
	int s_amax;			/* maximum argument */
	int (*s_func)();		/* the function */
};
static struct scmd scmd[] = {
	"%",		1, 1, 0, 0, s_select,
	"escape",	1, 0, 1, 1, s_escape,
	"label",	1, 0, 2, 2, s_label,
	"refresh",	1, 0, 1, 2, s_refresh,
	"source",	1, 0, 1, 1, s_source,
	"terse",	1, 0, 0, 1, s_terse,
	"window",	1, 0, 4, 4, s_window,
	"write",	2, 0, 2, 2, s_write,
	0
};

dosource(filename)
char *filename;
{
	register FILE *f;
	char buf[BUFSIZ];

	if ((f = fopen(filename, "r")) == 0)
		return -1;
	insource++;
	beginerror(filename);
	for (lineno = 1; fgets(buf, sizeof buf, f) != 0; lineno++)
		dolongcmd(buf);
	enderror();
	insource = 0;
	return 0;
}

dolongcmd(line)
char *line;
{
	register struct scmd *sp;
	register len;

	makeargv(line);
	if (argc == 0)
		return;
	for (sp = scmd; sp->s_name; sp++) {
		len = strlen(*argv);
		if (len < sp->s_lmin)
			continue;
		if (!strncmp(*argv, sp->s_name, sp->s_lmax ? sp->s_lmax : len))
			break;
	}
	if (sp->s_name) {
		if (sp->s_amin > argc - 1)
			error("Too few arguments.");
		else if (sp->s_amax < argc - 1)
			error("Too many arguments.");
		else
			(*sp->s_func)();
	} else
		error("%s: Unknown command.", *argv);
}

makeargv(p)
register char *p;
{
	static char buf[BUFSIZ];
	register char *q = buf, **pp = argv;
	char quote = 0, escape = 0;
	int i;

	for (; *p == ' ' || *p == '\t'; p++)
		;
	while (*p && *p != '\n' && *p != '#'
	       && pp < &argv[sizeof argv/sizeof *argv - 1]) {
		*pp++ = q;
		while (*p && *p != '\n') {
			if (escape) {
				switch (*p) {
				case 'n':
					*q++ = '\n';
					p++;
					break;
				case 'r':
					*q++ = '\r';
					p++;
					break;
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					*q = 0;
					for (i = 3; --i >= 0
					     && *p >= '0' && *p <= '9';)
						*q = *q << 3 | *p++ - '0';
					q++;
					break;
				default:
					*q++ = *p++;
					break;
				}
				escape = 0;
			} else if (*p == '\\') {
				escape = 1;
				p++;
			} else if (quote) {
				if (*p == quote) {
					quote = 0;
					p++;
				} else
					*q++ = *p++;
			} else {
				if (*p == '"' || *p == '\'')
					quote = *p++;
				else if (*p == ' ' || *p == '\t')
					break;
				else
					*q++ = *p++;
			}
		}
		*q++ = 0;
		for (; *p == ' ' || *p == '\t'; p++)
			;
	}
	*pp = 0;
	argc = pp - argv;
}

s_window()
{
	register char **pp = argv;
	register struct ww *w;
	int col, row, ncol, nrow, id;

	if ((id = findid()) < 0) {
		error("Too many windows.");
		return;
	}
	if (**++pp == '*')
		row = 0;
	else
		row = atoi(*pp);
	if (**++pp == '*')
		col = 0;
	else
		col = atoi(*pp);
	if (**++pp == '*')
		nrow = wwnrow - row;
	else
		nrow = atoi(*pp);
	if (**++pp == '*')
		ncol = wwncol - col;
	else
		ncol = atoi(*pp);
	w = doopen(id, nrow, ncol, row, col);
	if (w == 0)
		error("Can't open window: row %d col %d, %d rows %d cols.",
			row, col, nrow, ncol);
}

s_select()
{
	struct ww *w;

	if ((w = idtowin(*argv + 1)) == 0)
		return;
	setselwin(w);
}

s_escape()
{
	setescape(argv[1]);
}

s_label()
{
	struct ww *w;

	if ((w = idtowin(argv[1])) == 0)
		return;
	setlabel(w, argv[2]);
}

s_terse()
{
	if (argc < 2)
		terse = 1;
	else if (strcmp(argv[1], "off") == 0)
		terse = 0;
	else
		terse = 1;
	if (terse)
		Whide(cmdwin->ww_win);
	else
		Wunhide(cmdwin->ww_win);
}

s_refresh()
{
	struct ww *w;

	if ((w = idtowin(argv[1])) == 0)
		return;
	if (argc < 3)
		w->ww_refresh = 1;
	else if (strcmp(argv[2], "off") == 0)
		w->ww_refresh = 0;
	else
		w->ww_refresh = 1;
}

s_source()
{
	if (insource) {
		error("Recursive source.");
		return;
	}
	if (dosource(argv[1]) < 0)
		error("Can't open %s.", argv[1]);
}

s_write()
{
	struct ww *w;

	if ((w = idtowin(argv[1])) == 0)
		return;
	write(w->ww_pty, argv[2], strlen(argv[2]));
}

struct ww *
idtowin(idstr)
char *idstr;
{
	int id;
	struct ww *w = 0;

	id = atoi(idstr);
	if (id < 1 || id > 9 || (w = wwfind(id)) == 0)
		error("%d: No such window.", id);
	return w;
}
