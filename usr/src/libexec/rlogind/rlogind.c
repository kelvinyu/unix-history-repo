#ifndef lint
static	char sccsid[] = "@(#)rlogind.c	4.22 (Berkeley) %G%";
#endif

/*
 * remote login server:
 *	remuser\0
 *	locuser\0
 *	terminal type\0
 *	data
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sgtty.h>
#include <stdio.h>
#include <netdb.h>
#include <syslog.h>

extern	errno;
int	reapchild();
struct	passwd *getpwnam();
char	*crypt(), *rindex(), *index(), *malloc(), *ntoa();

main(argc, argv)
	int argc;
	char **argv;
{
	int on = 1, options = 0, fromlen;
	struct sockaddr_in from;

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0) {
		openlog(argv[0], LOG_PID, 0);
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	}
	doit(0, &from);
}

char	locuser[32], remuser[32];
char	buf[BUFSIZ];
int	child;
int	cleanup();
int	netf;
extern	errno;
char	*line;

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	char c;
	int i, p, cc, t, pid;
	int stop = TIOCPKT_DOSTOP;
	register struct hostent *hp;

	alarm(60);
	read(f, &c, 1);
	if (c != 0)
		exit(1);
	alarm(0);
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	hp = gethostbyaddr(&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp == 0) {
		char buf[BUFSIZ];

		fatal(f, sprintf(buf, "Host name for your address (%s) unknown",
			ntoa(fromp->sin_addr)));
	}
	if (fromp->sin_family != AF_INET ||
	    fromp->sin_port >= IPPORT_RESERVED)
		fatal(f, "Permission denied");
	write(f, "", 1);
	for (c = 'p'; c <= 's'; c++) {
		struct stat stb;
		line = "/dev/ptyXX";
		line[strlen("/dev/pty")] = c;
		line[strlen("/dev/ptyp")] = '0';
		if (stat(line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			line[strlen("/dev/ptyp")] = "0123456789abcdef"[i];
			p = open(line, 2);
			if (p > 0)
				goto gotpty;
		}
	}
	fatal(f, "All network ports in use");
	/*NOTREACHED*/
gotpty:
	netf = f;
	line[strlen("/dev/")] = 't';
#ifdef DEBUG
	{ int tt = open("/dev/tty", 2);
	  if (tt > 0) {
		ioctl(tt, TIOCNOTTY, 0);
		close(tt);
	  }
	}
