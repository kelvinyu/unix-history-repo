#ifndef lint
static	char *sccsid = "@(#)lcmd.c	3.7 83/11/22";
#endif

#include "defs.h"
#include "value.h"
#include "lcmd.h"

int l_buffer();
int l_close();
int l_escape();
int l_label();
int l_select();
int l_source();
int l_terse();
int l_window();
int l_write();

struct lcmd_arg arg_buffer[];
struct lcmd_arg arg_close[];
struct lcmd_arg arg_escape[];
struct lcmd_arg arg_label[];
struct lcmd_arg arg_select[];
struct lcmd_arg arg_source[];
struct lcmd_arg arg_terse[];
struct lcmd_arg arg_window[];
struct lcmd_arg arg_write[];

struct lcmd_tab lcmd_tab[] = {
	"%",		1,	l_select,	arg_select,
	"buffer",	1,	l_buffer,	arg_buffer,
	"close",	1,	l_close,	arg_close,
	"escape",	1,	l_escape,	arg_escape,
	"label",	1,	l_label,	arg_label,
	"select",	2,	l_select,	arg_select,
	"source",	2,	l_source,	arg_source,
	"terse",	1,	l_terse,	arg_terse,
	"window",	2,	l_window,	arg_window,
	"write",	2,	l_write,	arg_write,
	0,		0,	0,		0,
};

struct lcmd_tab *
lcmd_lookup(name)
char *name;
{
	register struct lcmd_tab *p;

	for (p = lcmd_tab; p->lc_name != 0; p++)
		if (str_match(name, p->lc_name, p->lc_minlen))
			return p;
	return 0;
}

dosource(filename)
char *filename;
{
	if (cx_setfile(filename) < 0)
		return -1;
	p_start();
	err_end();
	cx_end();
	return 0;
}

dolongcmd(buffer)
char *buffer;
{
	if (cx_setbuf(buffer) < 0)
		return -1;
	p_start();
	err_end();
	cx_end();
	return 0;
}
