/*
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1992, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mount_portal.c	8.2 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/syslog.h>
#include <sys/mount.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mntopts.h"
#include "pathnames.h"
#include "portald.h"

struct mntopt mopts[] = {
	MOPT_STDOPTS,
	{ NULL }
};

static void usage __P((void));

static sig_atomic_t readcf;	/* Set when SIGHUP received */

static void sigchld(sig)
int sig;
{
	pid_t pid;

	while ((pid = waitpid((pid_t) -1, (int *) 0, WNOHANG)) > 0)
		;
	if (pid < 0)
		syslog(LOG_WARNING, "waitpid: %s", strerror(errno));
}

int
main(c, argv)
	int argc;
	char *argv[];
{
	struct portal_args args;
	struct sockaddr_un un;
	char *conf;
	char *mountpt;
	int mntflags = 0;
	char tag[32];

	qelem q;
	int rc;
	int so;
	int error = 0;

	/*
	 * Crack command line args
	 */
	int ch;

	while ((ch = getopt(argc, argv, "o:")) != EOF) {
		switch (ch) {
		case 'o':
			getmntopts(optarg, mopts, &mntflags);
			break;
		default:
			error = 1;
			break;
		}
	}

	if (optind != (argc - 2))
		error = 1;

	if (error)
		usage();

	/*
	 * Get config file and mount point
	 */
	conf = argv[optind];
	mountpt = argv[optind+1];

	/*
	 * Construct the listening socket
	 */
	un.sun_family = AF_UNIX;
	if (sizeof(_PATH_TMPPORTAL) >= sizeof(un.sun_path)) {
		fprintf(stderr, "mount_portal: portal socket name too long\n");
		exit(1);
	}
	strcpy(un.sun_path, _PATH_TMPPORTAL);
	mktemp(un.sun_path);
	un.sun_len = strlen(un.sun_path);

	so = socket(AF_UNIX, SOCK_STREAM, 0);
	if (so < 0) {
		fprintf(stderr, "mount_portal: socket: %s\n", strerror(errno));
		exit(1);
	}
	(void) unlink(un.sun_path);
	if (bind(so, (struct sockaddr *) &un, sizeof(un)) < 0)
		err(1, NULL);
	(void) unlink(un.sun_path);

	(void) listen(so, 5);

	args.pa_socket = so;
	sprintf(tag, "portal:%d", getpid());
	args.pa_config = tag;

	rc = mount(MOUNT_PORTAL, mountpt, mntflags, &args);
	if (rc < 0)
		err(1, NULL);

#ifdef notdef
	/*
	 * Everything is ready to go - now is a good time to fork
	 */
	daemon(0, 0);
#endif

	/*
	 * Start logging (and change name)
	 */
	openlog("portald", LOG_CONS|LOG_PID, LOG_DAEMON);

	q.q_forw = q.q_back = &q;
	readcf = 1;

	signal(SIGCHLD, sigchld);

	/*
	 * Just loop waiting for new connections and activating them
	 */
	for (;;) {
		struct sockaddr_un un2;
		int len2 = sizeof(un2);
		int so2;
		pid_t pid;
		fd_set fdset;
		int rc;

		/*
		 * Check whether we need to re-read the configuration file
		 */
		if (readcf) {
			readcf = 0;
			conf_read(&q, conf);
			continue;
		}
	
		/*
		 * Accept a new connection
		 * Will get EINTR if a signal has arrived, so just
		 * ignore that error code
		 */
		FD_SET(so, &fdset);
		rc = select(so+1, &fdset, (void *) 0, (void *) 0, (void *) 0);
		if (rc < 0) {
			if (errno == EINTR)
				continue;
			syslog(LOG_ERR, "select: %s", strerror(errno));
			exit(1);
		}
		if (rc == 0)
			break;
		so2 = accept(so, (struct sockaddr *) &un2, &len2);
		if (so2 < 0) {
			/*
			 * The unmount function does a shutdown on the socket
			 * which will generated ECONNABORTED on the accept.
			 */
			if (errno == ECONNABORTED)
				break;
			if (errno != EINTR) {
				syslog(LOG_ERR, "accept: %s", strerror(errno));
				exit(1);
			}
			continue;
		}

		/*
		 * Now fork a new child to deal with the connection
		 */
#ifdef notdef
	eagain:;
		switch (pid = fork()) {
		case -1:
			if (errno == EAGAIN) {
				sleep(1);
				goto eagain;
			}
			syslog(LOG_ERR, "fork: %s", strerror(errno));
			break;
		case 0:
			(void) close(so);
			activate(&q, so2);
			break;
		default:
			(void) close(so2);
			break;
		}
#endif
		activate(&q, so2);
	}
	syslog(LOG_INFO, "%s unmounted", mountpt);
	exit(0);
}

static void
usage()
{
	(void)fprintf(stderr,
		"usage: mount_portal [-o options] config mount-point\n");
	exit(1);
}
