/*	vdfmt.h	1.1	86/07/05	*/

/*
**
*/

#include <setjmp.h>
#include "tahoe/mtpr.h"
#include "param.h"
#include "buf.h"
#include "inode.h" 
#include "fs.h"
#include "tahoevba/vbaparam.h"
#include "tahoevba/vdreg.h"

#include "tahoe/cp.h"
extern	struct cpdcb_i cpin;

/*
 * Poll console and return 1 if input is present.
 */
#define	input() \
    (uncache(&cpin.cp_hdr.cp_unit), (cpin.cp_hdr.cp_unit&CPDONE))

/* Configuration parameters */
#define	MAXCTLR		8			/* Maximum # of controllers */
#define	MAXDRIVE	16			/* Max drives per controller */

#define NUMMAP		1			/* # Cyls in bad sector map */
#define	NUMMNT		1			/* # cyls for diagnostics */
#define	NUMREL		3			/* # Cyls in relocation area */
#define	NUMSYS		(NUMREL+NUMMNT+NUMMAP)	/* Total cyls used by system */

#define	MAXTRKS		24
#define	MAXSECS_PER_TRK	64
#define	MAXERR		1000
#define SECSIZ		512
#define	TRKSIZ		((SECSIZ/sizeof(long)) * MAXSECS_PER_TRK)
#define bytes_trk	(CURRENT->vc_nsec * SECSIZ)

#define	HARD_ERROR	(DRVNRDY | INVDADR | DNEMEM | PARERR | OPABRT | \
			 WPTERR | DSEEKERR | NOTCYLERR)
#define DATA_ERROR	(CTLRERR | UCDATERR | DCOMPERR | DSERLY | DSLATE | \
			 TOPLUS | TOMNUS | CPDCRT | \
			 HRDERR | SFTERR)
#define HEADER_ERROR	(HCRCERR | HCMPERR)
#define	NRM		(short)0
#define	BAD		(short)VDUF
#define WPT		(short)(NRM | VDWPT)
#define RELOC_SECTOR	(short)(VDALT)
#define	ALT_SECTOR	(short)(VDALT)


/* New types used by VDFORMAT */

typedef enum {false, true} boolean;	/* Standard boolean expression */

typedef enum {u_false, u_true, u_unknown} uncertain;


	/* Free bad block allocation bit map */
typedef struct {
	long	free_error;
	enum	{ALLOCATED, NOTALLOCATED} free_status;
} fmt_free;

	/* Type of relocation that took place */
typedef enum {SINGLE_SECTOR, FULL_TRACK} rel_type;

	/* Error table format */
typedef struct {
	dskadr	err_adr;
	long	err_stat;
} fmt_err;

/* utilities */

int	blkcopy(), blkzero(), to_sector(), to_track(), data_ok();
boolean blkcmp(), get_yes_no(), is_in_map();
boolean	is_formatted(), read_bad_sector_map();
dskadr	*from_sector(), *from_track(), *from_unix();
dskadr	is_relocated(), *new_location();

/* Operation table */

extern int	format(), verify(), relocate(), info();
extern int	correct(), profile(), exercise();

#define	NUMOPS	7
	/* operation bit mask values (must match order in operations table) */
#define	FORMAT_OP	0x01	/* Format operation bit */
#define	VERIFY_OP	0x02	/* Verify operation bit */
#define	RELOCATE_OP	0x04	/* Relocate operation bit */
#define	INFO_OP		0x08	/* Info operation bit */
#define	CORRECT_OP	0x10	/* Correct operation bit */
#define	PROFILE_OP	0x20	/* Profile operation bit */
#define	EXERCISE_OP	0x40	/* Exercise operation bit */

typedef struct {
	int	(*routine)();
	char	*op_name;
	char	*op_action;
} op_tbl;

op_tbl	operations[NUMOPS];


/* Operation table type and definitions */

typedef struct {
	int	op;
	int	numpat;
} op_spec;

op_spec	ops_to_do[MAXCTLR][MAXDRIVE];


/* Contains all the current parameters */

typedef enum {formatted, half_formatted, unformatted, unknown} drv_stat;
typedef enum {fmt, vfy, rel, cor, inf, cmd, exec, prof} state;
typedef enum {sub_chk,sub_rcvr,sub_stat,sub_rel,sub_vfy,sub_fmt,sub_sk}substate;

/* Different environments for setjumps */
jmp_buf	reset_environ;	/* Use when reset is issued */
jmp_buf	quit_environ;	/* Use when you want to quit what your doing */
jmp_buf	abort_environ;	/* Use when nothing can be done to recover */


/* Flaw map definitions and storage */

typedef struct {
	short	bs_cyl;			/* Cylinder position of flaw */
	short	bs_trk;			/* Track position of flaw */
	long	bs_offset;		/* (byte) Position of flaw on track */
	long	bs_length;		/* Length (in bits) of flaw */
	dskadr	bs_alt;			/* Addr of alt sector (all 0 if none) */
	enum {flaw_map, scanning, operator} bs_how; /* How it was found */
} bs_entry ;


