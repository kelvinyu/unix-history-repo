/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#include <errno.h>
#include "sendmail.h"
# include <sys/mx.h>

#ifndef lint
#ifdef DAEMON
static char sccsid[] = "@(#)daemon.c	5.42 (Berkeley) %G% (with daemon mode)";
#else
static char sccsid[] = "@(#)daemon.c	5.42 (Berkeley) %G% (without daemon mode)";
#endif
#endif /* not lint */

#ifdef DAEMON

# include <netdb.h>
# include <sys/signal.h>
# include <sys/wait.h>
# include <sys/time.h>
# include <sys/resource.h>

/*
**  DAEMON.C -- routines to use when running as a daemon.
**
**	This entire file is highly dependent on the 4.2 BSD
**	interprocess communication primitives.  No attempt has
**	been made to make this file portable to Version 7,
**	Version 6, MPX files, etc.  If you should try such a
**	thing yourself, I recommend chucking the entire file
**	and starting from scratch.  Basic semantics are:
**
**	getrequests()
**		Opens a port and initiates a connection.
**		Returns in a child.  Must set InChannel and
**		OutChannel appropriately.
**	clrdaemon()
**		Close any open files associated with getting
**		the connection; this is used when running the queue,
**		etc., to avoid having extra file descriptors during
**		the queue run and to avoid confusing the network
**		code (if it cares).
**	makeconnection(host, port, outfile, infile, usesecureport)
**		Make a connection to the named host on the given
**		port.  Set *outfile and *infile to the files
**		appropriate for communication.  Returns zero on
**		success, else an exit status describing the
**		error.
**	maphostname(hbuf, hbufsize)
**		Convert the entry in hbuf into a canonical form.  It
**		may not be larger than hbufsize.
*/

static FILE	*MailPort;	/* port that mail comes in on */
/*
**  GETREQUESTS -- open mail IPC port and get requests.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Waits until some interesting activity occurs.  When
**		it does, a child is created to process it, and the
**		parent waits for completion.  Return from this
**		routine is always in the child.  The file pointers
**		"InChannel" and "OutChannel" should be set to point
**		to the communication channel.
*/

int	DaemonSocket	= -1;		/* fd describing socket */

getrequests()
{
	int t;
	register struct servent *sp;
	int on = 1;
	struct sockaddr_in srvraddr;
	extern void reapchild();

	/*
	**  Set up the address for the mailer.
	*/

	sp = getservbyname("smtp", "tcp");
	if (sp == NULL)
	{
		syserr("server \"smtp\" unknown");
		goto severe;
	}
	srvraddr.sin_family = AF_INET;
	srvraddr.sin_addr.s_addr = INADDR_ANY;
	srvraddr.sin_port = sp->s_port;

	/*
	**  Try to actually open the connection.
	*/

	if (tTd(15, 1))
		printf("getrequests: port 0x%x\n", srvraddr.sin_port);

	/* get a socket for the SMTP connection */
	DaemonSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (DaemonSocket < 0)
	{
		/* probably another daemon already */
		syserr("getrequests: can't create socket");
	  severe:
# ifdef LOG
		if (LogLevel > 0)
# endif LOG
		finis();
	}

	/* turn on network debugging? */
	if (tTd(15, 15))
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);

	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

	if (bind(DaemonSocket, (struct sockaddr *)&srvraddr, sizeof srvraddr) < 0)
	{
		syserr("getrequests: cannot bind");
		(void) close(DaemonSocket);
		goto severe;
	}
	if (listen(DaemonSocket, 10) < 0)
	{
		syserr("getrequests: cannot listen");
		(void) close(DaemonSocket);
		goto severe;
	}

	(void) signal(SIGCHLD, reapchild);

	if (tTd(15, 1))
		printf("getrequests: %d\n", DaemonSocket);

	struct wh wbuf;

	wbuf.index = index;
	wbuf.count = 0;
	wbuf.ccount = cnt;
	wbuf.data = buf;
	write(MailPort, &wbuf, sizeof wbuf);
}
/*
**  MAKECONNECTION -- make a connection to an SMTP socket on another machine.
**
**	Parameters:
**		host -- the name of the host.
**		port -- the port number to connect to.
**		outfile -- a pointer to a place to put the outfile
**			descriptor.
**		infile -- ditto for infile.
**		usesecureport -- if set, use a low numbered (reserved)
**			port to provide some rudimentary authentication.
**
**	Returns:
**		An exit code telling whether the connection could be
**			made and if not why not.
**
**	Side Effects:
**		none.
*/

