/*
 * Copyright (c) 1984 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)machpats.c	1.2 (Berkeley) %G%";
#endif

#include "inline.h"

/*
 * Pattern table for special instructions.
 */
struct pats machine_ptab[] = {

#if defined(vax)
	{ "3,_blkcpy\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	jbr	2f\n\
1:\n\
	subl2	r0,(sp)\n\
	movc3	r0,(r1),(r3)\n\
2:\n\
	movzwl	$65535,r0\n\
	cmpl	(sp),r0\n\
	jgtr	1b\n\
	movl	(sp)+,r0\n\
	movc3	r0,(r1),(r3)\n" },

	{ "3,_bcopy\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	movc3	r5,(r1),(r3)\n" },

	{ "3,_ovbcopy\n",
"	movl	(sp)+,r3\n\
	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	movc3	r5,(r3),(r4)\n" },

	{ "3,_blkcmp\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	jbr	2f\n\
1:\n\
	subl2	r0,(sp)\n\
	cmpc3	r0,(r1),(r3)\n\
	bneq	3f\n\
2:\n\
	movzwl	$65535,r0\n\
	cmpl	(sp),r0\n\
	jgtr	1b\n\
	movl	(sp)+,r0\n\
	cmpc3	r0,(r1),(r3)\n\
3:\n" },

	{ "3,_bcmp\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	cmpc3	r5,(r1),(r3)\n" },

	{ "2,_blkclr\n",
"	movl	(sp)+,r3\n\
	jbr	2f\n\
1:\n\
	subl2	r0,(sp)\n\
	movc5	$0,(r3),$0,r0,(r3)\n\
2:\n\
	movzwl	$65535,r0\n\
	cmpl	(sp),r0\n\
	jgtr	1b\n\
	movl	(sp)+,r0\n\
	movc5	$0,(r3),$0,r0,(r3)\n" },

	{ "2,_bzero\n",
"	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	movc5	$0,(r3),$0,r5,(r3)\n" },

	{ "3,_llocc\n",
"	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	movl	(sp)+,r1\n\
1:\n\
	movzwl	$65535,r0\n\
	cmpl	r5,r0\n\
	jleq	1f\n\
	subl2	r0,r5\n\
	locc	r4,r0,(r1)\n\
	jeql	1b\n\
	addl2	r5,r0\n\
	jbr	2f\n\
1:\n\
	locc	r4,r5,(r1)\n\
2:\n" },

	{ "3,_locc\n",
"	movl	(sp)+,r3\n\
	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	locc	r3,r4,(r5)\n" },

	{ "4,_scanc\n",
"	movl	(sp)+,r2\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	scanc	r2,(r3),(r4),r5\n" },

	{ "3,_skpc\n",
"	movl	(sp)+,r3\n\
	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	skpc	r3,r4,(r5)\n" },

	{ "2,_insque\n",
"	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	insque	(r4),(r5)\n" },

	{ "1,_remque\n",
"	movl	(sp)+,r5\n\
	remque	(r5),r0\n" },
#endif

#if defined(tahoe)
	{ "16,_blkcpy\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r1\n\
	movl	(sp)+,r2\n\
	movblk\n" },

	{ "16,_bcopy\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r1\n\
	movl	(sp)+,r2\n\
	movblk\n" },

	{ "12,_bzero\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r2\n\
	movab	1f,r0\n\
	movs3\n\
	.data\n\
1:	.byte	0\n\
	.text\n" },

	{ "12,_blkclr\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r2\n\
	movab	1f,r0\n\
	movs3\n\
	.data\n\
1:	.byte	0\n\
	.text\n" },
#endif

#if defined(mc68000)
/* someday... */
#endif

	{ "", "" }
};
