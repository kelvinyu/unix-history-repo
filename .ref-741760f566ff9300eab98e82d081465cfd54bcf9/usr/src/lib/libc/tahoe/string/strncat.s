#ifdef LIBC_SCCS
	.asciz	"@(#)strncat.s	1.1 (Berkeley/CCI) %G%"
#endif LIBC_SCCS

/*
 * Concatenate s2 on the end of s1.  S1's space must be large enough.
 * At most n characters are moved.
 * Return s1.
 * 
 * char *strncat(s1, s2, n)
 * register char *s1, *s2;
 * register n;
 */
#include "DEFS.h"

ENTRY(strncat, 0)
	tstl	12(fp)
	jgtr	n_ok
	movl	4(fp),r0
	clrb	r0
	ret
n_ok:
	movl	8(fp),r0
	movl	r0,r1
	cmps2			# r0 points at null at end of s2
	subl3	8(fp),r0,r2	# r2 = numberof non-null chars in s2
	cmpl	12(fp),r2
	jgeq	r2_ok
	movl	12(fp),r2	# r2 = min (n, length(s2))
r2_ok:
	movl	4(fp),r0
	movl	r0,r1
	cmps2			# r1 points at null at end of s1
	movl	8(fp),r0	# source for copy
	movs3			# actual copy
	clrb	(r1)		# null terminated string !
	movl	4(fp),r0
	ret
