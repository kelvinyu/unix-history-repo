/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#include <errno.h>
#include "sendmail.h"
# include <sys/mx.h>

#ifndef lint
#ifdef DAEMON
static char sccsid[] = "@(#)daemon.c	8.76 (Berkeley) %G% (with daemon mode)";
#else
static char sccsid[] = "@(#)daemon.c	8.76 (Berkeley) %G% (without daemon mode)";
#endif
#endif /* not lint */

#ifdef DAEMON

# include <arpa/inet.h>

#if NAMED_BIND
# include <resolv.h>
#endif

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
**	host_map_lookup(map, hbuf, avp, pstat)
**		Convert the entry in hbuf into a canonical form.
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

int		DaemonSocket	= -1;		/* fd describing socket */
SOCKADDR	DaemonAddr;			/* socket for incoming */
int		ListenQueueSize = 10;		/* size of listen queue */
int		TcpRcvBufferSize = 0;		/* size of TCP receive buffer */
int		TcpSndBufferSize = 0;		/* size of TCP send buffer */

void
getrequests()
{
	int t;
	bool refusingconnections = TRUE;
	FILE *pidf;
	int socksize;
#ifdef XDEBUG
	bool j_has_dot;
#endif
	extern void reapchild();

	/*
	**  Set up the address for the mailer.
	*/

	if (DaemonAddr.sin.sin_family == 0)
		DaemonAddr.sin.sin_family = AF_INET;
	if (DaemonAddr.sin.sin_addr.s_addr == 0)
		DaemonAddr.sin.sin_addr.s_addr = INADDR_ANY;
	if (DaemonAddr.sin.sin_port == 0)
	{
		register struct servent *sp;

		sp = getservbyname("smtp", "tcp");
		if (sp == NULL)
		{
			syserr("554 service \"smtp\" unknown");
			DaemonAddr.sin.sin_port = htons(25);
		}
		else
			DaemonAddr.sin.sin_port = sp->s_port;
	}

	/*
	**  Try to actually open the connection.
	*/

	if (tTd(15, 1))
		printf("getrequests: port 0x%x\n", DaemonAddr.sin.sin_port);

	/* get a socket for the SMTP connection */
	socksize = opendaemonsocket(TRUE);

	(void) setsignal(SIGCHLD, reapchild);

	/* write the pid to the log file for posterity */
	pidf = fopen(PidFile, "w");
	if (pidf != NULL)
	{
		extern char *CommandLineArgs;

		/* write the process id on line 1 */
		fprintf(pidf, "%d\n", getpid());

		/* line 2 contains all command line flags */
		fprintf(pidf, "%s\n", CommandLineArgs);

		/* flush and close */
		fclose(pidf);
	}

#ifdef XDEBUG
	{
		char jbuf[MAXHOSTNAMELEN];

		expand("\201j", jbuf, sizeof jbuf, CurEnv);
		j_has_dot = strchr(jbuf, '.') != NULL;
	}
#endif

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
**		mci -- a pointer to the mail connection information
**			structure to be filled in.
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

SOCKADDR	CurHostAddr;		/* address of current host */

int
makeconnection(host, port, mci, usesecureport)
	char *host;
	u_short port;
	register MCI *mci;
	bool usesecureport;
{
	register int i = 0;
	register int s;
	register struct hostent *hp = (struct hostent *)NULL;
	SOCKADDR addr;
	int sav_errno;
	int addrlen;
	bool firstconnect;
#if NAMED_BIND
	extern int h_errno;
#endif

	/*
	**  Set up the address for the mailer.
	**	Accept "[a.b.c.d]" syntax for host name.
	*/

#if NAMED_BIND
	h_errno = 0;
#endif
	errno = 0;
	bzero(&CurHostAddr, sizeof CurHostAddr);
	SmtpPhase = mci->mci_phase = "initial connection";
	CurHostName = host;

	if (host[0] == '[')
	{
		long hid;
		register char *p = strchr(host, ']');

		if (p != NULL)
		{
			*p = '\0';
#ifdef NETINET
			hid = inet_addr(&host[1]);
			if (hid == -1)
#endif
			{
				/* try it as a host name (avoid MX lookup) */
				hp = sm_gethostbyname(&host[1]);
				if (hp == NULL && p[-1] == '.')
				{
#if NAMED_BIND
					int oldopts = _res.options;

					_res.options &= ~(RES_DEFNAMES|RES_DNSRCH);
#endif
					p[-1] = '\0';
					hp = sm_gethostbyname(&host[1]);
					p[-1] = '.';
#if NAMED_BIND
					_res.options = oldopts;
#endif
				}
				*p = ']';
				goto gothostent;
			}
			*p = ']';
		}
		if (p == NULL)
		{
			usrerr("553 Invalid numeric domain spec \"%s\"", host);
			return (EX_NOHOST);
		}
#ifdef NETINET
		addr.sin.sin_family = AF_INET;		/*XXX*/
		addr.sin.sin_addr.s_addr = hid;
#endif
	}
	else
	{
		register char *p = &host[strlen(host) - 1];

		hp = sm_gethostbyname(host);
		if (hp == NULL && *p == '.')
		{
#if NAMED_BIND
			int oldopts = _res.options;

			_res.options &= ~(RES_DEFNAMES|RES_DNSRCH);
#endif
			*p = '\0';
			hp = sm_gethostbyname(host);
			*p = '.';
#if NAMED_BIND
			_res.options = oldopts;
#endif
		}
gothostent:
		if (hp == NULL)
		{
#if NAMED_BIND
			/* check for name server timeouts */
			if (errno == ETIMEDOUT || h_errno == TRY_AGAIN ||
			    (errno == ECONNREFUSED && UseNameServer))
			{
				mci->mci_status = "4.4.3";
				return (EX_TEMPFAIL);
			}
#endif
			return (EX_NOHOST);
		}
		addr.sa.sa_family = hp->h_addrtype;
		switch (hp->h_addrtype)
		{
#ifdef NETINET
		  case AF_INET:
			bcopy(hp->h_addr,
				&addr.sin.sin_addr,
				INADDRSZ);
			break;
#endif

		  default:
			bcopy(hp->h_addr,
				addr.sa.sa_data,
				hp->h_length);
			break;
		}
		i = 1;
	}

	/*
	**  Determine the port number.
	*/

	if (port != 0)
		port = htons(port);
	else
	{
		register struct servent *sp = getservbyname("smtp", "tcp");

		if (sp == NULL)
		{
			syserr("554 makeconnection: service \"smtp\" unknown");
			port = htons(25);
		}
		else
			port = sp->s_port;
	}

	switch (addr.sa.sa_family)
	{
#ifdef NETINET
	  case AF_INET:
		addr.sin.sin_port = port;
		addrlen = sizeof (struct sockaddr_in);
		break;
#endif

#ifdef NETISO
	  case AF_ISO:
		/* assume two byte transport selector */
		bcopy((char *) &port, TSEL((struct sockaddr_iso *) &addr), 2);
		addrlen = sizeof (struct sockaddr_iso);
		break;
#endif

	  default:
		syserr("Can't connect to address family %d", addr.sa.sa_family);
		return (EX_NOHOST);
	}

	/*
	**  Try to actually open the connection.
	*/

#ifdef XLA
	/* if too many connections, don't bother trying */
	if (!xla_noqueue_ok(host))
		return EX_TEMPFAIL;
#endif

	firstconnect = TRUE;
	for (;;)
	{
		if (tTd(16, 1))
			printf("makeconnection (%s [%s])\n",
				host, anynet_ntoa(&addr));

		/* save for logging */
		CurHostAddr = addr;

		if (usesecureport)
		{
			int rport = IPPORT_RESERVED - 1;

			s = rresvport(&rport);
		}
		else
		{
			s = socket(AF_INET, SOCK_STREAM, 0);
		}
		if (s < 0)
		{
			sav_errno = errno;
			syserr("makeconnection: no socket");
			goto failure;
		}

#ifdef SO_SNDBUF
		if (TcpSndBufferSize > 0)
		{
			if (setsockopt(s, SOL_SOCKET, SO_SNDBUF,
				       (char *) &TcpSndBufferSize,
				       sizeof(TcpSndBufferSize)) < 0)
				syserr("makeconnection: setsockopt(SO_SNDBUF)");
		}
#endif

		if (tTd(16, 1))
			printf("makeconnection: fd=%d\n", s);

		/* turn on network debugging? */
		if (tTd(16, 101))
		{
			int on = 1;
			(void) setsockopt(s, SOL_SOCKET, SO_DEBUG,
					  (char *)&on, sizeof on);
		}
		if (CurEnv->e_xfp != NULL)
			(void) fflush(CurEnv->e_xfp);		/* for debugging */
		errno = 0;					/* for debugging */
		if (connect(s, (struct sockaddr *) &addr, addrlen) >= 0)
			break;

		/* if running demand-dialed connection, try again */
		if (DialDelay > 0 && firstconnect)
		{
			if (tTd(16, 1))
				printf("Connect failed (%s); trying again...\n",
					errstring(sav_errno));
			firstconnect = FALSE;
			sleep(DialDelay);
			continue;
		}

		/* couldn't connect.... figure out why */
		sav_errno = errno;
		(void) close(s);
		if (hp != NULL && hp->h_addr_list[i])
		{
			if (tTd(16, 1))
				printf("Connect failed (%s); trying new address....\n",
					errstring(sav_errno));
			switch (addr.sa.sa_family)
			{
#ifdef NETINET
			  case AF_INET:
				bcopy(hp->h_addr_list[i++],
				      &addr.sin.sin_addr,
				      INADDRSZ);
				break;
#endif

			  default:
				bcopy(hp->h_addr_list[i++],
					addr.sa.sa_data,
					hp->h_length);
				break;
			}
			continue;
		}

		/* failure, decide if temporary or not */
	failure:
#ifdef XLA
		xla_host_end(host);
#endif
		if (transienterror(sav_errno))
			return EX_TEMPFAIL;
		else
		{
			message("%s", errstring(sav_errno));
			return (EX_UNAVAILABLE);
		}
	}

	/* connection ok, put it into canonical form */
	if ((mci->mci_out = fdopen(s, "w")) == NULL ||
	    (s = dup(s)) < 0 ||
	    (mci->mci_in = fdopen(s, "r")) == NULL)
	{
		syserr("cannot open SMTP client channel, fd=%d", s);
		return EX_TEMPFAIL;
	}

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
**		Adds numeric codes to $=w.
*/

struct hostent *
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	register struct hostent *hp;
	extern bool getcanonname();
	extern int h_errno;

	if (gethostname(hostbuf, size) < 0)
	{
		(void) strcpy(hostbuf, "localhost");
	}
	hp = sm_gethostbyname(hostbuf);
	if (hp == NULL)
		return NULL;
	if (strchr(hp->h_name, '.') != NULL || strchr(hostbuf, '.') == NULL)
	{
		(void) strncpy(hostbuf, hp->h_name, size - 1);
		hostbuf[size - 1] = '\0';
	}

#if NAMED_BIND
	/*
	**  If still no dot, try DNS directly (i.e., avoid NIS problems).
	**  This ought to be driven from the configuration file, but
	**  we are called before the configuration is read.  We could
	**  check for an /etc/resolv.conf file, but that isn't required.
	**  All in all, a bit of a mess.
	*/

	if (strchr(hostbuf, '.') == NULL &&
	    !getcanonname(hostbuf, size, TRUE) &&
	    h_errno == TRY_AGAIN)
	{
		/* try twice in case name server not yet started up */
		message("My unqualifed host name (%s) unknown to DNS; sleeping for retry",
			hostbuf);
		sleep(60);
		if (!getcanonname(hostbuf, size, TRUE))
			errno = h_errno + E_DNSBASE;
	}
#endif
	return (hp);
}
/*
**  GETAUTHINFO -- get the real host name asociated with a file descriptor
**
**	Uses RFC1413 protocol to try to get info from the other end.
**
**	Parameters:
**		fd -- the descriptor
**
**	Returns:
**		The user@host information associated with this descriptor.
*/

static jmp_buf	CtxAuthTimeout;

static void
authtimeout()
{
	longjmp(CtxAuthTimeout, 1);
}

char *
getauthinfo(fd)
	int fd;
{
	int falen;
	register char *p;
	SOCKADDR la;
	int lalen;
	register struct servent *sp;
	int s;
	int i;
	EVENT *ev;
	int nleft;
	char ibuf[MAXNAME + 1];
	char ibuf[MAXNAME + 1];
	static char hbuf[MAXNAME * 2 + 2];
	extern char *hostnamebyanyaddr();
	extern char RealUserName[];			/* main.c */

	falen = sizeof RealHostAddr;
	if (isatty(fd) || getpeername(fd, &RealHostAddr.sa, &falen) < 0 ||
	    falen <= 0 || RealHostAddr.sa.sa_family == 0)
	{
		(void) sprintf(hbuf, "%s@localhost", RealUserName);
		if (tTd(9, 1))
			printf("getauthinfo: %s\n", hbuf);
		return hbuf;
	}

	if (RealHostName == NULL)
	{
		/* translate that to a host name */
		RealHostName = newstr(hostnamebyanyaddr(&RealHostAddr));
	}

	if (TimeOuts.to_ident == 0)
		goto noident;

	lalen = sizeof la;
	if (RealHostAddr.sa.sa_family != AF_INET ||
	    getsockname(fd, &la.sa, &lalen) < 0 || lalen <= 0 ||
	    la.sa.sa_family != AF_INET)
	{
		/* no ident info */
		goto noident;
	}

	/* create ident query */
	(void) sprintf(ibuf, "%d,%d\r\n",
		ntohs(RealHostAddr.sin.sin_port), ntohs(la.sin.sin_port));

	/* create local address */
	la.sin.sin_port = 0;

	/* create foreign address */
	sp = getservbyname("auth", "tcp");
	if (sp != NULL)
		RealHostAddr.sin.sin_port = sp->s_port;
	else
		RealHostAddr.sin.sin_port = htons(113);

	s = -1;
	if (setjmp(CtxAuthTimeout) != 0)
	{
		if (s >= 0)
			(void) close(s);
		goto noident;
	}

	/* put a timeout around the whole thing */
	ev = setevent(TimeOuts.to_ident, authtimeout, 0);

	/* connect to foreign IDENT server using same address as SMTP socket */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		clrevent(ev);
		goto noident;
	}
	if (bind(s, &la.sa, sizeof la.sin) < 0 ||
	    connect(s, &RealHostAddr.sa, sizeof RealHostAddr.sin) < 0)
	{
		goto closeident;
	}

	if (tTd(9, 10))
		printf("getauthinfo: sent %s", ibuf);

	/* send query */
	if (write(s, ibuf, strlen(ibuf)) < 0)
		goto closeident;

	/* get result */
	p = &ibuf[0];
	nleft = sizeof ibuf - 1;
	while ((i = read(s, p, nleft)) > 0)
	{
		p += i;
		nleft -= i;
	}
	(void) close(s);
	clrevent(ev);
	if (i < 0 || p == &ibuf[0])
		goto noident;

	if (*--p == '\n' && *--p == '\r')
		p--;
	*++p = '\0';

	if (tTd(9, 3))
		printf("getauthinfo:  got %s\n", ibuf);

	/* parse result */
	p = strchr(ibuf, ':');
	if (p == NULL)
	{
		/* malformed response */
		goto noident;
	}
	while (isascii(*++p) && isspace(*p))
		continue;
	if (strncasecmp(p, "userid", 6) != 0)
	{
		/* presumably an error string */
		goto noident;
	}
	p += 6;
	while (isascii(*p) && isspace(*p))
		p++;
	if (*p++ != ':')
	{
		/* either useridxx or malformed response */
		goto noident;
	}

	/* p now points to the OSTYPE field */
	while (isascii(*p) && isspace(*p))
		p++;
	if (strncasecmp(p, "other", 5) == 0 &&
	    (p[5] == ':' || p[5] == ' ' || p[5] == ',' || p[5] == '\0'))
	{
		/* not useful information */
		goto noident;
	}
	p = strchr(p, ':');
	if (p == NULL)
	{
		/* malformed response */
		goto noident;
	}

	/* 1413 says don't do this -- but it's broken otherwise */
	while (isascii(*++p) && isspace(*p))
		continue;

	/* p now points to the authenticated name -- copy carefully */
	cleanstrcpy(hbuf, p, MAXNAME);
	i = strlen(hbuf);
	hbuf[i++] = '@';
	strcpy(&hbuf[i], RealHostName == NULL ? "localhost" : RealHostName);
	goto finish;

closeident:
	(void) close(s);
	clrevent(ev);

noident:
	if (RealHostName == NULL)
	{
		if (tTd(9, 1))
			printf("getauthinfo: NULL\n");
		return NULL;
	}
	(void) strcpy(hbuf, RealHostName);

finish:
	if (RealHostName != NULL && RealHostName[0] != '[')
	{
		p = &hbuf[strlen(hbuf)];
		(void) sprintf(p, " [%s]", anynet_ntoa(&RealHostAddr));
	}
	if (tTd(9, 1))
		printf("getauthinfo: %s\n", hbuf);
	return hbuf;
}
/*
**  HOST_MAP_LOOKUP -- turn a hostname into canonical form
**
**	Parameters:
**		map -- a pointer to this map (unused).
**		name -- the (presumably unqualified) hostname.
**		av -- unused -- for compatibility with other mapping
**			functions.
**		statp -- an exit status (out parameter) -- set to
**			EX_TEMPFAIL if the name server is unavailable.
**
**	Returns:
**		The mapping, if found.
**		NULL if no mapping found.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, return the canonical
**		name.
*/