#endif
	t = open(line, 2);
	if (t < 0)
		fatalperror(f, line, errno);
	{ struct sgttyb b;
	  gtty(t, &b); b.sg_flags = RAW|ANYP; stty(t, &b);
	}
	pid = fork();
	if (pid < 0)
		fatalperror(f, "", errno);
	if (pid) {
		char pibuf[1024], fibuf[1024], *pbp, *fbp;
		register pcc = 0, fcc = 0;
		int on = 1;
/* FILE *console = fopen("/dev/console", "w");  */
/* setbuf(console, 0); */

/* fprintf(console, "f %d p %d\r\n", f, p); */
		close(t);
		ioctl(f, FIONBIO, &on);
		ioctl(p, FIONBIO, &on);
		ioctl(p, TIOCPKT, &on);
		signal(SIGTSTP, SIG_IGN);
		signal(SIGCHLD, cleanup);
		for (;;) {
			int ibits = 0, obits = 0;

			if (fcc)
				obits |= (1<<p);
			else
				ibits |= (1<<f);
			if (pcc >= 0)
				if (pcc)
					obits |= (1<<f);
				else
					ibits |= (1<<p);
/* fprintf(console, "ibits from %d obits from %d\r\n", ibits, obits); */
			if (select(16, &ibits, &obits, 0, 0) < 0) {
				if (errno == EINTR)
					continue;
				fatalperror(f, "select", errno);
			}
/* fprintf(console, "ibits %d obits %d\r\n", ibits, obits); */
			if (ibits == 0 && obits == 0) {
				/* shouldn't happen... */
				sleep(5);
				continue;
			}
			if (ibits & (1<<f)) {
				fcc = read(f, fibuf, sizeof (fibuf));
/* fprintf(console, "%d from f\r\n", fcc); */
				if (fcc < 0 && errno == EWOULDBLOCK)
					fcc = 0;
				else {
					if (fcc <= 0)
						break;
					fbp = fibuf;
				}
			}
			if (ibits & (1<<p)) {
				pcc = read(p, pibuf, sizeof (pibuf));
/* fprintf(console, "%d from p, buf[0] %x, errno %d\r\n", pcc, buf[0], errno); */
				pbp = pibuf;
				if (pcc < 0 && errno == EWOULDBLOCK)
					pcc = 0;
				else if (pcc <= 0)
					break;
				else if (pibuf[0] == 0)
					pbp++, pcc--;
				else {
					if (pibuf[0]&(TIOCPKT_FLUSHWRITE|
						      TIOCPKT_NOSTOP|
						      TIOCPKT_DOSTOP)) {
					/* The following 3 lines do nothing. */
						int nstop = pibuf[0] &
						    (TIOCPKT_NOSTOP|
						     TIOCPKT_DOSTOP);
						if (nstop)
							stop = nstop;
						pibuf[0] |= nstop;
						send(f,&pibuf[0],1,MSG_OOB);
					}
					pcc = 0;
				}
			}
			if ((obits & (1<<f)) && pcc > 0) {
				cc = write(f, pbp, pcc);
				if (cc < 0 && errno == EWOULDBLOCK) {
					/* also shouldn't happen */
					sleep(5);
					continue;
				}
/* fprintf(console, "%d of %d to f\r\n", cc, pcc); */
				if (cc > 0) {
					pcc -= cc;
					pbp += cc;
				}
			}
			if ((obits & (1<<p)) && fcc > 0) {
				cc = write(p, fbp, fcc);
/* fprintf(console, "%d of %d to p\r\n", cc, fcc); */
				if (cc > 0) {
					fcc -= cc;
					fbp += cc;
				}
			}
		}
		cleanup();
	}
	close(f);
	close(p);
	dup2(t, 0);
	dup2(t, 1);
	dup2(t, 2);
	close(t);
	execl("/bin/login", "login", "-r", hp->h_name, 0);
	fatalperror(2, "/bin/login", errno);
	/*NOTREACHED*/
}

cleanup()
{

	rmut();
	vhangup();		/* XXX */
	shutdown(netf, 2);
	kill(0, SIGKILL);
	exit(1);
}

fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	buf[0] = '\01';		/* error indicator */
	(void) sprintf(buf + 1, "rlogind: %s.\r\n", msg);
	(void) write(f, buf, strlen(buf));
	exit(1);
}

fatalperror(f, msg, errno)
	int f;
	char *msg;
	int errno;
{
	char buf[BUFSIZ];
	extern int sys_nerr;
	extern char *sys_errlist[];

	if ((unsigned) errno < sys_nerr)
		(void) sprintf(buf, "%s: %s", msg, sys_errlist[errno]);
	else
		(void) sprintf(buf, "%s: Error %d", msg, errno);
	fatal(f, buf);
}

#include <utmp.h>

struct	utmp wtmp;
char	wtmpf[]	= "/usr/adm/wtmp";
char	utmp[] = "/etc/utmp";
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

rmut()
{
	register f;
	int found = 0;

	f = open(utmp, 2);
	if (f >= 0) {
		while(read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, line+5) || wtmp.ut_name[0]==0)
				continue;
			lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			found++;
		}
		close(f);
	}
	if (found) {
		f = open(wtmpf, 1);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, line+5);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			time(&wtmp.ut_time);
			lseek(f, (long)0, 2);
			write(f, (char *)&wtmp, sizeof(wtmp));
			close(f);
		}
	}
	chmod(line, 0666);
	chown(line, 0, 0);
	line[strlen("/dev/")] = 'p';
	chmod(line, 0666);
	chown(line, 0, 0);
}

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
char *
ntoa(in)
	struct in_addr in;
{
	static char b[18];
	register char *p;

	p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
	sprintf(b, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
	return (b);
}
