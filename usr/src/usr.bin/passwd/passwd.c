/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)passwd.c	5.1 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef KERBEROS
int use_kerberos = 1;
#define ARGSTR "l"
#else
#define ARGSTR ""
#endif

uid_t uid;

main(argc, argv)
	int argc;
	char **argv;
{
	register int ch;
	extern int errno, optind;
	extern char *optarg;
	struct passwd *pw;
	struct rlimit rlim;
	FILE *temp_fp;
	int fd;
	char *fend, *np, *passwd, *temp, *tend, *uname;
	char from[MAXPATHLEN], to[MAXPATHLEN];
	char *getnewpasswd(), *getlogin();

	uid = getuid();
	uname = getlogin();

#ifdef	KERBEROS
	while ((ch = getopt(argc, argv, ARGSTR)) != EOF)
		switch (ch) {
		/* change local password file */
		case 'l':
			use_kerberos = 0;
			break;
		default:
		case '?':
			usage();
			exit(1);
		}

	argc -= optind;
	argv += optind;
#endif

	switch(argc) {
	case 0:
		break;
	case 1:
#ifdef	KERBEROS
		if (use_kerberos && (strcmp(argv[1],uname) != 0)) { 
			fprintf(stderr,
				"must kinit to change another's password\n");
			exit(1);
		}
#endif
		uname = argv[0];
		break;
	default:
		usage();
		exit(1);
	}

#ifdef	KERBEROS
	if (use_kerberos) {
		exit(do_krb_passwd());
		/* NOTREACHED */
	}
#endif

	if (!(pw = getpwnam(uname))) {
		fprintf(stderr, "passwd: unknown user %s.\n", uname);
		exit(1);
	}
	if (uid && uid != pw->pw_uid) {
		fprintf(stderr, "passwd: %s\n", strerror(EACCES));
		exit(1);
	}

	(void)signal(SIGHUP, SIG_IGN);
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGTSTP, SIG_IGN);

	rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
	(void)setrlimit(RLIMIT_CPU, &rlim);
	(void)setrlimit(RLIMIT_FSIZE, &rlim);

	(void)umask(0);

	temp = _PATH_PTMP;
	if ((fd = open(temp, O_WRONLY|O_CREAT|O_EXCL, 0600)) < 0) {
		if (errno == EEXIST) {
			fprintf(stderr,
			    "passwd: password file busy -- try again later.\n");
			exit(0);
		}
		fprintf(stderr, "passwd: %s: %s", temp, strerror(errno));
		goto bad;
	}
	if (!(temp_fp = fdopen(fd, "w"))) {
		fprintf(stderr, "passwd: can't write %s", temp);
		goto bad;
	}
	passwd = _PATH_MASTERPASSWD;
	if (!freopen(passwd, "r", stdin)) {
		fprintf(stderr, "passwd: can't read %s", passwd);
		goto bad;
	}

	printf("Changing local password for %s.\n", pw->pw_name);
	np = getnewpasswd(pw, temp);

	if (!copy(pw->pw_name, np, temp_fp, pw))
		goto bad;

	(void)fclose(temp_fp);
	(void)fclose(stdin);

	switch(fork()) {
	case 0:
		break;
	case -1:
		fprintf(stderr, "passwd: can't fork");
		goto bad;
		/* NOTREACHED */
	default:
		exit(0);
		/* NOTREACHED */
	}

	if (makedb(temp)) {
		fprintf(stderr, "passwd: mkpasswd failed");
bad:		fprintf(stderr, "; password unchanged.\n");
		(void)unlink(temp);
		exit(1);
	}

	/*
	 * possible race; have to rename four files, and someone could slip
	 * in between them.  LOCK_EX and rename the ``passwd.dir'' file first
	 * so that getpwent(3) can't slip in; the lock should never fail and
	 * it's unclear what to do if it does.  Rename ``ptmp'' last so that
	 * passwd/vipw/chpass can't slip in.
	 */
	(void)setpriority(PRIO_PROCESS, 0, -20);
	fend = strcpy(from, temp) + strlen(temp);
	tend = strcpy(to, _PATH_PASSWD) + strlen(_PATH_PASSWD);
	bcopy(".dir", fend, 5);
	bcopy(".dir", tend, 5);
	if ((fd = open(from, O_RDONLY, 0)) >= 0)
		(void)flock(fd, LOCK_EX);
	/* here we go... */
	(void)rename(from, to);
	bcopy(".pag", fend, 5);
	bcopy(".pag", tend, 5);
	(void)rename(from, to);
	bcopy(".orig", fend, 6);
	(void)rename(from, _PATH_PASSWD);
	(void)rename(temp, passwd);
	/* done! */
	exit(0);
}

