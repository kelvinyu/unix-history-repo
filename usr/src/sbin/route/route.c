/*
 * Copyright (c) 1983, 1989 The Regents of the University of California.
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

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)route.c	5.22 (Berkeley) %G%";
#endif /* not lint */

#include <paths.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/mbuf.h>
#include <sys/kinfo.h>

#include <net/route.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netns/ns.h>
#include <netiso/iso.h>

#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

struct keytab {
	char	*kt_cp;
	int	kt_i;
} keywords[] = {
#include "keywords.h"
{0, 0}
};

struct	ortentry route;
union	sockunion {
	struct	sockaddr sa;
	struct	sockaddr_in sin;
	struct	sockaddr_ns sns;
	struct	sockaddr_iso siso;
	struct	sockaddr_dl sdl;
} so_dst, so_gate, so_mask, so_genmask, so_ifa, so_ifp, *so_addrs[] =
{ &so_dst, &so_gate, &so_mask, &so_genmask, &so_ifa, &so_ifp, 0}; 
typedef union sockunion *sup;
int	pid, rtm_addrs;
int	s;
int	forcehost, forcenet, doflush, nflag, af, qflag, Cflag, keyword();
int	iflag, verbose, aflen = sizeof (struct sockaddr_in);
struct	sockaddr_in sin = { sizeof(sin), AF_INET };
struct	in_addr inet_makeaddr();
char	*malloc(), *routename(), *netname();
extern	char *iso_ntoa(), *link_ntoa();
#define kget(p, d) \
	(lseek(kmem, (off_t)(p), 0), read(kmem, (char *)&(d), sizeof (d)))

usage(cp)
char *cp;
{
	fprintf(stderr,
	    "usage: route [ -nqCv ]  cmd [[ -<qualifers> ] args ]\n");
	if (cp) fprintf(stderr, "(botched keyword: %s)\n", cp);

	exit(1);
}
main(argc, argv)
	int argc;
	char *argv[];
{

	char *argvp;
	if (argc < 2)
		usage((char *)0);
	argc--, argv++;
	for (; argc >  0 && argv[0][0] == '-'; argc--, argv++) {
		for (argvp = argv[0]++; *argvp; argvp++)
			switch (*argv[0]) {
			case 'n':
				nflag++;
				break;
			case 'q':
				qflag++;
				break;
			case 'C':
				Cflag++; /* Use old ioctls */
				break;
			case 'v':
				verbose++;
			}
	}
	pid = getpid();
	if (Cflag)
		s = socket(AF_INET, SOCK_RAW, 0);
	else
		s = socket(PF_ROUTE, SOCK_RAW, 0);
	if (s < 0) {
		perror("route: socket");
		exit(1);
	}
	if (argc > 0) switch (keyword(*argv)) {
		case K_ADD:
		case K_CHANGE:
		case K_DELETE:
			newroute(argc, argv);
		case K_MONITOR:
			monitor();
		case K_FLUSH:
			flushroutes(argc, argv);
	}
	usage(*argv);
}

/*
 * Purge all entries in the routing tables not
 * associated with network interfaces.
 */
