#ifndef lint
static char sccsid[] = "@(#)syslog.c	4.5 (Berkeley) %G%";
#endif

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it
 * outputs to the log file instead of the standard output.
 * Also:
 *	adds a timestamp,
 *	prints the module name in front of the message,
 *	has some other formatting types (or will sometime),
 *	adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by /etc/syslogd.
 *
 * Author: Eric Allman
 * Modified to use UNIX domain IPC by Ralph Campbell
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <syslog.h>
#include <netdb.h>

#define	MAXLINE	1024			/* max message size */
#define NULL	0			/* manifest */

static char	logname[] = "/dev/log";
static char	ctty[] = "/dev/console";

static int	LogFile = -1;		/* fd for log */
static int	LogStat	= 0;		/* status bits, set by openlog() */
static char	*LogTag = NULL;		/* string to tag the entry with */
static int	LogMask = 0xffffffff;	/* mask of priorities to be logged */

static struct sockaddr SyslogAddr;

extern	int errno, sys_nerr;
extern	char *sys_errlist[];

syslog(pri, fmt, p0, p1, p2, p3, p4)
	int pri;
	char *fmt;
{
	char buf[MAXLINE + 1], outline[MAXLINE + 1];
	register char *b, *f, *o;
	register int c;
	long now;
	int pid, olderrno = errno;

	/* see if we should just throw out this message */
	if (pri <= 0 || pri >= 32 || ((1 << pri) & LogMask) == 0)
		return;
	if (LogFile < 0)
		openlog(NULL, 0, 0);
	o = outline;
	sprintf(o, "<%d>", pri);
	o += strlen(o);
	if (LogTag) {
		strcpy(o, LogTag);
		o += strlen(o);
	}
	if (LogStat & LOG_PID) {
		sprintf(o, "[%d]", getpid());
		o += strlen(o);
	}
	time(&now);
	sprintf(o, ": %.15s-- ", ctime(&now) + 4);
	o += strlen(o);

	b = buf;
	f = fmt;
	while ((c = *f++) != '\0' && c != '\n' && b < &buf[MAXLINE]) {
		if (c != '%') {
			*b++ = c;
			continue;
		}
		if ((c = *f++) != 'm') {
			*b++ = '%';
			*b++ = c;
			continue;
		}
		if ((unsigned)olderrno > sys_nerr)
			sprintf(b, "error %d", olderrno);
		else
			strcpy(b, sys_errlist[olderrno]);
		b += strlen(b);
	}
	*b++ = '\n';
	*b = '\0';
	sprintf(o, buf, p0, p1, p2, p3, p4);
	c = strlen(outline);
	if (c > MAXLINE)
		c = MAXLINE;
	if (sendto(LogFile, outline, c, 0, &SyslogAddr, sizeof SyslogAddr) >= 0)
		return;
	if (pri > LOG_CRIT)
		return;
	pid = fork();
	if (pid == -1)
		return;
	if (pid == 0) {
		LogFile = open(ctty, O_RDWR);
		write(LogFile, outline, c);
		close(LogFile);
		exit(0);
	}
	while ((c = wait((int *)0)) > 0 && c != pid)
		;
}

/*
 * OPENLOG -- open system log
 */
openlog(ident, logstat, logmask)
	char *ident;
	int logstat, logmask;
{

	LogTag = (ident != NULL) ? ident : "syslog";
	LogStat = logstat;
	if (logmask != 0)
		LogMask = logmask;
	if (LogFile >= 0)
		return;
	SyslogAddr.sa_family = AF_UNIX;
	strncpy(SyslogAddr.sa_data, logname, sizeof SyslogAddr.sa_data);
	LogFile = socket(AF_UNIX, SOCK_DGRAM, 0);
	fcntl(LogFile, F_SETFD, 1);
}

/*
 * CLOSELOG -- close the system log
 */
closelog()
{

	(void) close(LogFile);
	LogFile = -1;
}

/*
 * SETLOGMASK -- set the log mask level
 */
setlogmask(pmask)
	int pmask;
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}
