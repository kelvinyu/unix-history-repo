#ifndef lint
static char sccsid[] = "@(#)rlogin.c	4.5 82/11/27";
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdio.h>
#include <sgtty.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>

/*
 * rlogin - remote login
 */
char	*index(), *rindex(), *malloc(), *getenv();
struct	passwd *getpwuid();
char	*name;
int	rem;
char	cmdchar = '~';
int	rcmdoptions = 0;
int	eight;
char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
char	term[64] = "network";
extern	int errno;
int	lostpeer();

#define	CTRL(c)	('c' & 037)

main(argc, argv)
	int argc;
	char **argv;
{
	char *host, *cp;
	struct sgttyb ttyb;
	struct passwd *pwd;
	struct servent *sp;
	int uid;

	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rlogin"))
		host = *argv++, --argc;
another:
	if (!strcmp(*argv, "-d")) {
		argv++, argc--;
		rcmdoptions |= SO_DEBUG;
		goto another;
	}
	if (!strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc == 0)
			goto usage;
		name = *argv++; argc--;
		goto another;
	}
	if (!strncmp(*argv, "-e", 2)) {
		cmdchar = argv[0][2];
		argv++, argc--;
		goto another;
	}
	if (!strcmp(*argv, "-8")) {
		eight = 1;
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argc > 0)
		goto usage;
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "Who are you?\n");
		exit(1);
	}
	sp = getservbyname("login", "tcp");
	if (sp == 0) {
		fprintf(stderr, "rlogin: login/tcp: unknown service\n");
		exit(2);
	}
	cp = getenv("TERM");
	if (cp)
		strcpy(term, cp);
	if (gtty(0, &ttyb)==0) {
		strcat(term, "/");
		strcat(term, speeds[ttyb.sg_ospeed]);
	}
	signal(SIGPIPE, lostpeer);
        rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    name ? name : pwd->pw_name, term, 0);
        if (rem < 0)
                exit(1);
	uid = getuid();
	if (setuid(uid) < 0) {
		perror("rlogin: setuid");
		exit(1);
	}
	doit();
	/*NOTREACHED*/
usage:
	fprintf(stderr,
	    "usage: rlogin host [ -ex ] [ -l username ]\n");
	exit(1);
}

#define CRLF "\r\n"

int	child;
int	done();

char	tkill, terase;	/* current input kill & erase */
char	defkill, deferase, defflags;

struct	tchars deftchars;
struct	tchars notchars = { -1, -1, CTRL(q), CTRL(s), -1, -1 };
struct	ltchars defltchars;
struct	ltchars noltchars = { -1, -1, -1, -1, -1, -1 };

doit()
{
	struct sgttyb stbuf;
	int exit();

	ioctl(0, TIOCGETP, (char *)&stbuf);
	defkill = stbuf.sg_kill;
	deferase = stbuf.sg_erase;
	defflags = stbuf.sg_flags & (ECHO | CRMOD);
	ioctl(0, TIOCGETC, (char *)&deftchars);
	ioctl(0, TIOCGLTC, (char *)&defltchars);
	signal(SIGINT, exit);
	signal(SIGHUP, exit);
	signal(SIGQUIT, exit);
	child = fork();
	if (child == -1) {
		perror("rlogin: fork");
		done();
	}
	signal(SIGINT, SIG_IGN);
	if (child == 0) {
		signal(SIGPIPE, SIG_IGN);
		reader();
		prf("\007Lost connection.");
		exit(3);
	}
	signal(SIGCHLD, done);
	mode(1);
	writer();
	prf("Disconnected.");
	done();
}

done()
{

	mode(0);
	if (child > 0 && kill(child, SIGKILL) >= 0)
		wait((int *)0);
	exit(0);
}

/*
 * writer: write to remote: 0 -> line.
 * ~.	terminate
 * ~^Z	suspend rlogin process.
 */
