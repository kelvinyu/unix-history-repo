/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Michael Fischbein.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)print.c	5.27 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <utmp.h>
#include <tzfile.h>
#include <stdio.h>
#include "ls.h"
#include "extern.h"

static int	printaname __P((LS *));
static void	printlink __P((char *));
static void	printtime __P((time_t));
static int	printtype __P((u_int));
static char	*flags_from_fid __P((long));

void
printscol(stats, num)
	register LS *stats;
	register int num;
{
	for (; num--; ++stats) {
		(void)printaname(stats);
		(void)putchar('\n');
	}
}

void
printlong(stats, num)
	LS *stats;
	register int num;
{
	char modep[15], *user_from_uid(), *group_from_gid();

	if (f_total)
		(void)printf("total %lu\n", f_kblocks ?
		    howmany(stats[0].lstat.st_btotal, 2) :
		    stats[0].lstat.st_btotal);
	for (; num--; ++stats) {
		if (f_inode)
			(void)printf("%6lu ", stats->lstat.st_ino);
		if (f_size)
			(void)printf("%4ld ", f_kblocks ?
			    howmany(stats->lstat.st_blocks, 2) :
			    stats->lstat.st_blocks);
		(void)strmode(stats->lstat.st_mode, modep);
		(void)printf("%s %3u %-*s ", modep, stats->lstat.st_nlink,
		    UT_NAMESIZE, user_from_uid(stats->lstat.st_uid, 0));
		if (f_group)
			(void)printf("%-*s ", UT_NAMESIZE,
			    group_from_gid(stats->lstat.st_gid, 0));
		if (f_flags)
			(void)printf("%-*s ", FLAGSWIDTH,
			    flags_from_fid(stats->lstat.st_flags));
		if (S_ISCHR(stats->lstat.st_mode) ||
		    S_ISBLK(stats->lstat.st_mode))
			(void)printf("%3d, %3d ", major(stats->lstat.st_rdev),
			    minor(stats->lstat.st_rdev));
		else
			(void)printf("%8ld ", stats->lstat.st_size);
		if (f_accesstime)
			printtime(stats->lstat.st_atime);
		else if (f_statustime)
			printtime(stats->lstat.st_ctime);
		else
			printtime(stats->lstat.st_mtime);
		(void)printf("%s", stats->name);
		if (f_type)
			(void)printtype(stats->lstat.st_mode);
		if (S_ISLNK(stats->lstat.st_mode))
			printlink(stats->name);
		(void)putchar('\n');
	}
}

#define	TAB	8

void
printcol(stats, num)
	LS *stats;
	int num;
{
	extern int termwidth;
	register int base, chcnt, cnt, col, colwidth;
	int endcol, numcols, numrows, row;

	colwidth = stats[0].lstat.st_maxlen;
	if (f_inode)
		colwidth += 6;
	if (f_size)
		colwidth += 5;
	if (f_type)
		colwidth += 1;

	colwidth = (colwidth + TAB) & ~(TAB - 1);
	if (termwidth < 2 * colwidth) {
		printscol(stats, num);
		return;
	}

	numcols = termwidth / colwidth;
	numrows = num / numcols;
	if (num % numcols)
		++numrows;

	if (f_size && f_total)
		(void)printf("total %lu\n", f_kblocks ?
		    howmany(stats[0].lstat.st_btotal, 2) :
		    stats[0].lstat.st_btotal);
	for (row = 0; row < numrows; ++row) {
		endcol = colwidth;
		for (base = row, chcnt = col = 0; col < numcols; ++col) {
			chcnt += printaname(stats + base);
			if ((base += numrows) >= num)
				break;
			while ((cnt = (chcnt + TAB & ~(TAB - 1))) <= endcol) {
				(void)putchar('\t');
				chcnt = cnt;
			}
			endcol += colwidth;
		}
		putchar('\n');
	}
}

/*
 * print [inode] [size] name
 * return # of characters printed, no trailing characters
 */
static int
printaname(lp)
	LS *lp;
{
	int chcnt;

	chcnt = 0;
	if (f_inode)
		chcnt += printf("%5lu ", lp->lstat.st_ino);
	if (f_size)
		chcnt += printf("%4ld ", f_kblocks ?
		    howmany(lp->lstat.st_blocks, 2) : lp->lstat.st_blocks);
	chcnt += printf("%s", lp->name);
	if (f_type)
		chcnt += printtype(lp->lstat.st_mode);
	return (chcnt);
}

static void
printtime(ftime)
	time_t ftime;
{
	int i;
	char *longstring;

	longstring = ctime((time_t *)&ftime);
	for (i = 4; i < 11; ++i)
		(void)putchar(longstring[i]);

#define	SIXMONTHS	((DAYSPERNYEAR / 2) * SECSPERDAY)
	if (f_sectime)
		for (i = 11; i < 24; i++)
			(void)putchar(longstring[i]);
	else if (ftime + SIXMONTHS > time((time_t *)NULL))
		for (i = 11; i < 16; ++i)
			(void)putchar(longstring[i]);
	else {
		(void)putchar(' ');
		for (i = 20; i < 24; ++i)
			(void)putchar(longstring[i]);
	}
	(void)putchar(' ');
}

static int
printtype(mode)
	u_int mode;
{
	switch(mode & S_IFMT) {
	case S_IFDIR:
		(void)putchar('/');
		return (1);
	case S_IFLNK:
		(void)putchar('@');
		return (1);
	case S_IFSOCK:
		(void)putchar('=');
		return (1);
	}
	if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
		(void)putchar('*');
		return (1);
	}
	return (0);
}

static void
printlink(name)
	char *name;
{
	int lnklen;
	char path[MAXPATHLEN + 1];

	if ((lnklen = readlink(name, path, MAXPATHLEN)) == -1) {
		(void)fprintf(stderr, "\nls: %s: %s\n", name, strerror(errno));
		return;
	}
	path[lnklen] = '\0';
	(void)printf(" -> %s", path);
}

static char *
flags_from_fid(flags)
	long flags;
{
	static char buf[FLAGSWIDTH + 1];
	int size;

	size = FLAGSWIDTH;
	buf[0] = '\0';
	if (size && (flags & NODUMP)) {
		strncat(buf, "nodump", size);
		size -= 6;
	} else {
		strncat(buf, "dump", size);
		size -= 4;
	}
	if (size && (flags & IMMUTABLE)) {
		strncat(buf, ",nochg", size);
		size -= 6;
	}
	if (size && (flags & ARCHIVED)) {
		strncat(buf, ",arch", size);
		size -= 5;
	}
	return (buf);
}
