/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)SUCC.c 1.3 %G%";

#include "h01errs.h"

long
SUCC(value, lower, upper)

	long	value;
	long	lower;
	long	upper;
{
	if (value == upper) {
		ERROR("Cannot take succ of last element of a range\n");
		return;
	}
	value++;
	if (value < lower || value > upper) {
		ERROR("Value of %D is out of range\n", value);
		return;
	}
	return	value;
}