flushroutes(argc, argv)
char *argv[];
{
	int bufsize, needed, seqno, rlen;
	char *buf, *next, *lim;
	register struct rt_msghdr *rtm;
	struct {
		struct rt_msghdr m_rtm;
		union {
			char u_saddrs[200];
			struct sockaddr u_sa;
		} m_u;
	} m;

	if (argc > 1) {
		argv++;
		if (argc == 2 && **argv == '-') switch (keyword(1 + *argv)) {
			case K_INET:	af = AF_INET;	break;
			case K_XNS:	af = AF_NS;	break;
			case K_LINK:	af = AF_LINK;	break;
			case K_ISO: case K_OSI:	af = AF_ISO; break;
			default: goto bad;
		} else
			bad: usage(*argv);
	}
	if ((needed = getkerninfo(KINFO_RT_DUMP, 0, 0, 0)) < 0)
		{ perror("route-getkerninfo-estimate"); exit(1);}
	if ((buf = malloc(needed)) == 0)
		{ printf("out of space\n");; exit(1);}
	if ((rlen = getkerninfo(KINFO_RT_DUMP, buf, &needed, 0)) < 0)
		{ perror("actual retrieval of routing table"); exit(1);}
	lim = buf + rlen;
	for (next = buf; next < lim; next += rtm->rtm_msglen) {
		rtm = (struct rt_msghdr *)next;
		if ((rtm->rtm_flags & RTF_GATEWAY) == 0)
			continue;
		if (af) {
			struct sockaddr *sa = (struct sockaddr *)(rtm + 1);
			if (sa->sa_family != af)
				continue;
		}
		rtm->rtm_type = RTM_DELETE;
		rtm->rtm_seq = seqno;
		if ((rlen = write(s, next, rtm->rtm_msglen)) < 0) {
			perror("writing to routing socket");
			printf("got only %d for rlen\n", rlen);
			break;
		}
	again:
		if ((rlen = read(s, (char *)&m, sizeof (m))) < 0) {
			perror("reading from routing socket");
			printf("got only %d for rlen\n", rlen);
			break;
		}
		if ((m.m_rtm.rtm_pid != pid) || (m.m_rtm.rtm_seq != seqno)) {
			printf("Got response for somebody else's request");
			goto again;
		}
		seqno++;
		if (qflag)
			continue;
		if (verbose) {
			print_rtmsg(rtm, rlen);
		} else {
			struct sockaddr *sa = &m.m_u.u_sa;
			printf("%-20.20s ", (rtm->rtm_flags & RTF_HOST) ?
			    routename(sa) : netname(sa));
			sa = (struct sockaddr *)(sa->sa_len + (char *)sa);
			printf("%-20.20s ", routename(sa));
			printf("done\n");
		}
	}
	exit(0);
}
	
char *
routename(sa)
	struct sockaddr *sa;
{
	register char *cp;
	static char line[50];
	struct hostent *hp;
	static char domain[MAXHOSTNAMELEN + 1];
	static int first = 1;
	char *index();
	char *ns_print();

	if (first) {
		first = 0;
		if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
		    (cp = index(domain, '.')))
			(void) strcpy(domain, cp + 1);
		else
			domain[0] = 0;
	}
	switch (sa->sa_family) {

	case AF_INET:
	    {	struct in_addr in;
		in = ((struct sockaddr_in *)sa)->sin_addr;

		cp = 0;
		if (in.s_addr == INADDR_ANY)
			cp = "default";
		if (cp == 0 && !nflag) {
			hp = gethostbyaddr(&in, sizeof (struct in_addr),
				AF_INET);
			if (hp) {
				if ((cp = index(hp->h_name, '.')) &&
				    !strcmp(cp + 1, domain))
					*cp = 0;
				cp = hp->h_name;
			}
		}
		if (cp)
			strcpy(line, cp);
		else {
#define C(x)	((x) & 0xff)
			in.s_addr = ntohl(in.s_addr);
			(void)sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			   C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
		}
		break;
	    }

	case AF_NS:
		return (ns_print((struct sockaddr_ns *)sa));

	case AF_LINK:
		return (link_ntoa((struct sockaddr_dl *)sa));

	case AF_ISO:
		(void) sprintf(line, "iso %s",
		    iso_ntoa(&((struct sockaddr_iso *)sa)->siso_addr));
		break;

	default:
	    {	u_short *s = (u_short *)sa->sa_data;
		u_short *slim = s + ((sa->sa_len + 1)>>1);
		char *cp = line + sprintf(line, "(%d)", sa->sa_family);
		int n;

		while (s < slim) {
			n = sprintf(cp, " %x", *s);
			s++; cp += n;
		}
		break;
	    }
	}
	return (line);
}

/*
 * Return the name of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */
char *
netname(sa)
	struct sockaddr *sa;
{
	char *cp = 0;
	static char line[50];
	struct netent *np = 0;
	u_long net, mask;
	register u_long i;
	int subnetshift;
	char *ns_print();