writer()
{
	char b[600], c;
	register char *p;

top:
	p = b;
	while (read(0, &c, 1) > 0) {
		int local;

		if (eight == 0)
			c &= 0177;
		/*
		 * If we're at the beginning of the line
		 * and recognize a command character, then
		 * we echo locally.  Otherwise, characters
		 * are echo'd remotely.  If the command
		 * character is doubled, this acts as a 
		 * force and local echo is suppressed.
		 */
		if (p == b)
			local = (c == cmdchar);
		if (p == b + 1 && *b == cmdchar)
			local = (c != cmdchar);
		if (!local) {
			if (write(rem, &c, 1) == 0) {
				prf("line gone");
				return;
			}
			if (eight == 0)
				c &= 0177;
		} else {
			if (c == 0177)
				c = tkill;
			if (c == '\r' || c == '\n') {
				switch (b[1]) {

				case '.':
				case CTRL(d):
					write(0, CRLF, sizeof(CRLF));
					return;

				case CTRL(z):
					write(0, CRLF, sizeof(CRLF));
					mode(0);
					signal(SIGCHLD, SIG_IGN);
					kill(0, SIGTSTP);
					signal(SIGCHLD, done);
					mode(1);
					goto top;
				}
				*p++ = c;
				write(rem, b, p - b);
				goto top;
			}
			write(1, &c, 1);
		}
		*p++ = c;
		if (c == terase) {
			p -= 2; 
			if (p < b)
				goto top;
		}
		if (c == tkill || c == 0177 || c == CTRL(d) ||
		    c == '\r' || c == '\n')
			goto top;
	}
}

oob()
{
	int out = 1+1;
	char waste[BUFSIZ], mark;

	signal(SIGURG, oob);
	ioctl(1, TIOCFLUSH, (char *)&out);
	for (;;) {
		if (ioctl(rem, SIOCATMARK, &mark) < 0) {
			perror("ioctl");
			break;
		}
		if (mark)
			break;
		(void) read(rem, waste, sizeof (waste));
	}
	recv(rem, &mark, 1, SOF_OOB);
	if (mark & TIOCPKT_NOSTOP) {
		notchars.t_stopc = -1;
		notchars.t_startc = -1;
		ioctl(0, TIOCSETC, (char *)&notchars);
	}
	if (mark & TIOCPKT_DOSTOP) {
		notchars.t_stopc = CTRL(s);
		notchars.t_startc = CTRL(q);
		ioctl(0, TIOCSETC, (char *)&notchars);
	}
}

/*
 * reader: read from remote: line -> 1
 */
reader()
{
	char rb[BUFSIZ];
	register int cnt;

	signal(SIGURG, oob);
#ifdef notdef
	{ int pid = -getpid();
	  ioctl(rem, SIOCSPGRP, (char *)&pid); }
#endif
	for (;;) {
		cnt = read(rem, rb, sizeof (rb));
		if (cnt <= 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		write(1, rb, cnt);
	}
}

mode(f)
{
	struct sgttyb stbuf;

	ioctl(0, TIOCGETP, (char *)&stbuf);
	if (f == 0) {
		stbuf.sg_flags &= ~CBREAK;
		stbuf.sg_flags |= defflags;
		ioctl(0, TIOCSETC, (char *)&deftchars);
		ioctl(0, TIOCSLTC, (char *)&defltchars);
		stbuf.sg_kill = defkill;
		stbuf.sg_erase = deferase;
	}
	if (f == 1) {
		stbuf.sg_flags |= CBREAK;
		stbuf.sg_flags &= ~(ECHO|CRMOD);
		ioctl(0, TIOCSETC, (char *)&notchars);
		ioctl(0, TIOCSLTC, (char *)&noltchars);
		stbuf.sg_kill = -1;
		stbuf.sg_erase = -1;
	}
	if (f == 2) {
		stbuf.sg_flags &= ~CBREAK;
		stbuf.sg_flags &= ~(ECHO|CRMOD);
		ioctl(0, TIOCSETC, (char *)&deftchars);
		ioctl(0, TIOCSLTC, (char *)&defltchars);
		stbuf.sg_kill = -1;
		stbuf.sg_erase = -1;
	}
	ioctl(0, TIOCSETN, (char *)&stbuf);
}

/*VARARGS*/
prf(f, a1, a2, a3)
	char *f;
{
	fprintf(stderr, f, a1, a2, a3);
	fprintf(stderr, CRLF);
}

lostpeer()
{
	signal(SIGPIPE, SIG_IGN);
	prf("\007Lost connection");
	done();
}
