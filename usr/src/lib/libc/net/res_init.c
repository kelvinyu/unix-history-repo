/*
 * Copyright (c) 1985, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)res_init.c	6.11 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include <resolv.h>

/*
 * Resolver state default settings
 */

struct state _res = {
	RES_TIMEOUT,               	/* retransmition time interval */
	4,                         	/* number of times to retransmit */
	RES_DEFAULT,			/* options flags */
	1,                         	/* number of name servers */
};

/*
 * Set up default settings.  If the configuration file exist, the values
 * there will have precedence.  Otherwise, the server address is set to
 * INADDR_ANY and the default domain name comes from the gethostname().
 *
 * The configuration file should only be used if you want to redefine your
 * domain or run without a server on your machine.
 *
 * Return 0 if completes successfully, -1 on error
 */
res_init()
{
	register FILE *fp;
	register char *cp, **pp;
	register int n;
	char buf[BUFSIZ];
	extern u_long inet_addr();
	extern char *index();
	extern char *strcpy(), *strncpy();
	extern char *getenv();
	int nserv = 0;    /* number of nameserver records read from file */
	int havesearch = 0;

	_res.nsaddr.sin_addr.s_addr = INADDR_ANY;
	_res.nsaddr.sin_family = AF_INET;
	_res.nsaddr.sin_port = htons(NAMESERVER_PORT);
	_res.nscount = 1;

	/* Allow user to override the local domain definition */
	if ((cp = getenv("LOCALDOMAIN")) != NULL)
		(void)strncpy(_res.defdname, cp, sizeof(_res.defdname));

	if ((fp = fopen(_PATH_RESCONF, "r")) != NULL) {
	    /* read the config file */
	    while (fgets(buf, sizeof(buf), fp) != NULL) {
		/* read default domain name */
		if (!strncmp(buf, "domain", sizeof("domain") - 1)) {
		    if (_res.defdname[0])	/* skip if have from environ */
			    continue;
		    cp = buf + sizeof("domain") - 1;
		    while (*cp == ' ' || *cp == '\t')
			    cp++;
		    if (*cp == '\0')
			    continue;
		    (void)strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
		    if ((cp = index(_res.defdname, '\n')) != NULL)
			    *cp = '\0';
		    havesearch = 0;
		    continue;
		}
		/* set search list */
		if (!strncmp(buf, "search", sizeof("search") - 1)) {
		    cp = buf + sizeof("search") - 1;
		    while (*cp == ' ' || *cp == '\t')
			    cp++;
		    if (*cp == '\0')
			    continue;
		    (void)strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
		    if ((cp = index(_res.defdname, '\n')) != NULL)
			    *cp = '\0';
		    /*
		     * Set search list to be blank-separated strings
		     * on rest of line.
		     */
		    cp = _res.defdname;
		    pp = _res.dnsrch;
		    *pp++ = cp;
		    for (n = 0; *cp && pp < _res.dnsrch + MAXDNSRCH; cp++) {
			    if (*cp == ' ' || *cp == '\t') {
				    *cp = 0;
				    n = 1;
			    } else if (n) {
				    *pp++ = cp;
				    n = 0;
			    }
		    }
		    havesearch = 1;
		    continue;
		}
		/* read nameservers to query */
		if (!strncmp(buf, "nameserver", sizeof("nameserver") - 1) &&
		   nserv < MAXNS) {
		    cp = buf + sizeof("nameserver") - 1;
		    while (*cp == ' ' || *cp == '\t')
			    cp++;
		    if (*cp == '\0')
			    continue;
		    if ((_res.nsaddr_list[nserv].sin_addr.s_addr =
			inet_addr(cp)) == (unsigned)-1) 
			    continue;
		    _res.nsaddr_list[nserv].sin_family = AF_INET;
		    _res.nsaddr_list[nserv].sin_port = htons(NAMESERVER_PORT);
		    nserv++;
		    continue;
		}
	    }
	    if (nserv > 1) 
		_res.nscount = nserv;
	    (void) fclose(fp);
	}
	if (_res.defdname[0] == 0) {
		if (gethostname(buf, sizeof(_res.defdname)) == 0 &&
		   (cp = index(buf, '.')))
			(void)strcpy(_res.defdname, cp + 1);
	}

	/* find components of local domain that might be searched */
	if (havesearch == 0) {
		pp = _res.dnsrch;
		*pp++ = _res.defdname;
		for (cp = _res.defdname, n = 0; *cp; cp++)
			if (*cp == '.')
				n++;
		cp = _res.defdname;
		for (; n >= LOCALDOMAINPARTS && pp < _res.dnsrch + MAXDNSRCH;
		    n--) {
			cp = index(cp, '.');
			*pp++ = ++cp;
		}
	}
	_res.options |= RES_INIT;
	return (0);
}
