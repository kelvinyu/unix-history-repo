/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)time.h	5.9 (Berkeley) %G%
 */

#ifndef _TIME_H_
#define	_TIME_H_

#include <machine/types.h>

#ifndef	NULL
#define	NULL	0
#endif

#ifdef	_CLOCK_T_
typedef	_CLOCK_T_	clock_t;
#undef	_CLOCK_T_
#endif

#ifdef	_TIME_T_
typedef	_TIME_T_	time_t;
#undef	_TIME_T_
#endif

#ifdef	_SIZE_T_
typedef	_SIZE_T_	size_t;
#undef	_SIZE_T_
#endif

struct tm {
	int	tm_sec;		/* seconds after the minute [0-60] */
	int	tm_min;		/* minutes after the hour [0-59] */
	int	tm_hour;	/* hours since midnight [0-23] */
	int	tm_mday;	/* day of the month [1-31] */
	int	tm_mon;		/* months since January [0-11] */
	int	tm_year;	/* years since 1900 */
	int	tm_wday;	/* days since Sunday [0-6] */
	int	tm_yday;	/* days since January 1 [0-365] */
	int	tm_isdst;	/* Daylight Savings Time flag */
	long	tm_gmtoff;	/* offset from CUT in seconds */
	char	*tm_zone;	/* timezone abbreviation */
};

#include <sys/cdefs.h>

__BEGIN_DECLS
struct tm *gmtime __P((const time_t *));
struct tm *localtime __P((const time_t *));
time_t mktime __P((const struct tm *));
time_t time __P((time_t *));
clock_t clock __P((void));
double difftime __P((const time_t, const time_t));
char *asctime __P((const struct tm *));
char *ctime __P((const time_t *));
char *timezone __P((int , int));
void tzset __P((void));
void tzsetwall __P((void));
__END_DECLS

#endif /* !_TIME_H_ */
