/*-
 * Copyright (c) 1986, 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1986, 1992 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)savecore.c	5.32 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/time.h>

#include <errno.h>
#include <dirent.h>
#include <nlist.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	DAY	(60L*60L*24L)
#define	LEEWAY	(3*DAY)

#define ok(number) ((number) - KERNBASE)

struct nlist current_nl[] = {	/* namelist for currently running system */
#define X_DUMPDEV	0
	{ "_dumpdev" },
#define X_DUMPLO	1
	{ "_dumplo" },
#define X_TIME		2
	{ "_time" },
#define	X_DUMPSIZE	3
	{ "_dumpsize" },
#define X_VERSION	4
	{ "_version" },
#define X_PANICSTR	5
	{ "_panicstr" },
#define	X_DUMPMAG	6
	{ "_dumpmag" },
	{ "" },
};

struct nlist dump_nl[] = {	/* name list for dumped system */
	{ "_dumpdev" },		/* entries MUST be the same as */
	{ "_dumplo" },		/*	those in current_nl[]  */
	{ "_time" },
	{ "_dumpsize" },
	{ "_version" },
	{ "_panicstr" },
	{ "_dumpmag" },
	{ "" },
};

char	*vmunix;
char	*dirname;			/* directory to save dumps in */
char	*ddname;			/* name of dump device */
int	dumpfd;				/* read/write descriptor on block dev */
dev_t	dumpdev;			/* dump device */
time_t	dumptime;			/* time the dump was taken */
int	dumplo;				/* where dump starts on dumpdev */
int	dumpsize;			/* amount of memory dumped */
int	dumpmag;			/* magic number in dump */
time_t	now;				/* current date */
char	vers[80];
char	core_vers[80];
char	panic_mesg[80];
int	panicstr;
int	verbose;
int	force;
int	clear;

int	dump_exists __P(());
void	clear_dump __P(());
char	*find_dev __P((dev_t, int));
char	*rawname __P((char *s));
void	read_kmem __P(());
void	check_kmem __P(());
int	get_crashtime __P(());
char	*path __P((char *));
int	check_space __P(());
int	read_number __P((char *));
int	save_core __P(());
int	Open __P((char *, int rw));
int	Read __P((int, char *, int));
void	Lseek __P((int, off_t, int));
int	Create __P((char *, int));
void	Write __P((int, char *, int));
void	log __P((int, char *, ...));
void	Perror __P((int, char *, char *));
void	usage __P(());

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int ch;

	while ((ch = getopt(argc, argv, "cdfv")) != EOF)
		switch(ch) {
		case 'c':
			clear = 1;
			break;
		case 'd':		/* not documented */
		case 'v':
			verbose = 1;
			break;
		case 'f':
			force = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	/* This is wrong, but I want "savecore -c" to work. */
	if (!clear) {
		if (argc != 1 && argc != 2)
			usage();
		dirname = argv[0];
	}
	if (argc == 2)
		vmunix = argv[1];

	openlog("savecore", LOG_ODELAY, LOG_AUTH);

	read_kmem();
}

int
dump_exists()
{
	int word;

	Lseek(dumpfd, (off_t)(dumplo + ok(dump_nl[X_DUMPMAG].n_value)), L_SET);
	Read(dumpfd, (char *)&word, sizeof (word));
	if (verbose && word != dumpmag)
		printf("magic number mismatch: %x != %x\n", word, dumpmag);
	return (word == dumpmag);
}

void
clear_dump()
{
	int zero;

	zero = 0;
	Lseek(dumpfd, (off_t)(dumplo + ok(dump_nl[X_DUMPMAG].n_value)), L_SET);
	Write(dumpfd, (char *)&zero, sizeof (zero));
}

char *
find_dev(dev, type)
	register dev_t dev;
	register int type;
{
	struct stat statb;
	char *dp;

	dfd = opendir(_PATH_DEV);
	strcpy(devname, _PATH_DEV);
		if (stat(devname, &statb)) {
			perror(devname);
			continue;
		}
		if ((statb.st_mode&S_IFMT) != type)
			continue;
		if (dev == statb.st_rdev) {
			dp = malloc(strlen(devname)+1);
			strcpy(dp, devname);
			return (dp);
		}
	}
	log(LOG_ERR, "Can't find device %d/%d\n", major(dev), minor(dev));
	exit(1);
	/*NOTREACHED*/
}

char *
rawname(s)
	char *s;
{
	static char name[MAXPATHLEN];
	char *sl;

	if ((sl = rindex(s, '/')) == NULL || sl[1] == '0') {
		log(LOG_ERR, "can't make raw dump device name from %s?\n", s);
		return (s);
	}
	(void)snprintf(name, sizeof(name), "%.*s/r%s", sl - s, s, sl + 1);
	return (name);
}

int	cursyms[] =
    { X_DUMPDEV, X_DUMPLO, X_VERSION, X_DUMPMAG, -1 };
int	dumpsyms[] =
    { X_TIME, X_DUMPSIZE, X_VERSION, X_PANICSTR, X_DUMPMAG, -1 };

