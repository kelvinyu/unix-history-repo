/*
**  SENDMAIL.H -- Global definitions for sendmail.
**
**	@(#)sendmail.h	3.46	%G%
*/



# ifdef _DEFINE
# define EXTERN
# else  _DEFINE
# define EXTERN extern
# endif _DEFINE

# include <stdio.h>
# include <ctype.h>
# include "conf.h"
# include "useful.h"

/*
**  Configuration constants.
**	There shouldn't be much need to change these....
*/

# define MAXLINE	256		/* max line length */
# define MAXNAME	128		/* max length of a name */
# define MAXFIELD	2500		/* max total length of a hdr field */
# define MAXPV		40		/* max # of parms to mailers */
# define MAXHOP		30		/* max value of HopCount */
# define MAXATOM	30		/* max atoms per address */
# define MAXMAILERS	10		/* maximum mailers known to system */
# define SPACESUB	('.'|0200)	/* substitution for <lwsp> */

/* values for ArpaMode -- these are ordered!! */
# define ARPA_NONE	0	/* not in arpanet mode */
# define ARPA_OLD	1	/* in old arpanet mode */
# define ARPA_MAIL	2	/* in regular arpanet mail */
# define ARPA_FILE	3	/* reading over data connection */

extern char	Arpa_Info[];	/* the message number for Arpanet info */






/*
**  Address structure.
**	Addresses are stored internally in this structure.
*/

struct address
{
	char		*q_paddr;	/* the printname for the address */
	char		*q_user;	/* user name */
	char		*q_host;	/* host name */
	short		q_mailer;	/* mailer to use */
	short		q_rmailer;	/* real mailer (before mapping) */
	u_short		q_flags;	/* status flags, see below */
	short		q_uid;		/* user-id of receiver (if known) */
	short		q_gid;		/* group-id of receiver (if known) */
	char		*q_home;	/* home dir (local mailer only) */
	struct address	*q_next;	/* chain */
	struct address	*q_alias;	/* address this results from */
};

typedef struct address ADDRESS;

# define QDONTSEND	000001	/* don't send to this address */
# define QBADADDR	000002	/* this address is verified bad */
# define QGOODUID	000004	/* the q_uid q_gid fields are good */
# define QPRIMARY	000010	/* set from argv */





/*
**  Mailer definition structure.
**	Every mailer known to the system is declared in this
**	structure.  It defines the pathname of the mailer, some
**	flags associated with it, and the argument vector to
**	pass to it.  The flags are defined in conf.c
**
**	The host map is a list of lists of strings.  Within each
**	list, any host is mapped to the last host in the list.
**	This allows multiple names, as well as doing clever
**	mail grouping in point-to-point networks.  Note: this
**	is only used internally, so the apparent host is still
**	kept around.
**
**	The argument vector is expanded before actual use.  All
**	words except the first are passed through the macro
**	processor.
*/

struct mailer
{
	char	*m_name;	/* symbolic name of this mailer */
	char	*m_mailer;	/* pathname of the mailer to use */
	u_long	m_flags;	/* status flags, see below */
	short	m_badstat;	/* the status code to use on unknown error */
	short	m_mno;		/* mailer number internally */
	char	*m_from;	/* pattern for From: header */
	char	**m_argv;	/* template argument vector */
	ADDRESS	*m_sendq;	/* list of addresses to send to */
};

typedef struct mailer	MAILER;

# define M_FOPT		000001	/* mailer takes picky -f flag */
# define M_ROPT		000002	/* mailer takes picky -r flag */
# define M_QUIET	000004	/* don't print error on bad status */
# define M_RESTR	000010	/* must be daemon to execute */
# define M_NHDR		000020	/* don't insert From line */
# define M_LOCAL	000040	/* delivery is to this host */
# define M_STRIPQ	000100	/* strip quote characters from user/host */
# define M_MUSER	000200	/* mailer can handle multiple users at once */
# define M_NEEDFROM	000400	/* need arpa-style From: line */
# define M_NEEDDATE	001000	/* need arpa-style Date: line */
# define M_MSGID	002000	/* need Message-Id: field */
# define M_USR_UPPER	010000	/* preserve user case distinction */
# define M_HST_UPPER	020000	/* preserve host case distinction */
# define M_FULLNAME	040000	/* want Full-Name field */

# define M_ARPAFMT	(M_NEEDDATE|M_NEEDFROM|M_MSGID)

EXTERN MAILER *Mailer[MAXMAILERS+1];

/* special mailer numbers */
# define MN_LOCAL	0	/* local mailer */
# define MN_PROG	1	/* program mailer */
/* mailers from 2 on are arbitrary */



/*
**  Header structure.
**	This structure is used internally to store header items.
*/

struct header
{
	char		*h_field;	/* the name of the field */
	char		*h_value;	/* the value of that field */
	struct header	*h_link;	/* the next header */
	u_short		h_flags;	/* status bits, see below */
	u_long		h_mflags;	/* m_flags bits needed */
};

typedef struct header	HDR;

EXTERN HDR	*Header;	/* head of header list */

/*
**  Header information structure.
**	Defined in conf.c, this struct declares the header fields
**	that have some magic meaning.
*/

