/*
 * $Id: wire.c,v 5.2.1.1 91/03/17 17:42:58 jsp Alpha $
 *
 * Copyright (c) 1990 Jan-Simon Pendry
 * Copyright (c) 1990 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Imperial College of Science, Technology and Medicine, London, UK.
 * The names of the College and University may not be used to endorse
 * or promote products derived from this software without specific
 * prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)wire.c	5.1 (Berkeley) %G%
 */

/*
 * This routine returns the subnet (address&netmask) for the primary network
 * interface.  If the resulting address has an entry in the hosts file, the
 * corresponding name is retuned, otherwise the address is returned in
 * standard internet format.
 *
 * From: Paul Anderson (23/4/90)
 */

#include "am.h"

#include <sys/ioctl.h>

#define NO_SUBNET "notknown"

#ifdef SIOCGIFFLAGS
#include <net/if.h>
#include <netdb.h>

#if defined(IFF_LOCAL_LOOPBACK) && !defined(IFF_LOOPBACK)
#define IFF_LOOPBACK IFF_LOCAL_LOOPBACK
#endif

#define GFBUFLEN 1024
#define clist (ifc.ifc_ifcu.ifcu_req)
#define count (ifc.ifc_len/sizeof(struct ifreq))

char *getwire P((void));
char *getwire()
{
	struct hostent *hp;
	struct netent *np;
	struct ifconf ifc;
	struct ifreq *ifr;
	caddr_t cp, cplim;
	unsigned long address, netmask, subnet;
	char buf[GFBUFLEN], *s;
	int sk = -1;

	/*
	 * Get suitable socket
	 */
	if ((sk = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto out;

	/*
	 * Fill in ifconf details
	 */
	ifc.ifc_len = sizeof buf;
	ifc.ifc_buf = buf;

	/*
	 * Get network interface configurations
	 */
	if (ioctl(sk, SIOCGIFCONF, (caddr_t) &ifc) < 0)
		goto out;

	/*
	 * Upper bound on array
	 */
	cplim = buf + ifc.ifc_len;

	/*
	 * This is some magic to cope with both "traditional" and the
	 * new 4.4BSD-style struct sockaddrs.  The new structure has
	 * variable length and a size field to support longer addresses.
	 * AF_LINK is a new definition for 4.4BSD.
	 */
#ifdef AF_LINK
#define max(a, b) ((a) > (b) ? (a) : (b))
#define size(p) max((p).sa_len, sizeof(p))
#else
#define size(p) sizeof(p)
#endif
	/*
	 * Scan the list looking for a suitable interface
	 */
	for (cp = buf; cp < cplim; cp += sizeof(ifr->ifr_name) + size(ifr->ifr_addr)) {
		ifr = (struct ifreq *) cp;

		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;
		else
			address = ((struct sockaddr_in *) &ifr->ifr_addr)->sin_addr.s_addr;

		/*
		 * Get interface flags
		 */
		if (ioctl(sk, SIOCGIFFLAGS, (caddr_t) ifr) < 0)
			goto out;

		/*
		 * If the interface is a loopback, or its not running
		 * then ignore it.
		 */
		if ((ifr->ifr_flags & IFF_LOOPBACK) != 0)
			continue;
		if ((ifr->ifr_flags & IFF_RUNNING) == 0)
			continue;

		/*
		 * Get the netmask of this interface
		 */
		if (ioctl(sk, SIOCGIFNETMASK, (caddr_t) ifr) < 0)
			goto out;

		netmask = ((struct sockaddr_in *) &ifr->ifr_addr)->sin_addr.s_addr;

		(void) close(sk);

		/*
		 * Figure out the subnet's network address
		 */
		subnet = address & netmask;
		/*
		 * Now get a usable name.
		 * First use the network database,
		 * then the host database,
		 * and finally just make a dotted quad.
		 */
		np = getnetbyaddr(subnet, AF_INET);
		if (np)
			s = np->n_name;
		else {
			hp = gethostbyaddr((char *) &subnet, 4, AF_INET);
			if (hp)
				s = hp->h_name;
			else
				s = inet_dquad(buf, subnet);
		}

		return strdup(s);
	}

out:
	if (sk >= 0)
		(void) close(sk); 
	return strdup(NO_SUBNET);
}

#else

char *getwire P((void));
char *getwire()
{
	return strdup(NO_SUBNET);
}
#endif /* SIOCGIFFLAGS */