char *
host_map_lookup(map, name, av, statp)
	MAP *map;
	char *name;
	char **av;
	int *statp;
{
	register struct hostent *hp;
	struct in_addr in_addr;
	char *cp;
	register STAB *s;
	char hbuf[MAXNAME + 1];
#if NAMED_BIND
	extern int h_errno;
#endif

	/*
	**  See if we have already looked up this name.  If so, just
	**  return it.
	*/

	s = stab(name, ST_NAMECANON, ST_ENTER);
	if (bitset(NCF_VALID, s->s_namecanon.nc_flags))
	{
		if (tTd(9, 1))
			printf("host_map_lookup(%s) => CACHE %s\n",
				name, s->s_namecanon.nc_cname);
		errno = s->s_namecanon.nc_errno;
#if NAMED_BIND
		h_errno = s->s_namecanon.nc_herrno;
#endif
		*statp = s->s_namecanon.nc_stat;
		if (CurEnv->e_message == NULL && *statp == EX_TEMPFAIL)
		{
			sprintf(hbuf, "%s: Name server timeout",
				shortenstring(name, 33));
			CurEnv->e_message = newstr(hbuf);
		}
		return s->s_namecanon.nc_cname;
	}

	/*
	**  If first character is a bracket, then it is an address
	**  lookup.  Address is copied into a temporary buffer to
	**  strip the brackets and to preserve name if address is
	**  unknown.
	*/

	if (*name != '[')
	{
		extern bool getcanonname();

		if (tTd(9, 1))
			printf("host_map_lookup(%s) => ", name);
		s->s_namecanon.nc_flags |= NCF_VALID;		/* will be soon */
		if (strlen(name) < sizeof hbuf)
			(void) strcpy(hbuf, name);
		else
		{
			bcopy(name, hbuf, sizeof hbuf - 1);
			hbuf[sizeof hbuf - 1] = '\0';
		}
		if (getcanonname(hbuf, sizeof hbuf - 1, TRUE))
		{
			if (tTd(9, 1))
				printf("%s\n", hbuf);
			cp = map_rewrite(map, hbuf, strlen(hbuf), av);
			s->s_namecanon.nc_cname = newstr(cp);
			return cp;
		}
		else
		{
			register struct hostent *hp;

			s->s_namecanon.nc_errno = errno;
#if NAMED_BIND
			s->s_namecanon.nc_herrno = h_errno;
			if (tTd(9, 1))
				printf("FAIL (%d)\n", h_errno);
			switch (h_errno)
			{
			  case TRY_AGAIN:
				if (UseNameServer)
				{
					sprintf(hbuf, "%s: Name server timeout",
						shortenstring(name, 33));
					message("%s", hbuf);
					if (CurEnv->e_message == NULL)
						CurEnv->e_message = newstr(hbuf);
				}
				*statp = EX_TEMPFAIL;
				break;

			  case HOST_NOT_FOUND:
				*statp = EX_NOHOST;
				break;

			  case NO_RECOVERY:
				*statp = EX_SOFTWARE;
				break;

			  default:
				*statp = EX_UNAVAILABLE;
				break;
			}
#else
			if (tTd(9, 1))
				printf("FAIL\n");
			*statp = EX_NOHOST;
#endif
			s->s_namecanon.nc_stat = *statp;
			if ((*statp != EX_TEMPFAIL && *statp != EX_NOHOST) ||
			    UseNameServer)
				return NULL;

			/*
			**  Try to look it up in /etc/hosts
			*/

			hp = sm_gethostbyname(name);
			if (hp == NULL)
			{
				/* no dice there either */
				s->s_namecanon.nc_stat = *statp = EX_NOHOST;
				return NULL;
			}

			s->s_namecanon.nc_stat = *statp = EX_OK;
			cp = map_rewrite(map, hp->h_name, strlen(hp->h_name), av);
			s->s_namecanon.nc_cname = newstr(cp);
			return cp;
		}
	}
	if ((cp = strchr(name, ']')) == NULL)
		return (NULL);
	*cp = '\0';
	in_addr.s_addr = inet_addr(&name[1]);

	/* nope -- ask the name server */
	hp = sm_gethostbyaddr((char *)&in_addr, INADDRSZ, AF_INET);
	s->s_namecanon.nc_errno = errno;
#if NAMED_BIND
	s->s_namecanon.nc_herrno = h_errno;
#endif
	s->s_namecanon.nc_flags |= NCF_VALID;		/* will be soon */
	if (hp == NULL)
	{
		s->s_namecanon.nc_stat = *statp = EX_NOHOST;
		return (NULL);
	}

	/* found a match -- copy out */
	cp = map_rewrite(map, hp->h_name, strlen(hp->h_name), av);
	s->s_namecanon.nc_stat = *statp = EX_OK;
	s->s_namecanon.nc_cname = newstr(cp);
	return cp;
}
/*
**  ANYNET_NTOA -- convert a network address to printable form.
**
**	Parameters:
**		sap -- a pointer to a sockaddr structure.
**
**	Returns:
**		A printable version of that sockaddr.
*/

