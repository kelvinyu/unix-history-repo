#ifndef lint
static char *rcsid_Pause_c = "$Header: Pause.c,v 10.4 86/11/19 16:24:19 jg Rel $";
#endif	lint

/*
 *			COPYRIGHT 1985, 1986
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITIBILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission.
 *
 */


/*
 * MODIFICATION HISTORY
 *
 * 000 -- M. Gancarz, DEC Ultrix Engineering Group
 */

#ifndef lint
static char *sccsid = "@(#)Pause.c	3.8	1/24/86";
#endif

#include "uwm.h"

Bool Pause(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
short button;                           /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    XGrabServer();
    return(FALSE);
}

Bool Continue(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
short button;                           /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    XUngrabServer();
    return(FALSE);
}