	switch (sa->sa_family) {

	case AF_INET:
	    {	struct in_addr in;
		in = ((struct sockaddr_in *)sa)->sin_addr;

		i = in.s_addr = ntohl(in.s_addr);
		if (in.s_addr == 0)
			cp = "default";
		else if (!nflag) {
			if (IN_CLASSA(i)) {
				mask = IN_CLASSA_NET;
				subnetshift = 8;
			} else if (IN_CLASSB(i)) {
				mask = IN_CLASSB_NET;
				subnetshift = 8;
			} else {
				mask = IN_CLASSC_NET;
				subnetshift = 4;
			}
			/*
			 * If there are more bits than the standard mask
			 * would suggest, subnets must be in use.
			 * Guess at the subnet mask, assuming reasonable
			 * width subnet fields.
			 */
			while (in.s_addr &~ mask)
				mask = (long)mask >> subnetshift;
			net = in.s_addr & mask;
			while ((mask & 1) == 0)
				mask >>= 1, net >>= 1;
			np = getnetbyaddr(net, AF_INET);
			if (np)
				cp = np->n_name;
		}
		if (cp)
			strcpy(line, cp);
		else if ((in.s_addr & 0xffffff) == 0)
			(void)sprintf(line, "%u", C(in.s_addr >> 24));
		else if ((in.s_addr & 0xffff) == 0)
			(void)sprintf(line, "%u.%u", C(in.s_addr >> 24),
			    C(in.s_addr >> 16));
		else if ((in.s_addr & 0xff) == 0)
			(void)sprintf(line, "%u.%u.%u", C(in.s_addr >> 24),
			    C(in.s_addr >> 16), C(in.s_addr >> 8));
		else
			(void)sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			    C(in.s_addr >> 16), C(in.s_addr >> 8),
			    C(in.s_addr));
		break;
	    }

	case AF_NS:
		return (ns_print((struct sockaddr_ns *)sa));
		break;

	case AF_LINK:
		return (link_ntoa((struct sockaddr_dl *)sa));

	case AF_ISO:
		(void) sprintf(line, "iso %s",
		    iso_ntoa(&((struct sockaddr_iso *)sa)->siso_addr));
		break;

	default:
	    {	u_short *s = (u_short *)sa->sa_data;
		u_short *slim = s + ((sa->sa_len + 1)>>1);
		char *cp = line + sprintf(line, "af %d:", sa->sa_family);
		int n;

		while (s < slim) {
			n = sprintf(cp, " %x", *s);
			s++; cp += n;
		}
		break;
	    }
	}
	return (line);
}

