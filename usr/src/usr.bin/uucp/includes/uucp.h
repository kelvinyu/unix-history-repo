/*	uucp.h	5.4	85/01/22	*/

#include "stdio.h"

/*
 * Determine local uucp name of this machine.
 * Define one of the following:
 *
 * For UCB 4.1A and later systems, you will have the gethostname(2) call.
 * If this call exists, define GETHOSTNAME.
 *
 * For USG 3.0 and later systems, you will have the uname(2) call.
 * If this call exists, define UNAME.
 *
 * Some systems have a line of the form '#define sysname "myuucpname",'
 * in the file /usr/include/whoami.h, to identify their machine.
 * If your site does that, define WHOAMI.
 *
 * If your site has <whoami.h>, but you do not want to read
 * that file every time uucp runs, you can compile sysname into uucp.
 * This is faster and more reliable, but binaries do not port.
 * If you want to do that, define CCWHOAMI.
 *
 * Some systems put the local uucp name in a single-line file
 * named /etc/uucpname or /local/uucpname.
 * If your site does that, define UUNAME.
 *
 * Systems running 3Com's UNET will have the getmyhname() call.
 * If you want to, define GETMYHNAME.
 *
 * You should also define MYNAME to be your uucp name.
 *
 * For each of the above that are defined, uucp checks them in order.
 * It stops on the first method that returns a non null name.
 * If everything fails, it uses "unknown" for the system name.
 */
#define	GETHOSTNAME
/*#define CCWHOAMI	*/
/* If the above fails ... */
#define	MYNAME	"erewhon"

/*
 * If you have it, include <sysexits.h> to use exit
 * codes that will be understood by sendmail.
 * Otherwise, define EX_NOHOST, EX_CANTCREAT, and EX_NOINPUT.
 */
#include <sysexits.h>
/*#define EX_NOINPUT	66*/
/*#define EX_NOHOST	68*/
/*#define EX_CANTCREAT	73*/

/*
 * Define the various kinds of connections to include.
 * The complete list is in the condevs array in condevs.c
 */
/*#define DN11		/* standard dialer */
/*#define DATAKIT	/* ATT's datakit */
/*#define PNET		/* Purdue network */
/*#define DF02		/* Dec's DF02/DF03 */
/*#define HAYES		/* Hayes' Smartmodem */
/*#define VENTEL	/* ventel dialer */
/*#define VADIC		/* Racal-Vadic 345x */
/*#define VA212		/* Racal-Vadic 212 */
/*#define VA811S	/* Racal-Vadic 811S dialer, 831 adaptor */
#define VA820		/* Racal-Vadic 820 dialer, 831 adaptor */
/*#define RVMACS	/* Racal-Vadic MACS  820 dialer, 831 adaptor */
/*#define VMACS		/* Racal-Vadic MACS  811 dialer, 831 adaptor */
/*#define UNETTCP	/* 3Com's UNET */
#define BSDTCP		/* 4.2bsd TCP/IP */
#define PAD		/* X.25 PAD */
/*#define MICOM		/* micom mux port */
/*#define NOVATION	/* Novation modem */

#if defined(UNETTCP) || defined(BSDTCP)
#define TCPIP
#endif

#ifdef	VENTEL
/*
 * We need a timer to write slowly to ventels.
 * define INTERVALTIMER to use 4.2 bsd interval timer.
 * define FASTTIMER if you have the nap() system call.
 * define FTIME if you have the ftime() system call.
 * define BUSYLOOP if you must do a busy loop.
 * Look at uucpdelay() in condevs.c for details.
 */
#define	FTIME
#endif

/*
 * If your site is using "ndir.h" to retrofit the Berkeley
 * directory reading routines, define NDIR.
 * You will probably also have to set LIBNDIR in Makefile.
 * Otherwise, <dir.h> is assumed to have the Berkeley directory definitions.
 */
/*#define	NDIR	*/

/*
 * If yours is a BTL system III, IV, or so-on site, define USG.
 */
/*#define	USG	*/

