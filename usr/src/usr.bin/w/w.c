/*-
 * Copyright (c) 1980, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1991 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)w.c	5.37 (Berkeley) %G%";
#endif /* not lint */

/*
 * w - print system status (who and what)
 *
 * This program is similar to the systat command on Tenex/Tops 10/20
 *
 */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/tty.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <kvm.h>
#include <netdb.h>
#include <nlist.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tzfile.h>
#include <unistd.h>
#include <utmp.h>

#include "extern.h"

struct timeval boottime;
struct utmp utmp;
struct winsize ws;
kvm_t *kd;
time_t now;			/* the current time of day */
time_t uptime;			/* time of last reboot & elapsed time since */
int ttywidth;			/* width of tty */
int argwidth;			/* width of tty */
int header = 1;			/* true if -h flag: don't print heading */
int nflag;			/* true if -n flag: don't convert addrs */
int sortidle;			/* sort bu idle time */
char *sel_user;			/* login of particular user selected */
char *program;
char domain[MAXHOSTNAMELEN];

/*
 * One of these per active utmp entry.
 */
struct	entry {
	struct	entry *next;
	struct	utmp utmp;
	dev_t	tdev;		/* dev_t of terminal */
	time_t	idle;		/* idle time of terminal in seconds */
	struct	kinfo_proc *kp;	/* `most interesting' proc */
	char	*args;		/* arg list of interesting process */
} *ep, *ehead = NULL, **nextp = &ehead;

struct nlist nl[] = {
	{ "_boottime" },
#define X_BOOTTIME	0
#if defined(hp300) || defined(i386)
	{ "_cn_tty" },
#define X_CNTTY		1
#endif
	{ "" },
};

static void	 pr_header __P((kvm_t *, time_t *, int));
static struct stat
		*ttystat __P((char *));
static void	 usage __P((int));

char *fmt_argv __P((char **, char *, int));

int
main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	struct kinfo_proc *kp;
	struct hostent *hp;
	struct stat *stp;
	FILE *ut;
	u_long l;
	int ch, i, nentries, nusers, wcmd;
	char *memf, *nlistf, *p, *x;
	char buf[MAXHOSTNAMELEN], errbuf[256];

	/* Are we w(1) or uptime(1)? */
	program = argv[0];
	if ((p = rindex(program, '/')) || *(p = program) == '-')
		p++;
	if (*p == 'u') {
		wcmd = 0;
		p = "";
	} else {
		wcmd = 1;
		p = "hiflM:N:nsuw";
	}

	memf = nlistf = NULL;
	while ((ch = getopt(argc, argv, p)) != EOF)
		switch (ch) {
		case 'h':
			header = 0;
			break;
		case 'i':
			sortidle = 1;
			break;
		case 'M':
			memf = optarg;
			break;
		case 'N':
			nlistf = optarg;
			break;
		case 'n':
			nflag = 1;
			break;
		case 'f': case 'l': case 's': case 'u': case 'w':
			warnx("[-flsuw] no longer supported");
			/* FALLTHROUGH */
		case '?':
		default:
			usage(wcmd);
		}
	argc -= optind;
	argv += optind;

	if ((kd = kvm_openfiles(nlistf, memf, NULL, O_RDONLY, errbuf)) == NULL)
		err(1, "%s", errbuf);
	if (header && kvm_nlist(kd, nl) != 0)
		err(1, "can't read namelist");

	(void)time(&now);
	if ((ut = fopen(_PATH_UTMP, "r")) == NULL)
		err(1, "%s", _PATH_UTMP);

	if (*argv)
		sel_user = *argv;

	for (nusers = 0; fread(&utmp, sizeof(utmp), 1, ut);) {
		if (utmp.ut_name[0] == '\0')
			continue;
		++nusers;
		if (wcmd == 0 || (sel_user &&
		    strncmp(utmp.ut_name, sel_user, UT_NAMESIZE) != 0))
			continue;
		if ((ep = calloc(1, sizeof(struct entry))) == NULL)
			err(1, NULL);
		*nextp = ep;
		nextp = &(ep->next);
		bcopy(&utmp, &(ep->utmp), sizeof (struct utmp));
		stp = ttystat(ep->utmp.ut_line);
		ep->tdev = stp->st_rdev;
#if defined(hp300) || defined(i386)
		/*
		 * XXX
		 * If this is the console device, attempt to ascertain
		 * the true console device dev_t.
		 */
		if (ep->tdev == 0) {
			static dev_t cn_dev;

			if (nl[X_CNTTY].n_value) {
				struct tty cn_tty, *cn_ttyp;
				
				if (kvm_read(kd, (u_long)nl[X_CNTTY].n_value,
				    (char *)&cn_ttyp, sizeof(cn_ttyp)) > 0) {
					(void)kvm_read(kd, (u_long)cn_ttyp,
					    (char *)&cn_tty, sizeof (cn_tty));
					cn_dev = cn_tty.t_dev;
				}
				nl[X_CNTTY].n_value = 0;
			}
			ep->tdev = cn_dev;
		}
#endif
		if ((ep->idle = now - stp->st_atime) < 0)
			ep->idle = 0;
	}
	(void)fclose(ut);

	if (header || wcmd == 0) {
		pr_header(kd, &now, nusers);
		if (wcmd == 0)
			exit (0);
	}