newroute(argc, argv)
	int argc;
	register char **argv;
{
	struct sockaddr_in *sin;
	char *cmd, *dest, *gateway, *mask;
	int ishost, metric = 0, ret, attempts, oerrno, flags = 0, next;
	struct hostent *hp = 0;
	extern int errno;

	cmd = argv[0];
	while (--argc > 0) {
		if (**(++argv)== '-') {
			switch(keyword(1 + *argv)) {
			case K_LINK:
				af = AF_LINK;
				aflen = sizeof(struct sockaddr_dl);
				break;
			case K_OSI:
			case K_ISO:
				af = AF_ISO;
				aflen = sizeof(struct sockaddr_iso);
				break;
			case K_INET:
				af = AF_INET;
				aflen = sizeof(struct sockaddr_in);
				break;
			case K_XNS:
				af = AF_NS;
				aflen = sizeof(struct sockaddr_ns);
				break;
			case K_IFACE:
				iflag++;
				break;
			case K_HOST:
				forcehost++;
				break;
			case K_NETMASK:
				argc--;
				(void) getaddr(RTA_NETMASK, *++argv, 0);
				/* FALLTHROUGH */
			case K_NET:
				forcenet++;
				break;
			case K_GENMASK:
				argc--;
				(void) getaddr(RTA_GENMASK, *++argv, 0);
				break;
			default:
				usage(1+*argv);
			}
		} else {
			if ((rtm_addrs & RTA_DST) == 0) {
				dest = *argv;
				ishost = getaddr(RTA_DST, *argv, &hp);
			} else if ((rtm_addrs & RTA_GATEWAY) == 0) {
				gateway = *argv;
				(void) getaddr(RTA_GATEWAY, *argv, &hp);
			} else {
				int ret = atoi(*argv);
				if (ret == 0) {
				    printf("%s,%s", "old usage of trailing 0",
					   "assuming route to if\n");
				    iflag = 1;
				    continue;
				} else if (ret > 0 && ret < 10) {
				    printf("old usage of trailing digit, ");
				    printf("assuming route via gateway\n");
				    iflag = 0;
				    continue;
				}
				(void) getaddr(RTA_NETMASK, *argv, 0);
			}
		}
	}
	if (forcehost)
		ishost = 1;
	if (forcenet)
		ishost = 0;
	flags = RTF_UP;
	if (ishost)
		flags |= RTF_HOST;
	if (iflag == 0)
		flags |= RTF_GATEWAY;
	for (attempts = 1; ; attempts++) {
		errno = 0;
		if (Cflag && (af == AF_INET || af == AF_NS)) {
			route.rt_flags = flags;
			route.rt_dst = so_dst.sa;
			route.rt_gateway = so_gate.sa;
			if ((ret = ioctl(s, *cmd == 'a' ? SIOCADDRT : SIOCDELRT,
			     (caddr_t)&route)) == 0)
				break;
		} else {
		    if ((ret = rtmsg(*cmd, flags)) == 0);
				break;
		}
		if (errno != ENETUNREACH && errno != ESRCH)
			break;
		if (hp && hp->h_addr_list[1]) {
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (caddr_t)&so_dst.sin.sin_addr,
			    hp->h_length);
		} else
			break;
	}
	oerrno = errno;
	printf("%s %s %s: gateway %s", cmd, ishost? "host" : "net",
		dest, gateway);
	if (attempts > 1 && ret == 0)
	    printf(" (%s)",
		inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
	if (ret == 0)
		printf("\n");
	else {
		printf(": ");
		fflush(stdout);
		errno = oerrno;
		error("");
	}
	exit(0);
}

error(cmd)
	char *cmd;
{
	extern int errno;

	switch(errno) {
	case ESRCH:
		printf("not in table\n");
		break;
	case EBUSY:
		printf("entry in use\n");
		break;
	case ENOBUFS:
		printf("routing table overflow\n");
		break;
	default:
		printf("ioctl returns %d\n", errno);
		perror(cmd);
	}
	fflush(stdout);
	errno = 0;
}

char *
savestr(s)
	char *s;
{
	char *sav;

	sav = malloc(strlen(s) + 1);
	if (sav == NULL) {
		fprintf("route: out of memory\n");
		exit(1);
	}
	strcpy(sav, s);
	return (sav);
}

inet_makenetandmask(net, sin)
u_long net;
register struct sockaddr_in *sin;
{
	u_long addr;
	u_long mask = 0;
	register char *cp;

	rtm_addrs |= RTA_NETMASK;
	if (net == 0)
		mask = addr = 0;
	else if (net < 128) {
		addr = net << IN_CLASSA_NSHIFT;
		mask = IN_CLASSA_NET;
	} else if (net < 65536) {
		addr = net << IN_CLASSB_NSHIFT;
		mask = IN_CLASSB_NET;
	} else if (net < 16777216L) {
		addr = net << IN_CLASSC_NSHIFT;
		mask = IN_CLASSC_NET;
	} else {
		addr = net;
		if ((addr & IN_CLASSA_HOST) == 0)
			mask =  IN_CLASSA_NET;
		else if ((addr & IN_CLASSB_HOST) == 0)
			mask =  IN_CLASSB_NET;
		else if ((addr & IN_CLASSC_HOST) == 0)
			mask =  IN_CLASSC_NET;
		else
			mask = -1;
	}
	sin->sin_addr.s_addr = htonl(addr);
	sin = &so_mask.sin;
	sin->sin_addr.s_addr = htonl(mask);
	sin->sin_len = 0;
	sin->sin_family = 0;
	cp = (char *)(1 + &(sin->sin_addr));
	while (*--cp == 0 && cp > (char *)sin)
		;
	sin->sin_len = 1 + cp - (char *)sin;
}

