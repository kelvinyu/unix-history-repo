#ifndef lint
static char rcsid[] = "$Header$";
#endif

#include "defs.h"

/*
 * Address family support routines
 */
int	null_hash(), null_netmatch(), null_output(),
	null_portmatch(), null_portcheck(),
	null_checkhost(), null_ishost(), null_canon();
int	xnnet_hash(), xnnet_netmatch(), xnnet_output(),
	xnnet_portmatch();
	xnnet_checkhost(), xnnet_ishost(), xnnet_canon();
#define NIL \
	{ null_hash,		null_netmatch,		null_output, \
	  null_portmatch,	null_portcheck,		null_checkhost, \
	  null_ishost,		null_canon }
#define	XNSNET \
	{ xnnet_hash,		xnnet_netmatch,		xnnet_output, \
	  xnnet_portmatch,	xnnet_portmatch,	xnnet_checkhost, \
	  xnnet_ishost,		xnnet_canon }

struct afswitch afswitch[AF_MAX] =
	{ NIL, NIL, NIL, NIL, NIL, NIL, XNSNET, NIL, NIL, NIL, NIL };

struct sockaddr_ns xnnet_default = { AF_NS };

xnnet_hash(sns, hp)
	register struct sockaddr_ns *sns;
	struct afhash *hp;
{
	register long hash = 0;
	register u_short *s = sns->sns_addr.x_host.s_host;
	hp->afh_nethash = xnnet(sns->sns_addr.x_net);
	hash = *s++; hash <<= 8; hash += *s++; hash <<= 8; hash += *s;
	hp->afh_hosthash = hash;
}

xnnet_netmatch(sxn1, sxn2)
	struct sockaddr_ns *sxn1, *sxn2;
{

	return (xnnet(sxn1->sns_addr.x_net) == xnnet(sxn2->sns_addr.x_net));
}

/*
 * Verify the message is from the right port.
 */
xnnet_portmatch(sns)
	register struct sockaddr_ns *sns;
{
	
	return (ntohs(sns->sns_addr.x_port) == IDPPORT_RIF );
}


/*
 * xns output routine.
 */
#ifdef DEBUG
int do_output = 0;
#endif
xnnet_output(flags, sns, size)
	int flags;
	struct sockaddr_ns *sns;
	int size;
{
	struct sockaddr_ns dst;

	dst = *sns;
	sns = &dst;
	if (sns->sns_addr.x_port == 0)
		sns->sns_addr.x_port = htons(IDPPORT_RIF);
#ifdef DEBUG
	if(do_output || ntohs(msg->rip_cmd) == RIPCMD_REQUEST)
#endif	
	if (sendto(s, msg, size, flags, sns, sizeof (*sns)) < 0)
		perror("sendto");
}

/*
 * Return 1 if the address is believed
 *  -- THIS IS A KLUDGE.
 */
xnnet_checkhost(sns)
	struct sockaddr_ns *sns;
{
	return (1);
}

/*
 * Return 1 if the address is
 * for a host, 0 for a network.
 */
xnnet_ishost(sns)
struct sockaddr_ns *sns;
{
	register u_short *s = sns->sns_addr.x_host.s_host;

	if ((s[0]==0xffff) && (s[1]==0xffff) && (s[2]==0xffff))
		return (0);
	else
		return (1);
}

xnnet_canon(sns)
	struct sockaddr_ns *sns;
{

	sns->sns_addr.x_port = 0;
}

/*ARGSUSED*/
null_hash(addr, hp)
	struct sockaddr *addr;
	struct afhash *hp;
{

	hp->afh_nethash = hp->afh_hosthash = 0;
}

/*ARGSUSED*/
null_netmatch(a1, a2)
	struct sockaddr *a1, *a2;
{

	return (0);
}

/*ARGSUSED*/
null_output(s, f, a1, n)
	int s, f;
	struct sockaddr *a1;
	int n;
{

	;
}

/*ARGSUSED*/
null_portmatch(a1)
	struct sockaddr *a1;
{

	return (0);
}

/*ARGSUSED*/
null_portcheck(a1)
	struct sockaddr *a1;
{

	return (0);
}

/*ARGSUSED*/
null_ishost(a1)
	struct sockaddr *a1;
{

	return (0);
}

/*ARGSUSED*/
null_checkhost(a1)
	struct sockaddr *a1;
{

	return (0);
}

/*ARGSUSED*/
null_canon(a1)
	struct sockaddr *a1;
{

	;
}
