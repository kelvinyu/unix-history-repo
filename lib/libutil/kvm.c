/*-
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)kvm.c	5.18 (Berkeley) 5/7/91";
#endif /* LIBC_SCCS and not lint */

#define DEBUG 0
#define PROCFS 1

#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/kinfo.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/types.h>
#include <machine/vmparam.h>
#include <fcntl.h>
#include <nlist.h>
#include <kvm.h>
#include <ndbm.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#define	btop(x)		(((unsigned)(x)) >> PGSHIFT)	/* XXX */
#define	ptob(x)		((caddr_t)((x) << PGSHIFT))	/* XXX */
#include <vm/vm.h>	/* ??? kinfo_proc currently includes this*/
#include <vm/vm_page.h>
#include <vm/swap_pager.h>
#include <sys/kinfo_proc.h>

/*
 * files
 */
static	const char *unixf, *memf, *kmemf, *swapf;
static	int unixx, mem, kmem, swap;
static	DBM *db;
/*
 * flags
 */
static	int deadkernel;
static	int kvminit = 0;
static	int kvmfilesopen = 0;
/*
 * state
 */
static	struct kinfo_proc *kvmprocbase, *kvmprocptr;
static	int kvmnprocs;
/*
 * u. buffer
 */
static union {
	struct	user user;
	char	upages[UPAGES][NBPG];
} user;

struct swapblk {
	long	offset;		/* offset in swap device */
	long	size;		/* remaining size of block in swap device */
};
/*
 * random other stuff
 */
static	int	dmmin, dmmax;
static	int	pcbpf;
static	int	argaddr0;	/* XXX */
static	int	argaddr1;
static	int	swaddr;
static	int	nswap;
static	char	*tmp;
static	struct pde *PTD;

#define basename(cp)	((tmp=rindex((cp), '/')) ? tmp+1 : (cp))
#define	MAXSYMSIZE	256

#ifndef pftoc
#define pftoc(f)	(f)
#endif
#ifndef iskva
#define iskva(v)	((u_long)(v) & KERNBASE)
#endif

static struct nlist nl[] = {
	{ "_Usrptmap" },
#define	X_USRPTMAP	0
	{ "_usrpt" },
#define	X_USRPT		1
	{ "_nswap" },
#define	X_NSWAP		2
	{ "_dmmin" },
#define	X_DMMIN		3
	{ "_dmmax" },
#define	X_DMMAX		4
	{ "_vm_page_buckets" },
#define X_VM_PAGE_BUCKETS	5
	{ "_vm_page_hash_mask" },
#define X_VM_PAGE_HASH_MASK	6
	{ "_page_shift" },
#define X_PAGE_SHIFT	7
	{ "_kstack" },
#define X_KSTACK 8
	{ "_kernel_object" },
#define X_KERNEL_OBJECT 9
	{ "_btext",},
#define X_KERNEL_BTEXT 10
	/*
	 * everything here and down, only if a dead kernel
	 */
	{ "_Sysmap" },
#define	X_SYSMAP	11
#define	X_DEADKERNEL	X_SYSMAP
	{ "_Syssize" },
#define	X_SYSSIZE	12
	{ "_allproc" },
#define X_ALLPROC	13
	{ "_zombproc" },
#define X_ZOMBPROC	14
	{ "_nproc" },
#define	X_NPROC		15
#define	X_LAST		15
	{ "_IdlePTD" },
#define	X_IdlePTD	(X_LAST+1)
	{ "" },
};

static void seterr(), setsyserr(), vstodb();
static int getkvars(), kvm_doprocs(), kvm_init();

/*
 * returns 	0 if files were opened now,
 * 		1 if files were already opened,
 *		-1 if files could not be opened.
 */
