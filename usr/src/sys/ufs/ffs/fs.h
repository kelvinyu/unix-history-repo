/* Copyright (c) 1981 Regents of the University of California */

/*	fs.h	1.2	%G%	*/

/*
 * Each disk drive contains some number of file systems.
 * A file system consists of a number of cylinder groups.
 * Each cylinder group has inodes and data.
 *
 * A file system is described by its super-block, which in turn
 * describes the cylinder groups.  The super-block is critical
 * data and is replicated in each cylinder group to protect against
 * catastrophic loss.  This is done at mkfs time and the critical
 * super-block data does not change, so the copies need not be
 * referenced further unless disaster strikes.
 *
 * For file system fs and a cylinder group number cg:
 *	[BBLOCK]	Boot sector and bad block information
 *	[SBLOCK]	Super-block
 *	[CBLOCK]	Cylinder group block
 *	[IBLOCK..IBLOCK+fs.fs_ipg/INOPB)
 *			Inode blocks
 *	[IBLOCK+fs.fs_ipg/INOPB..fs.fs_fpg/FRAG)
 *			Data blocks
 * The beginning of data blocks for cg in fs is also given by cgdmin(cg,fs).
 */
#define	BBLOCK	((daddr_t)(0*FRAG))
#define	SBLOCK	((daddr_t)(1*FRAG))
#define	CBLOCK	((daddr_t)(2*FRAG))
#define	IBLOCK	((daddr_t)(3*FRAG))

/*
 * Addresses stored in inodes are capable of addressing fragments of `blocks.'
 * File system blocks of size BSIZE can be broken into FRAG pieces,
 * each of which is addressible; these pieces may be sectors, or some
 * multiple of a sector size (e.g. 1k byte units).
 *
 * Large files consist of exclusively large (BSIZE) data blocks.  To avoid
 * undue fragmentation, the last data block of a small file may be
 * allocated as only as many pieces
 * of a large block as are necessary.  The file system format retains
 * only a single pointer to such a fragment, which is a piece of a single
 * BSIZE block which has been divided.  The size of such a fragment is
 * determinable from information in the inode.
 *
 * The file system records space availability at the fragment level;
 * to determine block availability, aligned fragments are examined.
 */ 

/*
 * Information per cylinder group summarized in blocks allocated
 * from first cylinder group data blocks.  These blocks have to be
 * specially noticed in mkfs, icheck and fsck.
 */
struct csum {
	short	cs_ndir;	/* number of directories */
	short	cs_nbfree;	/* number of free blocks */
	short	cs_nifree;	/* number of free inodes */
	short	cs_xx;		/* for later use... */
};
#define	cssize(fs)	((fs)->fs_ncg*sizeof(struct csum))
#define	csaddr(fs)	(cgdmin(0, fs))

/*
 * Each file system has a number of inodes statically allocated.
 * We allocate one inode slot per NBPI data bytes, expecting this
 * to be far more than we will ever need.  Actually, the directory
 * structure has inode numbers kept in 16 bits, so no more than
 * 65K inodes are possible, and this usually cuts off well above
 * the number suggested by NBPI.
 * 
 * THE DIRECTORY STRUCTURE SHOULD BE CHANGED SOON TO ALLOW
 * LARGER INODE NUMBERS (SEE DIR.H).
 */
#define	NBPI	2048

#define	DESCPG	8			/* desired fs_cpg */
#define	MAXCPG	16			/* maximum fs_cpg */
/* MAXCPG is limited only to dimension an array in (struct cg); */
/* it can be made larger as long as that structures size remains sane. */
 
/*
 * Super block for a file system.
 *
 * The super block is nominally located at disk block 1 although
 * this is naive due to bad blocks.  Inode 0 can't be used for normal
 * purposes, thus the root inode is inode 1.
 */
#define	ROOTINO	((ino_t)1)	/* i number of all roots */

