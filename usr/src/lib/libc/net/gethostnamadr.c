/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)gethostnamadr.c	5.3 (Berkeley) %G%";
#endif not lint

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include <arpa/resolv.h>

#define	MAXALIASES	35

#define XX 2
static char h_addr_buf[sizeof(struct in_addr) * XX];
static char *h_addr_ptr[XX] = {
	&h_addr_buf[0],
	&h_addr_buf[sizeof(struct in_addr)]
};
static struct hostent host = {
	NULL,		/* official name of host */
	NULL,		/* alias list */
	0,		/* host address type */
	0,		/* length of address */
	h_addr_ptr	/* list of addresses from name server */
};
static char *host_aliases[MAXALIASES];
static char hostbuf[BUFSIZ+1];

#ifdef BOMBPROOFING
#include <ndbm.h>
DBM *_host_db = NULL;
int	_host_stayopen = 0;
int	_host_bombed;
struct hostent *_oldgethostbyname(), *_oldgethostbyaddr();
#endif BOMBPROOFING

static struct hostent *
getanswer(msg, msglen, iquery)
	char *msg;
	int msglen, iquery;
{
	register HEADER *hp;
	register char *cp;
	register int n;
	char answer[PACKETSZ];
	char *eom, *bp, **ap;
	int type, class, ancount, buflen;

#ifdef BOMBPROOFING
	_host_bombed = 0;
#endif BOMBPROOFING
	n = res_send(msg, msglen, answer, sizeof(answer));
	if (n < 0) {
		if (_res.options & RES_DEBUG)
			printf("res_send failed\n");
#ifdef BOMBPROOFING
		_host_bombed++;
#endif BOMBPROOFING
		return (NULL);
	}
	eom = answer + n;
	/*
	 * find first satisfactory answer
	 */
	hp = (HEADER *) answer;
	ancount = ntohs(hp->ancount);
	if (hp->rcode != NOERROR || ancount == 0) {
		if (_res.options & RES_DEBUG)
			printf("rcode = %d, ancount=%d\n", hp->rcode, ancount);
#ifdef BOMBPROOFING
		if (!(hp->rcode == NOERROR && ancount == 0))
			_host_bombed++;
#endif BOMBPROOFING
		return (NULL);
	}
	bp = hostbuf;
	buflen = sizeof(hostbuf);
	ap = host_aliases;
	cp = answer + sizeof(HEADER);
	if (hp->qdcount) {
		if (iquery) {
			if ((n = dn_expand(answer, cp, bp, buflen)) < 0)
				return (NULL);
			cp += n + QFIXEDSZ;
			host.h_name = bp;
			n = strlen(bp) + 1;
			bp += n;
			buflen -= n;
		} else
			cp += dn_skip(cp) + QFIXEDSZ;
	} else if (iquery)
		return (NULL);
	while (--ancount >= 0 && cp < eom) {
		if ((n = dn_expand(answer, cp, bp, buflen)) < 0)
			return (NULL);
		cp += n;
		type = getshort(cp);
 		cp += sizeof(u_short);
		class = getshort(cp);
 		cp += sizeof(u_short) + sizeof(u_long);
		n = getshort(cp);
		cp += sizeof(u_short);
		if (type == T_CNAME) {
			cp += n;
			if (ap >= &host_aliases[MAXALIASES-1])
				continue;
			*ap++ = bp;
			n = strlen(bp) + 1;
			bp += n;
			buflen -= n;
			continue;
		}
		if (type != T_A || n != 4) {
			if (_res.options & RES_DEBUG)
				printf("unexpected answer type %d, size %d\n",
					type, n);
			continue;
		}
		if (!iquery) {
			host.h_name = bp;
			bp += strlen(bp) + 1;
		}
		*ap = NULL;
		host.h_aliases = host_aliases;
		host.h_addrtype = class == C_IN ? AF_INET : AF_UNSPEC;
		if (bp + n >= &hostbuf[sizeof(hostbuf)]) {
			if (_res.options & RES_DEBUG)
				printf("size (%d) too big\n", n);
			return (NULL);
		}
		bcopy(cp, host.h_addr = bp, host.h_length = n);
		return (&host);
	}
	return (NULL);
}

struct hostent *
gethostbyname(name)
	char *name;
{
	int n;
#ifdef BOMBPROOFING
	register struct hostent *hp;
#endif BOMBPROOFING

	n = res_mkquery(QUERY, name, C_ANY, T_A, NULL, 0, NULL,
		hostbuf, sizeof(hostbuf));
	if (n < 0) {
		if (_res.options & RES_DEBUG)
			printf("res_mkquery failed\n");
		return (NULL);
#ifndef BOMBPROOFING
	}
	return(getanswer(hostbuf, n, 0));
#else
	} else
		hp = getanswer(hostbuf, n, 0);
	if (n < 0 || (hp == NULL && _host_bombed))
		return (_oldgethostbyname(name));
	else
		return (hp);
#endif BOMBPROOFING
}

struct hostent *
gethostbyaddr(addr, len, type)
	char *addr;
	int len, type;
{
	int n;
#ifdef BOMBPROOFING
	register struct hostent *hp;
#endif BOMBPROOFING

	if (type != AF_INET)
		return (NULL);
	n = res_mkquery(IQUERY, NULL, C_IN, T_A, addr, len, NULL,
		hostbuf, sizeof(hostbuf));
	if (n < 0) {
		if (_res.options & RES_DEBUG)
			printf("res_mkquery failed\n");
		return (NULL);
#ifndef BOMBPROOFING
	}
	return (getanswer(hostbuf, n, 1));
#else
	} else
		hp = getanswer(hostbuf, n, 1);
	if (n < 0 || (hp == NULL && _host_bombed))
		return (_oldgethostbyaddr(addr));
	else
		return (hp);
#endif BOMBPROOFING
}

#ifdef BOMBPROOFING
static
struct hostent *
_oldgethostbyname(name)
	register char *name;
{
	register struct hostent *hp;
	register char **cp;
        datum key;

	if ((_host_db == (DBM *)NULL)
	  && ((_host_db = dbm_open(_host_file, O_RDONLY)) == (DBM *)NULL)) {
		sethostent(_host_stayopen);
		while (hp = gethostent()) {
			if (strcmp(hp->h_name, nam) == 0)
				break;
			for (cp = hp->h_aliases; cp != 0 && *cp != 0; cp++)
				if (strcmp(*cp, nam) == 0)
					goto found;
		}
	found:
		if (!_host_stayopen)
			endhostent();
		return (hp);
	}
        key.dptr = nam;
        key.dsize = strlen(nam);
	hp = fetchhost(key);
	if (!_host_stayopen) {
		dbm_close(_host_db);
		_host_db = (DBM *)NULL;
	}
        return (hp);
}


static
struct hostent *
_oldgethostbyaddr(addr, len, type)
	char *addr;
	register int len, type;
{

	register struct hostent *hp;
        datum key;

	if ((_host_db == (DBM *)NULL)
	  && ((_host_db = dbm_open(_host_file, O_RDONLY)) == (DBM *)NULL)) {
		sethostent(_host_stayopen);
		while (hp = gethostent()) {
			if (hp->h_addrtype == type && hp->h_length == length
			    && bcmp(hp->h_addr, addr, length) == 0)
				break;
		}
		if (!_host_stayopen)
			endhostent();
		return (hp);
	}
        key.dptr = addr;
        key.dsize = length;
	hp = fetchhost(key);
	if (!_host_stayopen) {
		dbm_close(_host_db);
		_host_db = (DBM *)NULL;
	}
        return (hp);
}
#endif BOMBPROOFING

