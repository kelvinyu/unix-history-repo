/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)stdlib.h	5.18 (Berkeley) %G%
 */

#ifndef _STDLIB_H_
#define _STDLIB_H_
#include <sys/types.h>

#ifdef	_BSD_SIZE_T_
typedef	_BSD_SIZE_T_	size_t;
#undef	_BSD_SIZE_T_
#endif

#ifdef	_BSD_WCHAR_T_
typedef	_BSD_WCHAR_T_	wchar_t;
#undef	_BSD_WCHAR_T_
#endif

typedef struct {
	int quot;		/* quotient */
	int rem;		/* remainder */
} div_t;

typedef struct {
	long quot;		/* quotient */
	long rem;		/* remainder */
} ldiv_t;

#define	EXIT_FAILURE	1
#define	EXIT_SUCCESS	0

#define	RAND_MAX	0x7fffffff

#define	MB_CUR_MAX	1	/* XXX */

#include <sys/cdefs.h>

__BEGIN_DECLS
__dead void
	 abort __P((void));
__pure int
	 abs __P((int));
int	 atexit __P((void (*)(void)));
double	 atof __P((const char *));
int	 atoi __P((const char *));
long	 atol __P((const char *));
void	*bsearch __P((const void *, const void *, size_t,
	    size_t, int (*)(const void *, const void *)));
void	*calloc __P((size_t, size_t));
__pure div_t
	 div __P((int, int));
__dead void
	 exit __P((int));
void	 free __P((void *));
char	*getenv __P((const char *));
__pure long
	 labs __P((long));
__pure ldiv_t
	 ldiv __P((long, long));
void	*malloc __P((size_t));
void	 qsort __P((void *, size_t, size_t,
	    int (*)(const void *, const void *)));
int	 rand __P((void));
void	*realloc __P((void *, size_t));
void	 srand __P((unsigned));
double	 strtod __P((const char *, char **));
long	 strtol __P((const char *, char **, int));
unsigned long
	 strtoul __P((const char *, char **, int));
int	 system __P((const char *));

/* These are currently just stubs. */
int	 mblen __P((const char *, size_t));
size_t	 mbstowcs __P((wchar_t *, const char *, size_t));
int	 wctomb __P((char *, wchar_t));
int	 mbtowc __P((wchar_t *, const char *, size_t));
size_t	 wcstombs __P((char *, const wchar_t *, size_t));

#ifndef _ANSI_SOURCE
void	 cfree __P((void *));
int	 putenv __P((const char *));
int	 setenv __P((const char *, const char *, int));
#endif

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
extern char *optarg;			/* getopt(3) external variables */
extern int opterr, optind, optopt;
int	 getopt __P((int, char * const *, const char *));

extern char *suboptarg;			/* getsubopt(3) external variable */
int	 getsubopt __P((char **, char * const *, char **));

void	*alloca __P((size_t));	/* built-in for gcc */
int	 heapsort __P((void *, size_t, size_t,
	    int (*)(const void *, const void *)));
char	*initstate __P((unsigned, char *, int));
int	 radixsort __P((const u_char **, int, const u_char *, u_int));
long	 random __P((void));
char	*setstate __P((char *));
void	 srandom __P((unsigned));
void	 unsetenv __P((const char *));
#endif
__END_DECLS

#endif /* _STDLIB_H_ */
