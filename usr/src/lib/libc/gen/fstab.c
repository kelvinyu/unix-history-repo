/*
 * Copyright (c) 1980, 1988 Regents of the University of California.
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)fstab.c	5.10 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <fstab.h>
#include <unistd.h>
#include <stdio.h>

static FILE *_fs_fp;
static struct fstab _fs_fstab;

static
fstabscan()
{
	register char *cp;
#define	MAXLINELENGTH	1024
	static char line[MAXLINELENGTH];
	char subline[MAXLINELENGTH];
	char *fgets(), *strtok();
	int typexx;

	for (;;) {
		if (!(cp = fgets(line, sizeof(line), _fs_fp)))
			return(0);
		_fs_fstab.fs_spec = strtok(cp, " \t\n");
		if (!_fs_fstab.fs_spec || *_fs_fstab.fs_spec == '#')
			continue;
		_fs_fstab.fs_file = strtok((char *)NULL, " \t\n");
		_fs_fstab.fs_vfstype = strtok((char *)NULL, " \t\n");
		_fs_fstab.fs_mntops = strtok((char *)NULL, " \t\n");
		if (_fs_fstab.fs_mntops == NULL)
			goto bad;
		_fs_fstab.fs_freq = 0;
		_fs_fstab.fs_passno = 0;
		if ((cp = strtok((char *)NULL, " \t\n")) != NULL) {
			_fs_fstab.fs_freq = atoi(cp);
			if ((cp = strtok((char *)NULL, " \t\n")) != NULL)
				_fs_fstab.fs_passno = atoi(cp);
		}
		strcpy(subline, _fs_fstab.fs_mntops);
		for (typexx = 0, cp = strtok(subline, ","); cp;
		     cp = strtok((char *)NULL, ",")) {
			if (strlen(cp) != 2)
				continue;
			if (!strcmp(cp, FSTAB_RW)) {
				_fs_fstab.fs_type = FSTAB_RW;
				break;
			}
			if (!strcmp(cp, FSTAB_RQ)) {
				_fs_fstab.fs_type = FSTAB_RQ;
				break;
			}
			if (!strcmp(cp, FSTAB_RO)) {
				_fs_fstab.fs_type = FSTAB_RO;
				break;
			}
			if (!strcmp(cp, FSTAB_SW)) {
				_fs_fstab.fs_type = FSTAB_SW;
				break;
			}
			if (!strcmp(cp, FSTAB_XX)) {
				_fs_fstab.fs_type = FSTAB_XX;
				typexx++;
				break;
			}
		}
		if (typexx)
			continue;
		if (cp != NULL)
			return(1);
	bad:
		/* no way to distinguish between EOF and syntax error */
		(void)write(STDERR_FILENO, "fstab: ", 7);
		(void)write(STDERR_FILENO, _PATH_FSTAB,
		    sizeof(_PATH_FSTAB) - 1);
		(void)write(STDERR_FILENO, ": syntax error.\n", 16);
	}
	/* NOTREACHED */
}

struct fstab *
getfsent()
{
	if (!_fs_fp && !setfsent() || !fstabscan())
		return((struct fstab *)NULL);
	return(&_fs_fstab);
}

struct fstab *
getfsspec(name)
	register char *name;
{
	if (setfsent())
		while (fstabscan())
			if (!strcmp(_fs_fstab.fs_spec, name))
				return(&_fs_fstab);
	return((struct fstab *)NULL);
}

struct fstab *
getfsfile(name)
	register char *name;
{
	if (setfsent())
		while (fstabscan())
			if (!strcmp(_fs_fstab.fs_file, name))
				return(&_fs_fstab);
	return((struct fstab *)NULL);
}

setfsent()
{
	if (_fs_fp) {
		rewind(_fs_fp);
		return(1);
	}
	return((_fs_fp = fopen(_PATH_FSTAB, "r")) != NULL);
}

void
endfsent()
{
	if (_fs_fp) {
		(void)fclose(_fs_fp);
		_fs_fp = NULL;
	}
}