char *
anynet_ntoa(sap)
	register SOCKADDR *sap;
{
	register char *bp;
	register char *ap;
	int l;
	static char buf[100];

	/* check for null/zero family */
	if (sap == NULL)
		return "NULLADDR";
	if (sap->sa.sa_family == 0)
		return "0";

	switch (sap->sa.sa_family)
	{
#ifdef NETUNIX
	  case AF_UNIX:
	  	if (sap->sunix.sun_path[0] != '\0')
	  		sprintf(buf, "[UNIX: %.64s]", sap->sunix.sun_path);
	  	else
	  		sprintf(buf, "[UNIX: localhost]");
		return buf;
#endif

#ifdef NETINET
	  case AF_INET:
		return inet_ntoa(((struct sockaddr_in *) sap)->sin_addr);
#endif

	  default:
	  	/* this case is only to ensure syntactic correctness */
	  	break;
	}

	/* unknown family -- just dump bytes */
	(void) sprintf(buf, "Family %d: ", sap->sa.sa_family);
	bp = &buf[strlen(buf)];
	ap = sap->sa.sa_data;
	for (l = sizeof sap->sa.sa_data; --l >= 0; )
	{
		(void) sprintf(bp, "%02x:", *ap++ & 0377);
		bp += 3;
	}
	*--bp = '\0';
	return buf;
}
/*
**  HOSTNAMEBYANYADDR -- return name of host based on address
**
**	Parameters:
**		sap -- SOCKADDR pointer
**
**	Returns:
**		text representation of host name.
**
**	Side Effects:
**		none.
*/

