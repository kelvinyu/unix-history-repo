/*	frexp.s	4.1	84/08/22	*/

/* C library -- frexp(value, eptr) */

#include "DEFS.h"

ENTRY(frexp)
	movd	4(ap),r0		# (r0,r1) := value
	extzv	$7,$8,r0,*12(ap)	# Fetch exponent
	jeql	1f			# If exponent zero, we're done
	subl2	$128,*12(ap)		# Bias the exponent appropriately
	insv	$128,$7,$8,r0		# Force result exponent to biased 0
1:
	ret
