#ifndef lint
static	char sccsid[] = "@(#)comsat.c	4.12 (Berkeley) %G%";
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <syslog.h>

/*
 * comsat
 */
int	debug = 0;
#define	dprintf	if (debug) printf

#define MAXUTMP 100		/* down from init */

struct	sockaddr_in sin = { AF_INET };
extern	errno;

char	hostname[32];
struct	utmp utmp[100];
int	nutmp;
int	uf;
unsigned utmpmtime;			/* last modification time for utmp */
int	onalrm();
int	reapchildren();
long	lastmsgtime;

#define	MAXIDLE	120
#define NAMLEN (sizeof (uts[0].ut_name) + 1)

main(argc, argv)
	int argc;
	char *argv[];
{
	register int cc;
	char buf[BUFSIZ];
	char msgbuf[100];
	struct sockaddr_in from;
	int fromlen;

	/* verify proper invocation */
	fromlen = sizeof (from);
	if (getsockname(0, &from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getsockname");
		_exit(1);
	}
	chdir("/usr/spool/mail");
	if ((uf = open("/etc/utmp",0)) < 0) {
		openlog("comsat", 0, 0);
		syslog(LOG_ERR, "/etc/utmp: %m");
		(void) recv(0, msgbuf, sizeof (msgbuf) - 1, 0);
		exit(1);
	}
	lastmsgtime = time(0);
	gethostname(hostname, sizeof (hostname));
	onalrm();
	signal(SIGALRM, onalrm);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGCHLD, reapchildren);
	for (;;) {
		cc = recv(0, msgbuf, sizeof (msgbuf) - 1, 0);
		if (cc <= 0) {
			if (errno != EINTR)
				sleep(1);
			errno = 0;
			continue;
		}
		sigblock(1<<SIGALRM);
		msgbuf[cc] = 0;
		lastmsgtime = time(0);
		mailfor(msgbuf);
		sigsetmask(0);
	}
}

reapchildren()
{

	while (wait3((struct wait *)0, WNOHANG, (struct rusage *)0) > 0)
		;
}

onalrm()
{
	struct stat statbf;
	struct utmp *utp;

	if (time(0) - lastmsgtime >= MAXIDLE)
		exit(0);
	dprintf("alarm\n");
	alarm(15);
	fstat(uf, &statbf);
	if (statbf.st_mtime > utmpmtime) {
		dprintf(" changed\n");
		utmpmtime = statbf.st_mtime;
		lseek(uf, 0, 0);
		nutmp = read(uf,utmp,sizeof(utmp))/sizeof(struct utmp);
	} else
		dprintf(" ok\n");
}

mailfor(name)
	char *name;
{
	register struct utmp *utp = &utmp[nutmp];
	register char *cp;
	char *rindex();
	int offset;

	dprintf("mailfor %s\n", name);
	cp = name;
	while (*cp && *cp != '@')
		cp++;
	if (*cp == 0) {
		dprintf("bad format\n");
		return;
	}
	*cp = 0;
	offset = atoi(cp+1);
	while (--utp >= utmp)
		if (!strncmp(utp->ut_name, name, sizeof(utmp[0].ut_name)))
			notify(utp, offset);
}

char	*cr;

notify(utp, offset)
	register struct utmp *utp;
{
	int fd, flags, n, err, msglen;
	struct sgttyb gttybuf;
	char tty[20], msgbuf[BUFSIZ];
	char name[sizeof (utmp[0].ut_name) + 1];
	struct stat stb;

	strcpy(tty, "/dev/");
	strncat(tty, utp->ut_line, sizeof(utp->ut_line));
	dprintf("notify %s on %s\n", utp->ut_name, tty);
	if (stat(tty, &stb) == 0 && (stb.st_mode & 0100) == 0) {
		dprintf("wrong mode\n");
		return;
	}
	if ((fd = open(tty, O_WRONLY|O_NDELAY)) < 0) {
		dprintf("%s: open failed\n", tty);
		return;
	}
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
		dprintf("fcntl(F_GETFL) failed %d\n", errno);
		return;
	}
	ioctl(fd, TIOCGETP, &gttybuf);
	cr = (gttybuf.sg_flags & CRMOD) ? "" : "\r";
	strncpy(name, utp->ut_name, sizeof (utp->ut_name));
	name[sizeof (name) - 1] = '\0';
	sprintf(msgbuf, "%s\n\007New mail for %s@%s\007 has arrived:%s\n",
	    cr, name, hostname, cr);
	jkfprintf(msgbuf+strlen(msgbuf), name, offset);
	if (fcntl(fd, F_SETFL, flags | FNDELAY) == -1)
		goto oldway;
	msglen = strlen(msgbuf);
	n = write(fd, msgbuf, msglen);
	err = errno;
	(void) fcntl(fd, F_SETFL, flags);
	(void) close(fd);
	if (n == msglen)
		return;
	if (err != EWOULDBLOCK) {
		dprintf("write failed %d\n", errno);
		return;
	}
oldway:
	if (fork()) {
		(void) close(fd);
		return;
	}
	signal(SIGALRM, SIG_DFL);
	alarm(30);
	(void) write(fd, msgbuf, msglen);
	exit(0);
}

jkfprintf(mp, name, offset)
	register char *mp;
{
	register FILE *fi;
	register int linecnt, charcnt;
	char line[BUFSIZ];
	int inheader;

	dprintf("HERE %s's mail starting at %d\n", name, offset);

	if ((fi = fopen(name, "r")) == NULL) {
		dprintf("Cant read the mail\n");
		return;
	}
	fseek(fi, offset, L_SET);
	/* 
	 * Print the first 7 lines or 560 characters of the new mail
	 * (whichever comes first).  Skip header crap other than
	 * From, Subject, To, and Date.
	 */
	linecnt = 7;
	charcnt = 560;
	inheader = 1;
	while (fgets(line, sizeof (line), fi) != NULL) {
		register char *cp;
		char *index();
		int cnt;

		if (linecnt <= 0 || charcnt <= 0) {  
			sprintf(mp, "...more...%s\n", cr);
			mp += strlen(mp);
			return;
		}
		if (strncmp(line, "From ", 5) == 0)
			continue;
		if (inheader && (line[0] == ' ' || line[0] == '\t'))
			continue;
		cp = index(line, ':');
		if (cp == 0 || (index(line, ' ') && index(line, ' ') < cp))
			inheader = 0;
		else
			cnt = cp - line;
		if (inheader &&
		    strncmp(line, "Date", cnt) &&
		    strncmp(line, "From", cnt) &&
		    strncmp(line, "Subject", cnt) &&
		    strncmp(line, "To", cnt))
			continue;
		cp = index(line, '\n');
		if (cp)
			*cp = '\0';
		sprintf(mp, "%s%s\n", line, cr);
		mp += strlen(mp);
		linecnt--, charcnt -= strlen(line);
	}
	sprintf(mp, "----%s\n", cr);
	mp += strlen(mp);
}
