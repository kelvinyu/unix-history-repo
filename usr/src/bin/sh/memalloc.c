/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)memalloc.c	8.3 (Berkeley) %G%";
#endif /* not lint */

#include "shell.h"
#include "output.h"
#include "memalloc.h"
#include "error.h"
#include "machdep.h"
#include "mystring.h"
#include <stdlib.h>
#include <unistd.h>

/*
 * Like malloc, but returns an error when out of space.
 */

pointer
ckmalloc(nbytes) 
	int nbytes;
{
	register pointer p;

	if ((p = malloc(nbytes)) == NULL)
		error("Out of space");
	return p;
}


/*
 * Same for realloc.
 */

pointer
ckrealloc(p, nbytes)
	register pointer p;
	int nbytes;
{

	if ((p = realloc(p, nbytes)) == NULL)
		error("Out of space");
	return p;
}


/*
 * Make a copy of a string in safe storage.
 */

char *
savestr(s)
	char *s;
	{
	register char *p;

	p = ckmalloc(strlen(s) + 1);
	scopy(s, p);
	return p;
}


/*
 * Parse trees for commands are allocated in lifo order, so we use a stack
 * to make this more efficient, and also to avoid all sorts of exception
 * handling code to handle interrupts in the middle of a parse.
 *
 * The size 504 was chosen because the Ultrix malloc handles that size
 * well.
 */

#define MINSIZE 504		/* minimum size of a block */


struct stack_block {
	struct stack_block *prev;
	char space[MINSIZE];
};

struct stack_block stackbase;
struct stack_block *stackp = &stackbase;
char *stacknxt = stackbase.space;
int stacknleft = MINSIZE;
int sstrnleft;
int herefd = -1;



pointer
stalloc(nbytes) 
	int nbytes;
{
	register char *p;

	nbytes = ALIGN(nbytes);
	if (nbytes > stacknleft) {
		int blocksize;
		struct stack_block *sp;

		blocksize = nbytes;
		if (blocksize < MINSIZE)
			blocksize = MINSIZE;
		INTOFF;
		sp = ckmalloc(sizeof(struct stack_block) - MINSIZE + blocksize);
		sp->prev = stackp;
		stacknxt = sp->space;
		stacknleft = blocksize;
		stackp = sp;
		INTON;
	}
	p = stacknxt;
	stacknxt += nbytes;
	stacknleft -= nbytes;
	return p;
}


void
stunalloc(p)
	pointer p;
	{
	if (p == NULL) {		/*DEBUG */
		write(2, "stunalloc\n", 10);
		abort();
	}
	stacknleft += stacknxt - (char *)p;
	stacknxt = p;
}



void
setstackmark(mark)
	struct stackmark *mark;
	{
	mark->stackp = stackp;
	mark->stacknxt = stacknxt;
	mark->stacknleft = stacknleft;
}


void
popstackmark(mark)
	struct stackmark *mark;
	{
	struct stack_block *sp;

	INTOFF;
	while (stackp != mark->stackp) {
		sp = stackp;
		stackp = sp->prev;
		ckfree(sp);
	}
	stacknxt = mark->stacknxt;
	stacknleft = mark->stacknleft;
	INTON;
}


/*
 * When the parser reads in a string, it wants to stick the string on the
 * stack and only adjust the stack pointer when it knows how big the
 * string is.  Stackblock (defined in stack.h) returns a pointer to a block
 * of space on top of the stack and stackblocklen returns the length of
 * this block.  Growstackblock will grow this space by at least one byte,
 * possibly moving it (like realloc).  Grabstackblock actually allocates the
 * part of the block that has been used.
 */

void
growstackblock() {
	char *p;
	int newlen = stacknleft * 2 + 100;
	char *oldspace = stacknxt;
	int oldlen = stacknleft;
	struct stack_block *sp;

	if (stacknxt == stackp->space && stackp != &stackbase) {
		INTOFF;
		sp = stackp;
		stackp = sp->prev;
		sp = ckrealloc((pointer)sp, sizeof(struct stack_block) - MINSIZE + newlen);
		sp->prev = stackp;
		stackp = sp;
		stacknxt = sp->space;
		stacknleft = newlen;
		INTON;
	} else {
		p = stalloc(newlen);
		memcpy(p, oldspace, oldlen);
		stacknxt = p;			/* free the space */
		stacknleft += ALIGN(newlen);	/* we just allocated */
	}
}



void
grabstackblock(len) 
	int len;
{
	len = ALIGN(len);
	stacknxt += len;
	stacknleft -= len;
}



/*
 * The following routines are somewhat easier to use that the above.
 * The user declares a variable of type STACKSTR, which may be declared
 * to be a register.  The macro STARTSTACKSTR initializes things.  Then
 * the user uses the macro STPUTC to add characters to the string.  In
 * effect, STPUTC(c, p) is the same as *p++ = c except that the stack is
 * grown as necessary.  When the user is done, she can just leave the
 * string there and refer to it using stackblock().  Or she can allocate
 * the space for it using grabstackstr().  If it is necessary to allow
 * someone else to use the stack temporarily and then continue to grow
 * the string, the user should use grabstack to allocate the space, and
 * then call ungrabstr(p) to return to the previous mode of operation.
 *
 * USTPUTC is like STPUTC except that it doesn't check for overflow.
 * CHECKSTACKSPACE can be called before USTPUTC to ensure that there
 * is space for at least one character.
 */


char *
growstackstr() {
	int len = stackblocksize();
	if (herefd >= 0 && len >= 1024) {
		xwrite(herefd, stackblock(), len);
		sstrnleft = len - 1;
		return stackblock();
	}
	growstackblock();
	sstrnleft = stackblocksize() - len - 1;
	return stackblock() + len;
}


/*
 * Called from CHECKSTRSPACE.
 */

char *
makestrspace() {
	int len = stackblocksize() - sstrnleft;
	growstackblock();
	sstrnleft = stackblocksize() - len;
	return stackblock() + len;
}



void
ungrabstackstr(s, p)
	char *s;
	char *p;
	{
	stacknleft += stacknxt - s;
	stacknxt = s;
	sstrnleft = stacknleft - (p - s);
}
