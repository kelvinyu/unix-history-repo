/*
 * Copyright (c) 1980, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)pass1.c	8.5 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/time.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ffs/fs.h>

#include <err.h>
#include <string.h>

#include "fsck.h"

static ufs_daddr_t badblk;
static ufs_daddr_t dupblk;
static void checkinode __P((ino_t inumber, struct inodesc *));

void
pass1()
{
	ino_t inumber;
	int c, i, cgd;
	struct inodesc idesc;

	/*
	 * Set file system reserved blocks in used block map.
	 */
	for (c = 0; c < sblock.fs_ncg; c++) {
		cgd = cgdmin(&sblock, c);
		if (c == 0) {
			i = cgbase(&sblock, c);
			cgd += howmany(sblock.fs_cssize, sblock.fs_fsize);
		} else
			i = cgsblock(&sblock, c);
		for (; i < cgd; i++)
			setbmap(i);
	}
	/*
	 * Find all allocated blocks.
	 */
	bzero((char *)&idesc, sizeof(struct inodesc));
	idesc.id_type = ADDR;
	idesc.id_func = pass1check;
	inumber = 0;
	n_files = n_blks = 0;
	resetinodebuf();
	for (c = 0; c < sblock.fs_ncg; c++) {
		for (i = 0; i < sblock.fs_ipg; i++, inumber++) {
			if (inumber < ROOTINO)
				continue;
			checkinode(inumber, &idesc);
		}
	}
	freeinodebuf();
}

static void
checkinode(inumber, idesc)
	ino_t inumber;
	register struct inodesc *idesc;
{
	register struct dinode *dp;
	struct zlncnt *zlnp;
	int ndb, j;
	mode_t mode;
	char *symbuf;

	dp = getnextinode(inumber);
	mode = dp->di_mode & IFMT;
	if (mode == 0) {
		if (bcmp((char *)dp->di_db, (char *)zino.di_db,
			NDADDR * sizeof(ufs_daddr_t)) ||
		    bcmp((char *)dp->di_ib, (char *)zino.di_ib,
			NIADDR * sizeof(ufs_daddr_t)) ||
		    dp->di_mode || dp->di_size) {
			pfatal("PARTIALLY ALLOCATED INODE I=%lu", inumber);
			if (reply("CLEAR") == 1) {
				dp = ginode(inumber);
				clearinode(dp);
				inodirty();
			}
		}
		statemap[inumber] = USTATE;
		return;
	}
	lastino = inumber;
	if (/* dp->di_size < 0 || */
	    dp->di_size + sblock.fs_bsize - 1 < dp->di_size ||
	    (mode == IFDIR && dp->di_size > MAXDIRSIZE)) {
		if (debug)
			printf("bad size %qu:", dp->di_size);
		goto unknown;
	}
	if (!preen && mode == IFMT && reply("HOLD BAD BLOCK") == 1) {
		dp = ginode(inumber);
		dp->di_size = sblock.fs_fsize;
		dp->di_mode = IFREG|0600;
		inodirty();
	}
	ndb = howmany(dp->di_size, sblock.fs_bsize);
	if (ndb < 0) {
		if (debug)
			printf("bad size %qu ndb %d:",
				dp->di_size, ndb);
		goto unknown;
	}
	if (mode == IFBLK || mode == IFCHR)
		ndb++;
	if (mode == IFLNK) {
		if (doinglevel2 &&
		    dp->di_size > 0 && dp->di_size < MAXSYMLINKLEN &&
		    dp->di_blocks != 0) {
			symbuf = alloca(secsize);
			if (bread(fsreadfd, symbuf,
			    fsbtodb(&sblock, dp->di_db[0]),
			    (long)secsize) != 0)
				errx(EEXIT, "cannot read symlink");
			if (debug) {
				symbuf[dp->di_size] = 0;
				printf("convert symlink %d(%s) of size %d\n",
					inumber, symbuf, (long)dp->di_size);
			}
			dp = ginode(inumber);
			bcopy(symbuf, (caddr_t)dp->di_shortlink,
			    (long)dp->di_size);
			dp->di_blocks = 0;
			inodirty();
		}
		/*
		 * Fake ndb value so direct/indirect block checks below
		 * will detect any garbage after symlink string.
		 */
		if (dp->di_size < sblock.fs_maxsymlinklen) {
			ndb = howmany(dp->di_size, sizeof(ufs_daddr_t));
			if (ndb > NDADDR) {
				j = ndb - NDADDR;
				for (ndb = 1; j > 1; j--)
					ndb *= NINDIR(&sblock);
				ndb += NDADDR;
			}
		}
	}
	for (j = ndb; j < NDADDR; j++)
		if (dp->di_db[j] != 0) {
			if (debug)
				printf("bad direct addr: %ld\n", dp->di_db[j]);
			goto unknown;
		}
	for (j = 0, ndb -= NDADDR; ndb > 0; j++)
		ndb /= NINDIR(&sblock);
	for (; j < NIADDR; j++)
		if (dp->di_ib[j] != 0) {
			if (debug) {
				for (j = 0; j < NIADDR; j++) {
				printf("bad indirect addr[%d]: %lx\n",
					j, dp->di_ib[j]);
				}
				printf("flags: %x, blocks %d\n",
					dp->di_flags, dp->di_blocks);
				pfatal("BAD THIRD INDIRECT");
				if (reply("ZERO")) {
					dp = ginode(inumber);
					dp->di_ib[2] = 0;
					inodirty();
				} else
					goto unknown;
			} else
				goto unknown;
		}
	if (ftypeok(dp) == 0)
		goto unknown;
	n_files++;
	lncntp[inumber] = dp->di_nlink;
	if (dp->di_nlink <= 0) {
		zlnp = (struct zlncnt *)malloc(sizeof *zlnp);
		if (zlnp == NULL) {
			pfatal("LINK COUNT TABLE OVERFLOW");
			if (reply("CONTINUE") == 0)
				exit(EEXIT);
		} else {
			zlnp->zlncnt = inumber;
			zlnp->next = zlnhead;
			zlnhead = zlnp;
		}
	}
	if (mode == IFDIR) {
		if (dp->di_size == 0)
			statemap[inumber] = DCLEAR;
		else
			statemap[inumber] = DSTATE;
		cacheino(dp, inumber);
	} else
		statemap[inumber] = FSTATE;
	typemap[inumber] = IFTODT(mode);
	if (doinglevel2 &&
	    (dp->di_ouid != (u_short)-1 || dp->di_ogid != (u_short)-1)) {
		dp = ginode(inumber);
		dp->di_uid = dp->di_ouid;
		dp->di_ouid = -1;
		dp->di_gid = dp->di_ogid;
		dp->di_ogid = -1;
		inodirty();
	}
	badblk = dupblk = 0;
	idesc->id_number = inumber;
	(void)ckinode(dp, idesc);
	idesc->id_entryno *= btodb(sblock.fs_fsize);
	if (dp->di_blocks != idesc->id_entryno) {
		pwarn("INCORRECT BLOCK COUNT I=%lu (%ld should be %ld)",
		    inumber, dp->di_blocks, idesc->id_entryno);
		if (preen)
			printf(" (CORRECTED)\n");
		else if (reply("CORRECT") == 0)
			return;
		dp = ginode(inumber);
		dp->di_blocks = idesc->id_entryno;
		inodirty();
	}
	return;
unknown:
	pfatal("UNKNOWN FILE TYPE I=%lu", inumber);
	statemap[inumber] = FCLEAR;
	if (reply("CLEAR") == 1) {
		statemap[inumber] = USTATE;
		dp = ginode(inumber);
		clearinode(dp);
		inodirty();
	}
}