#define HEADER	"USER    TTY FROM              LOGIN@  IDLE WHAT\n"
#define WUSED	(sizeof (HEADER) - sizeof ("WHAT\n"))
	(void)printf(HEADER);

	if ((kp = kvm_getprocs(kd, KERN_PROC_ALL, 0, &nentries)) == NULL)
		err(1, "%s", kvm_geterr(kd));
	for (i = 0; i < nentries; i++, kp++) {
		register struct proc *p = &kp->kp_proc;
		register struct eproc *e;

		if (p->p_stat == SIDL || p->p_stat == SZOMB)
			continue;
		e = &kp->kp_eproc;
		for (ep = ehead; ep != NULL; ep = ep->next) {
			if (ep->tdev == e->e_tdev && e->e_pgid == e->e_tpgid) {
				/*
				 * Proc is in foreground of this terminal
				 */
				if (proc_compare(&ep->kp->kp_proc, p))
					ep->kp = kp;
				break;
			}
		}
	}
	if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 &&
	     ioctl(STDERR_FILENO, TIOCGWINSZ, &ws) == -1 &&
	     ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) || ws.ws_col == 0)
	       ttywidth = 79;
        else
	       ttywidth = ws.ws_col - 1;
	argwidth = ttywidth - WUSED;
	if (argwidth < 4)
		argwidth = 8;
	for (ep = ehead; ep != NULL; ep = ep->next) {
		if (ep->kp == NULL) {
			ep->args = "-";
			continue;
		}
		ep->args = fmt_argv(kvm_getargv(kd, ep->kp, argwidth),
		    ep->kp->kp_proc.p_comm, MAXCOMLEN);
		if (ep->args == NULL)
			err(1, NULL);
	}
	/* sort by idle time */
	if (sortidle && ehead != NULL) {
		struct entry *from = ehead, *save;
		
		ehead = NULL;
		while (from != NULL) {
			for (nextp = &ehead;
			    (*nextp) && from->idle >= (*nextp)->idle;
			    nextp = &(*nextp)->next)
				continue;
			save = from;
			from = from->next;
			save->next = *nextp;
			*nextp = save;
		}
	}
			
	if (!nflag)
		if (gethostname(domain, sizeof(domain) - 1) < 0 ||
		    (p = index(domain, '.')) == 0)
			domain[0] = '\0';
		else {
			domain[sizeof(domain) - 1] = '\0';
			bcopy(p, domain, strlen(p) + 1);
		}

	for (ep = ehead; ep != NULL; ep = ep->next) {
		p = *ep->utmp.ut_host ? ep->utmp.ut_host : "-";
		if (x = index(p, ':'))
			*x++ = '\0';
		if (!nflag && isdigit(*p) &&
		    (long)(l = inet_addr(p)) != -1 &&
		    (hp = gethostbyaddr((char *)&l, sizeof(l), AF_INET))) {
			if (domain[0] != '\0') {
				p = hp->h_name;
				p += strlen(hp->h_name);
				p -= strlen(domain);
				if (p > hp->h_name && !strcmp(p, domain))
					*p = '\0';
			}
			p = hp->h_name;
		}
		if (x) {
			(void)snprintf(buf, sizeof(buf), "%s:%s", p, x);
			p = buf;
		}
		(void)printf("%-*.*s %-2.2s %-*.*s ",
		    UT_NAMESIZE, UT_NAMESIZE, ep->utmp.ut_name,
		    strncmp(ep->utmp.ut_line, "tty", 3) ?
		    ep->utmp.ut_line : ep->utmp.ut_line + 3,
		    UT_HOSTSIZE, UT_HOSTSIZE, *p ? p : "-");
		pr_attime(&ep->utmp.ut_time, &now);
		pr_idle(ep->idle);
		(void)printf("%.*s\n", argwidth, ep->args);
	}
	exit(0);
}

