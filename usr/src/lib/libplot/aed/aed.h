/*	aed.h	4.1	83/11/11	*/
/*
 * Displays plot files on an AED512 graphics terminal.
 */

#include <stdio.h>
#include <sgtty.h>

extern char dbuf[BUFSIZ];	/* Used to buffer display characters */
extern struct sgttyb sgttyb;	/* Used to save terminal control bits */
extern curx, cury;		/* Current screen position */
extern int xbot, ybot;		/* Coordinates of screen lower-left corner */
extern int scale;		/* The number of pixels per 2**12 units
				 * of world coordinates.
				 */

/* The following variables describe the screen. */

#define GRXMAX	511	/* Maximum x-coordinate of screen */
#define GRYMAX	482	/* Maximum y-coordinate of screen */
