/*-
 * Copyright (c) 1985, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)networkdelta.c	8.2 (Berkeley) %G%";
#endif /* not lint */

#ifdef sgi
#ident "$Revision: 1.4 $"
#endif

#include "globals.h"

static long median __P((float, float *, long *, long *, unsigned int));

/*
 * Compute a corrected date.
 *	Compute the median of the reasonable differences.  First compute
 *	the median of all authorized differences, and then compute the
 *	median of all differences that are reasonably close to the first
 *	median.
 *
 * This differs from the original BSD implementation, which looked for
 *	the largest group of machines with essentially the same date.
 *	That assumed that machines with bad clocks would be uniformly
 *	distributed.  Unfortunately, in real life networks, the distribution
 *	of machines is not uniform among models of machines, and the
 *	distribution of errors in clocks tends to be quite consistent
 *	for a given model.  In other words, all model VI Supre Servres
 *	from GoFast Inc. tend to have about the same error.
 *	The original BSD implementation would chose the clock of the
 *	most common model, and discard all others.
 *
 *	Therefore, get best we can do is to try to average over all
 *	of the machines in the network, while discarding "obviously"
 *	bad values.
 */
long
networkdelta()
{
	struct hosttbl *htp;
	long med;
	long lodelta, hidelta;
	long logood, higood;
	long x[NHOSTS];
	long *xp;
	int numdelta;
	float eps;

	/*
	 * compute the median of the good values
	 */
	med = 0;
	numdelta = 1;
	xp = &x[0];
	*xp = 0;			/* account for ourself */
	for (htp = self.l_fwd; htp != &self; htp = htp->l_fwd) {
		if (htp->good
		    && htp->noanswer == 0
		    && htp->delta != HOSTDOWN) {
			med += htp->delta;
			numdelta++;
			*++xp = htp->delta;
		}
	}

	/*
	 * If we are the only trusted time keeper, then do not change our
	 * clock.  There may be another time keeping service active.
	 */
	if (numdelta == 1)
		return 0;

	/* get average of trusted values */
	med /= numdelta;

	if (trace)
		fprintf(fd, "median of %d values starting at %ld is about ",
			numdelta, med);
	/* get median of all trusted values, using average as initial guess */
	eps = med - x[0];
	med = median(med, &eps, &x[0], xp+1, VALID_RANGE);

	/* Compute the median of all good values.
	 * Good values are those of all clocks, including untrusted clocks,
	 * that are
	 *	- trusted and somewhat close to the median of the
	 *		trusted clocks
	 *	- trusted or untrusted and very close to the median of the
	 *		trusted clocks
	 */
	hidelta = med + GOOD_RANGE;
	lodelta = med - GOOD_RANGE;
	higood = med + VGOOD_RANGE;
	logood = med - VGOOD_RANGE;
	xp = &x[0];
	htp = &self;
	do {
		if (htp->noanswer == 0
		    && htp->delta >= lodelta
		    && htp->delta <= hidelta
		    && (htp->good
			|| (htp->delta >= logood
			    && htp->delta <= higood))) {
			*xp++ = htp->delta;
		}
	} while (&self != (htp = htp->l_fwd));

	if (xp == &x[0]) {
		if (trace)
			fprintf(fd, "nothing close to median %ld\n", med);
		return med;
	}

	if (xp == &x[1]) {
		if (trace)
			fprintf(fd, "only value near median is %ld\n", x[0]);
		return x[0];
	}

	if (trace)
		fprintf(fd, "median of %d values starting at %ld is ",
		        xp-&x[0], med);
	return median(med, &eps, &x[0], xp, 1);
}


/*
 * compute the median of an array of signed integers, using the idea
 *	in <<Numerical Recipes>>.
 */
static long
median(a, eps_ptr, x, xlim, gnuf)
	float a;			/* initial guess for the median */
	float *eps_ptr;			/* spacing near the median */
	long *x, *xlim;			/* the data */
	unsigned int gnuf;		/* good enough estimate */
{
	long *xptr;
	float ap = LONG_MAX;		/* bounds on the median */
	float am = -LONG_MAX;
	float aa;
	int npts;			/* # of points above & below guess */
	float xp;			/* closet point above the guess */
	float xm;			/* closet point below the guess */
	float eps;
	float dum, sum, sumx;
	int pass;
#define AMP	1.5			/* smoothing constants */
#define AFAC	1.5

	eps = *eps_ptr;
	if (eps < 1.0) {
		eps = -eps;
		if (eps < 1.0)
			eps = 1.0;
	}

	for (pass = 1; ; pass++) {	/* loop over the data */
		sum = 0.0;
		sumx = 0.0;
		npts = 0;
		xp = LONG_MAX;
		xm = -LONG_MAX;

		for (xptr = x; xptr != xlim; xptr++) {
			float xx = *xptr;

			dum = xx - a;
			if (dum != 0.0) {   /* avoid dividing by 0 */
				if (dum > 0.0) {
					npts++;
					if (xx < xp)
						xp = xx;
				} else {
					npts--;
					if (xx > xm)
						xm = xx;
					dum = -dum;
				}
				dum = 1.0/(eps + dum);
				sum += dum;
				sumx += xx * dum;
			}
		}

		if (ap-am < gnuf || sum == 0) {
			if (trace)
				fprintf(fd,
					"%ld in %d passes;"
					" early out balance=%d\n",
				        (long)a, pass, npts);
			return a;	/* guess was good enough */
		}

		aa = (sumx/sum-a)*AMP;
		if (npts >= 2) {	/* guess was too low */
			am = a;
			aa = xp + max(0.0, aa);
			if (aa >= ap)
				aa = (a + ap)/2;

		} else if (npts <= -2) {    /* guess was two high */
			ap = a;
			aa = xm + min(0.0, aa);
			if (aa <= am)
				aa = (a + am)/2;

		} else {
			break;		/* got it */
		}

		if (a == aa) {
			if (trace)
				fprintf(fd, "%ld in %d passes;"
					" force out balance=%d\n",
				        (long)a, pass, npts);
			return a;
		}
		eps = AFAC*abs(aa - a);
		*eps_ptr = eps;
		a = aa;
	}

	if (((x - xlim) % 2) != 0) {    /* even number of points? */
		if (npts == 0)		/* yes, return an average */
			a = (xp+xm)/2;
		else if (npts > 0)
			a =  (a+xp)/2;
		else
			a = (xm+a)/2;

	} else if (npts != 0) {		/* odd number of points */
		if (npts > 0)
			a = xp;
		else
			a = xm;
	}

	if (trace)
		fprintf(fd, "%ld in %d passes\n", (long)a, pass);
	return a;
}
