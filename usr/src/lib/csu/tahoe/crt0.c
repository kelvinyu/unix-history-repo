/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)crt0.c	5.2 (Berkeley) %G%";
#endif /* not lint */

/*
 *	C start up routine.
 *	Robert Henry, UCB, 20 Oct 81
 *
 *	We make the following (true) assumptions:
 *	1) When the kernel calls start, it does a jump to location 2,
 *	and thus avoids the register save mask.  We are NOT called
 *	with a calls!
 *	2) The only register variable that we can trust is sp,
 *	which points to the base of the kernel calling frame.
 */

char **environ = (char **)0;
static int fd;

extern	unsigned char	etext;
extern	unsigned char	eprol asm ("eprol");
extern			start() asm("start");

/*
 * Some kluges: store sp at entry in environ, and
 * install 16 bits of 0 at location 0 (a zero register save mask).
 * These two hacks remove limits on the use of local
 * and register variables in start().
 * The reason for using 'moval (sp),...' is that 'movl sp,...' generates
 * a privileged instruction trap (argh).
 * XXX 'addl3 $start,$2,r0; jmp (r0)' should be replaced with
 * XXX 'jbr start+2' when we convert over to gas.
 */
asm(".text; .word 0; moval (sp),_environ; addl3 $start,$2,r0; jmp (r0)");

start()
{
	struct kframe {
		int	kargc;
		char	*kargv[1];	/* size depends on kargc */
		char	kargstr[1];	/* size varies */
		char	kenvstr[1];	/* size varies */
	};
	register struct kframe *kfp;
	register char **targv;
	register char **argv;
	extern int errno;

	kfp = (struct kframe *) environ;
	for (argv = targv = &kfp->kargv[0]; *targv++; /* void */)
		/* void */ ;
	if (targv >= (char **)(*argv))
		--targv;
	environ = targv;
asm("eprol:");

#ifdef MCRT0
	monstartup(&eprol, &etext);
#endif MCRT0
	errno = 0;
	exit(main(kfp->kargc, argv, environ));
}

#ifdef MCRT0
/*ARGSUSED*/
exit(code)
	register int code;
{
	monitor(0);
	_cleanup();
	_exit(code);
}
#endif MCRT0

#ifdef CRT0
/*
 * null mcount and moncontrol,
 * just in case some routine is compiled for profiling
 */
moncontrol(val)
	int val;
{

}

mcount()
{

}
#endif CRT0