/*
 * Interpret an argument as a network address of some kind,
 * returning 1 if a host address, 0 if a network address.
 */
getaddr(which, s, hpp)
	char *s;
	struct hostent **hpp;
{
	register union sockunion *su;
	struct	ns_addr ns_addr();
	struct iso_addr *iso_addr();
	struct hostent *hp;
	struct netent *np;
	u_long val;

	if (af == 0) {
		af = AF_INET;
		aflen = sizeof(struct sockaddr_in);
	}
	rtm_addrs |= which;
	switch (which) {
	case RTA_DST:		su = so_addrs[0]; su->sa.sa_family = af; break;
	case RTA_GATEWAY:	su = so_addrs[1]; su->sa.sa_family = af; break;
	case RTA_NETMASK:	su = so_addrs[2]; break;
	case RTA_GENMASK:	su = so_addrs[3]; break;
	default:		usage("Internal Error"); /*NOTREACHED*/
	}
	su->sa.sa_len = aflen;
	if (strcmp(s, "default") == 0) {
		switch (which) {
		case RTA_DST:
			forcenet++;
			getaddr(RTA_NETMASK, s, 0);
			break;
		case RTA_NETMASK:
		case RTA_GENMASK:
			su->sa.sa_len = 0;
		}
		return 0;
	}
	if (af == AF_NS)
		goto do_xns;
	if (af == AF_OSI)
		goto do_osi;
	if (af == AF_LINK)
		goto do_link;
	if (hpp == 0) hpp = &hp;
	*hpp = 0;
	if (((val = inet_addr(s)) != -1) &&
	    (which != RTA_DST || forcenet == 0)) {
		su->sin.sin_addr.s_addr = val;
		if (inet_lnaof(su->sin.sin_addr) != INADDR_ANY)
			return (1);
		else {
			val = ntohl(val);
		out:	if (which == RTA_DST)
				inet_makenetandmask(val, &su->sin);
			return (0);
		}
	}
	val = inet_network(s);
	if (val != -1) {
		goto out;
	}
	np = getnetbyname(s);
	if (np) {
		val = np->n_net;
		goto out;
	}
	hp = gethostbyname(s);
	if (hp) {
		*hpp = hp;
		su->sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr, &su->sin.sin_addr, hp->h_length);
		return (1);
	}
	fprintf(stderr, "%s: bad value\n", s);
	exit(1);
do_xns:
	if (val == 0)
		return(0);
	if (which == RTA_DST) {
		extern short ns_bh[3];
		struct sockaddr_ns *sms = &(so_mask.sns);
		bzero((char *)sms, sizeof(*sms));
		sms->sns_family = 0;
		sms->sns_len = 6;
		sms->sns_addr.x_net = *(union ns_net *)ns_bh;
		rtm_addrs |= RTA_NETMASK;
	}
	su->sns.sns_addr = ns_addr(s);
	return (!ns_nullhost(su->sns.sns_addr));
do_osi:
	su->siso.siso_addr = *iso_addr(s);
	if (which == RTA_NETMASK || which == RTA_GENMASK) {
		register char *cp = (char *)TSEL(&su->siso);
		su->siso.siso_nlen = 0;
		do {--cp ;} while ((cp > (char *)su) && (*cp == 0));
		su->siso.siso_len = 1 + cp - (char *)su;
	}
	return (1);
do_link:
	link_addr(s, &su->sdl);
	return (1);
}

short ns_nullh[] = {0,0,0};
short ns_bh[] = {-1,-1,-1};

