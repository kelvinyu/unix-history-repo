/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)lrscan.c	5.3 (Berkeley) 6/18/88";
#endif /* not lint */

# include	"trek.h"

/*
**  LONG RANGE OF SCANNERS
**
**	A summary of the quadrants that surround you is printed.  The
**	hundreds digit is the number of Klingons in the quadrant,
**	the tens digit is the number of starbases, and the units digit
**	is the number of stars.  If the printout is "///" it means
**	that that quadrant is rendered uninhabitable by a supernova.
**	It also updates the "scanned" field of the quadrants it scans,
**	for future use by the "chart" option of the computer.
*/

lrscan()
{
	register int			i, j;
	register struct quad		*q;

	if (check_out(LRSCAN))
	{
		return;
	}
	printf("Long range scan for quadrant %d,%d\n\n", Ship.quadx, Ship.quady);

	/* print the header on top */
	for (j = Ship.quady - 1; j <= Ship.quady + 1; j++)
	{
		if (j < 0 || j >= NQUADS)
			printf("      ");
		else
			printf("     %1d", j);
	}

	/* scan the quadrants */
	for (i = Ship.quadx - 1; i <= Ship.quadx + 1; i++)
	{
		printf("\n  -------------------\n");
		if (i < 0 || i >= NQUADS)
		{
			/* negative energy barrier */
			printf("  !  *  !  *  !  *  !");
			continue;
		}

		/* print the left hand margin */
		printf("%1d !", i);
		for (j = Ship.quady - 1; j <= Ship.quady + 1; j++)
		{
			if (j < 0 || j >= NQUADS)
			{
				/* negative energy barrier again */
				printf("  *  !");
				continue;
			}
			q = &Quad[i][j];
			if (q->stars < 0)
			{
				/* supernova */
				printf(" /// !");
				q->scanned = 1000;
				continue;
			}
			q->scanned = q->klings * 100 + q->bases * 10 + q->stars;
			printf(" %3d !", q->scanned);
		}
	}
	printf("\n  -------------------\n");
	return;
}
