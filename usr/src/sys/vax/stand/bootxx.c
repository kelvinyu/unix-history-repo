/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)bootxx.c	6.3 (Berkeley) %G%
 */

#include "../h/param.h"
#include "../h/inode.h"
#include "../h/fs.h"
#include "../h/vm.h"
#include <a.out.h>
#include "saio.h"
#include "../h/reboot.h"

char bootprog[] = "xx(0,0)boot";

/*
 * Boot program... arguments passed in r10 and r11
 * are passed through to the full boot program.
 */

main()
{
	register howto, devtype;	/* howto=r11, devtype=r10 */
	int io;

#ifdef lint
	howto = 0; devtype = 0;
#endif
	printf("loading %s\n", bootprog);
	io = open(bootprog, 0);
	if (io >= 0)
		copyunix(howto, devtype, io);
	_stop("boot failed\n");
}

/*ARGSUSED*/
copyunix(howto, devtype, io)
	register howto, devtype, io;	/* howto=r11, devtype=r10 */
{
	struct exec x;
	register int i;
	char *addr;

	i = read(io, (char *)&x, sizeof x);
	if (i != sizeof x ||
	    (x.a_magic != 0407 && x.a_magic != 0413 && x.a_magic != 0410))
		_stop("Bad format\n");
	if ((x.a_magic == 0413 || x.a_magic == 0410) &&
	    lseek(io, 0x400, 0) == -1)
		goto shread;
	if (read(io, (char *)0, x.a_text) != x.a_text)
		goto shread;
	addr = (char *)x.a_text;
	if (x.a_magic == 0413 || x.a_magic == 0410)
		while ((int)addr & CLOFSET)
			*addr++ = 0;
	if (read(io, addr, x.a_data) != x.a_data)
		goto shread;
	addr += x.a_data;
	x.a_bss += 128*512;	/* slop */
	for (i = 0; i < x.a_bss; i++)
		*addr++ = 0;
	x.a_entry &= 0x7fffffff;
	(*((int (*)()) x.a_entry))();
	return;
shread:
	_stop("Short read\n");
}
