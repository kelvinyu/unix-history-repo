/*
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)types.h	8.1 (Berkeley) %G%
 */

typedef struct {
    char *modedescriptions;
    char modetype;
} Modelist;

extern Modelist modelist[];

typedef struct {
    int
	system,			/* what the current time is */
	echotoggle,		/* last time user entered echo character */
	modenegotiated,		/* last time operating mode negotiated */
	didnetreceive,		/* last time we read data from network */
	gotDM;			/* when did we last see a data mark */
} Clocks;

extern Clocks clocks;