struct hdrinfo
{
	char	*hi_field;	/* the name of the field */
	u_short	hi_flags;	/* status bits, see below */
	u_short	hi_mflags;	/* m_flags needed for this field */
};

extern struct hdrinfo	HdrInfo[];

/* bits for h_flags and hi_flags */
# define H_EOH		00001	/* this field terminates header */
# define H_DEFAULT	00004	/* if another value is found, drop this */
# define H_USED		00010	/* indicates that this has been output */
# define H_CHECK	00020	/* check h_mflags against m_flags */
# define H_ACHECK	00040	/* ditto, but always (not just default) */
# define H_FORCE	00100	/* force this field, even if default */
# define H_ADDR		00200	/* this field contains addresses */


/*
**  Rewrite rules.
*/

struct rewrite
{
	char	**r_lhs;	/* pattern match */
	char	**r_rhs;	/* substitution value */
	struct rewrite	*r_next;/* next in chain */
};

extern struct rewrite	*RewriteRules[];

# define MATCHANY	'\020'	/* match one or more tokens */
# define MATCHONE	'\021'	/* match exactly one token */
# define MATCHCLASS	'\022'	/* match one token in a class */
# define MATCHREPL	'\023'	/* replacement on RHS for above */

# define CANONNET	'\025'	/* canonical net, next token */
# define CANONHOST	'\026'	/* canonical host, next token */
# define CANONUSER	'\027'	/* canonical user, next N tokens */



/*
**  Symbol table definitions
*/

struct symtab
{
	char		*s_name;	/* name to be entered */
	char		s_type;		/* general type (see below) */
	struct symtab	*s_next;	/* pointer to next in chain */
	union
	{
		long	sv_class;	/* bit-map of word classes */
		ADDRESS	*sv_addr;	/* pointer to address header */
		MAILER	*sv_mailer;	/* pointer to mailer */
		char	*sv_alias;	/* alias */
	}	s_value;
};

typedef struct symtab	STAB;

/* symbol types */
# define ST_UNDEF	0	/* undefined type */
# define ST_CLASS	1	/* class map */
# define ST_ADDRESS	2	/* an address in parsed format */
# define ST_MAILER	3	/* a mailer header */
# define ST_ALIAS	4	/* an alias */

# define s_class	s_value.sv_class
# define s_addr		s_value.sv_addr
# define s_mailer	s_value.sv_mailer
# define s_alias	s_value.sv_alias

extern STAB	*stab();

/* opcodes to stab */
# define ST_FIND	0	/* find entry */
# define ST_ENTER	1	/* enter if not there */




/*
**  Statistics structure.
*/

struct statistics
{
	time_t	stat_itime;		/* file initialization time */
	short	stat_size;		/* size of this structure */
	long	stat_nf[MAXMAILERS];	/* # msgs from each mailer */
	long	stat_bf[MAXMAILERS];	/* kbytes from each mailer */
	long	stat_nt[MAXMAILERS];	/* # msgs to each mailer */
	long	stat_bt[MAXMAILERS];	/* kbytes to each mailer */
};

EXTERN struct statistics	Stat;
extern long			kbytes();	/* for _bf, _bt */




/*
**  Global variables.
*/

EXTERN bool	FromFlag;	/* if set, "From" person is explicit */
EXTERN bool	MailBack;	/* mail back response on error */
EXTERN bool	BerkNet;	/* called from BerkNet */
EXTERN bool	WriteBack;	/* write back response on error */
EXTERN bool	NoAlias;	/* if set, don't do any aliasing */
EXTERN bool	ForceMail;	/* if set, mail even if already got a copy */
EXTERN bool	MeToo;		/* send to the sender also */
EXTERN bool	IgnrDot;	/* don't let dot end messages */
EXTERN bool	SaveFrom;	/* save leading "From" lines */
EXTERN bool	Verbose;	/* set if blow-by-blow desired */
EXTERN bool	GrabTo;		/* if set, get recipients from msg */
EXTERN bool	DontSend;	/* mark recipients as QDONTSEND */
EXTERN bool	NoReturn;	/* don't return letter to sender */
EXTERN int	OldUmask;	/* umask when sendmail starts up */
EXTERN int	Debug;		/* debugging level */
EXTERN int	Errors;		/* set if errors */
EXTERN int	ExitStat;	/* exit status code */
EXTERN int	ArpaMode;	/* ARPANET handling mode */
EXTERN long	MsgSize;	/* size of the message in bytes */
extern char	InFileName[];	/* input file name */
extern char	Transcript[];	/* the transcript file name */
EXTERN FILE	*TempFile;	/* mail temp file */
EXTERN ADDRESS	From;		/* the person it is from */
EXTERN char	*To;		/* the target person */
EXTERN int	HopCount;	/* hop count */
EXTERN long	CurTime;	/* time of this message */
EXTERN int	AliasLevel;	/* depth of aliasing */
EXTERN char	*OrigFrom;	/* the From: line read from the message */
EXTERN bool	SuprErrs;	/* set if we are suppressing errors */
EXTERN bool	HasXscrpt;	/* set if we have a transcript */


# include	<sysexits.h>

# define setstat(s)		{ if (ExitStat == EX_OK) ExitStat = s; }


/* useful functions */

extern char	*newstr();
extern ADDRESS	*parse();
extern char	*xalloc();
extern char	*expand();
extern bool	sameaddr();
