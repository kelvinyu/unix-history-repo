/*
 * Copyright (c) 1990 Jan-Simon Pendry
 * Copyright (c) 1990 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)mount.h	5.3 (Berkeley) %G%
 *
 * $Id: mount.h,v 5.2.1.2 91/05/07 22:18:54 jsp Alpha $
 *
 */

#define MNTPATHLEN 1024
#define MNTNAMLEN 255
#define FHSIZE 32

typedef char fhandle[FHSIZE];
bool_t xdr_fhandle();


struct fhstatus {
	u_int fhs_status;
	union {
		fhandle fhs_fhandle;
	} fhstatus_u;
};
typedef struct fhstatus fhstatus;
bool_t xdr_fhstatus();


typedef char *dirpath;
bool_t xdr_dirpath();


typedef char *name;
bool_t xdr_name();


typedef struct mountbody *mountlist;
bool_t xdr_mountlist();


struct mountbody {
	name ml_hostname;
	dirpath ml_directory;
	mountlist ml_next;
};
typedef struct mountbody mountbody;
bool_t xdr_mountbody();


typedef struct groupnode *groups;
bool_t xdr_groups();


struct groupnode {
	name gr_name;
	groups gr_next;
};
typedef struct groupnode groupnode;
bool_t xdr_groupnode();


typedef struct exportnode *exports;
bool_t xdr_exports();


struct exportnode {
	dirpath ex_dir;
	groups ex_groups;
	exports ex_next;
};
typedef struct exportnode exportnode;
bool_t xdr_exportnode();


#define MOUNTPROG ((u_long)100005)
#define MOUNTVERS ((u_long)1)
#define MOUNTPROC_NULL ((u_long)0)
extern voidp mountproc_null_1();
#define MOUNTPROC_MNT ((u_long)1)
extern fhstatus *mountproc_mnt_1();
#define MOUNTPROC_DUMP ((u_long)2)
extern mountlist *mountproc_dump_1();
#define MOUNTPROC_UMNT ((u_long)3)
extern voidp mountproc_umnt_1();
#define MOUNTPROC_UMNTALL ((u_long)4)
extern voidp mountproc_umntall_1();
#define MOUNTPROC_EXPORT ((u_long)5)
extern exports *mountproc_export_1();
#define MOUNTPROC_EXPORTALL ((u_long)6)
extern exports *mountproc_exportall_1();

