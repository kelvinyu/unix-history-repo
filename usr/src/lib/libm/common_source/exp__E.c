/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)exp__E.c	5.7 (Berkeley) %G%";
#endif /* not lint */

/* exp__E(x,c)
 * ASSUMPTION: c << x  SO THAT  fl(x+c)=x.
 * (c is the correction term for x)
 * exp__E RETURNS
 *
 *			 /  exp(x+c) - 1 - x ,  1E-19 < |x| < .3465736
 *       exp__E(x,c) = 	| 		     
 *			 \  0 ,  |x| < 1E-19.
 *
 * DOUBLE PRECISION (IEEE 53 bits, VAX D FORMAT 56 BITS)
 * KERNEL FUNCTION OF EXP, EXPM1, POW FUNCTIONS
 * CODED IN C BY K.C. NG, 1/31/85;
 * REVISED BY K.C. NG on 3/16/85, 4/16/85.
 *
 * Required system supported function:
 *	copysign(x,y)	
 *
 * Method:
 *	1. Rational approximation. Let r=x+c.
 *	   Based on
 *                                   2 * sinh(r/2)     
 *                exp(r) - 1 =   ----------------------   ,
 *                               cosh(r/2) - sinh(r/2)
 *	   exp__E(r) is computed using
 *                   x*x            (x/2)*W - ( Q - ( 2*P  + x*P ) )
 *                   --- + (c + x*[---------------------------------- + c ])
 *                    2                          1 - W
 * 	   where  P := p1*x^2 + p2*x^4,
 *	          Q := q1*x^2 + q2*x^4 (for 56 bits precision, add q3*x^6)
 *	          W := x/2-(Q-x*P),
 *
 *	   (See the listing below for the values of p1,p2,q1,q2,q3. The poly-
 *	    nomials P and Q may be regarded as the approximations to sinh
 *	    and cosh :
 *		sinh(r/2) =  r/2 + r * P  ,  cosh(r/2) =  1 + Q . )
 *
 *         The coefficients were obtained by a special Remez algorithm.
 *
 * Approximation error:
 *
 *   |	exp(x) - 1			   |        2**(-57),  (IEEE double)
 *   | ------------  -  (exp__E(x,0)+x)/x  |  <= 
 *   |	     x			           |	    2**(-69).  (VAX D)
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 */

#include "mathimpl.h"

vc(p1, 1.5150724356786683059E-2 ,3abe,3d78,066a,67e1,  -6, .F83ABE67E1066A)
vc(p2, 6.3112487873718332688E-5 ,5b42,3984,0173,48cd, -13, .845B4248CD0173)
vc(q1, 1.1363478204690669916E-1 ,b95a,3ee8,ec45,44a2,  -3, .E8B95A44A2EC45)
vc(q2, 1.2624568129896839182E-3 ,7905,3ba5,f5e7,72e4,  -9, .A5790572E4F5E7)
vc(q3, 1.5021856115869022674E-6 ,9eb4,36c9,c395,604a, -19, .C99EB4604AC395)

ic(p1, 1.3887401997267371720E-2,  -7, 1.C70FF8B3CC2CF)
ic(p2, 3.3044019718331897649E-5, -15, 1.15317DF4526C4)
ic(q1, 1.1110813732786649355E-1,  -4, 1.C719538248597)
ic(q2, 9.9176615021572857300E-4, -10, 1.03FC4CB8C98E8)

#ifdef vccast
#define       p1    vccast(p1)
#define       p2    vccast(p2)
#define       q1    vccast(q1)
#define       q2    vccast(q2)
#define       q3    vccast(q3)
#endif

double __exp__E(x,c)
double x,c;
{
	const static double zero=0.0, one=1.0, half=1.0/2.0, small=1.0E-19;
	double z,p,q,xp,xh,w;
	if(copysign(x,one)>small) {
           z = x*x  ;
	   p = z*( p1 +z* p2 );
#if defined(vax)||defined(tahoe)
           q = z*( q1 +z*( q2 +z* q3 ));
#else	/* defined(vax)||defined(tahoe) */
           q = z*( q1 +z*  q2 );
#endif	/* defined(vax)||defined(tahoe) */
           xp= x*p     ; 
	   xh= x*half  ;
           w = xh-(q-xp)  ;
	   p = p+p;
	   c += x*((xh*w-(q-(p+xp)))/(one-w)+c);
	   return(z*half+c);
	}
	/* end of |x| > small */

	else {
	    if(x!=zero) one+small;	/* raise the inexact flag */
	    return(copysign(zero,x));
	}
}
