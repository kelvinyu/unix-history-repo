/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)pathnames.h	8.3 (Berkeley) %G%
 */

#ifndef _PATH_SENDMAILCF
# define _PATH_SENDMAILCF	"/etc/sendmail.cf"
#endif

#ifndef _PATH_SENDMAILPID
# ifdef BSD4_4
#  define _PATH_SENDMAILPID	"/var/run/sendmail.pid"
# else
#  define _PATH_SENDMAILPID	"/etc/sendmail.pid"
# endif
#endif

#ifndef _PATH_HOSTS
# define _PATH_HOSTS		"/etc/hosts"
#endif