copy(name, np, fp, pw)
	char *name, *np;
	FILE *fp;
	struct passwd *pw;
{
	register int done;
	register char *p;
	char buf[1024];

	for (done = 0; fgets(buf, sizeof(buf), stdin);) {
		/* skip lines that are too big */
		if (!index(buf, '\n')) {
			fprintf(stderr, "passwd: line too long.\n");
			return(0);
		}
		if (done) {
			fprintf(fp, "%s", buf);
			continue;
		}
		if (!(p = index(buf, ':'))) {
			fprintf(stderr, "passwd: corrupted entry.\n");
			return(0);
		}
		*p = '\0';
		if (strcmp(buf, name)) {
			*p = ':';
			fprintf(fp, "%s", buf);
			continue;
		}
		if (!(p = index(++p, ':'))) {
			fprintf(stderr, "passwd: corrupted entry.\n");
			return(0);
		}
		/*
		 * reset change time to zero; when classes are implemented,
		 * go and get the "offset" value for this class and reset
		 * the timer.
		 */
		fprintf(fp, "%s:%s:%d:%d:%s:%ld:%ld:%s:%s:%s\n",
		    pw->pw_name, np, pw->pw_uid, pw->pw_gid,
		    pw->pw_class, 0L, pw->pw_expire, pw->pw_gecos,
		    pw->pw_dir, pw->pw_shell);
		done = 1;
	}
	return(1);
}

char *
getnewpasswd(pw, temp)
	register struct passwd *pw;
	char *temp;
{
	register char *p, *t;
	char buf[_PASSWORD_LEN+1], salt[2], *crypt(), *getpass();
	int tries = 0;
	time_t time();

	if (uid && pw->pw_passwd &&
	    strcmp(crypt(getpass("Old password:"), pw->pw_passwd),
	    pw->pw_passwd)) {
		(void)printf("passwd: %s.\n", strerror(EACCES));
		(void)unlink(temp);
		exit(1);
	}

	for (buf[0] = '\0';;) {
		p = getpass("New password:");
		if (!*p) {
			(void)printf("Password unchanged.\n");
			(void)unlink(temp);
			exit(0);
		}
		if (strlen(p) <= 5 && (uid != 0 || tries++ < 2)) {
			printf("Please enter a longer password.\n");
			continue;
		}
		for (t = p; *t && islower(*t); ++t);
		if (!*t && (uid != 0 || tries++ < 2)) {
			printf("Please don't use an all-lower case password.\nUnusual capitalization, control characters or digits are suggested.\n");
			continue;
		}
		(void)strcpy(buf, p);
		if (!strcmp(buf, getpass("Retype new password:")))
			break;
		printf("Mismatch; try again, EOF to quit.\n");
	}
	/* grab a random printable character that isn't a colon */
	(void)srandom((int)time((time_t *)NULL));
#ifdef NEWSALT
	salt[0] = '_';
	to64(&salt[1], (long)(29*25), 4);
	to64(&salt[5], (long)random(), 4);
#else
	to64(&salt[0], (long)random(), 2);
#endif
	return(crypt(buf, salt));
}

static unsigned char itoa64[] =		/* 0..63 => ascii-64 */
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

to64(s, v, n)
	register char *s;
	register long v;
	register int n;
{
	while (--n >= 0) {
		*s++ = itoa64[v&0x3f];
		v >>= 6;
	}
}

makedb(file)
	char *file;
{
	union wait pstat;
	pid_t pid, waitpid();

	if (!(pid = vfork())) {
		execl(_PATH_MKPASSWD, "mkpasswd", "-p", file, NULL);
		_exit(127);
	}
	return(waitpid(pid, &pstat, 0) == -1 ? -1 : pstat.w_status);
}

usage()
{
#ifdef	KERBEROS
	fprintf(stderr, "usage: passwd [-l] user\n");
#else
	fprintf(stderr, "usage: passwd user\n");
#endif
}
