#ifndef lint
static char sccsid[] = "@(#)point.c	4.1 (Berkeley) %G%";
#endif

point(xi, yi)
int xi, yi;
{
	move(xi, yi);
	label(".");
}