kvm_openfiles(uf, mf, sf)
	const char *uf, *mf, *sf; 
{
	if (kvmfilesopen)
		return (1);
	unixx = mem = kmem = swap = -1;
	unixf = (uf == NULL) ? _PATH_UNIX : uf; 
	memf = (mf == NULL) ? _PATH_MEM : mf;

	if ((unixx = open(unixf, O_RDONLY, 0)) == -1) {
		setsyserr("can't open %s", unixf);
		goto failed;
	}
	if ((mem = open(memf, O_RDONLY, 0)) == -1) {
		setsyserr("can't open %s", memf);
		goto failed;
	}
	if (sf != NULL)
		swapf = sf;
	if (mf != NULL) {
		deadkernel++;
		kmemf = mf;
		kmem = mem;
		swap = -1;
	} else {
		kmemf = _PATH_KMEM;
		if ((kmem = open(kmemf, O_RDONLY, 0)) == -1) {
			setsyserr("can't open %s", kmemf);
			goto failed;
		}
		swapf = (sf == NULL) ?  _PATH_DRUM : sf;
		/*
		 * live kernel - avoid looking up nlist entries
		 * past X_DEADKERNEL.
		 */
		nl[X_DEADKERNEL].n_name = "";
	}
	if (swapf != NULL && ((swap = open(swapf, O_RDONLY, 0)) == -1)) {
		seterr("can't open %s", swapf);
		goto failed;
	}
	kvmfilesopen++;
	if (kvminit == 0 && kvm_init(NULL, NULL, NULL, 0) == -1) /*XXX*/
		return (-1);
	return (0);
failed:
	kvm_close();
	return (-1);
}

static
kvm_init(uf, mf, sf)
	char *uf, *mf, *sf;
{
	if (kvmfilesopen == 0 && kvm_openfiles(NULL, NULL, NULL) == -1)
		return (-1);
	if (getkvars() == -1)
		return (-1);
	kvminit = 1;

	return (0);
}

kvm_close()
{
	if (unixx != -1) {
		close(unixx);
		unixx = -1;
	}
	if (kmem != -1) {
		if (kmem != mem)
			close(kmem);
		/* otherwise kmem is a copy of mem, and will be closed below */
		kmem = -1;
	}
	if (mem != -1) {
		close(mem);
		mem = -1;
	}
	if (swap != -1) {
		close(swap);
		swap = -1;
	}
	if (db != NULL) {
		dbm_close(db);
		db = NULL;
	}
	kvminit = 0;
	kvmfilesopen = 0;
	deadkernel = 0;
}

kvm_nlist(nl)
	struct nlist *nl;
{
	datum key, data;
	char dbname[MAXPATHLEN];
	char dbversion[_POSIX2_LINE_MAX];
	char kversion[_POSIX2_LINE_MAX];
	int dbversionlen;
	char symbuf[MAXSYMSIZE];
	struct nlist nbuf, *n;
	int num, did;

	if (kvmfilesopen == 0 && kvm_openfiles(NULL, NULL, NULL) == -1)
		return (-1);
	if (deadkernel)
		goto hard2;
	/*
	 * initialize key datum
	 */
	key.dptr = symbuf;

