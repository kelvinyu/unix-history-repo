/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)odsyntax.c	5.1 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <stdio.h>
#include "hexdump.h"

int deprecated;

oldsyntax(argc, argvp)
	int argc;
	char ***argvp;
{
	extern enum _vflag vflag;
	extern FS *fshead;
	extern char *optarg;
	extern int length, optind;
	int ch;
	char **argv;

	deprecated = 1;
	argv = *argvp;
	while ((ch = getopt(argc, argv, "aBbcDdeFfHhIiLlOoPpswvXx")) != EOF)
		switch (ch) {
		case 'a':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 16/1 \"%3_u \" \"\\n\"");
			break;
		case 'B':
		case 'o':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 8/2 \"%06o \" \"\\n\"");
			break;
		case 'b':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 16/1 \"%03o \" \"\\n\"");
			break;
		case 'c':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 16/1 \"%3_c \" \"\\n\"");
			break;
		case 'd':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 8/2 \"%05u \" \"\\n\"");
			break;
		case 'D':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 4/4 \"%010u \" \"\\n\"");
			break;
		case 'e':		/* undocumented in od */
		case 'F':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 2/8 \"%21.14e \" \"\\n\"");
			break;
			
		case 'f':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 4/4 \"%14.7e \" \"\\n\"");
			break;
		case 'H':
		case 'X':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 4/4 \"%08x \" \"\\n\"");
			break;
		case 'h':
		case 'x':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 8/2 \"%04x \" \"\\n\"");
			break;
		case 'I':
		case 'L':
		case 'l':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 4/4 \"%11d \" \"\\n\"");
			break;
		case 'i':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 8/2 \"%6d \" \"\\n\"");
			break;
		case 'O':
			add("\"%07.7_Ao\n\"");
			add("\"%07.7_ao  \" 4/4 \"%011o \" \"\\n\"");
			break;
		case 'v':
			vflag = ALL;
			break;
		case 'P':
		case 'p':
		case 's':
		case 'w':
		case '?':
		default:
			(void)fprintf(stderr,
			    "od: od(1) has been deprecated for hexdump(1).\n");
			if (ch != '?')
				(void)fprintf(stderr,
"od: hexdump(1) compatibility doesn't support the -%c option%s\n",
				    ch, ch == 's' ? "; see strings(1)." : ".");
			usage();
		}

	if (!fshead) {
		add("\"%07.7_Ao\n\"");
		add("\"%07.7_ao  \" 8/2 \"%06o \" \"\\n\"");
	}

	*argvp += optind;
}
