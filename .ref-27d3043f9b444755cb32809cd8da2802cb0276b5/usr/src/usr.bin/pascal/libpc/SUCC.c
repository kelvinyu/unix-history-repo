/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)SUCC.c 1.1 %G%";

#include "h01errs.h"

SUCC(value, lower, upper)

	int	value;
	int	lower;
	int	upper;
{
	value++;
	if (value < lower || value > upper) {
		ERROR(ERANGE, value);
		return;
	}
	return	value;
}
