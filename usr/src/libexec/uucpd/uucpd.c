#ifndef lint
static char sccsid[] = "@(#)uucpd.c	5.2 (Berkeley) %G%";
#endif

/*
 * 4.2BSD TCP/IP server for uucico
 * uucico's TCP channel causes this server to be run at the remote end.
 */

#include "uucp.h"
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <pwd.h>
#include <lastlog.h>

char lastlog[] = "/usr/adm/lastlog";
struct	sockaddr_in hisctladdr;
int hisaddrlen = sizeof hisctladdr;
struct	sockaddr_in myctladdr;
int mypid;

main(argc, argv)
int argc;
char **argv;
{
	register int s, tcp_socket;
	struct servent *sp;
	extern int errno;
	int dologout();

#ifdef BSDINETD
	close(1); close(2);
	dup(0); dup(0);
	hisaddrlen = sizeof (hisctladdr);
	if (getpeername(0, &hisctladdr, &hisaddrlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (fork() == 0)
		doit(&hisctladdr);
	dologout();
	exit(1);
#else !BSDINETD
	sp = getservbyname("uucp", "tcp");
	if (sp == NULL){
		perror("uucpd: getservbyname");
		exit(1);
	}
	if ((s=open("/dev/tty", 2)) >= 0){
		ioctl(s, TIOCNOTTY, (char *)0);
		close(s);
	}

	bzero((char *)&myctladdr, sizeof (myctladdr));
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0, 0);
	if ( tcp_socket<0 ) {
		perror("uucpd: socket");
		exit(1);
	}
	myctladdr.sin_port = sp->s_port;
	if (bind(tcp_socket, (char *)&myctladdr, sizeof (myctladdr), 0) < 0) {
		perror("uucpd: bind");
		exit(1);
	}
	signal(SIGCHLD, dologout);
	listen(tcp_socket, 5);	/* 5's as good as any */

	for(;;) {
		s = accept(tcp_socket, &hisctladdr, &hisaddrlen);
		if (s < 0){
			if (errno == EINTR)
				continue;
			perror("uucpd: accept");
			exit(1);
		}
		if (fork()== 0) {
			close(0); close(1); close(2);
			dup(s); dup(s); dup(s);
			close(tcp_socket);close(s);
			doit(&hisctladdr);
			exit(1);
		}
		close(s);
	}
#endif
}

doit(sinp)
struct sockaddr_in *sinp;
{
	char ebuf[32];
	char user[64];
	char passwd[64];
	char *xpasswd, *crypt();
	struct passwd *pw, *getpwnam();

	alarm(60);
	if (readline(user, sizeof user) < 0) {
		fprintf(stderr, "user read\n");
		return;
	}
	/* truncate username to 8 characters */
	user[8] = '\0';
	pw = getpwnam(user);
	if (pw == NULL) {
		fprintf(stderr, "user unknown\n");
		return;
	}
	if (strcmp(pw->pw_shell, UUCICO)) {
		fprintf(stderr, "Login incorrect.");
		return;
	}
	if (pw->pw_passwd && *pw->pw_passwd != '\0') {
		if (readline(passwd, sizeof passwd) < 0) {
			fprintf(stderr, "passwd read\n");
			return;
		}
		xpasswd = crypt(passwd, pw->pw_passwd);
		if (strcmp(xpasswd, pw->pw_passwd)) {
			fprintf(stderr, "Login incorrect.");
			return;
		}
	}
	alarm(0);
	dologin(pw, sinp);
	setegid(pw->pw_gid);
	initgroups(pw->pw_name, pw->pw_gid);
	chdir(pw->pw_dir);
	seteuid(pw->pw_uid);
	execl(UUCICO, "uucico", (char *)0);
	perror("uucico server: execl");
}

readline(p, n)
	register char *p;
	register int n;
{
	char c;

	while (n-- > 0) {
		if (read(0, &c, 1) <= 0)
			return(-1);
		c &= 0177;
		if (c == '\n' || c == '\r') {
			*p = '\0';
			return(0);
		}
		*p++ = c;
	}
	return(-1);
}

#include <utmp.h>
#include <fcntl.h>

#define	SCPYN(a, b)	strncpy(a, b, sizeof (a))

struct	utmp utmp;

dologout()
{
	union wait status;
	int pid, wtmp;

#ifdef BSDINETD
	while ((pid=wait(&status)) > 0 ) {
#else  !BSDINETD
	while ((pid=wait3(&status,WNOHANG,0)) > 0 ) {
#endif !BSDINETD
		wtmp = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
		if (wtmp >= 0) {
			sprintf(utmp.ut_line, "uucp%.4d", pid);
			SCPYN(utmp.ut_name, "");
			SCPYN(utmp.ut_host, "");
			utmp.ut_time = time(0);
			(void) write(wtmp, (char *)&utmp, sizeof (utmp));
			(void) close(wtmp);
		}
	}
}

/*
 * Record login in wtmp file.
 */
dologin(pw, sin)
struct passwd *pw;
struct sockaddr_in *sin;
{
	char line[32];
	char remotehost[32];
	int wtmp, f;
	struct hostent *hp = gethostbyaddr(&sin->sin_addr,
		sizeof (struct in_addr), AF_INET);

	if (hp) {
		strncpy(remotehost, hp->h_name, sizeof (remotehost));
		endhostent();
	} else
		strncpy(remotehost, inet_ntoa(sin->sin_addr),
		    sizeof (remotehost));
	wtmp = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
	if (wtmp >= 0) {
		/* hack, but must be unique and no tty line */
		sprintf(line, "uucp%.4d", getpid());
		SCPYN(utmp.ut_line, line);
		SCPYN(utmp.ut_name, pw->pw_name);
		SCPYN(utmp.ut_host, remotehost);
		utmp.ut_time = time(0);
		(void) write(wtmp, (char *)&utmp, sizeof (utmp));
		(void) close(wtmp);
	}
	if ((f = open(lastlog, 2)) >= 0) {
		struct lastlog ll;

		time(&ll.ll_time);
		lseek(f, (long)pw->pw_uid * sizeof(struct lastlog), 0);
		strcpy(line, remotehost);
		SCPYN(ll.ll_line, line);
		SCPYN(ll.ll_host, remotehost);
		(void) write(f, (char *) &ll, sizeof ll);
		(void) close(f);
	}
}
