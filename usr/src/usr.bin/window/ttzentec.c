#ifndef lint
static char sccsid[] = "@(#)ttzentec.c	3.1 %G%";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "ww.h"
#include "tt.h"

/*
 * Zentec 1021
 *
 * We let the termcap entry specify how to enter and exit graphics mode,
 * since it varies with what the terminal is emulating.
 */

#define G (WWM_GRP << WWC_MSHIFT)
short zentec_frame[16] = {
	' ',	'x'|G,	'q'|G,	'm'|G,
	'x'|G,	'x'|G,	'l'|G,	't'|G,
	'q'|G,	'j'|G,	'q'|G,	'v'|G,
	'k'|G,	'u'|G,	'w'|G,	'n'|G
};

tt_zentec()
{
	if (tt_generic() < 0)
		return -1;
	if (tt.tt_availmodes | WWM_GRP)
		tt.tt_frame = zentec_frame;
	return 0;
}
