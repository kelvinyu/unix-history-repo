#ifndef lint
static	char *sccsid = "@(#)wwdata.c	3.2 83/08/11";
#endif

#include "ww.h"

struct ww wwhead = {
	&wwhead, &wwhead
};

struct ww_tty wwnewtty = {
	{ 0, 0, -1, -1, 0 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	0, 0, 0
};

int tt_h19();
int tt_generic();
struct tt_tab tt_tab[] = {
	{ "h19", 3, tt_h19 },
	{ "generic", 0, tt_generic },
	0
};

int wwbaudmap[] = {
	0,
	50,
	75,
	110,
	134,
	150,
	200,
	300,
	600,
	1200,
	1800,
	2400,
	4800,
	9600,
	19200,
	38400
};
