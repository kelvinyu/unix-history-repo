/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)error.h	8.1 (Berkeley) %G%
 */

/*
 * Types of operations (passed to the errmsg routine).
 */

#define E_OPEN 01	/* opening a file */
#define E_CREAT 02	/* creating a file */
#define E_EXEC 04	/* executing a program */


/*
 * We enclose jmp_buf in a structure so that we can declare pointers to
 * jump locations.  The global variable handler contains the location to
 * jump to when an exception occurs, and the global variable exception
 * contains a code identifying the exeception.  To implement nested
 * exception handlers, the user should save the value of handler on entry
 * to an inner scope, set handler to point to a jmploc structure for the
 * inner scope, and restore handler on exit from the scope.
 */

#include <setjmp.h>

struct jmploc {
	jmp_buf loc;
};

extern struct jmploc *handler;
extern int exception;

/* exceptions */
#define EXINT 0		/* SIGINT received */
#define EXERROR 1	/* a generic error */
#define EXSHELLPROC 2	/* execute a shell procedure */


/*
 * These macros allow the user to suspend the handling of interrupt signals
 * over a period of time.  This is similar to SIGHOLD to or sigblock, but
 * much more efficient and portable.  (But hacking the kernel is so much
 * more fun than worrying about efficiency and portability. :-))
 */

extern volatile int suppressint;
extern volatile int intpending;
extern char *commandname;	/* name of command--printed on error */

#define INTOFF suppressint++
#define INTON if (--suppressint == 0 && intpending) onint(); else
#define FORCEINTON {suppressint = 0; if (intpending) onint();}
#define CLEAR_PENDING_INT intpending = 0
#define int_pending() intpending

#ifdef __STDC__
void exraise(int);
void onint(void);
void error2(char *, char *);
void error(char *, ...);
char *errmsg(int, int);
#else
void exraise();
void onint();
void error2();
void error();
char *errmsg();
#endif


/*
 * BSD setjmp saves the signal mask, which violates ANSI C and takes time,
 * so we use _setjmp instead.
 */

#ifdef BSD
#define setjmp(jmploc)	_setjmp(jmploc)
#define longjmp(jmploc, val)	_longjmp(jmploc, val)
#endif
