/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)conf.h	8.21 (Berkeley) %G%
 */

/*
**  CONF.H -- All user-configurable parameters for sendmail
*/

# include <sys/param.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <signal.h>

/**********************************************************************
**  Table sizes, etc....
**	There shouldn't be much need to change these....
**********************************************************************/

# define MAXLINE	2048		/* max line length */
# define MAXNAME	256		/* max length of a name */
# define MAXPV		40		/* max # of parms to mailers */
# define MAXATOM	200		/* max atoms per address */
# define MAXMAILERS	25		/* maximum mailers known to system */
# define MAXRWSETS	100		/* max # of sets of rewriting rules */
# define MAXPRIORITIES	25		/* max values for Precedence: field */
# define MAXMXHOSTS	20		/* max # of MX records */
# define SMTPLINELIM	990		/* maximum SMTP line length */
# define MAXKEY		128		/* maximum size of a database key */
# define MEMCHUNKSIZE	1024		/* chunk size for memory allocation */
# define MAXUSERENVIRON	100		/* max envars saved, must be >= 3 */
# define MAXIPADDR	16		/* max # of IP addrs for this host */
# define MAXALIASDB	12		/* max # of alias databases */
# define PSBUFSIZE	(MAXLINE + MAXATOM)	/* size of prescan buffer */

# ifndef QUEUESIZE
# define QUEUESIZE	1000		/* max # of jobs per queue run */
# endif

/**********************************************************************
**  Compilation options.
**
**	#define these if they are available; comment them out otherwise.
**********************************************************************/

# define LOG		1	/* enable logging */
# define UGLYUUCP	1	/* output ugly UUCP From lines */
# define NETINET	1	/* include internet support */
# define SETPROCTITLE	1	/* munge argv to display current status */
# define NAMED_BIND	1	/* use Berkeley Internet Domain Server */
# define MATCHGECOS	1	/* match user names from gecos field */
# define XDEBUG		1	/* enable extended debugging */

# ifdef NEWDB
# define USERDB		1	/* look in user database (requires NEWDB) */
# endif

/*
**  Due to a "feature" in some operating systems such as Ultrix 4.3 and
**  HPUX 8.0, if you receive a "No route to host" message (ICMP message
**  ICMP_UNREACH_HOST) on _any_ connection, all connections to that host
**  are closed.  Some firewalls return this error if you try to connect
**  to the IDENT port (113), so you can't receive email from these hosts
**  on these systems.  The firewall really should use a more specific
**  message such as ICMP_UNREACH_PROTOCOL or _PORT or _NET_PROHIB.  This
**  will get #undefed below as needed.
*/

# define IDENTPROTO	1	/* use IDENT proto (RFC 1413) */

/**********************************************************************
**  Operating system configuration.
**
**	Unless you are porting to a new OS, you shouldn't have to
**	change these.
**********************************************************************/

/*
**  Per-Operating System defines
*/

/*
**  HP-UX -- tested for 8.07
*/

# ifdef __hpux
# define SYSTEM5	1	/* include all the System V defines */
# define HASINITGROUPS	1	/* has initgroups(3) call */
# define HASSETREUID	1	/* has setreuid(2) call */
# define setreuid(r, e)		setresuid(r, e, -1)	
# define LA_TYPE	LA_FLOAT
# define _PATH_UNIX	"/hp-ux"
# undef IDENTPROTO
# endif

/*
**  IBM AIX 3.x -- actually tested for 3.2.3
*/

# ifdef _AIX3
# define HASINITGROUPS	1	/* has initgroups(3) call */
# define FORK		fork	/* no vfork primitive available */
# endif

/*
**  Silicon Graphics IRIX
**
**	Compiles on 4.0.1.
*/

# ifdef IRIX
# define HASSETREUID	1	/* has setreuid(2) call */
# define HASINITGROUPS	1	/* has initgroups(3) call */
# define FORK		fork	/* no vfork primitive available */
# define setpgid	BSDsetpgrp
# define GIDSET_T	gid_t
# endif


/*
**  SunOS
*/

#if defined(sun) && !defined(BSD)

# define LA_TYPE	LA_INT
# define HASSETREUID	1	/* has setreuid(2) call */
# define HASINITGROUPS	1	/* has initgroups(3) call */

# ifdef SOLARIS
			/* Solaris 2.x (a.k.a. SunOS 5.x) */
#  define SYSTEM5	1	/* use System V definitions */
#  define setreuid(r, e)	seteuid(e)
#  include <sys/time.h>
#  define gethostbyname	__switch_gethostbyname	/* get working version */
#  define gethostbyaddr	__switch_gethostbyaddr	/* get working version */
#  define _PATH_UNIX	"/kernel/unix"
#  ifndef _PATH_SENDMAILCF
#   define _PATH_SENDMAILCF	"/etc/mail/sendmail.cf"
#  endif
#  ifndef _PATH_SENDMAILFC
#   define _PATH_SENDMAILFC	"/etc/mail/sendmail.fc"
#  endif
#  ifndef _PATH_SENDMAILPID
#   define _PATH_SENDMAILPID	"/etc/mail/sendmail.pid"
#  endif