makeconnection(host, port, outfile, infile, usesecureport)
	char *host;
	u_short port;
	FILE **outfile;
	FILE **infile;
	bool usesecureport;
{
	register int i, s;
	register struct hostent *hp = (struct hostent *)NULL;
	struct sockaddr_in addr;
	int sav_errno;
	extern char *inet_ntoa();
#ifdef NAMED_BIND
	extern int h_errno;
#endif

	/*
	**  Set up the address for the mailer.
	**	Accept "[a.b.c.d]" syntax for host name.
	*/

#ifdef NAMED_BIND
	h_errno = 0;
#endif
	errno = 0;

	if (host[0] == '[')
	{
		long hid;
		register char *p = index(host, ']');

		if (p != NULL)
		{
			*p = '\0';
			hid = inet_addr(&host[1]);
			*p = ']';
		}
		if (p == NULL || hid == -1)
		{
			usrerr("Invalid numeric domain spec \"%s\"", host);
			return (EX_NOHOST);
		}
		addr.sin_addr.s_addr = hid;
	}
	else
	{
		hp = gethostbyname(host);
		if (hp == NULL)
		{
#ifdef NAMED_BIND
			if (errno == ETIMEDOUT || h_errno == TRY_AGAIN)
				return (EX_TEMPFAIL);

			/* if name server is specified, assume temp fail */
			if (errno == ECONNREFUSED && UseNameServer)
				return (EX_TEMPFAIL);
#endif

			/*
			**  XXX Should look for mail forwarder record here
			**  XXX if (h_errno == NO_ADDRESS).
			*/

			return (EX_NOHOST);
		}
		bcopy(hp->h_addr, (char *) &addr.sin_addr, hp->h_length);
		i = 1;
	}

	/*
	**  Determine the port number.
	*/

	if (port != 0)
		addr.sin_port = htons(port);
	else
	{
		register struct servent *sp = getservbyname("smtp", "tcp");

		if (sp == NULL)
		{
			syserr("makeconnection: server \"smtp\" unknown");
			return (EX_OSFILE);
		}
		addr.sin_port = sp->s_port;
	}

	/*
	**  Try to actually open the connection.
	*/

again:
	if (tTd(16, 1))
		printf("makeconnection (%s [%s])\n", host,
		    inet_ntoa(addr.sin_addr));

#ifdef NVMUNIX
	s = socket(AF_INET, SOCK_STREAM, 0, 0);
#else NVMUNIX
	if (usesecureport)
	{
		int rport = IPPORT_RESERVED - 1;

		s = rresvport(&rport);
	}
	else
	{
		s = socket(AF_INET, SOCK_STREAM, 0);
	}
#endif NVMUNIX
	if (s < 0)
	{
		sav_errno = errno;
		syserr("makeconnection: no socket");
		goto failure;
	}

	if (tTd(16, 1))
		printf("makeconnection: %d\n", s);

	/* turn on network debugging? */
	if (tTd(16, 14))
	{
		int on = 1;
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);
	}
	if (CurEnv->e_xfp != NULL)
		(void) fflush(CurEnv->e_xfp);		/* for debugging */
	errno = 0;					/* for debugging */
#ifdef NVMUNIX
	bind(s, &SendmailAddress, sizeof SendmailAddress, 0);
	if (connect(s, &SendmailAddress, sizeof SendmailAddress, 0) < 0)
#else NVMUNIX
	addr.sin_family = AF_INET;
	if (connect(s, (struct sockaddr *) &addr, sizeof addr) < 0)
