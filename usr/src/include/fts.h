/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)fts.h	5.3 (Berkeley) %G%
 */

typedef struct fts {
	struct ftsent *fts_cur;		/* current node */
	struct ftsent *fts_child;	/* linked list of children */
	struct ftsent *fts_savelink;	/* saved link if node had a cycle */
	struct ftsent **fts_array;	/* sort array */
	dev_t sdev;			/* starting device # */
	char *fts_path;			/* path for this descent */
	char *fts_wd;			/* starting directory */
	int fts_pathlen;		/* sizeof(path) */
	int fts_nitems;			/* elements in the sort array */
	int (*fts_compar)();		/* compare function */
#define	FTS__STOP	0x001		/* private: unrecoverable error */
#define	FTS_LOGICAL	0x002		/* user: use stat(2) */
#define	FTS_MULTIPLE	0x004		/* user: multiple args */
#define	FTS_NOCHDIR	0x008		/* user: don't use chdir(2) */
#define	FTS_NOSTAT	0x010		/* user: don't require stat info */
#define	FTS_PHYSICAL	0x020		/* user: use lstat(2) */
#define	FTS_SEEDOT	0x040		/* user: return dot and dot-dot */
#define	FTS_XDEV	0x080		/* user: don't cross devices */
	int fts_options;		/* openfts() options */
} FTS;

typedef struct ftsent {
	struct ftsent *fts_parent;	/* parent directory */
	struct ftsent *fts_link;	/* next/cycle node */
	union {
		long number;		/* local numeric value */
		void *pointer;		/* local address value */
	} fts_local;
	char *fts_accpath;		/* path from current directory */
	char *fts_path;			/* path from starting directory */
	short fts_pathlen;		/* strlen(path) */
	short fts_namelen;		/* strlen(name) */
	short fts_level;		/* depth (-1 to N) */
#define	FTS_D		 1		/* preorder directory */
#define	FTS_DC		 2		/* directory that causes cycles */
#define	FTS_DNR		 3		/* unreadable directory */
#define	FTS_DNX		 4		/* unsearchable directory */
#define	FTS_DP		 5		/* postorder directory */
#define	FTS_ERR		 6		/* error; errno is set */
#define	FTS_F		 7		/* regular file */
#define	FTS_NS		 8		/* no stat(2) information */
#define	FTS_SL		 9		/* symbolic link */
#define	FTS_SLNONE	10		/* symbolic link without target */
#define	FTS_DEFAULT	11		/* none of the above */
	u_short fts_info;		/* file information */
#define	FTS_AGAIN	 1		/* user: read node again */
#define	FTS_SKIP	 2		/* user: discard node */
#define	FTS_FOLLOW	 3		/* user: follow symbolic link */
	short fts_instr;		/* setfts() instructions */
	struct stat fts_statb;		/* stat(2) information */
	char fts_name[1];		/* file name */
} FTSENT;

#ifdef __STDC__
extern FTS *ftsopen(const char **, int, int (*)(const FTSENT *, const FTSENT *);
extern FTSENT *ftsread(FTS *);
extern FTSENT *ftschildren(FTS *);
extern int ftsset(FTS *, FTSENT *, int);
extern int ftsclose(FTS *);
#else
extern FTS *ftsopen();
extern FTSENT *ftschildren(), *ftsread();
extern int ftsclose(), ftsset();
#endif