	if (db != NULL)
		goto win;	/* off to the races */
	/*
	 * open database
	 */
	sprintf(dbname, "%s/kvm_%s", _PATH_VARRUN, basename(unixf));
	if ((db = dbm_open(dbname, O_RDONLY, 0)) == NULL)
		goto hard2;
	/*
	 * read version out of database
	 */
	bcopy("VERSION", symbuf, sizeof ("VERSION")-1);
	key.dsize = (sizeof ("VERSION") - 1);
	data = dbm_fetch(db, key);
	if (data.dptr == NULL)
		goto hard1;
	bcopy(data.dptr, dbversion, data.dsize);
	dbversionlen = data.dsize;
	/*
	 * read version string from kernel memory
	 */
	bcopy("_version", symbuf, sizeof ("_version")-1);
	key.dsize = (sizeof ("_version")-1);
	data = dbm_fetch(db, key);
	if (data.dptr == NULL)
		goto hard1;
	if (data.dsize != sizeof (struct nlist))
		goto hard1;
	bcopy(data.dptr, &nbuf, sizeof (struct nlist));
	lseek(kmem, nbuf.n_value, 0);
	if (read(kmem, kversion, dbversionlen) != dbversionlen)
		goto hard1;
	/*
	 * if they match, we win - otherwise do it the hard way
	 */
	if (bcmp(dbversion, kversion, dbversionlen) != 0)
		goto hard1;
	/*
	 * getem from the database.
	 */
win:
	num = did = 0;
	for (n = nl; n->n_name && n->n_name[0]; n++, num++) {
		int len;
		/*
		 * clear out fields from users buffer
		 */
		n->n_type = 0;
		n->n_other = 0;
		n->n_desc = 0;
		n->n_value = 0;
		/*
		 * query db
		 */
		if ((len = strlen(n->n_name)) > MAXSYMSIZE) {
			seterr("symbol too large");
			return (-1);
		}
		(void)strcpy(symbuf, n->n_name);
		key.dsize = len;
		data = dbm_fetch(db, key);
		if (data.dptr == NULL || data.dsize != sizeof (struct nlist))
			continue;
		bcopy(data.dptr, &nbuf, sizeof (struct nlist));
		n->n_value = nbuf.n_value;
		n->n_type = nbuf.n_type;
		n->n_desc = nbuf.n_desc;
		n->n_other = nbuf.n_other;
		did++;
	}
	return (num - did);
hard1:
	dbm_close(db);
	db = NULL;
hard2:
	num = nlist(unixf, nl);
	if (num == -1)
		seterr("nlist (hard way) failed");
	return (num);
}

kvm_getprocs(what, arg)
	int what, arg;
{
	static int	ocopysize = -1;

	if (kvminit == 0 && kvm_init(NULL, NULL, NULL, 0) == -1)
		return (NULL);
	if (!deadkernel) {
		int ret, copysize;

		if ((ret = getkerninfo(what, NULL, NULL, arg)) == -1) {
			setsyserr("can't get estimate for kerninfo");
			return (-1);
		}
		copysize = ret;
		if (copysize > ocopysize &&
			(kvmprocbase = (struct kinfo_proc *)
				realloc(kvmprocbase, copysize)) == NULL) {
			seterr("out of memory");
			return (-1);
		}
		ocopysize = copysize;
		if ((ret = getkerninfo(what, kvmprocbase, &copysize, 
		     arg)) == -1) {
			setsyserr("can't get proc list");
			return (-1);
		}
		if (copysize % sizeof (struct kinfo_proc)) {
			seterr("proc size mismatch (got %d total, kinfo_proc: %d)",
				copysize, sizeof (struct kinfo_proc));
			return (-1);
		}
		kvmnprocs = copysize / sizeof (struct kinfo_proc);
	} else {
		int nproc;

		if (kvm_read((void *) nl[X_NPROC].n_value, &nproc,
		    sizeof (int)) != sizeof (int)) {
			seterr("can't read nproc");
			return (-1);
		}
		if ((kvmprocbase = (struct kinfo_proc *)
		     malloc(nproc * sizeof (struct kinfo_proc))) == NULL) {
			seterr("out of memory (addr: %x nproc = %d)",
				nl[X_NPROC].n_value, nproc);
			return (-1);
		}
		kvmnprocs = kvm_doprocs(what, arg, kvmprocbase);
		realloc(kvmprocbase, kvmnprocs * sizeof (struct kinfo_proc));
	}
	kvmprocptr = kvmprocbase;

	return (kvmnprocs);
}

/*
 * XXX - should NOT give up so easily - especially since the kernel
 * may be corrupt (it died).  Should gather as much information as possible.
 * Follows proc ptrs instead of reading table since table may go
 * away soon.
 */