# else
			/* SunOS 4.1.x */
#  define HASSTATFS	1	/* has the statfs(2) syscall */
/* #  define HASFLOCK	1	/* has flock(2) call */
#  include <vfork.h>

# endif
#endif

/*
**  Digital Ultrix 4.2A or 4.3
*/

#ifdef ultrix
# define HASSTATFS	1	/* has the statfs(2) syscall */
# define HASSETREUID	1	/* has setreuid(2) call */
# define HASUNSETENV	1	/* has unsetenv(3) call */
# define HASINITGROUPS	1	/* has initgroups(3) call */
/* # define HASFLOCK	1	/* has flock(2) call */
# define LA_TYPE	LA_INT
# define LA_AVENRUN	"avenrun"
# undef IDENTPROTO
#endif

/*
**  OSF/1 (tested on Alpha)
*/

#ifdef __osf__
# define HASUNSETENV	1	/* has unsetenv(3) call */
# define HASSETREUID	1	/* has setreuid(2) call */
# define HASINITGROUPS	1	/* has initgroups(3) call */
/* # define HASFLOCK	1	/* has flock(2) call */
# define LA_TYPE	LA_INT
#endif

/*
**  NeXTstep
*/

#ifdef NeXT
# define HASINITGROUPS	1	/* has initgroups(3) call */
# define HASFLOCK	1	/* has flock(2) call */
# define NEEDGETOPT	1	/* need a replacement for getopt(3) */
# define sleep		sleepX
# define setpgid	setpgrp
# define LA_TYPE	LA_ZERO
typedef int		pid_t;
# ifndef _PATH_SENDMAILCF
#  define _PATH_SENDMAILCF	"/etc/sendmail/sendmail.cf"
# endif
# ifndef _PATH_SENDMAILFC
#  define _PATH_SENDMAILFC	"/etc/sendmail/sendmail.fc"
# endif
# ifndef _PATH_SENDMAILPID
#  define _PATH_SENDMAILPID	"/etc/sendmail/sendmail.pid"
# endif
#endif

/*
**  4.4 BSD
*/

#ifdef BSD4_4
# define HASUNSETENV	1	/* has unsetenv(3) call */
# include <sys/cdefs.h>
# define ERRLIST_PREDEFINED	/* don't declare sys_errlist */
# ifndef LA_TYPE
#  define LA_TYPE	LA_SUBR
# endif
#endif

/*
**  4.3 BSD -- this is for very old systems
**
**	You'll also have to install a new resolver library.
**	I don't guarantee that support for this environment is complete.
*/

#ifdef oldBSD43
# define NEEDVPRINTF	1	/* need a replacement for vprintf(3) */
# define NEEDGETOPT	1	/* need a replacement for getopt(3) */
# define ARBPTR_T	char *
# define setpgid	setpgrp
# ifndef LA_TYPE
#  define LA_TYPE	LA_FLOAT
# endif
# ifndef _PATH_SENDMAILCF
#  define _PATH_SENDMAILCF	"/usr/lib/sendmail.cf"
# endif
# ifndef _PATH_SENDMAILFC
#  define _PATH_SENDMAILFC	"/usr/lib/sendmail.fc"
# endif
#endif

/*
**  SCO Unix
*/

#ifdef _SCO_unix_
# define SYSTEM5	1	/* include all the System V defines */
# define SYS5SIGNALS	1	/* SysV signal semantics -- reset on each sig */
# define HASSTATFS	1	/* has the statfs(2) syscall */
# define FORK		fork
# define MAXPATHLEN	PATHSIZE
# define LA_TYPE	LA_ZERO
#endif

/*
**  ConvexOS 11.0 and later
*/

#ifdef _CONVEX_SOURCE
# define BSD		1	/* include all the BSD defines */
# define HASUNAME	1	/* use System V uname(2) system call */
# define HASSTATFS	1	/* has the statfs(2) syscall */
# define HASSETSID	1	/* has POSIX setsid(2) call */
# define NEEDGETOPT	1	/* need replacement for getopt(3) */
# define LA_TYPE	LA_FLOAT
# undef IDENTPROTO
#endif

/*
**  RISC/os 4.51
**
**	Untested...
*/

#ifdef RISCOS
# define HASUNSETENV	1	/* has unsetenv(3) call */
/* # define HASFLOCK	1	/* has flock(2) call */
# define LA_TYPE	LA_INT
# define LA_AVENRUN	"avenrun"
# define _PATH_UNIX	"/unix"
#endif

/*
**  Linux 0.99pl10 and above...
**	From Karl London <karl@borg.demon.co.uk>.
*/

#ifdef linux
# define BSD		1	/* pretend to be BSD based today */
# undef  NEEDVPRINTF	1	/* need a replacement for vprintf(3) */
# define NEEDGETOPT	1	/* need a replacement for getopt(3) */
# ifndef LA_TYPE
#  define LA_TYPE	LA_FLOAT
# endif
#endif


