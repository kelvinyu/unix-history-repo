/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#include <errno.h>
#include <signal.h>
#include "sendmail.h"
# include <sys/mx.h>

#ifndef lint
#ifdef DAEMON
static char sccsid[] = "@(#)daemon.c	6.22 (Berkeley) %G% (with daemon mode)";
#else
static char sccsid[] = "@(#)daemon.c	6.22 (Berkeley) %G% (without daemon mode)";
#endif
#endif /* not lint */

#ifdef DAEMON

# include <netdb.h>
# include <sys/wait.h>
# include <sys/time.h>

#ifdef NETISO
# include <netiso/iso.h>
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
**	maphostname(map, hbuf, hbufsiz, avp)
**		Convert the entry in hbuf into a canonical form.
*/

extern char	*anynet_ntoa();

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
	bool refusingconnections = TRUE;
	FILE *pidf;
	SOCKADDR srvraddr;
	extern void reapchild();

	/*
	**  Set up the address for the mailer.
	*/

	sp = getservbyname("smtp", "tcp");
	if (sp == NULL)
	{
		syserr("554 server \"smtp\" unknown");
		goto severe;
	}
	srvraddr.sin.sin_family = AF_INET;
	srvraddr.sin.sin_addr.s_addr = INADDR_ANY;
	srvraddr.sin.sin_port = sp->s_port;

	/*
	**  Try to actually open the connection.
	*/

	if (tTd(15, 1))
		printf("getrequests: port 0x%x\n", srvraddr.sin.sin_port);

	/* get a socket for the SMTP connection */
	DaemonSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (DaemonSocket < 0)
	{
		/* probably another daemon already */
		syserr("getrequests: can't create socket");
	  severe:
# ifdef LOG
		if (LogLevel > 0)
# endif /* LOG */
		finis();
	}

	/* turn on network debugging? */
	if (tTd(15, 101))
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);

	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

	if (bind(DaemonSocket, &srvraddr.sa, sizeof srvraddr) < 0)
	{
		syserr("getrequests: cannot bind");
		(void) close(DaemonSocket);
		goto severe;
	}

	(void) signal(SIGCHLD, reapchild);

	/* write the pid to the log file for posterity */
	pidf = fopen(PidFile, "w");
	if (pidf != NULL)
	{
		fprintf(pidf, "%d\n", getpid());
		fclose(pidf);
	}


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
	register int i, s;
	register struct hostent *hp = (struct hostent *)NULL;
	SOCKADDR addr;
	int sav_errno;
	int addrlen;
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
		register char *p = strchr(host, ']');

		if (p != NULL)
		{
			*p = '\0';
			hid = inet_addr(&host[1]);
			if (hid == -1)
			{
				/* try it as a host name (avoid MX lookup) */
				hp = gethostbyname(&host[1]);
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
		addr.sin.sin_family = AF_INET;
		addr.sin.sin_addr.s_addr = hid;
	}
	else
	{
		hp = gethostbyname(host);
gothostent:
		if (hp == NULL)
		{
#ifdef NAMED_BIND
			if (errno == ETIMEDOUT || h_errno == TRY_AGAIN)
				return (EX_TEMPFAIL);

			/* if name server is specified, assume temp fail */
			if (errno == ECONNREFUSED && UseNameServer)
				return (EX_TEMPFAIL);
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
				hp->h_length);
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
			syserr("554 makeconnection: server \"smtp\" unknown");
			return (EX_OSERR);
		}
		port = sp->s_port;
	}

	switch (addr.sa.sa_family)
	{
	  case AF_INET:
		addr.sin.sin_port = port;
		addrlen = sizeof (struct sockaddr_in);
		break;

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

		if (tTd(16, 1))
			printf("makeconnection: fd=%d\n", s);

		/* turn on network debugging? */
		if (tTd(16, 101))
		{
			int on = 1;
			(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG,
					  (char *)&on, sizeof on);
		}
		if (CurEnv->e_xfp != NULL)
			(void) fflush(CurEnv->e_xfp);		/* for debugging */
		errno = 0;					/* for debugging */
		if (connect(s, (struct sockaddr *) &addr, addrlen) >= 0)
			break;

		/* couldn't connect.... figure out why */
		sav_errno = errno;
		(void) close(s);
		if (hp && hp->h_addr_list[i])
		{
			extern char *errstring();

			if (tTd(16, 1))
				printf("Connect failed (%s); trying new address....\n",
					errstring(sav_errno));
			switch (addr.sa.sa_family)
			{
#ifdef NETINET
			  case AF_INET:
				bcopy(hp->h_addr_list[i++],
				      &addr.sin.sin_addr,
				      hp->h_length);
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
		if (transienterror(sav_errno))
			return EX_TEMPFAIL;
		else
		{
			extern char *errstring();

			message("%s", errstring(sav_errno));
			return (EX_UNAVAILABLE);
		}
	}

	/* connection ok, put it into canonical form */
	mci->mci_out = fdopen(s, "w");
	mci->mci_in = fdopen(dup(s), "r");

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
**		Sets the MyIpAddrs buffer to a list of my IP addresses.
*/

struct in_addr	MyIpAddrs[MAXIPADDR + 1];

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	register struct hostent *hp;
	extern struct hostent *gethostbyname();

	if (gethostname(hostbuf, size) < 0)
	{
		(void) strcpy(hostbuf, "localhost");
	}
	hp = gethostbyname(hostbuf);
	if (hp != NULL)
	{
		(void) strncpy(hostbuf, hp->h_name, size - 1);
		hostbuf[size - 1] = '\0';

		if (hp->h_addrtype == AF_INET && hp->h_length == 4)
		{
			register int i;

			for (i = 0; i < MAXIPADDR; i++)
			{
				if (hp->h_addr_list[i] == NULL)
					break;
				MyIpAddrs[i].s_addr = *(u_long *) hp->h_addr_list[i];
			}
			MyIpAddrs[i].s_addr = 0;
		}

		return (hp->h_aliases);
	}
	else
		return (NULL);
}
/*
**  GETREALHOSTNAME -- get the real host name asociated with a file descriptor
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
getrealhostname(fd)
	int fd;
{
	register struct hostent *hp;
	SOCKADDR sa;
	int salen;
	char hbuf[MAXNAME];
	extern struct hostent *gethostbyaddr();

	salen = sizeof sa;
	if (getsockname(fd, &sa, &salen) < 0 || salen <= 0)
		return NULL;
	hp = gethostbyaddr(sa.sa_data, salen, sa.sa_family);
	if (hp != NULL)
		(void) strcpy(hbuf, hp->h_name);
	else
		(void) sprintf(hbuf, "[%s]", anynet_ntoa(&sa));
	return hbuf;
}
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		map -- a pointer to this map (unused).
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**		avp -- unused -- for compatibility with other mapping
**			functions.
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
maphostname(map, hbuf, hbsize, avp)
	MAP *map;
	char *hbuf;
	int hbsize;
	char **avp;
{
	register struct hostent *hp;
	u_long in_addr;
	char *cp;
	int i;
	struct hostent *gethostbyaddr();

	/* allow room for null */
	hbsize--;

	/*
	 * If first character is a bracket, then it is an address
	 * lookup.  Address is copied into a temporary buffer to
	 * strip the brackets and to preserve hbuf if address is
	 * unknown.
	 */

	if (*hbuf != '[')
	{
		extern bool getcanonname();

		if (getcanonname(hbuf, hbsize))
			return hbuf;
		else
			return NULL;
	}
	if ((cp = strchr(hbuf, ']')) == NULL)
		return (NULL);
	*cp = '\0';
	in_addr = inet_addr(&hbuf[1]);

	/* check to see if this is one of our addresses */
	for (i = 0; MyIpAddrs[i].s_addr != 0; i++)
	{
		if (MyIpAddrs[i].s_addr == in_addr)
		{
			strncpy(hbuf, MyHostName, hbsize);
			hbuf[hbsize] = '\0';
			return hbuf;
		}
	}

	/* nope -- ask the name server */
	hp = gethostbyaddr((char *)&in_addr, sizeof(struct in_addr), AF_INET);
	if (hp == NULL)
		return (NULL);

	/* found a match -- copy out */
	if (strlen(hp->h_name) > hbsize)
		hp->h_name[hbsize] = '\0';
	(void) strcpy(hbuf, hp->h_name);
	return hbuf;
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
	static char buf[80];

#ifdef NETINET
	if (sap->sa.sa_family == AF_INET)
	{
		extern char *inet_ntoa();

		return inet_ntoa(((struct sockaddr_in *) sap)->sin_addr);
	}
#endif

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
**  GETREALHOSTNAME -- get the real host name asociated with a file descriptor
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
getrealhostname(fd)
	int fd;
{
	return NULL;
}
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		map -- a pointer to the database map.
**		hbuf -- a buffer containing a hostname.
**		avp -- a pointer to a (cf file defined) argument vector.
**
**	Returns:
**		mapped host name
**		FALSE otherwise.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/

/*ARGSUSED*/
char *
maphostname(map, hbuf, hbsize, avp)
	MAP *map;
	char *hbuf;
	int hbsize;
	char **avp;
{
	return NULL;
}

#endif /* DAEMON */
