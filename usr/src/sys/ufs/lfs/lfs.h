/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)lfs.h	5.4 (Berkeley) %G%
 */

typedef struct buf	BUF;
typedef struct dinode	DINODE;
typedef struct inode	INODE;
typedef struct mount	MOUNT;
typedef struct ucred	UCRED;
typedef struct ufsmount	UFSMOUNT;
typedef struct vnode	VNODE;

#define	LFS_LABELPAD	8192		/* LFS label size */
#define	LFS_SBPAD	8192		/* LFS superblock size */
#define MAXMNTLEN	512		/* XXX move from fs.h to mount.h */

/* On-disk and in-memory checkpoint segment usage structure. */
typedef struct segusage SEGUSE;
struct segusage {
	u_long	su_nbytes;		/* number of live bytes */
	u_long	su_lastmod;		/* last modified timestamp */
#define	SEGUSE_DIRTY	0x1		/* XXX fill in comment */
	u_long	su_flags;
};

/* On-disk file information.  One per file with data blocks in the segment. */
typedef struct finfo FINFO;
struct finfo {
	u_long	fi_nblocks;		/* number of blocks */
	u_long	fi_version;		/* version number */
	ino_t	fi_ino;			/* inode number */
	long	fi_blocks[1];		/* array of logical block numbers */
};

/* In-memory description of a segment about to be written */
typedef struct segment SEGMENT;
struct segment {
	SEGMENT	*nextp;			/* Links segments together */
	BUF	**bpp;			/* Pointer to buffer array */
	BUF	**cbpp;			/* Pointer to next available bp */
	void	*segsum;		/* Segment Summary info */
	u_long	sum_bytes_left;		/* Bytes left in summary */
	u_long	seg_bytes_left;		/* Bytes left in segment */
	daddr_t	saddr;			/* Current disk address */
	daddr_t	sum_addr;		/* Address of current summary */
	u_long	ninodes;		/* Number of inodes in this segment */
	u_long	sum_num;		/* Number of current summary block */
	u_long	seg_number;		/* Number of this segment */
	FINFO	*fip;			/* Current fileinfo pointer */
};

/* On-disk and in-memory super block. */
typedef struct lfs LFS;
struct lfs {
#define	LFS_MAGIC	0xbedead
	u_long	lfs_magic;		/* magic number */
#define	LFS_VERSION	1
	u_long	lfs_version;		/* version number */

	u_long	lfs_size;		/* number of blocks in fs */
	u_long	lfs_ssize;		/* number of blocks per segment */
	u_long	lfs_dsize;		/* number of data blocks in fs */
	u_long	lfs_bsize;		/* size of basic blocks in fs */
	u_long	lfs_fsize;		/* size of frag blocks in fs */
	u_long	lfs_frag;		/* number of frags in a block in fs */

/* Checkpoint region. */
	ino_t	lfs_free;		/* start of the free list */
	u_long	lfs_bfree;		/* number of free blocks */
	u_long	lfs_nfiles;		/* number of allocated inodes */
	daddr_t	lfs_idaddr;		/* inode file disk address */
	ino_t	lfs_ifile;		/* inode file inode number */
	daddr_t	lfs_lastseg;		/* last segment written */
	daddr_t	lfs_nextseg;		/* next segment to write */
	u_long	lfs_tstamp;		/* time stamp */

/* These are configuration parameters. */
	u_long	lfs_minfree;		/* minimum percentage of free blocks */

/* These fields can be computed from the others. */
	u_long	lfs_inopb;		/* inodes per block */
	u_long	lfs_ifpb;		/* IFILE entries per block */
	u_long	lfs_nindir;		/* indirect pointers per block */
	u_long	lfs_nseg;		/* number of segments */
	u_long	lfs_nspf;		/* number of sectors per fragment */
	u_long	lfs_segtabsz;		/* segment table size in blocks */

