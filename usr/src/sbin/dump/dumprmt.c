/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)dumprmt.c	5.9 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <ufs/dinode.h>

#include <netinet/in.h>

#include <netdb.h>
#include <protocols/dumprestore.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pathnames.h"

#define	TS_CLOSED	0
#define	TS_OPEN		1

static	int rmtstate = TS_CLOSED;
int	rmtape;
void	rmtgetconn();
void	rmtconnaborted();
int	rmtreply();
int	rmtgetb();
void	rmtgets();
int	rmtcall();
char	*rmtpeer;

extern int ntrec;		/* blocking factor on tape */
extern void msg();

int
rmthost(host)
	char *host;
{

	rmtpeer = host;
	signal(SIGPIPE, rmtconnaborted);
	rmtgetconn();
	if (rmtape < 0)
		return (0);
	return (1);
}

void
rmtconnaborted()
{

	fprintf(stderr, "rdump: Lost connection to remote host.\n");
	exit(1);
}

void
rmtgetconn()
{
	static struct servent *sp = 0;
	struct passwd *pw;
	char *name = "root";
	int size;

	if (sp == 0) {
		sp = getservbyname("shell", "tcp");
		if (sp == 0) {
			fprintf(stderr, "rdump: shell/tcp: unknown service\n");
			exit(1);
		}
	}
	pw = getpwuid(getuid());
	if (pw && pw->pw_name)
		name = pw->pw_name;
	rmtape = rcmd(&rmtpeer, sp->s_port, name, name, _PATH_RMT, 0);
	size = ntrec * TP_BSIZE;
	while (size > TP_BSIZE &&
	    setsockopt(rmtape, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size)) < 0)
		size -= TP_BSIZE;
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

int
rmtcall(cmd, buf)
	char *cmd, *buf;
{

	if (write(rmtape, buf, strlen(buf)) != strlen(buf))
		rmtconnaborted();
	return (rmtreply(cmd));
}

int
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
rmtgets(cp, len)
	char *cp;
	int len;
{

	while (len > 1) {
		*cp = rmtgetb();
		if (*cp == '\n') {
			cp[1] = 0;
			return;
		}
		cp++;
		len--;
	}
	msg("Protocol to remote tape server botched (in rmtgets).\n");
	rmtconnaborted();
}