char *
ns_print(sns)
struct sockaddr_ns *sns;
{
	struct ns_addr work;
	union { union ns_net net_e; u_long long_e; } net;
	u_short port;
	static char mybuf[50], cport[10], chost[25];
	char *host = "";
	register char *p; register u_char *q; u_char *q_lim;

	work = sns->sns_addr;
	port = ntohs(work.x_port);
	work.x_port = 0;
	net.net_e  = work.x_net;
	if (ns_nullhost(work) && net.long_e == 0) {
		if (port ) {
			(void)sprintf(mybuf, "*.%xH", port);
			upHex(mybuf);
		} else
			(void)sprintf(mybuf, "*.*");
		return (mybuf);
	}

	if (bcmp(ns_bh, work.x_host.c_host, 6) == 0) { 
		host = "any";
	} else if (bcmp(ns_nullh, work.x_host.c_host, 6) == 0) {
		host = "*";
	} else {
		q = work.x_host.c_host;
		(void)sprintf(chost, "%02x%02x%02x%02x%02x%02xH",
			q[0], q[1], q[2], q[3], q[4], q[5]);
		for (p = chost; *p == '0' && p < chost + 12; p++);
		host = p;
	}
	if (port)
		(void)sprintf(cport, ".%xH", htons(port));
	else
		*cport = 0;

	(void)sprintf(mybuf,"%xH.%s%s", ntohl(net.long_e), host, cport);
	upHex(mybuf);
	return(mybuf);
}

upHex(p0)
char *p0;
{
	register char *p = p0;
	for (; *p; p++) switch (*p) {

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		*p += ('A' - 'a');
	}
}

monitor()
{
	int n;
	char msg[2048];
	verbose = 1;
	for(;;) {
		n = read(s, msg, 2048);
		printf("got message of size %d\n", n);
		print_rtmsg((struct rt_msghdr *)msg);
	}
}

struct {
	struct	rt_msghdr m_rtm;
	char m_space[512];
} m_rtmsg;

rtmsg(cmd, flags)
{
	static int seq;
	int rlen;
	extern int errno;
	register char *cp = m_rtmsg.m_space;
	register int l;

	errno = 0;
	bzero((char *)&m_rtmsg, sizeof(m_rtmsg));
	if (cmd == 'a')
		cmd = RTM_ADD;
	else if (cmd == 'c')
		cmd = RTM_CHANGE;
	else
		cmd = RTM_DELETE;
	m_rtmsg.m_rtm.rtm_flags = flags;
	m_rtmsg.m_rtm.rtm_version = RTM_VERSION;
	m_rtmsg.m_rtm.rtm_seq = ++seq;
	m_rtmsg.m_rtm.rtm_addrs = rtm_addrs;

#define ROUND(a) (1 + (((a) - 1) | (sizeof(long) - 1)))
#define NEXTADDR(w, u) { if (rtm_addrs & (w)) {l = (u).sa.sa_len;\
	if(verbose)sodump(&(u),"u");if(l == 0) l = sizeof(int); l = ROUND(l);\
		bcopy((char *)&(u), cp, l); cp += l;}}

	NEXTADDR(RTA_DST, so_dst);
	NEXTADDR(RTA_GATEWAY, so_gate);
	NEXTADDR(RTA_NETMASK, so_mask);
	NEXTADDR(RTA_GENMASK, so_genmask);
	m_rtmsg.m_rtm.rtm_msglen = l = cp - (char *)&m_rtmsg;
	m_rtmsg.m_rtm.rtm_type = cmd;
	if (verbose)
		print_rtmsg(&m_rtmsg.m_rtm, l);
	if ((rlen = write(s, (char *)&m_rtmsg, l)) < 0) {
		perror("writing to routing socket");
		printf("got only %d for rlen\n", rlen);
		return (-1);
	}