#define	FS_MAGIC	0x110854
struct	fs
{
	long	fs_magic;		/* magic number */
	daddr_t	fs_sblkno;		/* offset of super-block in filesys */
	time_t 	fs_time;    		/* last time written */
	daddr_t	fs_size;		/* number of blocks in fs */
	short	fs_ncg;			/* number of cylinder groups */
/* sizes determined by number of cylinder groups and their sizes */
	short	fs_cssize;		/* size of cyl grp summary area */
	short	fs_cgsize;		/* cylinder group size */
/* these fields should be derived from the hardware */
	short	fs_ntrak;		/* tracks per cylinder */
	short	fs_nsect;		/* sectors per track */
	short  	fs_spc;   		/* sectors per cylinder */
/* this comes from the disk driver partitioning */
	short	fs_ncyl;   		/* cylinders in file system */
/* these fields can be computed from the others */
	short	fs_cpg;			/* cylinders per group */
	short	fs_fpg;			/* blocks per group*FRAG */
	short	fs_ipg;			/* inodes per group */
/* these fields must be re-computed after crashes */
	daddr_t	fs_nffree;   		/* total free fragments */
	daddr_t	fs_nbfree;		/* free data blocks */
	ino_t  	fs_nifree;  		/* total free inodes */
/* these fields are cleared at mount time */
	char   	fs_fmod;    		/* super block modified flag */
	char   	fs_ronly;   		/* mounted read-only flag */
	char	fs_fsmnt[32];		/* name mounted on */
	struct	csum *fs_cs;
};

/*
 * Cylinder group macros to locate things in cylinder groups.
 */

/* cylinder group to disk block at very beginning */
#define	cgbase(c,fs)	((daddr_t)((fs)->fs_fpg*(c)))

/* cylinder group to spare super block address */
#define	cgsblock(c,fs)	\
	(cgbase(c,fs) + SBLOCK)

/* convert cylinder group to index of its cg block */
#define	cgtod(c,fs)	\
	(cgbase(c,fs) + CBLOCK)

/* give address of first inode block in cylinder group */
#define	cgimin(c,fs)	\
	(cgbase(c,fs) + IBLOCK)

/* give address of first data block in cylinder group */
#define	cgdmin(c,fs)	(cgimin(c,fs) + (fs)->fs_ipg / INOPF)

/* turn inode number into cylinder group number */
#define	itog(x,fs)	((x)/(fs)->fs_ipg)

/* turn inode number into disk block address */
#define	itod(x,fs)	(cgimin(itog(x,fs),fs)+FRAG*((x)%(fs)->fs_ipg/INOPB))

/* turn inode number into disk block offset */
#define	itoo(x)		((x)%INOPB)

/* give cylinder group number for a disk block */
#define	dtog(d,fs)	((d)/(fs)->fs_fpg)

/* give cylinder group block number for a disk block */
#define	dtogd(d,fs)	((d)%(fs)->fs_fpg)

/*
 * Cylinder group related limits.
 */

/*
 * For each cylinder we keep track of the availability of blocks at different
 * rotational positions, so that we can lay out the data to be picked
 * up with minimum rotational latency.  NRPOS is the number of rotational
 * positions which we distinguish.  With NRPOS 16 the resolution of our
 * summary information is 1ms for a typical 3600 rpm drive.
 */
#define	NRPOS	16		/* number distinct rotational positions */

/*
 * MAXIPG bounds the number of inodes per cylinder group, and
 * is needed only to keep the structure simpler by having the
 * only a single variable size element (the free bit map).
 *
 * N.B.: MAXIPG must be a multiple of INOPB.
 */
#define	MAXIPG	2048		/* max number inodes/cyl group */

/*
 * MAXBPG bounds the number of blocks of data per cylinder group,
 * and is limited by the fact that cylinder groups are at most one block.
 * Its size is derived from the size of blocks and the (struct cg) size,
 * by the number of remaining bits.
 */
#define	MAXBPG	(NBBY*(BSIZE-(sizeof (struct cg)))/FRAG)

#define	CG_MAGIC	0x092752
struct	cg {
	long	cg_magic;		/* magic number */
	time_t	cg_time;		/* time last written */
	short	cg_cgx;			/* we are the cgx'th cylinder group */
	short	cg_ncyl;		/* number of cyl's this cg */
	short	cg_niblk;		/* number of inode blocks this cg */
	short	cg_ndblk;		/* number of data blocks this cg */
	short	cg_nifree;		/* free inodes */
	short	cg_ndir;		/* allocated directories */
	short	cg_nffree;		/* free block fragments */
	short	cg_nbfree;		/* free blocks */
	short	cg_rotor;		/* position of last used block */
	short	cg_irotor;		/* position of last used inode */
	short	cg_frotor;		/* position of last used frag */
	short	cg_frsum[FRAG];		/* counts of available frags */
	short	cg_b[MAXCPG][NRPOS];	/* positions of free blocks */
	char	cg_iused[MAXIPG/NBBY];	/* used inode map */
	char	cg_free[1];		/* free block map */
/* actually longer */
};
#define	cgsize(fp)	(sizeof (struct cg) + ((fp)->fs_fpg+NBBY-1)/NBBY)

#ifdef KERNEL
struct	fs *getfs();
int inside[], around[];
char fragtbl[];
#endif