int
pass1check(idesc)
	register struct inodesc *idesc;
{
	int res = KEEPON;
	int anyout, nfrags;
	ufs_daddr_t blkno = idesc->id_blkno;
	register struct dups *dlp;
	struct dups *new;

	if ((anyout = chkrange(blkno, idesc->id_numfrags)) != 0) {
		blkerror(idesc->id_number, "BAD", blkno);
		if (badblk++ >= MAXBAD) {
			pwarn("EXCESSIVE BAD BLKS I=%lu",
				idesc->id_number);
			if (preen)
				printf(" (SKIPPING)\n");
			else if (reply("CONTINUE") == 0)
				exit(EEXIT);
			return (STOP);
		}
	}
	for (nfrags = idesc->id_numfrags; nfrags > 0; blkno++, nfrags--) {
		if (anyout && chkrange(blkno, 1)) {
			res = SKIP;
		} else if (!testbmap(blkno)) {
			n_blks++;
			setbmap(blkno);
		} else {
			blkerror(idesc->id_number, "DUP", blkno);
			if (dupblk++ >= MAXDUP) {
				pwarn("EXCESSIVE DUP BLKS I=%lu",
					idesc->id_number);
				if (preen)
					printf(" (SKIPPING)\n");
				else if (reply("CONTINUE") == 0)
					exit(EEXIT);
				return (STOP);
			}
			new = (struct dups *)malloc(sizeof(struct dups));
			if (new == NULL) {
				pfatal("DUP TABLE OVERFLOW.");
				if (reply("CONTINUE") == 0)
					exit(EEXIT);
				return (STOP);
			}
			new->dup = blkno;
			if (muldup == 0) {
				duplist = muldup = new;
				new->next = 0;
			} else {
				new->next = muldup->next;
				muldup->next = new;
			}
			for (dlp = duplist; dlp != muldup; dlp = dlp->next)
				if (dlp->dup == blkno)
					break;
			if (dlp == muldup && dlp->dup != blkno)
				muldup = new;
		}
		/*
		 * count the number of blocks found in id_entryno
		 */
		idesc->id_entryno++;
	}
	return (res);
}
