/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
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
 *	@(#)nfsnode.h	7.2 (Berkeley) %G%
 */

/*
 * The nfsnode is the nfs equivalent to ufs's inode. Any similarity
 * is purely coincidental.
 * There is a unique nfsnode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 * An nfsnode is 'named' by its file handle. (nget/nfs_node.c)
 */

struct nfsnode {
	struct	nfsnode *n_chain[2];	/* must be first */
	nfsv2fh_t n_fh;			/* NFS File Handle */
	long	n_flag;			/* Flag for locking.. */
	long	n_id;		/* unique identifier */
	struct	vnode n_vnode;	/* vnode associated with this nfsnode */
	long	n_attrstamp;	/* Time stamp (sec) for attributes */
	struct	vattr n_vattr;	/* Vnode attribute cache */
	struct	sillyrename *n_sillyrename;	/* Ptr to silly rename struct */
	struct nfsnode  *n_freef;	/* free list forward */
	struct nfsnode **n_freeb;	/* free list back */
	daddr_t	n_lastr;	/* Last block read for read ahead */
	u_long	n_size;		/* Current size of file */
	time_t	n_mtime;	/* Prev modify time to maintain data cache consistency*/
};

#define	n_forw		n_chain[0]
#define	n_back		n_chain[1]

#ifdef KERNEL
struct nfsnode *nfsnode;		/* the nfsnode table itself */
struct nfsnode *nfsnodeNNFSNODE;	/* the end of the nfsnode table */
int	nnfsnode;			/* number of slots in the table */
long	nextnfsnodeid;		/* unique id generator */

extern struct vnodeops nfsv2_vnodeops;	/* vnode operations for nfsv2 */
extern struct vnodeops nfsv2chr_vnodeops; /* vnode operations for chr devices */

/*
 * Convert between nfsnode pointers and vnode pointers
 */
#define VTONFS(vp)	((struct nfsnode *)(vp)->v_data)
#define NFSTOV(np)	((struct vnode *)&(np)->n_vnode)
#endif
/*
 * Flags for n_flag
 */
#define	NLOCKED		0x1
#define	NWANT		0x2
#define NMODIFIED	0x4