void
read_kmem()
{
	FILE *fp;
	int kmem, i;
	char *dump_sys;
	
	dump_sys = vmunix ? vmunix : _PATH_UNIX;
	nlist(_PATH_UNIX, current_nl);
	nlist(dump_sys, dump_nl);
	/*
	 * Some names we need for the currently running system,
	 * others for the system that was running when the dump was made.
	 * The values obtained from the current system are used
	 * to look for things in /dev/kmem that cannot be found
	 * in the dump_sys namelist, but are presumed to be the same
	 * (since the disk partitions are probably the same!)
	 */
	for (i = 0; cursyms[i] != -1; i++)
		if (current_nl[cursyms[i]].n_value == 0) {
			log(LOG_ERR, "%s: %s not in namelist\n", _PATH_UNIX,
			    current_nl[cursyms[i]].n_name);
			exit(1);
		}
	for (i = 0; dumpsyms[i] != -1; i++)
		if (dump_nl[dumpsyms[i]].n_value == 0) {
			log(LOG_ERR, "%s: %s not in namelist\n", dump_sys,
			    dump_nl[dumpsyms[i]].n_name);
			exit(1);
		}
	kmem = Open(_PATH_KMEM, O_RDONLY);
	Lseek(kmem, (off_t)current_nl[X_DUMPDEV].n_value, L_SET);
	Read(kmem, (char *)&dumpdev, sizeof (dumpdev));
	Lseek(kmem, (off_t)current_nl[X_DUMPLO].n_value, L_SET);
	Read(kmem, (char *)&dumplo, sizeof (dumplo));
	if (verbose)
		printf("dumplo = %d (%d * %d)\n", dumplo, dumplo/DEV_BSIZE,
		    DEV_BSIZE);
	Lseek(kmem, (off_t)current_nl[X_DUMPMAG].n_value, L_SET);
	Read(kmem, (char *)&dumpmag, sizeof (dumpmag));
	dumplo *= DEV_BSIZE;
	ddname = find_dev(dumpdev, S_IFBLK);
	dumpfd = Open(ddname, O_RDWR);
	fp = fdopen(kmem, "r");
	if (fp == NULL) {
		log(LOG_ERR, "Couldn't fdopen kmem\n");
		exit(1);
	}
	if (vmunix)
		return;
	fseek(fp, (off_t)current_nl[X_VERSION].n_value, L_SET);
	fgets(vers, sizeof (vers), fp);
	(void)fclose(fp);
}

void
check_kmem()
{
	FILE *fp;
	register char *cp;

	fp = fdopen(dumpfd, "r");
	if (fp == NULL) {
		log(LOG_ERR, "Can't fdopen dumpfd\n");
		exit(1);
	}
	fseek(fp, (off_t)(dumplo+ok(dump_nl[X_VERSION].n_value)), L_SET);
	fgets(core_vers, sizeof (core_vers), fp);
	if (strcmp(vers, core_vers) && vmunix == 0) {
		log(LOG_WARNING, "Warning: %s version mismatch:\n", _PATH_UNIX);
		log(LOG_WARNING, "\t%s\n", vers);
		log(LOG_WARNING, "and\t%s\n", core_vers);
	}
	fseek(fp, (off_t)(dumplo + ok(dump_nl[X_PANICSTR].n_value)), L_SET);
	fread((char *)&panicstr, sizeof (panicstr), 1, fp);
	if (panicstr) {
		fseek(fp, dumplo + ok(panicstr), L_SET);
		cp = panic_mesg;
		do
			*cp = getc(fp);
		while (*cp++ && cp < &panic_mesg[sizeof(panic_mesg)]);
	}
	/* don't fclose(fp); we want the file descriptor */
}

int
get_crashtime()
{

	Lseek(dumpfd, (off_t)(dumplo + ok(dump_nl[X_TIME].n_value)), L_SET);
	Read(dumpfd, (char *)&dumptime, sizeof dumptime);
	if (dumptime == 0) {
		if (verbose)
			printf("Dump time is zero.\n");
		return (0);
	}
	printf("System went down at %s", ctime(&dumptime));
	if (dumptime < now - LEEWAY || dumptime > now + LEEWAY) {
		printf("dump time is unreasonable\n");
		return (0);
	}
	return (1);
}

char *
path(file)
	char *file;
{
	register char *cp = malloc(strlen(file) + strlen(dirname) + 2);

	(void) strcpy(cp, dirname);
	(void) strcat(cp, "/");
	(void) strcat(cp, file);
	return (cp);
}

int
check_space()
{
	long minfree, spacefree;
	struct statfs fsbuf;

	if (statfs(dirname, &fsbuf) < 0) {
		Perror(LOG_ERR, "%s: %m\n", dirname);
		exit(1);
	}
 	spacefree = fsbuf.f_bavail * fsbuf.f_bsize / 1024;
	minfree = read_number("minfree");
 	if (minfree > 0 && spacefree - dumpsize < minfree) {
		log(LOG_WARNING, "Dump omitted, not enough space on device\n");
		return (0);
	}
	if (spacefree - dumpsize < minfree)
		log(LOG_WARNING,
		    "Dump performed, but free space threshold crossed\n");
	return (1);
}

