#include "../h/config.h"

/*
 * lsusp - suspends a result from a limited expression.  The limit counter
 *  for the expression is decremented.  If the counter becomes zero,
 *  lsusp exits the current expression frame, replacing the limit
 *  counter with the result which was to be suspended.  Otherwise,
 *  lsusp suspends from the current expression frame leaving the
 *  value being suspended on top of the stack.  A generator frame
 *  hiding the current expression frame is created.  The surrounding
 *  expression frame is duplicated and the value being suspended
 *  is copied to the top of the stack.  The generator frame that
 *  is created uses efail as a return point, thus if an alternative
 *  is needed, efail will return to itself via the generator frame
 *  and then resume execution at the failure label specified in
 *  the expression marker.
 *
 * The value being suspended appears as an argument.
 */
Global(_efail)		/* Signal failure in an expression */
Global(_boundary)	/* Icon/C boundary address */
Global(_line)		/* Current line number */
Global(_file)		/* Current file name */
Global(_k_level)	/* Value of &level */

Global(_lsusp)
#ifdef VAX
_lsusp:
	Mask	STDSV		# Partially create generator frame
				#  upon entry.
	decl	8(efp)		# Decrement the limit counter,
	jneq	dosusp		#  and if this result is not the last, go
				#  to dosusp to actually suspend the value.
/*
 * The limit counter has reached zero.  Replace the counter with the
 *  value that was to be suspended, remove the current expression frame
 *  and return.
 */
	movq	8(ap),4(efp)	# Replace limit counter with value
	movl	-4(efp),gfp	# Restore gfp from expression frame
	movl	16(sp),r0	# Save return pc in r0 for later use
	movl	efp,sp		# Point sp at 0'th word of expression
	movl	(sp)+,efp	#  frame and restore efp from frame,
				#  also moving sp up to point at low
				#  word of return value.
	movl	8(fp),ap	# Restore ap
	movl	12(fp),fp	#  and fp to state they were in before
				#  call to lsusp.
	jmp	(r0)		# Return from lsusp

/*
 * From this point on, the code is EXACTLY the same as esusp, with the
 *  exception of the line denoted by "--->".
 */
dosusp:
/*
 * Construct the generator frame.
 */
	movl	fp,_boundary	# Set the boundary 
	pushl	fp		# Push boundary as part of generator frame
	movl	sp,gfp		# The generator frame pointer points at
				#  the word containing the boundary.
	pushl	_k_level        # Save &level,
	pushl	_line		#  line number,
	pushl	_file		#  and file name in generator frame,
				#  completing the frame.
/*			
 * Determine region to be copied.
 */			
	addl3	$12,efp,r0	# Point r0 at first word above the
				#  expression frame marker.  This
				#  word is the lower end of the region
				#  that will be copied.
 				# ---> In esusp, the preceding instruction is
				#    addl3  $4,efp,r0
				#  It is 12 instead of 4 here because the
				#   descriptor for the limit counter is
				#   on the stack above the expression marker
				#   and the limit counter is not to be
				#   in the duplicated region. 
				#
				# If the saved gfp is non-zero, the
				#  generator frame marker serves as the
				#  upper bound of the expression frame.
				# If it is zero, the expression frame
				#  marker pointed at by the saved
				#  efp is the upper bound of the frame
				#  to be copied.
				# Note that the marker itself is not
				#  copied, the region only extends to
				#  the marker and not through it.
	movl	-4(efp),r2	# Get gfp from expression marker.
	jneq	f1		# If it is zero,
	subl3	$8,(efp),r2 	#   use saved efp - 8.
	jmp	f2
f1:				# gfp is not zero,
	subl2	$12,r2		#  use gfp - 12.
/*
 * Copy surrounding expression frame.
 */
 				# r0 points to the lowest word to be copied
				#  and r2 points to high end of the region.
				# The word that r2 points at is part of
				#  a generator or expression frame marker
				#  is not copied. 
f2:	subl2   r0,r2		# Calculate length in bytes of region and
				#  put it in r2.
	subl2	r2,sp		# Move stack pointer down to accommodate
				#  copied region.
	movc3	r2,(r0),(sp)	# Copy the region by moving r2 bytes from
				#  the address pointed at by r2 to the
				#  address pointed at by the stack pointer.

	movq	8(ap),-(sp)    	# Copy the value being suspended to
				#  the top of the stack.