/*
 * If you are running 4.2bsd, define BSD4_2
 */
#define BSD4_2

/*
 * If you are using /etc/inetd with 4.2bsd, define BSDINETD
 */
#define BSDINETD

/*#define VMSDTR	/* Turn on modem control on vms(works DTR) for
			   develcon and gandalf ports to gain access */

/* define the last characters for ACU */
#define ACULAST "-<"

/* define the value of WFMASK - for umask call - used for all uucp work files */
#define WFMASK 0137

/* define UUSTAT if you need "uustat" command */
/* #define UUSTAT	*/

/*	define UUSUB if you need "uusub" command */
/* #define UUSUB /**/

/* define the value of LOGMASK - for LOGFILE, SYSLOG, ERRLOG */
#define	LOGMASK	0133

/* All files are given at least the following at the final destination */
/* It is also the default mode, so '666' is recommended */
/* and 444 is minimal (minimally useful, maximally annoying) */
#define	BASEMODE	0666

/*
 * Define NOSTRANGERS if you don't want to accept transactions from
 * sites that are not in your L.sys file (see cico.c)
 */
#define NOSTRANGERS
/*
 * Traditionally LCK (lock) files have been kept in /usr/spool/uucp.
 * If you want that define LOCKDIR to be ".".
 * If you want the locks kept in a subdirectory, define LOCKDIR as "LCK.".
 * Good news about LCK. subdirectory: the directory can be mode 777 so
 * unprivileged programs can share the uucp locking system,
 * and the subdirectory keeps down clutter in the main directory.
 * The BAD news: you have to change 'tip' and another programs that
 * know where the LCK files are kept, and you have to change your /etc/rc
 * if your rc cleans out the lock files (as it should).
 */
/*#define	LOCKDIR	"LCK." */
#define	LOCKDIR	"."

/* 
 * If you want uucp and uux to copy the data files by default,
 * don't define DONTCOPY (This is the way older 4bsd uucps worked)
 * If you want uucp and uux to use the original files instead of
 * copies, define DONTCOPY (This is the way System III and V work)
 */
#define DONTCOPY

/*
 * Very few (none I know of) systems use the sequence checking feature.
 * If you are not going to use it (hint: you are not),
 * do not define GNXSEQ.  This saves precious room on PDP11s.
 */
/*#define	GNXSEQ/* comment this out to save space */

#define XQTDIR		"/usr/spool/uucp/XTMP"
#define SQFILE		"/usr/lib/uucp/SQFILE"
#define SQTMP		"/usr/lib/uucp/SQTMP"
#define SLCKTIME	5400	/* system/device timeout (LCK.. files) */
#define SEQFILE		"/usr/lib/uucp/SEQF"
#define SYSFILE		"/usr/lib/uucp/L.sys"
#define DEVFILE		"/usr/lib/uucp/L-devices"
#define DIALFILE	"/usr/lib/uucp/L-dialcodes"
#define USERFILE	"/usr/lib/uucp/USERFILE"
#define	CMDFILE		"/usr/lib/uucp/L.cmds"

#define SPOOL		"/usr/spool/uucp"
#define SQLOCK		"LCK.SQ"
#define SYSLOG		"/usr/spool/uucp/SYSLOG"
#define PUBDIR		"/usr/spool/uucppublic"

#define SEQLOCK		"LCK.SEQL"
#define CMDPRE		'C'
#define DATAPRE		'D'
#define XQTPRE		'X'

#define LOGFILE	"/usr/spool/uucp/LOGFILE"
#define ERRLOG	"/usr/spool/uucp/ERRLOG"
#define CMDSDIR	"/usr/spool/uucp/C."
#define DATADIR	"/usr/spool/uucp/D."
#define XEQTDIR	"/usr/spool/uucp/X."

#define RMTDEBUG	"AUDIT"
#define CORRUPT		"CORRUPT"
#define SQTIME		60
#define TRYCALLS	2	/* number of tries to dial call */

#define LLEN	50
#define MAXRQST	250