	u_long	lfs_segmask;		/* calculate offset within a segment */
	u_long	lfs_segshift;		/* fast mult/div for segments */
	u_long	lfs_bmask;		/* calc block offset from file offset */
	u_long	lfs_bshift;		/* calc block number from file offset */
	u_long	lfs_ffmask;		/* calc frag offset from file offset */
	u_long	lfs_ffshift;		/* fast mult/div for frag from file */
	u_long	lfs_fbmask;		/* calc frag offset from block offset */
	u_long	lfs_fbshift;		/* fast mult/div for frag from block */
	u_long	lfs_fsbtodb;		/* fsbtodb and dbtofsb shift constant */

#define	LFS_MIN_SBINTERVAL	5	/* minimum superblock segment spacing */
#define	LFS_MAXNUMSB		10	/* superblock disk offsets */
	daddr_t	lfs_sboffs[LFS_MAXNUMSB];

/* These fields are set at mount time and are meaningless on disk. */
	VNODE	*lfs_ivnode;		/* vnode for the ifile */
	SEGUSE	*lfs_segtab;		/* in-memory segment usage table */
	SEGMENT	*lfs_seglist;		/* list of segments being written */
	u_long	lfs_iocount;		/* Number of ios pending */
	u_char	lfs_fmod;		/* super block modified flag */
	u_char	lfs_clean;		/* file system is clean flag */
	u_char	lfs_ronly;		/* mounted read-only flag */
	u_char	lfs_flags;		/* currently unused flag */
	u_char	lfs_fsmnt[MAXMNTLEN];	/* name mounted on */
	u_char	pad[3];			/* long-align */

/* Checksum; valid on disk. */
	u_long	lfs_cksum;		/* checksum for superblock checking */
};

/*
 * The root inode is the root of the file system.  Inode 0 is the out-of-band
 * inode, and inode 1 is the inode number for the ifile.  Thus the root inode
 * is 2.
 */
#define ROOTINO         ((ino_t)2)
#define	LOSTFOUNDINO	((ino_t)3)

/* Fixed inode numbers. */
#define	LFS_UNUSED_INUM	0		/* Out of band inode number. */
#define	LFS_IFILE_INUM	1		/* Inode number of the ifile. */
					/* First free inode number. */
#define	LFS_FIRST_INUM	(LOSTFOUNDINO + 1)

/*
 * Used to access the first spare of the dinode which we use to store
 * the ifile number so we can identify them
 */
#define	di_inum	di_spare[0]

/*
 * Logical block numbers of indirect blocks.
 */
#define S_INDIR	-1
#define D_INDIR -2
#define T_INDIR -3

typedef struct ifile IFILE;
struct ifile {
	u_long	if_version;		/* inode version number */
#define	LFS_UNUSED_DADDR	0	/* out-of-band daddr */
	daddr_t	if_daddr;		/* inode disk address */
	union {
		ino_t	nextfree;	/* next-unallocated inode */
		time_t	st_atime;	/* access time */
	} __ifile_u;
#define	if_st_atime	__ifile_u.st_atime
#define	if_nextfree	__ifile_u.nextfree
};

/* Segment table size, in blocks. */
#define	SEGTABSIZE(fs) \
	(((fs)->fs_nseg * sizeof(SEGUSE) + \
	    ((fs)->fs_bsize - 1)) >> (fs)->fs_bshift)

#define	SEGTABSIZE_SU(fs) \
	(((fs)->lfs_nseg * sizeof(SEGUSE) + \
	    ((fs)->lfs_bsize - 1)) >> (fs)->lfs_bshift)

/*
 * All summary blocks are the same size, so we can always read a summary
 * block easily from a segment
 */
#define	LFS_SUMMARY_SIZE	512

/* On-disk segment summary information */
typedef struct segsum SEGSUM;
struct segsum {
	u_long	ss_cksum;		/* check sum */
	daddr_t	ss_next;		/* next segment */
	daddr_t	ss_prev;		/* next segment */
	daddr_t	ss_nextsum;		/* next summary block */
	u_long	ss_create;		/* creation time stamp */
	u_long	ss_nfinfo;		/* number of file info structures */
	u_long	ss_ninos;		/* number of inode blocks */
	/* FINFO's... */
};

/* NINDIR is the number of indirects in a file system block. */
#define	NINDIR(fs)	((fs)->lfs_nindir)

/* INOPB is the number of inodes in a secondary storage block. */
#define	INOPB(fs)	((fs)->lfs_inopb)

/* IFPB -- IFILE's per block */
#define	IFPB(fs)	((fs)->lfs_ifpb)

#define	blksize(fs)		((fs)->lfs_bsize)
#define	blkoff(fs, loc)		((loc) & (fs)->lfs_bmask)
#define	fsbtodb(fs, b)		((b) << (fs)->lfs_fsbtodb)
#define	lblkno(fs, loc)		((loc) >> (fs)->lfs_bshift)
#define	lblktosize(fs, blk)	((blk) << (fs)->lfs_bshift)
#define numfrags(fs, loc)	/* calculates (loc / fs->fs_fsize) */ \
	((loc) >> (fs)->lfs_bshift)
#define satosn(fs, saddr) \
	((int)((saddr - fs->lfs_sboffs[0]) / fsbtodb(fs, fs->lfs_ssize)))
#define sntosa(fs, sn) \
	((daddr_t)(sn * (fs->lfs_ssize << fs->lfs_fsbtodb) + fs->lfs_sboffs[0]))