static
kvm_doprocs(what, arg, buff)
	int what, arg;
	char *buff;
{
	struct proc *p, proc;
	register char *bp = buff;
	int i = 0;
	int doingzomb = 0;
	struct eproc eproc;
	struct pgrp pgrp;
	struct session sess;
	struct tty tty;

	/* allproc */
	if (kvm_read((void *) nl[X_ALLPROC].n_value, &p, 
	    sizeof (struct proc *)) != sizeof (struct proc *)) {
		seterr("can't read allproc");
		return (-1);
	}

again:
	for (; p; p = proc.p_nxt) {
		if (kvm_read(p, &proc, sizeof (struct proc)) !=
		    sizeof (struct proc)) {
			seterr("can't read proc at %x", p);
			return (-1);
		}
		if (kvm_read(proc.p_cred, &eproc.e_pcred,
		    sizeof (struct pcred)) == sizeof (struct pcred))
			(void) kvm_read(eproc.e_pcred.pc_ucred, &eproc.e_ucred,
			    sizeof (struct ucred));
		switch(ki_op(what)) {
			
		case KINFO_PROC_PID:
			if (proc.p_pid != (pid_t)arg)
				continue;
			break;


		case KINFO_PROC_UID:
			if (eproc.e_ucred.cr_uid != (uid_t)arg)
				continue;
			break;

		case KINFO_PROC_RUID:
			if (eproc.e_pcred.p_ruid != (uid_t)arg)
				continue;
			break;
		}
		/*
		 * gather eproc
		 */
		eproc.e_paddr = p;
		if (kvm_read(proc.p_pgrp, &pgrp, sizeof (struct pgrp)) !=
	            sizeof (struct pgrp)) {
			seterr("can't read pgrp at %x", proc.p_pgrp);
			return (-1);
		}
		eproc.e_sess = pgrp.pg_session;
		eproc.e_pgid = pgrp.pg_id;
		eproc.e_jobc = pgrp.pg_jobc;
		if (kvm_read(pgrp.pg_session, &sess, sizeof (struct session))
		   != sizeof (struct session)) {
			seterr("can't read session at %x", pgrp.pg_session);
			return (-1);
		}
		if ((proc.p_flag&SCTTY) && sess.s_ttyp != NULL) {
			if (kvm_read(sess.s_ttyp, &tty, sizeof (struct tty))
			    != sizeof (struct tty)) {
				seterr("can't read tty at %x", sess.s_ttyp);
				return (-1);
			}
			eproc.e_tdev = tty.t_dev;
			eproc.e_tsess = tty.t_session;
			if (tty.t_pgrp != NULL) {
				if (kvm_read(tty.t_pgrp, &pgrp, sizeof (struct
				    pgrp)) != sizeof (struct pgrp)) {
					seterr("can't read tpgrp at &x", 
						tty.t_pgrp);
					return (-1);
				}
				eproc.e_tpgid = pgrp.pg_id;
			} else
				eproc.e_tpgid = -1;
		} else
			eproc.e_tdev = NODEV;
		if (proc.p_wmesg)
			kvm_read(proc.p_wmesg, eproc.e_wmesg, WMESGLEN);
		(void) kvm_read(proc.p_vmspace, &eproc.e_vm,
		    sizeof (struct vmspace));
		eproc.e_xsize = eproc.e_xrssize =
			eproc.e_xccount = eproc.e_xswrss = 0;

		switch(ki_op(what)) {

		case KINFO_PROC_PGRP:
			if (eproc.e_pgid != (pid_t)arg)
				continue;
			break;

		case KINFO_PROC_TTY:
			if ((proc.p_flag&SCTTY) == 0 || 
			     eproc.e_tdev != (dev_t)arg)
				continue;
			break;
		}

		i++;
		bcopy(&proc, bp, sizeof (struct proc));
		bp += sizeof (struct proc);
		bcopy(&eproc, bp, sizeof (struct eproc));
		bp+= sizeof (struct eproc);
	}
	if (!doingzomb) {
		/* zombproc */
		if (kvm_read((void *) nl[X_ZOMBPROC].n_value, &p, 
		    sizeof (struct proc *)) != sizeof (struct proc *)) {
			seterr("can't read zombproc");
			return (-1);
		}
		doingzomb = 1;
		goto again;
	}

	return (i);
}

