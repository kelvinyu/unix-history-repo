/*
 *	@(#)disklabel.h	7.1 (Berkeley) %G%
 */

/*
 * Disk description table, see disktab(5)
 */
#define	DISKTAB		"/etc/disktab"

/*
 * Each disk has a label which includes information about the hardware
 * disk geometry, filesystem partitions, and drive specific information.
 * The label is in block 0 or 1, possibly offset from the beginning
 * to leave room for a bootstrap, etc.
 */

#define LABELSECTOR	0		/* sector containing label */
#define LABELOFFSET	64		/* offset of label in sector */
#define DISKMAGIC	0x82564557	/* The disk magic number */
#ifndef MAXPARTITIONS
#define	MAXPARTITIONS	8
#endif

#define	DTYPE_SMD		1		/* SMD, XSMD; VAX hp/up */
#define	DTYPE_MSCP		2		/* MSCP */
#define	DTYPE_DEC		3		/* other DEC (rk, rl) */
#define	DTYPE_SCSI		4		/* SCSI/ST506 etc. */
#define	DTYPE_ESDI		5		/* ESDI interface */
#define	DTYPE_FLOPPY		10		/* floppy */

#ifdef DKTYPENAMES
static char *dktypenames[] = {
	"unknown",
	"SMD",
	"MSCP",
	"old DEC",
	"SCSI",
	"ESDI",
	"type 6",
	"type 7",
	"type 8",
	"type 9",
	"floppy",
	0
};
#define DKMAXTYPES	(sizeof(dktypenames) / sizeof(dktypenames[0]) - 1)
#endif


#ifndef LOCORE
struct disklabel {
	u_long	d_magic;		/* the magic number */
	short	d_type;			/* drive type */
	short	d_subtype;		/* controller/d_type specific */
			/* disk geometry: */
	u_long	d_secsize;		/* # of bytes per sector */
	u_long	d_nsectors;		/* # of sectors per track */
	u_long	d_ntracks;		/* # of tracks per cylinder */
	u_long	d_ncylinders;		/* # of cylinders per unit */
	u_long	d_acylinders;		/* # of alt. cylinders per unit */
	u_long	d_secpercyl;		/* # of sectors per cylinder */
	u_long	d_secperunit;		/* # of sectors per unit */
			/* hardware characteristics: */
	u_long	d_rpm;			/* rotational speed */
	u_short	d_interleave;		/* hardware sector interleave */
	u_short	d_sectorskew;		/* sector 0 skew, per track */
	u_short	d_headswitch;		/* head switch time, usec */
	u_short	d_trkseek;		/* track-to-track seek, usec */
	u_long	d_flags;		/* generic flags */
#define NDDATA 10
	u_long	d_drivedata[NDDATA];	/* drive-type specific information */
			/* other identification: */
	char	d_typename[16];		/* type name, e.g. "eagle" */
	char	d_name[16];		/* pack identifier */
	u_long	d_magic2;		/* the magic number (again) */
	u_short	d_checksum;		/* xor of data incl. partitions */
			/* filesystem and partition information: */
	u_short	d_npartitions;		/* number of partitions in following */
	u_long	d_bbsize;		/* size of boot area at sn0, bytes */
	u_long	d_sbsize;		/* max size of fs superblock, bytes */
	struct	partition {		/* the partition table */
		u_long	p_size;		/* number of sectors in partition */
		u_long	p_offset;	/* starting sector */
		u_long	p_fsize;	/* filesystem basic fragment size */
		u_char	p_fstype;	/* filesystem type, see below */
		u_char	p_frag;		/* filesystem fragments per block */
		u_short	p_cpg;		/* filesystem cylinders per group */
	} d_partitions[MAXPARTITIONS];	/* actually may be more */
};
#endif LOCORE

/*
 * Filesystem type and version.
 * Used to interpret other filesystem-specific
 * per-partition information.
 */
#define	FS_UNUSED	0		/* unused */
#define	FS_SWAP		1		/* swap */
#define	FS_V6		2		/* Sixth Edition */
#define	FS_V7		3		/* Seventh Edition */
#define	FS_SYSV		4		/* System V */
#define	FS_V71K		5		/* V7 with 1K blocks (4.1, 2.9) */
#define	FS_V8		6		/* Eighth Edition, 4K blocks */
#define	FS_BSDFFS	7		/* 4.2BSD fast file system */

#ifdef	DKTYPENAMES
static char *fstypenames[] = {
	"unused",
	"swap",
	"Version 6",
	"Version 7",
	"System V",
	"4.1BSD",
	"Eighth Edition",
	"4.2BSD",
	0
};
#define FSMAXTYPES	(sizeof(fstypenames) / sizeof(fstypenames[0]) - 1)
#endif

/*
 * flags shared by various drives:
 */
#define		D_REMOVABLE	0x01		/* removable media */
#define		D_ECC		0x02		/* supports ECC */
#define		D_BADSECT	0x04		/* supports bad sector forw. */
#define		D_RAMDISK	0x08		/* disk emulator */
#define		D_CHAIN		0x10		/* can do back-back transfers */

/*
 * Drive data for SMD.
 */
#define	d_smdflags	d_drivedata[0]
#define		D_SSE		0x1		/* supports skip sectoring */
#define	d_mindist	d_drivedata[1]
#define	d_maxdist	d_drivedata[2]
#define	d_sdist		d_drivedata[3]

/*
 * Drive data for ST506.
 */
#define d_precompcyl	d_drivedata[0]
#define d_gap3		d_drivedata[1]		/* used only when formatting */

#ifndef LOCORE
/*
 * Structure used to perform a format
 * or other raw operation, returning data
 * and/or register values.
 * Register identification and format
 * are device- and driver-dependent.
 */
struct format_op {
	char	*df_buf;
	int	df_count;		/* value-result */
	daddr_t	df_startblk;
	int	df_reg[8];		/* result */
};

/*
 * Disk-specific ioctls.
 */
		/* get and set disklabel; last form used internally */
#define DIOCGDINFO	_IOR(d, 101, struct disklabel)	/* get */
#define DIOCSDINFO	_IOW(d, 102, struct disklabel)	/* set */
#define DIOCWDINFO	_IOW(d, 103, struct disklabel)	/* set and write back */
#define DIOCGDINFOP	_IOW(d, 104, struct disklabel *) /* get pointer */

/* do format operation, read or write */
#define DIOCRFORMAT	_IOWR(d, 105, struct format_op)
#define DIOCWFORMAT	_IOWR(d, 106, struct format_op)

#define DIOCSSTEP	_IOW(d, 107, int)	/* set step rate */
#define DIOCSRETRIES	_IOW(d, 108, int)	/* set # of retries */

#endif LOCORE

#if !defined(KERNEL) && !defined(LOCORE)
struct	disklabel *getdiskbyname();
#endif