/**********************************************************************
**  End of Per-Operating System defines
**********************************************************************/

/**********************************************************************
**  More general defines
**********************************************************************/

/* general BSD defines */
#ifdef BSD
# define HASGETDTABLESIZE 1	/* has getdtablesize(2) call */
# define HASSETREUID	1	/* has setreuid(2) call */
# define HASINITGROUPS	1	/* has initgroups(2) call */
# define HASFLOCK	1	/* has flock(2) call */
#endif

/* general System V defines */
# ifdef SYSTEM5
# define HASUNAME	1	/* use System V uname(2) system call */
# define HASUSTAT	1	/* use System V ustat(2) syscall */
# ifndef LA_TYPE
#  define LA_TYPE	LA_INT
# endif
# define bcopy(s, d, l)		(memmove((d), (s), (l)))
# define bzero(d, l)		(memset((d), '\0', (l)))
# define bcmp(s, d, l)		(memcmp((s), (d), (l)))
# endif

/* general "standard C" defines */
#if defined(__STDC__) || defined(SYSTEM5)
# define HASSETVBUF	1	/* we have setvbuf(3) in libc */
#endif

/* general POSIX defines */
#ifdef _POSIX_VERSION
# define HASSETSID	1	/* has setsid(2) call */
# define HASWAITPID	1	/* has waitpid(2) call */
#endif

/*
**  If no type for argument two of getgroups call is defined, assume
**  it's an integer -- unfortunately, there seem to be several choices
**  here.
*/

#ifndef GIDSET_T
# define GIDSET_T	int
#endif

/*
**  Remaining definitions should never have to be changed.  They are
**  primarily to provide back compatibility for older systems -- for
**  example, it includes some POSIX compatibility definitions
*/

/* System 5 compatibility */
#ifndef S_ISREG
#define S_ISREG(foo)	((foo & S_IFREG) == S_IFREG)
#endif
#ifndef S_IWGRP
#define S_IWGRP		020
#endif
#ifndef S_IWOTH
#define S_IWOTH		002
#endif

/*
**  Older systems don't have this error code -- it should be in
**  /usr/include/sysexits.h.
*/

# ifndef EX_CONFIG
# define EX_CONFIG	78	/* configuration error */
# endif

/*
**  These are used in a few cases where we need some special
**  error codes, but where the system doesn't provide something
**  reasonable.  They are printed in errstring.
*/

#ifndef E_PSEUDOBASE
# define E_PSEUDOBASE	256
#endif

#define EOPENTIMEOUT	(E_PSEUDOBASE + 0)	/* timeout on open */
#define E_DNSBASE	(E_PSEUDOBASE + 20)	/* base for DNS h_errno */

/* type of arbitrary pointer */
#ifndef ARBPTR_T
# define ARBPTR_T	void *
#endif

#ifndef __P
# include "cdefs.h"
#endif

/*
**  Do some required dependencies
*/

#if defined(NETINET) || defined(NETISO)
# define SMTP		1	/* enable user and server SMTP */
# define QUEUE		1	/* enable queueing */
# define DAEMON		1	/* include the daemon (requires IPC & SMTP) */
#endif


/*
**  Arrange to use either varargs or stdargs
*/

# ifdef __STDC__

# include <stdarg.h>

# define VA_LOCAL_DECL	va_list ap;
# define VA_START(f)	va_start(ap, f)
# define VA_END		va_end(ap)

# else

# include <varargs.h>

# define VA_LOCAL_DECL	va_list ap;
# define VA_START(f)	va_start(ap)
# define VA_END		va_end(ap)

# endif

#ifdef HASUNAME
# include <sys/utsname.h>
# ifdef newstr
#  undef newstr
# endif
#else /* ! HASUNAME */
# define NODE_LENGTH 32
struct utsname
{
	char nodename[NODE_LENGTH+1];
};
#endif /* HASUNAME */

#if !defined(MAXHOSTNAMELEN) && !defined(_SCO_unix_)
# define MAXHOSTNAMELEN	256
#endif

#if !defined(SIGCHLD) && defined(SIGCLD)
# define SIGCHLD	SIGCLD
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO	0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO	1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO	2
#endif

#ifdef HASFLOCK
# include <sys/file.h>
#endif

#ifndef LOCK_SH
# define LOCK_SH	0x01	/* shared lock */
# define LOCK_EX	0x02	/* exclusive lock */
# define LOCK_NB	0x04	/* non-blocking lock */
# define LOCK_UN	0x08	/* unlock */
#endif

#ifndef SIG_ERR
# define SIG_ERR	((void (*)()) -1)
#endif

/*
**  Size of tobuf (deliver.c)
**	Tweak this to match your syslog implementation.  It will have to
**	allow for the extra information printed.
*/

#ifndef TOBUFSIZE
# define TOBUFSIZE (1024 - 256)
#endif

/* fork routine -- set above using #ifdef _osname_ or in Makefile */
# ifndef FORK
# define FORK		vfork		/* function to call to fork mailer */
# endif
