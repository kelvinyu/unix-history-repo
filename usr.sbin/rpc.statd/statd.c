/*
 * Copyright (c) 1995
 *	A.R. Gordon (andrew.gordon@net-tel.co.uk).  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed for the FreeBSD project
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREW GORDON AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef lint
static const char rcsid[] =
	"$Id: statd.c,v 1.3 1997/10/13 11:13:30 charnier Exp $";
#endif /* not lint */

/* main() function for status monitor daemon.  Some of the code in this	*/
/* file was generated by running rpcgen /usr/include/rpcsvc/sm_inter.x	*/
/* The actual program logic is in the file procs.c			*/

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "statd.h"

int debug = 0;		/* Controls syslog() calls for debug messages	*/

extern void sm_prog_1(struct svc_req *rqstp, SVCXPRT *transp);
static void handle_sigchld();
static void usage __P((void));

int
main(int argc, char **argv)
{
  SVCXPRT *transp;
  struct sigaction sa;

  if (argc > 1)
  {
    if (strcmp(argv[1], "-d"))
	usage();
    debug = 1;
  }

  (void)pmap_unset(SM_PROG, SM_VERS);

  transp = svcudp_create(RPC_ANYSOCK);
  if (transp == NULL)
    errx(1, "cannot create udp service");
  if (!svc_register(transp, SM_PROG, SM_VERS, sm_prog_1, IPPROTO_UDP))
    errx(1, "unable to register (SM_PROG, SM_VERS, udp)");

  transp = svctcp_create(RPC_ANYSOCK, 0, 0);
  if (transp == NULL)
    errx(1, "cannot create tcp service");
  if (!svc_register(transp, SM_PROG, SM_VERS, sm_prog_1, IPPROTO_TCP))
    errx(1, "unable to register (SM_PROG, SM_VERS, tcp)");
  init_file("/var/db/statd.status");

  /* Note that it is NOT sensible to run this program from inetd - the 	*/
  /* protocol assumes that it will run immediately at boot time.	*/
  daemon(0, 0);
  openlog("rpc.statd", 0, LOG_DAEMON);
  if (debug) syslog(LOG_INFO, "Starting - debug enabled");
  else syslog(LOG_INFO, "Starting");

  /* Install signal handler to collect exit status of child processes	*/
  sa.sa_handler = handle_sigchld;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGCHLD);
  sa.sa_flags = SA_RESTART;
  sigaction(SIGCHLD, &sa, NULL);

  /* Initialisation now complete - start operating			*/
  notify_hosts();	/* Forks a process (if necessary) to do the	*/
			/* SM_NOTIFY calls, which may be slow.		*/

  svc_run();	/* Should never return					*/
  exit(1);
}

static void
usage()
{
      fprintf(stderr, "usage: rpc.statd [-d]\n");
      exit(1);
}

/* handle_sigchld ---------------------------------------------------------- */
/*
   Purpose:	Catch SIGCHLD and collect process status
   Retruns:	Nothing.
   Notes:	No special action required, other than to collect the
		process status and hence allow the child to die:
		we only use child processes for asynchronous transmission
		of SM_NOTIFY to other systems, so it is normal for the
		children to exit when they have done their work.
*/

static void handle_sigchld(int sig, int code, struct sigcontext *scp)
{
  int pid, status;
  pid = wait4(-1, &status, WNOHANG, (struct rusage*)0);
  if (!pid) syslog(LOG_ERR, "Phantom SIGCHLD??");
  else if (status == 0)
  {
    if (debug) syslog(LOG_DEBUG, "Child %d exited OK", pid);
  }
  else syslog(LOG_ERR, "Child %d failed with status %d", pid,
    WEXITSTATUS(status));
}

