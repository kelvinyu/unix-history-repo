#ifndef lint
static char sccsid[] = "@(#)inet.c	4.5 82/11/14";
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>

#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_seq.h>
#define TCPSTATES
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_debug.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <netdb.h>

struct	inpcb inpcb;
struct	tcpcb tcpcb;
struct	socket socket;
struct	protosw proto;
extern	int kmem;
extern	int Aflag;
extern	int aflag;
extern	int nflag;

static	int first = 1;
char	*inetname();

/*
 * Print a summary of connections related to an Internet
 * protocol.  For TCP, also give state of connection.
 * Listening processes (aflag) are suppressed unless the
 * -a (all) flag is specified.
 */
protopr(off, name)
	off_t off;
	char *name;
{
	struct inpcb cb;
	register struct inpcb *prev, *next;
	int istcp;

	if (off == 0) {
		printf("%s control block: symbol not in namelist\n", name);
		return;
	}
	istcp = strcmp(name, "tcp") == 0;
	klseek(kmem, off, 0);
	read(kmem, &cb, sizeof (struct inpcb));
	inpcb = cb;
	prev = (struct inpcb *)off;
	if (first) {
		printf("Active connections");
		if (aflag)
			printf(" (including servers)");
		putchar('\n');
		if (Aflag)
			printf("%-8.8s ", "PCB");
		printf("%-5.5s %-6.6s %-6.6s  %-18.18s %-18.18s %s\n",
			"Proto", "Recv-Q", "Send-Q",
			"Local Address", "Foreign Address", "(state)");
		first = 0;
	}
	while (inpcb.inp_next != (struct inpcb *)off) {
		char *cp;

		next = inpcb.inp_next;
		klseek(kmem, (off_t)next, 0);
		read(kmem, &inpcb, sizeof (inpcb));
		if (inpcb.inp_prev != prev) {
			printf("???\n");
			break;
		}
		if (!aflag &&
		  inet_lnaof(inpcb.inp_laddr.s_addr) == INADDR_ANY) {
			prev = next;
			continue;
		}
		klseek(kmem, (off_t)inpcb.inp_socket, 0);
		read(kmem, &socket, sizeof (socket));
		if (istcp) {
			klseek(kmem, (off_t)inpcb.inp_ppcb, 0);
			read(kmem, &tcpcb, sizeof (tcpcb));
		}
		if (Aflag)
			printf("%8x ", inpcb.inp_ppcb);
		printf("%-5.5s %6d %6d ", name, socket.so_rcv.sb_cc,
			socket.so_snd.sb_cc);
		inetprint(&inpcb.inp_laddr, inpcb.inp_lport, name);
		inetprint(&inpcb.inp_faddr, inpcb.inp_fport, name);
		if (istcp) {
			if (tcpcb.t_state < 0 || tcpcb.t_state >= TCP_NSTATES)
				printf(" %d", tcpcb.t_state);
			else
				printf(" %s", tcpstates[tcpcb.t_state]);
		}
		putchar('\n');
		prev = next;
	}
}

/*
 * Pretty print an Internet address (net address + port).
 * If the nflag was specified, use numbers instead of names.
 */
inetprint(in, port, proto)
	register struct in_addr *in;
	int port;
	char *proto;
{
	struct servent *sp = 0;
	char line[80], *cp, *index();

	sprintf(line, "%.10s.", inetname(*in));
	cp = index(line, '\0');
#if vax || pdp11
	port = ntohs((u_short)port);
#endif
	if (!nflag && port)
		sp = getservbyport(port, proto);
	if (sp || port == 0)
		sprintf(cp, "%.8s", sp ? sp->s_name : "*");
	else
		sprintf(cp, "%d", port);
	printf(" %-18.18s", line);
}

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give 
 * numeric value, otherwise try for symbolic name.
 */
char *
inetname(in)
	struct in_addr in;
{
	char *cp = 0;
	static char line[50];

	if (!nflag) {
		if (inet_lnaof(in) == INADDR_ANY) {
			struct netent *np;

			np = getnetbyaddr(inet_netof(in), AF_INET);
			if (np)
				cp = np->n_name;
		} else {
			struct hostent *hp;

			hp = gethostbyaddr(&in, sizeof (struct in_addr),
				AF_INET);
			if (hp)
				cp = hp->h_name;
		}
	}
	if (in.s_addr == INADDR_ANY)
		strcpy(line, "*");
	else if (cp)
		strcpy(line, cp);
	else {
		u_char *ucp = (u_char *)&in;
		sprintf(line, "%u.%u.%u.%u", ucp[0], ucp[1], ucp[2], ucp[3]);
	}
	return (line);
}