struct {
	int		controller;
	int		drive;
	state		state;
	substate	substate;
	int		error;
	dskadr		daddr;
} cur;


/* Controller specific information */

typedef struct {
	uncertain	alive;
	cdr		*addr;
	char		*name;
	int		type;
	fmt_err		(*decode_pos)();
	bs_entry	(*code_pos)();
	int		(*cylinder_skew)();
	int		(*track_skew)();
} ctlr_info;

ctlr_info	c_info[MAXCTLR];


/* drive specific information */

typedef struct {
	uncertain	alive;
	int		id;
	struct	vdconfig *info;
	int		trk_size;
	int		num_slip;
	int		track_skew;
	drv_stat	condition;
} drive_info;

drive_info	d_info[MAXCTLR][MAXDRIVE];


#define	D_INFO	 d_info[cur.controller][cur.drive]
#define	C_INFO	 c_info[cur.controller]

#define	CURRENT	 D_INFO.info
typedef struct {
	unsigned int	bs_id;		/* Pack id */
	unsigned int	bs_count;	/* number of known bad sectors */
	unsigned int	bs_max;		/* Maximum allowable bad sectors */
	bs_entry	list[1];
} bs_map;

#define MAX_FLAWS (((TRKSIZ*sizeof(long))-sizeof(bs_map))/sizeof(bs_entry))

long	bs_map_space[TRKSIZ];
bs_map		*bad_map;

boolean		kill_processes;
int		num_controllers;

/* Pattern buffers and the sort */
fmt_free	free_tbl[NUMREL*MAXTRKS][MAXSECS_PER_TRK];
fmt_mdcb	mdcb;		/* Master device control block */
fmt_dcb		dcb;		/* Device control blocks */

long	pattern_0[TRKSIZ],  pattern_1[TRKSIZ];
long	pattern_2[TRKSIZ],  pattern_3[TRKSIZ];
long	pattern_4[TRKSIZ],  pattern_5[TRKSIZ];
long	pattern_6[TRKSIZ],  pattern_7[TRKSIZ];
long	pattern_8[TRKSIZ],  pattern_9[TRKSIZ];
long	pattern_10[TRKSIZ], pattern_11[TRKSIZ];
long	pattern_12[TRKSIZ], pattern_13[TRKSIZ];
long	pattern_14[TRKSIZ], pattern_15[TRKSIZ];

/* Will be filled in at boot time with pointers to above patterns */
long	*pattern_address[16];

/* Double buffer for scanning existing file systems and general scratch */
long	scratch[TRKSIZ];
long	save[TRKSIZ];


/* XXX */
/*
 * Flaw map stuff 
 */
typedef struct {
	long	flaw_sync;
	short	flaw_cyl;
	char	flaw_trk;
	char	flaw_sec;
	struct {
		short	flaw_offset;
		short	flaw_length;
	} flaw_pos[4];
	char	flaw_status;
	char	flaw_junk[1024]; /* Fill up 518 byte block */
} flaw;

typedef struct {
	long		smde_sync;
	unsigned	adr_cyl  : 12;
	unsigned	adr_trk  : 8;
	unsigned	adr_sec  : 8;
	unsigned	sec_flgs : 4;
	unsigned	alt_cyl  : 12;
	unsigned	alt_trk  : 8;
	unsigned	alt_sec  : 8;
	char		smde_junk[1024];
} smde_hdr;

#define	CDCSYNC		0x1919
#define	SMDSYNC		0x0019
#define	SMDESYNC	0x0009
#define	SMDE1SYNC	0x000d

/* XXX */

/*
 * Disk drive geometry definition.
 */
struct	geometry {
	u_short	g_nsec;		/* sectors/track */
	u_short	g_ntrak;	/* tracks/cylinder */
	u_int	g_ncyl;		/* cylinders/drive */
};

/*
 * Flaw testing patterns.
 */
struct	flawpat {
	u_int	fp_pat[16];
};

/*
 * Disk drive configuration information.
 */
struct	vdconfig {
	char	*vc_name;		/* drive name for selection */
	struct	geometry vc_geometry;	/* disk geometry */
#define	vc_nsec	vc_geometry.g_nsec
#define	vc_ntrak vc_geometry.g_ntrak
#define	vc_ncyl	vc_geometry.g_ncyl
	int	vc_nslip;		/* # of slip sectors */
	int	vc_rpm;			/* revolutions/minute */
	int	vc_traksize;		/* bits/track for flaw map interp */
	struct	flawpat *vc_pat;	/* flaw testing patterns */
	char	*vc_type;		/* informative description */
};
struct	vdconfig vdconfig[];
int	ndrives;