#endif NVMUNIX
	{
		sav_errno = errno;
		(void) close(s);
		if (hp && hp->h_addr_list[i])
		{
			bcopy(hp->h_addr_list[i++], (char *) &addr.sin_addr,
					hp->h_length);
			goto again;
		}

		/* failure, decide if temporary or not */
	failure:
		switch (sav_errno)
		{
		  case EISCONN:
		  case ETIMEDOUT:
		  case EINPROGRESS:
		  case EALREADY:
		  case EADDRINUSE:
		  case EHOSTDOWN:
		  case ENETDOWN:
		  case ENETRESET:
		  case ENOBUFS:
		  case ECONNREFUSED:
		  case ECONNRESET:
		  case EHOSTUNREACH:
		  case ENETUNREACH:
#ifdef ENOSR
		  case ENOSR:
#endif
			/* there are others, I'm sure..... */
			return (EX_TEMPFAIL);

		  case EPERM:
			/* why is this happening? */
			syserr("makeconnection: funny failure, addr=%lx, port=%x",
				addr.sin_addr.s_addr, addr.sin_port);
			return (EX_TEMPFAIL);

		  default:
			{
				extern char *errstring();

				message(Arpa_Info, "%s", errstring(sav_errno));
				return (EX_UNAVAILABLE);
			}
		}
	}

	/* connection ok, put it into canonical form */
	*outfile = fdopen(s, "w");
	*infile = fdopen(dup(s), "r");

	return (EX_OK);
}
/*
**  MYHOSTNAME -- return the name of this host.
**
**	Parameters:
**		hostbuf -- a place to return the name of this host.
**		size -- the size of hostbuf.
**
**	Returns:
**		A list of aliases for this host.
**
**	Side Effects:
**		none.
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	extern struct hostent *gethostbyname();
	struct hostent *hp;

	if (gethostname(hostbuf, size) < 0)
	{
		(void) strcpy(hostbuf, "localhost");
	}
	hp = gethostbyname(hostbuf);
	if (hp != NULL)
	{
		(void) strcpy(hostbuf, hp->h_name);
		return (hp->h_aliases);
	}
	else
		return (NULL);
}
/*
 *  MAPHOSTNAME -- turn a hostname into canonical form
 *
 *	Parameters:
 *		hbuf -- a buffer containing a hostname.
 *		hbsize -- the size of hbuf.
 *
 *	Returns:
 *		TRUE if the host name was mapped.
 *		FALSE otherwise.
 *
 *	Side Effects:
 *		Looks up the host specified in hbuf.  If it is not
 *		the canonical name for that host, replace it with
 *		the canonical name.  If the name is unknown, or it
 *		is already the canonical name, leave it unchanged.
 */

bool
maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	register struct hostent *hp;
	u_long in_addr;
	char ptr[256], *cp;
	struct hostent *gethostbyaddr();

	/*
	 * If first character is a bracket, then it is an address
	 * lookup.  Address is copied into a temporary buffer to
	 * strip the brackets and to preserve hbuf if address is
	 * unknown.
	 */
	if (*hbuf != '[')
		return (getcanonname(hbuf, hbsize));
	if ((cp = index(strcpy(ptr, hbuf), ']')) == NULL)
		return (FALSE);
	*cp = '\0';
	in_addr = inet_addr(&ptr[1]);
	hp = gethostbyaddr((char *)&in_addr, sizeof(struct in_addr), AF_INET);
	if (hp == NULL)
		return (FALSE);
	if (strlen(hp->h_name) >= hbsize)
		hp->h_name[hbsize - 1] = '\0';
	(void)strcpy(hbuf, hp->h_name);
	return (TRUE);
}

# else DAEMON
/* code for systems without sophisticated networking */

/*
**  MYHOSTNAME -- stub version for case of no daemon code.
**
**	Can't convert to upper case here because might be a UUCP name.
**
**	Mark, you can change this to be anything you want......
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	register FILE *f;

	hostbuf[0] = '\0';
	f = fopen("/usr/include/whoami", "r");
	if (f != NULL)
	{
		(void) fgets(hostbuf, size, f);
		fixcrlf(hostbuf, TRUE);
		(void) fclose(f);
	}
	return (NULL);
}
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**
**	Returns:
**		TRUE if the hostname was mapped.
**		FALSE otherwise.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/

/*ARGSUSED*/
bool
maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	return (FALSE);
}

#endif DAEMON
