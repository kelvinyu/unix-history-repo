/*
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratories.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)rcons_font.c	7.2 (Berkeley) %G%
 *
 * from: $Header: rcons_font.c,v 1.8 92/06/17 06:23:37 torek Exp $
 */

#ifdef KERNEL
#include "sys/param.h"
#include "sys/kernel.h"
#include "sys/fbio.h"
#include "sys/device.h"
#include "machine/fbvar.h"
#else
#include <sys/types.h>
#include "myfbdevice.h"
#endif

#include "raster.h"

#include "gallant19.h"

void
rcons_font(fb)
	register struct fbdevice *fb;
{

	/* XXX really rather get this from the prom */
	fb->fb_font = &gallant19;

	/* Get distance to top and bottom of font from font origin */
	fb->fb_font_ascent = -(fb->fb_font->chars)['a'].homey;
}