again:
	if ((rlen = read(s, (char *)&m_rtmsg, l)) < 0) {
		perror("reading from routing socket");
		printf("got only %d for rlen\n", rlen);
		return (-1);
	}
	if ((m_rtmsg.m_rtm.rtm_pid != pid) ||
	    (m_rtmsg.m_rtm.rtm_seq != seq)) {
		printf("Got response for somebody else's request");
		goto again;
	}
	if (qflag == 0)
		print_rtmsg( &m_rtmsg.m_rtm, rlen);
	if ((m_rtmsg.m_rtm.rtm_flags & RTF_DONE) == 0) {
		errno = m_rtmsg.m_rtm.rtm_errno;
		perror("response from routing socket turned down");
		return (-1);
	}
	return (0);
}

char *msgtypes[] = {
"",
"RTM_ADD: Add Route",
"RTM_DELETE: Delete Route",
"RTM_CHANGE: Change Metrics or flags",
"RTM_GET: Report Metrics",
"RTM_LOSING: Kernel Suspects Partitioning",
"RTM_REDIRECT: Told to use different route",
"RTM_MISS: Lookup failed on this address",
"RTM_LOCK: fix specified metrics",
"RTM_OLDADD: caused by SIOCADDRT",
"RTM_OLDDEL: caused by SIOCDELRT",
0, };

char metricnames[] =
"\010rttvar\7rtt\6ssthresh\7sendpipe\4recvpipe\3expire\2hopcount\1mtu";

#define ROUNDUP(a) ((char *)(1 + (((((int)a)) - 1) | (sizeof(long) - 1))))

print_rtmsg(rtm, n)
register struct rt_msghdr *rtm;
{
	char *cp;
	register struct sockaddr *sa;
	int i;

	if (verbose == 0)
		return;
	if (rtm->rtm_version != RTM_VERSION) {
	    printf("routing message version %d not understood\n",
							rtm->rtm_version);
	} else {
	    printf("%s\npid: %d, len %d, seq %d, errno %d, flags:",
		    msgtypes[rtm->rtm_type], rtm->rtm_pid, rtm->rtm_msglen,
		    rtm->rtm_seq, rtm->rtm_errno); 
	    bprintf(stdout, rtm->rtm_flags,
		"\1UP\2GATEWAY\3HOST\5DYNAMIC\6MODIFIED\7DONE\010MASK_PRESENT");
	    printf("\nlocks: "); bprintf(stdout, rtm->rtm_rmx.rmx_locks, metricnames);
	    printf(" inits: "); bprintf(stdout, rtm->rtm_inits, metricnames);
	    printf("\nsockaddrs: ");
	    bprintf(stdout, rtm->rtm_addrs,
		"\1DST\2GATEWAY\3NETMASK\4GENMASK\5IFP\6IFA\7AUTHOR");
	    putchar('\n');
	    cp = ((char *)(rtm + 1));
	    if (rtm->rtm_addrs)
		for (i = 1; i; i <<= 1)
		    if (i & rtm->rtm_addrs) {
			    sa = (struct sockaddr *)cp;
			    printf(" %s", routename(sa));
			    cp = ROUNDUP(cp + sa->sa_len);
		    }
	    putchar('\n');
	}
	fflush(stdout);
}

bprintf(fp, b, s)
register FILE *fp;
register int b;
register u_char *s;
{
	register int i;
	int gotsome = 0;

	if (b == 0)
		return;
	while (i = *s++) {
		if (b & (1 << (i-1))) {
			if (gotsome == 0) i = '<'; else i = ',';
			putc(i, fp);
			gotsome = 1;
			for (; (i = *s) > 32; s++)
				putc(i, fp);
		} else
			while (*s > 32)
				s++;
	}
	if (gotsome)
		putc('>', fp);
}
int
keyword(cp)
char *cp;
{
	register struct keytab *kt = keywords;
	while (kt->kt_cp && strcmp(kt->kt_cp, cp))
		kt++;
	return kt->kt_i;
}

