/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)dumprmt.c	5.17 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#ifdef sunos
#include <sys/vnode.h>

#include <ufs/inode.h>
#else
#include <ufs/ufs/dinode.h>
#endif

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <protocols/dumprestore.h>

#include <ctype.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "pathnames.h"
#include "dump.h"

#define	TS_CLOSED	0
#define	TS_OPEN		1

static	int rmtstate = TS_CLOSED;
static	int rmtape;
static	char *rmtpeer;

static	int okname __P((char *));
static	int rmtcall __P((char *, char *));
static	void rmtconnaborted __P((/* int, int */));
static	int rmtgetb __P((void));
static	void rmtgetconn __P((void));
static	void rmtgets __P((char *, int));
static	int rmtreply __P((char *));

extern	int ntrec;		/* blocking factor on tape */

int
rmthost(host)
	char *host;
{

	rmtpeer = malloc(strlen(host) + 1);
	if (rmtpeer)
		strcpy(rmtpeer, host);
	else
		rmtpeer = host;
	signal(SIGPIPE, rmtconnaborted);
	rmtgetconn();
	if (rmtape < 0)
		return (0);
	return (1);
}

static void
rmtconnaborted()
{

	(void) fprintf(stderr, "rdump: Lost connection to remote host.\n");
	exit(1);
}

void
rmtgetconn()
{
	register char *cp;
	static struct servent *sp = 0;
	static struct passwd *pwd = 0;
#ifdef notdef
	static int on = 1;
#endif
	char *tuser;
	int size;
	int maxseg;

	if (sp == 0) {
		sp = getservbyname("shell", "tcp");
		if (sp == 0) {
			(void) fprintf(stderr,
			    "rdump: shell/tcp: unknown service\n");
			exit(1);
		}
		pwd = getpwuid(getuid());
		if (pwd == 0) {
			(void) fprintf(stderr, "rdump: who are you?\n");
			exit(1);
		}
	}
	if (cp = index(rmtpeer, '@')) {
		tuser = rmtpeer;
		*cp = '\0';
		if (!okname(tuser))
			exit(1);
		rmtpeer = ++cp;
	} else
		tuser = pwd->pw_name;
	rmtape = rcmd(&rmtpeer, (u_short)sp->s_port, pwd->pw_name, tuser,
	    _PATH_RMT, (int *)0);
	size = ntrec * TP_BSIZE;
	if (size > 60 * 1024)		/* XXX */
		size = 60 * 1024;
	/* Leave some space for rmt request/response protocol */
	size += 2 * 1024;
	while (size > TP_BSIZE &&
	    setsockopt(rmtape, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size)) < 0)
		    size -= TP_BSIZE;
	(void)setsockopt(rmtape, SOL_SOCKET, SO_RCVBUF, &size, sizeof (size));
	maxseg = 1024;
	if (setsockopt(rmtape, IPPROTO_TCP, TCP_MAXSEG,
	    &maxseg, sizeof (maxseg)) < 0)
		perror("TCP_MAXSEG setsockopt");

#ifdef notdef
	if (setsockopt(rmtape, IPPROTO_TCP, TCP_NODELAY, &on, sizeof (on)) < 0)
		perror("TCP_NODELAY setsockopt");
#endif
}

static int
okname(cp0)
	char *cp0;
{
	register char *cp;
	register int c;

	for (cp = cp0; *cp; cp++) {
		c = *cp;
		if (!isascii(c) || !(isalnum(c) || c == '_' || c == '-')) {
			(void) fprintf(stderr, "rdump: invalid user name %s\n",
			    cp0);
			return (0);
		}
	}
	return (1);
}

int
rmtopen(tape, mode)
	char *tape;
	int mode;
{
	char buf[256];

	(void)sprintf(buf, "O%s\n%d\n", tape, mode);
	rmtstate = TS_OPEN;
	return (rmtcall(tape, buf));
}

void
rmtclose()
{

	if (rmtstate != TS_OPEN)
		return;
	rmtcall("close", "C\n");
	rmtstate = TS_CLOSED;
}

int
rmtread(buf, count)
	char *buf;
	int count;
{
	char line[30];
	int n, i, cc;
	extern errno;

	(void)sprintf(line, "R%d\n", count);
	n = rmtcall("read", line);
	if (n < 0) {
		errno = n;
		return (-1);
	}
	for (i = 0; i < n; i += cc) {
		cc = read(rmtape, buf+i, n - i);
		if (cc <= 0) {
			rmtconnaborted();
		}
	}
	return (n);
}

int
rmtwrite(buf, count)
	char *buf;
	int count;
{
	char line[30];

	(void)sprintf(line, "W%d\n", count);
	write(rmtape, line, strlen(line));
	write(rmtape, buf, count);
	return (rmtreply("write"));
}

void
rmtwrite0(count)
	int count;
{
	char line[30];

	(void)sprintf(line, "W%d\n", count);
	write(rmtape, line, strlen(line));
}

void
rmtwrite1(buf, count)
	char *buf;
	int count;
{

	write(rmtape, buf, count);
}

int
rmtwrite2()
{

	return (rmtreply("write"));
}

int
rmtseek(offset, pos)
	int offset, pos;
{
	char line[80];

	(void)sprintf(line, "L%d\n%d\n", offset, pos);
	return (rmtcall("seek", line));
}

struct	mtget mts;

struct mtget *
rmtstatus()
{
	register int i;
	register char *cp;

	if (rmtstate != TS_OPEN)
		return (0);
	rmtcall("status", "S\n");
	for (i = 0, cp = (char *)&mts; i < sizeof(mts); i++)
		*cp++ = rmtgetb();
	return (&mts);
}

int
rmtioctl(cmd, count)
	int cmd, count;
{
	char buf[256];

	if (count < 0)
		return (-1);
	(void)sprintf(buf, "I%d\n%d\n", cmd, count);
	return (rmtcall("ioctl", buf));
}

static int
rmtcall(cmd, buf)
	char *cmd, *buf;
{

	if (write(rmtape, buf, strlen(buf)) != strlen(buf))
		rmtconnaborted();
	return (rmtreply(cmd));
}

static int
rmtreply(cmd)
	char *cmd;
{
	char code[30], emsg[BUFSIZ];

	rmtgets(code, sizeof (code));
	if (*code == 'E' || *code == 'F') {
		rmtgets(emsg, sizeof (emsg));
		msg("%s: %s\n", cmd, emsg, code + 1);
		if (*code == 'F') {
			rmtstate = TS_CLOSED;
			return (-1);
		}
		return (-1);
	}
	if (*code != 'A') {
		msg("Protocol to remote tape server botched (code %s?).\n",
		    code);
		rmtconnaborted();
	}
	return (atoi(code + 1));
}

int
rmtgetb()
{
	char c;

	if (read(rmtape, &c, 1) != 1)
		rmtconnaborted();
	return (c);
}

void
rmtgets(line, len)
	char *line;
	int len;
{
	register char *cp = line;

	while (len > 1) {
		*cp = rmtgetb();
		if (*cp == '\n') {
			cp[1] = 0;
			return;
		}
		cp++;
		len--;
	}
	*cp = 0;
	msg("Protocol to remote tape server botched.\n");
	msg("(rmtgets got \"%s\").\n", line);
	rmtconnaborted();
}
