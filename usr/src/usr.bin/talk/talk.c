/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)talk.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include "talk.h"

/*
 * talk:	A visual form of write. Using sockets, a two way 
 *		connection is set up between the two people talking. 
 *		With the aid of curses, the screen is split into two 
 *		windows, and each users text is added to the window,
 *		one character at a time...
 *
 *		Written by Kipp Hickman
 *		
 *		Modified to run under 4.1a by Clem Cole and Peter Moore
 *		Modified to run between hosts by Peter Moore, 8/19/82
 *		Modified to run under 4.1c by Peter Moore 3/17/83
 */

main(argc, argv)
	int argc;
	char *argv[];
{
	get_names(argc, argv);
	init_display();
	open_ctl();
	open_sockt();
	start_msgs();
	if (!check_local())
		invite_remote();
	end_msgs();
	set_edit_chars();
	talk();
}