static void
pr_header(kd, nowp, nusers)
	kvm_t *kd;
	time_t *nowp;
	int nusers;
{
	double avenrun[3];
	time_t uptime;
	int days, hrs, i, mins;
	char buf[256], fmt[10];

	/*
	 * Print time of day.
	 *
	 * Note, SCCS forces the string manipulation below, as it
	 * replaces w.c with file information.
	 */
	(void)strcpy(fmt, "%l:%%%p");
	fmt[4] = 'M';
	(void)strftime(buf, sizeof(buf), fmt, localtime(nowp));
	(void)printf("%s ", buf);

	/*
	 * Print how long system has been up.
	 * (Found by looking for "boottime" in kernel)
	 */
	if ((kvm_read(kd, (u_long)nl[X_BOOTTIME].n_value,
	    &boottime, sizeof(boottime))) != sizeof(boottime))
		err(1, "can't read kernel bootime variable");

	uptime = now - boottime.tv_sec;
	uptime += 30;
	days = uptime / SECSPERDAY;
	uptime %= SECSPERDAY;
	hrs = uptime / SECSPERHOUR;
	uptime %= SECSPERHOUR;
	mins = uptime / SECSPERMIN;
	(void)printf(" up");
	if (days > 0)
		(void)printf(" %d day%s,", days, days > 1 ? "s" : "");
	if (hrs > 0 && mins > 0)
		(void)printf(" %2d:%02d,", hrs, mins);
	else {
		if (hrs > 0)
			(void)printf(" %d hr%s,",
			    hrs, hrs > 1 ? "s" : "");
		if (mins > 0)
			(void)printf(" %d min%s,",
			    mins, mins > 1 ? "s" : "");
	}

	/* Print number of users logged in to system */
	(void)printf(" %d user%s", nusers, nusers > 1 ? "s" : "");

	/*
	 * Print 1, 5, and 15 minute load averages.
	 */
	if (kvm_getloadavg(kd,
	    avenrun, sizeof(avenrun) / sizeof(avenrun[0])) == -1)
		(void)printf(", no load average information available\n");
	else {
		(void)printf(", load averages:");
		for (i = 0; i < (sizeof(avenrun) / sizeof(avenrun[0])); i++) {
			if (i > 0)
				(void)printf(",");
			(void)printf(" %.2f", avenrun[i]);
		}
		(void)printf("\n");
	}
}

static struct stat *
ttystat(line)
	char *line;
{
	static struct stat sb;
	char ttybuf[MAXPATHLEN];

	(void)snprintf(ttybuf, sizeof(ttybuf), "%s/%s", _PATH_DEV, line);
	if (stat(ttybuf, &sb))
		err(1, "%s", ttybuf);
	return (&sb);
}

static void
usage(wcmd)
	int wcmd;
{
	if (wcmd)
		(void)fprintf(stderr,
		    "usage: w: [-hin] [-M core] [-N system] [user]\n");
	else
		(void)fprintf(stderr, "uptime\n");
	exit (1);
}
