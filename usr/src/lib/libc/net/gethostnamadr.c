/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)gethostnamadr.c	5.6 (Berkeley) %G%";
#endif not lint

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include <arpa/resolv.h>

#define	MAXALIASES	35
#define MAXADDRS	35

static char *h_addr_ptrs[MAXADDRS + 1];

static struct hostent host;
static char *host_aliases[MAXALIASES];
static char hostbuf[BUFSIZ+1];


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
	int haveanswer, getclass;
	char **hap;

	n = res_send(msg, msglen, answer, sizeof(answer));
	if (n < 0) {
		if (_res.options & RES_DEBUG)
			printf("res_send failed\n");
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
		return (NULL);
	}
	bp = hostbuf;
	buflen = sizeof(hostbuf);
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
	ap = host_aliases;
	host.h_aliases = host_aliases;
	hap = h_addr_ptrs;
	host.h_addr_list = h_addr_ptrs;
	haveanswer = 0;
	while (--ancount >= 0 && cp < eom) {
		if ((n = dn_expand(answer, cp, bp, buflen)) < 0)
			break;
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
		if (type != T_A)  {
			if (_res.options & RES_DEBUG)
				printf("unexpected answer type %d, size %d\n",
					type, n);
			cp += n;
			continue;
		}
		if (haveanswer) {
			if (n != host.h_length) {
				cp += n;
				continue;
			}
			if (class != getclass) {
				cp += n;
				continue;
			}
		} else {
			host.h_length = n;
			getclass = class;
			host.h_addrtype = C_IN ? AF_INET : AF_UNSPEC;
			if (!iquery) {
				host.h_name = bp;
				bp += strlen(bp) + 1;
			}
		}
		if (bp + n >= &hostbuf[sizeof(hostbuf)]) {
			if (_res.options & RES_DEBUG)
				printf("size (%d) too big\n", n);
			break;
		}
		bcopy(cp, *hap++ = bp, n);
		bp +=n;
		cp += n;
		haveanswer++;
	}
	if (haveanswer) {
		*ap = NULL;
		*hap = NULL;
		return (&host);
	} else
		return (NULL);
}

struct hostent *
gethostbyname(name)
	char *name;
{
	int n;
	char buf[BUFSIZ+1];

	n = res_mkquery(QUERY, name, C_ANY, T_A, (char *)NULL, 0, NULL,
		buf, sizeof(buf));
	if (n < 0) {
		if (_res.options & RES_DEBUG)
			printf("res_mkquery failed\n");
		return (NULL);
	}
	return(getanswer(buf, n, 0));
}

struct hostent *
gethostbyaddr(addr, len, type)
	char *addr;
	int len, type;
{
	int n;
	char buf[BUFSIZ+1];

	if (type != AF_INET)
		return (NULL);
	n = res_mkquery(IQUERY, (char *)NULL, C_IN, T_A, addr, len, NULL,
		buf, sizeof(buf));
	if (n < 0) {
		if (_res.options & RES_DEBUG)
			printf("res_mkquery failed\n");
		return (NULL);
	}
	return(getanswer(buf, n, 1));
}