struct proc *
kvm_nextproc()
{

	if (!kvmprocbase && kvm_getprocs(0, 0) == -1)
		return (NULL);
	if (kvmprocptr >= (kvmprocbase + kvmnprocs)) {
#if 0
		seterr("end of proc list");
#endif
		return (NULL);
	}
	return((struct proc *)(kvmprocptr++));
}

struct eproc *
kvm_geteproc(p)
	const struct proc *p;
{
	return ((struct eproc *)(((char *)p) + sizeof (struct proc)));
}

kvm_setproc()
{
	kvmprocptr = kvmprocbase;
}

kvm_freeprocs()
{

	if (kvmprocbase) {
		free(kvmprocbase);
		kvmprocbase = NULL;
	}
}

proc_getmem(const struct proc *p, void *buffer, vm_offset_t size, vm_offset_t offset) {
	int fd;
	char fn[512+1];
	sprintf(fn,"/proc/%d",p->p_pid);
	if( p->p_flag & SSYS)
		return 0;
	fd = open(fn,O_RDONLY);
	if( fd == -1) {
		return 0;
	}
	
	if( lseek(fd, offset, 0) == -1) {
		close(fd);
		return 0;
	}
	if( read(fd, buffer, size) <= 0) {
		close(fd);
		return 0;
	}
	close(fd);
	return 1;
}

struct user *
kvm_getu(p)
	const struct proc *p;
{
	register struct kinfo_proc *kp = (struct kinfo_proc *)p;
	register int i;
	register char *up;
	u_int vaddr;
	struct swapblk swb;
	int arg_size;

	if (kvminit == 0 && kvm_init(NULL, NULL, NULL, 0) == -1)
		return (NULL);
	if (p->p_stat == SZOMB) {
		seterr("zombie process");
		return (NULL);
	}

	if (proc_getmem(p, user.upages, sizeof user.upages, USRSTACK)) {
		kp->kp_eproc.e_vm.vm_rssize =
		    kp->kp_eproc.e_vm.vm_pmap.pm_stats.resident_count; /* XXX */
		return &user.user;
	}

	return (NULL);
}

