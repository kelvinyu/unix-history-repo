/* brk.s 4.3 83/10/06 */

#include "SYS.h"

#define	SYS_brk		17

	.globl	curbrk
	.globl	minbrk
ENTRY(_brk)
	jbr	ok

ENTRY(brk)
	cmpl	4(ap),minbrk
	bgeq	ok
	movl	minbrk,4(ap)
ok:
	chmk	$SYS_brk
	jcs	err
	movl	4(ap),curbrk
	clrl	r0
	ret
err:
	jmp	cerror