char *
hostnamebyanyaddr(sap)
	register SOCKADDR *sap;
{
	register struct hostent *hp;
	int saveretry;

#if NAMED_BIND
	/* shorten name server timeout to avoid higher level timeouts */
	saveretry = _res.retry;
	_res.retry = 3;
#endif /* NAMED_BIND */

	switch (sap->sa.sa_family)
	{
#ifdef NETINET
	  case AF_INET:
		hp = sm_gethostbyaddr((char *) &sap->sin.sin_addr,
			INADDRSZ,
			AF_INET);
		break;
#endif

#ifdef NETISO
	  case AF_ISO:
		hp = sm_gethostbyaddr((char *) &sap->siso.siso_addr,
			sizeof sap->siso.siso_addr,
			AF_ISO);
		break;
#endif

	  case AF_UNIX:
		hp = NULL;
		break;

	  default:
		hp = sm_gethostbyaddr(sap->sa.sa_data,
			   sizeof sap->sa.sa_data,
			   sap->sa.sa_family);
		break;
	}

#if NAMED_BIND
	_res.retry = saveretry;
#endif /* NAMED_BIND */

	if (hp != NULL)
		return hp->h_name;
	else
	{
		/* produce a dotted quad */
		static char buf[512];

		(void) sprintf(buf, "[%s]", anynet_ntoa(sap));
		return buf;
	}
}

# else /* DAEMON */
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
**  GETAUTHINFO -- get the real host name asociated with a file descriptor
**
**	Parameters:
**		fd -- the descriptor
**
**	Returns:
**		The host name associated with this descriptor, if it can
**			be determined.
**		NULL otherwise.
**
**	Side Effects:
**		none
*/

char *
getauthinfo(fd)
	int fd;
{
	return NULL;
}
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		map -- a pointer to the database map.
**		name -- a buffer containing a hostname.
**		avp -- a pointer to a (cf file defined) argument vector.
**		statp -- an exit status (out parameter).
**
**	Returns:
**		mapped host name
**		FALSE otherwise.
**
**	Side Effects:
**		Looks up the host specified in name.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/

/*ARGSUSED*/
char *
host_map_lookup(map, name, avp, statp)
	MAP *map;
	char *name;
	char **avp;
	char *statp;
{
	register struct hostent *hp;

	hp = sm_gethostbyname(name);
	if (hp != NULL)
		return hp->h_name;
	*statp = EX_NOHOST;
	return NULL;
}

#endif /* DAEMON */
