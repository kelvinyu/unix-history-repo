/*
 * Copyright (c) 1982, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)dinode.h	8.5 (Berkeley) %G%
 */

/*
 * The root inode is the root of the file system.  Inode 0 can't be used for
 * normal purposes and historically bad blocks were linked to inode 1, thus
 * the root inode is 2.  (Inode 1 is no longer used for this purpose, however
 * numerous dump tapes make this assumption, so we are stuck with it).
 */
#define	ROOTINO	((ino_t)2)

/*
 * The Whiteout inode# is a dummy non-zero inode number which will
 * never be allocated to a real file.  It is used as a place holder
 * in the directory entry which has been tagged as a DT_W entry.
 * See the comments about ROOTINO above.
 */
#define	WINO	((ino_t)1)

/*
 * A dinode contains all the meta-data associated with a UFS file.
 * This structure defines the on-disk format of a dinode.
 */

#define	NDADDR	12			/* Direct addresses in inode. */
#define	NIADDR	3			/* Indirect addresses in inode. */

struct dinode {
	u_int16_t	di_mode;	/*   0: IFMT, permissions; see below. */
	int16_t		di_nlink;	/*   2: File link count. */
	union {
		u_int16_t oldids[2];	/*   4: Ffs: old user and group ids. */
		ino_t	  inumber;	/*   4: Lfs: inode number. */
	} di_u;
	u_quad_t	di_size;	/*   8: File byte count. */
	struct timespec	di_atime;	/*  16: Last access time. */
	struct timespec	di_mtime;	/*  24: Last modified time. */
	struct timespec	di_ctime;	/*  32: Last inode change time. */
	daddr_t		di_db[NDADDR];	/*  40: Direct disk blocks. */
	daddr_t		di_ib[NIADDR];	/*  88: Indirect disk blocks. */
	u_int32_t	di_flags;	/* 100: Status flags (chflags). */
	int32_t		di_blocks;	/* 104: Blocks actually held. */
	int32_t		di_gen;		/* 108: Generation number. */
	u_int32_t	di_uid;		/* 112: File owner. */
	u_int32_t	di_gid;		/* 116: File group. */
	int32_t		di_spare[2];	/* 120: Reserved; currently unused */
};

/*
 * The di_db fields may be overlaid with other information for
 * file types that do not have associated disk storage. Block
 * and character devices overlay the first data block with their
 * dev_t value. Short symbolic links place their path in the
 * di_db area.
 */
#define	di_inumber	di_u.inumber
#define	di_ogid		di_u.oldids[1]
#define	di_ouid		di_u.oldids[0]
#define	di_rdev		di_db[0]
#define	di_shortlink	di_db
#define	MAXSYMLINKLEN	((NDADDR + NIADDR) * sizeof(daddr_t))

/* File permissions. */
#define	IEXEC		0000100		/* Executable. */
#define	IWRITE		0000200		/* Writeable. */
#define	IREAD		0000400		/* Readable. */
#define	ISVTX		0001000		/* Sticky bit. */
#define	ISGID		0002000		/* Set-gid. */
#define	ISUID		0004000		/* Set-uid. */

/* File types. */
#define	IFMT		0170000		/* Mask of file type. */
#define	IFIFO		0010000		/* Named pipe (fifo). */
#define	IFCHR		0020000		/* Character device. */
#define	IFDIR		0040000		/* Directory file. */
#define	IFBLK		0060000		/* Block device. */
#define	IFREG		0100000		/* Regular file. */
#define	IFLNK		0120000		/* Symbolic link. */
#define	IFSOCK		0140000		/* UNIX domain socket. */