#define DEBUG(l, f, s) if (Debug >= l) fprintf(stderr, f, s); else

#define ASSERT(e, s1, s2, i1) if (!(e)) {assert(s1, s2, i1);cleanup(FAIL);}else

#define SAME 0
#define ANYREAD 0004
#define ANYWRITE 02
#define FAIL -1
#define SUCCESS 0
#define CNULL (char *) 0
#define STBNULL (struct sgttyb *) 0
#define MASTER 1
#define SLAVE 0
#define MAXFULLNAME 255
#define MAXMSGTIME 45
#define NAMESIZE 255
#define EOTMSG "\04\n\04\n"
#define CALLBACK 1
#define ONEDAY	86400L

	/*  commands  */
#define SHELL		"/bin/sh"
#define MAIL		"/usr/lib/sendmail"
#define UUCICO		"/usr/lib/uucp/uucico"
#define UUXQT		"/usr/lib/uucp/uuxqt"
#define UUCP		"uucp"

	/*  call connect fail stuff  */
#define CF_SYSTEM	-1
#define CF_TIME		-2
#define CF_LOCK		-3
#define	CF_NODEV	-4
#define CF_DIAL		-5
#define CF_LOGIN	-6

#define F_NAME 0
#define F_TIME 1
#define F_LINE 2
#define F_CLASS 3	/* an optional prefix and the speed */
#define F_PHONE 4
#define F_LOGIN 5

#define MAXPH	60	/* maximum length of a phone number */

	/* This structure tells how to get to a device */
struct condev {
	char *CU_meth;		/* method, such as 'ACU' or 'DIR' */
	char *CU_brand;		/* brand, such as 'Hayes' or 'Vadic' */
	int (*CU_gen)();	/* what to call to search for brands */
	int (*CU_open)();	/* what to call to open brand */
	int (*CU_clos)();	/* what to call to close brand */
};

	/* This structure tells about a device */
struct Devices {
#define	D_type		D_arg[0]
#define	D_line		D_arg[1]
#define	D_calldev	D_arg[2]
#define	D_class		D_arg[3]
#define	D_brand		D_arg[4]
#define	D_CHAT		5
	int  D_numargs;
	int  D_speed;
	char *D_arg[20];
	char D_argbfr[100];
};

	/*  system status stuff  */
#define SS_OK		0
#define SS_FAIL		4
#define SS_NODEVICE	1
#define SS_CALLBACK	2
#define SS_INPROGRESS	3
#define SS_BADSEQ	5
#define SS_WRONGTIME	6

	/*  fail/retry parameters  */
#define RETRYTIME 600
#define MAXRECALLS 20

	/*  stuff for command execution  */
#define X_RQDFILE	'F'
#define X_STDIN		'I'
#define X_STDOUT	'O'
#define X_CMD		'C'
#define X_USER		'U'
#define X_SENDFILE	'S'
#define	X_NONOTI	'N'
#define X_RETURNTO	'R'
#define	X_NONZERO	'Z'
#define X_LOCK		"LCK.XQT"
#define X_LOCKTIME	3600

#define WKDSIZE 100	/*  size of work dir name  */

extern int Ifn, Ofn;
extern char Rmtname[];
extern char User[];
extern char Loginuser[];
extern char *Spool;
extern char Myname[];
extern char Myfullname[];
extern int Debug;
extern int Bspeed;
extern char Wrkdir[];
extern long Retrytime;
extern short Usrf;
extern int IsTcpIp;
extern char Progname[];
extern int (*CU_end)();
extern struct condev condevs[];
extern int nologinflag;
extern char NOLOGIN[];

extern	char DLocal[], DLocalX[], *subfile(), *subdir();

/* Commonly called routines which return non-int value */
extern	char *ttyname(), *strcpy(), *strcat(), *index(), *rindex(),
		*fgets(), *calloc(), *malloc(), *fdig(), *ttyname(),
		*cfgets();
extern	long lseek();
extern	FILE *rpopen();

extern char _FAILED[], CANTOPEN[], DEVNULL[];