int
read_number(fn)
	char *fn;
{
	char lin[80];
	register FILE *fp;

	fp = fopen(path(fn), "r");
	if (fp == NULL)
		return (0);
	if (fgets(lin, 80, fp) == NULL) {
		(void)fclose(fp);
		return (0);
	}
	(void)fclose(fp);
	return (atoi(lin));
}

#define	BUFSIZE		(256*1024)		/* 1/4 Mb */

int
save_core()
{
	register int n;
	register char *cp;
	register int ifd, ofd, bounds, ret, stat;
	char *bfile;
	register FILE *fp;

	cp = malloc(BUFSIZE);
	if (cp == 0) {
		log(LOG_ERR, "savecore: Can't allocate i/o buffer.\n");
		return (0);
	}
	bounds = read_number("bounds");
	ifd = Open(vmunix ? vmunix : _PATH_UNIX, O_RDONLY);
	while((n = Read(ifd, cp, BUFSIZE)) > 0)
		Write(ofd, cp, n);
	close(ifd);
	close(ofd);
	if ((ifd = open(rawname(ddname), O_RDONLY)) == -1) {
		log(LOG_WARNING, "Can't open %s (%m); using block device",
			rawname(ddname));
		ifd = dumpfd;
	}
	Lseek(dumpfd, (off_t)(dumplo + ok(dump_nl[X_DUMPSIZE].n_value)), L_SET);
	Read(dumpfd, (char *)&dumpsize, sizeof (dumpsize));
	(void)sprintf(cp, "vmcore.%d", bounds);
	ofd = Create(path(cp), 0644);
	Lseek(ifd, (off_t)dumplo, L_SET);
	dumpsize *= NBPG;
	log(LOG_NOTICE, "Saving %d bytes of image in vmcore.%d\n",
	    dumpsize, bounds);
	stat = 1;
	while (dumpsize > 0) {
		n = read(ifd, cp, dumpsize > BUFSIZE ? BUFSIZE : dumpsize);
		if (n <= 0) {
			if (n == 0)
				log(LOG_WARNING,
				    "WARNING: EOF on dump device; %s\n",
				    "vmcore may be incomplete");
			else
				Perror(LOG_ERR, "read from dumpdev: %m",
				    "read");
			stat = 0;
			break;
		}
		if ((ret = write(ofd, cp, n)) < n) {
			if (ret < 0)
				Perror(LOG_ERR, "write: %m", "write");
			else
				log(LOG_ERR, "short write: wrote %d of %d\n",
				    ret, n);
			log(LOG_WARNING, "WARNING: vmcore may be incomplete\n");
			stat = 0;
			break;
		}
		dumpsize -= n;
		(void)fprintf(stderr, "%6dK\r", dumpsize / 1024);
	}
	fputc('\n', stderr);
	close(ifd);
	close(ofd);
	bfile = path("bounds");
	fp = fopen(bfile, "w");
	if (fp) {
		(void)fprintf(fp, "%d\n", bounds+1);
		(void)fclose(fp);
	} else
		Perror(LOG_ERR, "Can't create bounds file %s: %m", bfile);
	free(cp);
	return (stat);
}

/*
 * Versions of std routines that exit on error.
 */
int
Open(name, rw)
	char *name;
	int rw;
{
	int fd;

	fd = open(name, rw);
	if (fd < 0) {
		Perror(LOG_ERR, "%s: %m", name);
		exit(1);
	}
	return (fd);
}

int
Read(fd, buff, size)
	int fd;
	char *buff;
	int size;
{
	int ret;

	ret = read(fd, buff, size);
	if (ret < 0) {
		Perror(LOG_ERR, "read: %m", "read");
		exit(1);
	}
	return (ret);
}

void
Lseek(fd, off, flag)
	int fd;
	off_t off;
	int flag;
{
	long ret;

	ret = lseek(fd, (off_t)off, flag);
	if (ret == -1) {
		Perror(LOG_ERR, "lseek: %m", "lseek");
		exit(1);
	}
}

int
Create(file, mode)
	char *file;
	int mode;
{
	register int fd;

	fd = creat(file, mode);
	if (fd < 0) {
		Perror(LOG_ERR, "%s: %m", file);
		exit(1);
	}
	return (fd);
}

void
Write(fd, buf, size)
	int fd;
	char *buf;
	int size;
{
	int n;

	if ((n = write(fd, buf, size)) < size) {
		if (n < 0)
			Perror(LOG_ERR, "write: %m", "write");
		else
			log(LOG_ERR, "short write: wrote %d of %d\n", n, size);
		exit(1);
	}
}

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

void
#if __STDC__
log(int level, char *fmt, ...)
#else
log(level, fmt, va_alist)
	int level;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif

	(void)vfprintf(stderr, fmt, ap);
	vsyslog(level, fmt, ap);
	va_end(ap);
}

void
Perror(level, msg, s)
	int level;
	char *msg, *s;
{
	int oerrno = errno;
	
	perror(s);
	errno = oerrno;
	syslog(level, msg, s);
}

void
usage()
{
	(void)fprintf(stderr, "usage: savecore [-cfv] dirname [system]\n");
	exit(1);
}