/*
 * Fix things up for return.
 */
	movl	16(fp),r1	# Get the saved pc (the return point for
				#  lsusp) out of the frame and save it in r1.
	movl	$_efail,16(fp)	# Replace saved pc with efail so that when
				#  a return using the generator frame is
				#  performed, it will go to efail.
	movl	8(fp),ap	# Restore ap
	movl	12(fp),fp	#  and fp.
	clrl	_boundary	# Clear the boundary since control is
				#  going back into Icon code.
	movl	(efp),efp	# Point efp at bounding expression frame
				#  marker.
	jmp	(r1)		# Return by branching back to desired
				#  return point.  The suspended value is
				#  on the top of the stack, gfp points
				#  at the newly constructed generator
				#  frame.
#endif VAX

#ifdef PORT
DummyFcn(_lsusp)
#endif PORT

#ifdef PDP11
/ lsusp - Suspend or return from a limited expression.
/ Decrements the limit counter; if it becomes zero, lsusp
/ exits the current expression frame.  If not, lsusp
/ suspends from the current expression frame.

_lsusp:
	dec	4(r4)		/ decrement the limit counter
	bne	1f		/ branch if still > 0

/ Return from the limited expression.

	mov	(sp)+,r0	/ pop return pc
	tst	(sp)+		/ skip nargs
	mov	(sp)+,2(r4)	/ copy expression value
	mov	(sp)+,4(r4)
	mov	-2(r4),r3	/ exit expression frame
	mov	r4,sp
	mov	(sp)+,r4
	jmp	(r0)		/ return

/ Suspend from the limited expression.
/ Duplicates the most recent generator frame outside the
/ current expression frame.  Lsusp does not return directly.
/ The expression is reactivated when an alternative is needed;
/ the return actually comes from efail.

/ Register usage:
/   r0:    pointer to top of stack region to be copied,
/	     which is just above the procedure descriptor (arg0) of the
/	     suspending procedure
/   r2:	   old generator frame pointer, indexed down to r0 during copy
/   r3:    new generator frame pointer
/   r4:    suspending expression frame pointer
/   r5:    current procedure frame pointer

/ This code is exactly the same as esusp.s except for the line marked ***
/ The difference is due to the extra descriptor below the expression frame
/ marker that holds the limit counter.

1:
	mov	r5,-(sp)	/ create new procedure frame
	mov	sp,r5
	mov	r4,-(sp)    	/ save registers
	mov	r3,-(sp)
	mov	r2,-(sp)
	mov	r5,-(sp)	/ create Icon/C boundary
	mov	r5,_boundary

/ Calculate addresses of new generator frame.

	mov	sp,r3		/ r3 <- pointer to new generator frame
	mov	_k_level,-(sp)	/ save &level
	mov	_line,-(sp)	/ save current line number
	mov	_file,-(sp)	/   and file name
	mov	r4,r0		/ r0 <- pointer to top of region to be copied
	add	$6,r0		/	(= r4 + 6) ***
	mov	-2(r4),r2	/ r2 <- generator frame pointer from caller
	bne	1f		/   use saved gfp - 6 if non-zero,
	mov	(r4),r2  	/   else use saved efp - 4
	cmp	-(r2),-(r2)
	br	2f
1:
	sub	$6,r2
	br	2f

/ Copy surrounding expression frame.

1:
	mov	-(r2),-(sp)
2:
	cmp	r2,r0		/ stop at end of frame
	bhi	1b

/ Copy value of suspending expression.

	mov	8.(r5),-(sp)	/ push return value
	mov	6(r5),-(sp)

/ Return to code; reactivation will go directly to efail.

	mov	2(r5),r1	/ r1 <- return pc
	mov	$_efail,2(r5)	/ fix reactivation pc to propagate failure
	mov	-6(r5),r2
	mov	(r5),r5		/ restore old registers,
	mov	(r4),r4		/   and exit suspending expression frame
	clr	_boundary	/ returning to Icon code
	jmp	(r1)		/ this really suspends
#endif PDP11