char *
kvm_getargs(p, up)
	const struct proc *p;
	const struct user *up;
{
	int arg_size, arg_offset;
	static char cmdbuf[ARG_MAX];
	char argc[ARG_MAX*3];
	int *argv;
	register char *cp;
	char c;
	int nbad;
	char *cmdbufp;
	vm_offset_t vaddr;
	char procfile[16];
	int mmfd;
#if 0
	char *argc = NULL;
#endif

	*cmdbuf = 0;

	vaddr = (u_int)((struct kinfo_proc *)p)->kp_eproc.e_vm.vm_minsaddr;
	arg_size = USRSTACK - vaddr;

	if (arg_size >= 3*ARG_MAX)
		goto bad;

#if 0
	sprintf(procfile, "/proc/%d", p->p_pid);
	if ((mmfd = open(procfile, O_RDONLY, 0)) == -1) {
printf("failed to open %s\n",procfile);
		goto bad;
	}

	if ((argc = mmap(0, arg_size, PROT_READ, MAP_FILE, mmfd, vaddr))
		== (char *)-1) {
printf("failed to mmap %s error=%s\n", procfile, strerror(errno));
		goto bad;
	}
#endif

	if (!proc_getmem(p, argc, arg_size, vaddr))
		goto bad;

	argv = (int *)argc;

	arg_offset = argv[0] - vaddr;
	if (arg_offset >= 3*ARG_MAX)
		goto bad;

	nbad = 0;

	cmdbufp = cmdbuf;
	for (cp = &argc[arg_offset]; cp < &argc[arg_size]; cp++, cmdbufp++) {
		c = *cmdbufp = *cp;
		if (c == 0) {	/* convert null between arguments to space */
			*cmdbufp = ' ';
			if (*(cp+1) == 0) break;	/* if null argument follows then no more args */
			}
		else if (c < ' ' || c > 0176) {
			if (++nbad >= 5*(0+1)) {	/* eflg -> 0 XXX */ /* limit number of bad chars to 5 */
				*cmdbufp++ = '?';
				break;
			}
			*cmdbufp = '?';
		}
		else if (0 == 0 && c == '=') {		/* eflg -> 0 XXX */
			while (*--cmdbufp != ' ')
				if (cmdbufp <= cmdbuf)
					break;
			break;
		}
	}
	*cmdbufp = 0;

	while (*--cmdbufp == ' ')
		*cmdbufp = 0;

	if (cmdbuf[0] == '-' || cmdbuf[0] == '?' || cmdbuf[0] <= ' ') {
bad:
		(void) strcat(cmdbuf, " (");
		(void) strncat(cmdbuf, p->p_comm, sizeof(p->p_comm));
		(void) strcat(cmdbuf, ")");
	}
#if 0
	if (argc && argc != (char *)-1)
		munmap(argc, arg_size);
	if (mmfd && mmfd != -1)
		close (mmfd);
#endif
	return (cmdbuf);
}

static
getkvars()
{
	if (kvm_nlist(nl) == -1)
		return (-1);
	if (kvm_read((void *) nl[X_NSWAP].n_value, &nswap, sizeof (long)) !=
	    sizeof (long)) {
		seterr("can't read nswap");
		return (-1);
	}
	if (kvm_read((void *) nl[X_DMMIN].n_value, &dmmin, sizeof (long)) !=
	    sizeof (long)) {
		seterr("can't read dmmin");
		return (-1);
	}
	if (kvm_read((void *) nl[X_DMMAX].n_value, &dmmax, sizeof (long)) !=
	    sizeof (long)) {
		seterr("can't read dmmax");
		return (-1);
	}
	return (0);
}

kvm_read(loc, buf, len)
	void *loc;
	void *buf;
{
	if (kvmfilesopen == 0 && kvm_openfiles(NULL, NULL, NULL) == -1)
		return (-1);
	if (iskva(loc)) {
		lseek(kmem, (off_t) loc, 0);
		if (read(kmem, buf, len) != len) {
			seterr("error reading kmem at %x", loc);
			return (-1);
		}
	} else {
		lseek(mem, (off_t) loc, 0);
		if (read(mem, buf, len) != len) {
			seterr("error reading mem at %x", loc);
			return (-1);
		}
	}
	return (len);
}

#include <varargs.h>
static char errbuf[_POSIX2_LINE_MAX];

static void
seterr(va_alist)
	va_dcl
{
	char *fmt;
	va_list ap;

	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vsnprintf(errbuf, _POSIX2_LINE_MAX, fmt, ap);
#if DEBUG
	(void) vfprintf(stderr, fmt, ap);
#endif
	va_end(ap);
}

static void
setsyserr(va_alist)
	va_dcl
{
	char *fmt, *cp;
	va_list ap;
	extern int errno;

	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vsnprintf(errbuf, _POSIX2_LINE_MAX, fmt, ap);
	for (cp=errbuf; *cp; cp++)
		;
	snprintf(cp, _POSIX2_LINE_MAX - (cp - errbuf), ": %s", strerror(errno));
#if DEBUG
	(void) fprintf(stderr, "%s\n", errbuf);
#endif
	va_end(ap);
}

char *
kvm_geterr()
{
	return (errbuf);
}