sodump(su, which)
register union sockunion *su;
char *which;
{
	switch (su->sa.sa_family) {
	case AF_LINK:
		printf("%s: link %s; ", which, link_ntoa(&su->sdl));
		break;
	case AF_ISO:
		printf("%s: iso %s; ", which, iso_ntoa(&su->siso.siso_addr));
		break;
	case AF_INET:
		printf("%s: inet %s; ", which, inet_ntoa(su->sin.sin_addr));
		break;
	case AF_NS:
		printf("%s: xns %s; ", which, ns_ntoa(&su->sns.sns_addr));
		break;
	}
	fflush(stdout);
}

/*
 * Copyright (c) 1990 Regents of the University of California.
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
static char sccsid[] = "@(#)linkaddr.c	5.1 (Berkeley) 4/1/90";
#endif /* LIBC_SCCS and not lint */

#ifndef AF_LINK
#include <sys/param.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#endif
/* States*/
#define NAMING	0
#define GOTONE	1
#define GOTTWO	2
#define RESET	3
/* Inputs */
#define	DIGIT	(4*0)
#define	END	(4*1)
#define DELIM	(4*2)
#define LETTER	(4*3)

link_addr(addr, sdl)
register char *addr;
register struct sockaddr_dl *sdl;
{
	register char *cp = sdl->sdl_data;
	char *cplim = sdl->sdl_len + (char *)sdl;
	register int byte = 0, state = NAMING, new;

	bzero((char *)&sdl->sdl_family, sdl->sdl_len - 1);
	sdl->sdl_family = AF_LINK;
	do {
		state &= ~LETTER;
		if ((*addr >= '0') && (*addr <= '9')) {
			new = *addr - '0';
		} else if ((*addr >= 'a') && (*addr <= 'f')) {
			new = *addr - 'a' + 10;
		} else if ((*addr >= 'A') && (*addr <= 'F')) {
			new = *addr - 'A' + 10;
		} else if (*addr == 0) {
			state |= END;
		} else if (state == NAMING &&
			   (((*addr >= 'A') && (*addr <= 'Z')) ||
			   ((*addr >= 'a') && (*addr <= 'z'))))
			state |= LETTER;
		else
			state |= DELIM;
		addr++;
		switch (state /* | INPUT */) {
		case NAMING | DIGIT:
		case NAMING | LETTER:
			*cp++ = addr[-1]; continue;
		case NAMING | DELIM:
			state = RESET; sdl->sdl_nlen = cp - sdl->sdl_data; continue;
		case GOTTWO | DIGIT:
			*cp++ = byte; /*FALLTHROUGH*/
		case RESET | DIGIT:
			state = GOTONE; byte = new; continue;
		case GOTONE | DIGIT:
			state = GOTTWO; byte = new + (byte << 4); continue;
		default: /* | DELIM */
			state = RESET; *cp++ = byte; byte = 0; continue;
		case GOTONE | END:
		case GOTTWO | END:
			*cp++ = byte; /* FALLTHROUGH */
		case RESET | END:
			break;
		}
		break;
	} while (cp < cplim); 
	sdl->sdl_alen = cp - LLADDR(sdl);
	new = cp - (char *)sdl;
	if (new > sizeof(*sdl))
		sdl->sdl_len = new;
	return;
}
static char hexlist[] = "0123456789abcdef";

char *
link_ntoa(sdl)
register struct sockaddr_dl *sdl;
{
	static char obuf[64];
	register char *out = obuf; 
	register int i;
	register u_char *in = (u_char *)LLADDR(sdl);
	u_char *inlim = in + sdl->sdl_nlen;
	int firsttime = 1;

	if (sdl->sdl_nlen) {
		bcopy(sdl->sdl_data, obuf, sdl->sdl_nlen);
		out += sdl->sdl_nlen;
		*out++ = ':';
	}
	while (in < inlim) {
		if (firsttime) firsttime = 0; else *out++ = '.';
		i = *in++;
		if (i > 0xf) {
			out[1] = hexlist[i & 0xf];
			i >>= 4;
			out[0] = hexlist[i];
			out += 2;
		} else
			*out++ = hexlist[i];
	}
	*out = 0;
	return(obuf);
}
