/* ip_output.c 1.1 81/10/14 */
#include "../h/param.h"
#include "../bbnnet/net.h"
#include "../bbnnet/tcp.h"
#include "../bbnnet/ip.h"
#include "../bbnnet/imp.h"
#include "../bbnnet/ucb.h"

/*****************************************************************************
*                                                                            *
*         internet level output:  called from higher level protocol          *
*         or "raw internet driver."  passed a pointer to an mbuf             *
*         chain containing the message to be sent, a partially filled        *
*         in ip leader, and room for an 1822 leader and 2 pointers.          *
*         this routine does fragmentation and mapping of ip parameters       *
*         to 1822 ones.                                                      *
*                                                                            *
*****************************************************************************/

ip_output(mp)
struct mbuf *mp;
{
	register i, rnd;
	register struct mbuf *m, *n;
	register struct ip *p;
	struct mbuf *mm;
	int hlen, adj, max, len, off;

COUNT(IP_OUTPUT);
	p = (struct ip *)((int)mp + mp->m_off); /* -> ip header */
	hlen = sizeof(struct ip);               /* header length */

	/* fill in unspecified fields and byte swap others */

	p->ip_v = IPVERSION;    
	p->ip_hl = hlen >> 2;
	p->ip_off = 0 | (p->ip_off & ip_df);
	p->ip_ttl = MAXTTL;
	p->ip_id = netcb.n_ip_cnt++;

	if (p->ip_len > MTU) {          /* must fragment */
		if (p->ip_off & ip_df)
			return(FALSE);
		max = MTU - hlen;       /* maximum data length in fragment */
		len = p->ip_len - hlen; /* data length */
		off = 0;                /* fragment offset */
		m = mp;

		while (len > 0) {

			/* correct the header */

			p->ip_off |= off >> 3;

			/* find the end of the fragment */

			i = -hlen;
			while (m != NULL) {
				i += m->m_len;
				if (i > max)
					break;
				n = m;
				m = m->m_next;
			}

			if (i < max || m == NULL) {     /* last fragment */
				p->ip_off = p->ip_off & ~ip_mf;
				p->ip_len = i + hlen;
				break;

			} else {                        /* more fragments */

				/* allocate header mbuf for next fragment */

				if ((mm = m_get(1)) == NULL)    /* no more bufs */
					return(FALSE);

				p->ip_off |= ip_mf;

				/* terminate fragment at 8 byte boundary (round down) */

				i -= m->m_len;
        			rnd = i & ~7;           /* fragment length */
				adj = i - rnd;          /* leftover in mbuf */
				p->ip_len = rnd + hlen; 

				/* setup header for next fragment and 
				   append remaining fragment data */

				n->m_next = NULL;                   
				mm->m_next = m;        
				m = mm;
				m->m_off = MSIZE - hlen - adj;
				m->m_len = hlen + adj;

				/* copy old header to new */

				bcopy(p, (caddr_t)((int)m + m->m_off), hlen);

				/* copy leftover data from previous frag */

				if (adj) {
					n->m_len -= adj;
					bcopy((caddr_t)((int)n + n->m_len + n->m_off),
					      (caddr_t)((int)m + m->m_off + hlen), adj);
				}
			}

			ip_send(p);             /* pass frag to local net level */

			p = (struct ip *)((int)m + m->m_off);   /* -> new hdr */
			len -= rnd;
			off += rnd;
		}
	} 

	return(ip_send(p));     /* pass datagram to local net level */
}

ip_send(p)      /* format header and send message to 1822 level */
struct ip *p;
{
	register struct mbuf *m;
	register struct imp *l;
COUNT(IP_SEND);

	m = dtom(p);                    /* ->header mbuf */

	/* set up 1822 leader fields for transmit */

	l = (struct imp *)((int)m + m->m_off - L1822);
/*
	l->i_hst = p->ip_dst.s_host;
	l->i_impno = p->ip_dst.s_imp;
	l->i_mlen = p->ip_len + L1822;
	l->i_link = IPLINK;
	l->i_type = 0;
	l->i_htype = 0;
	l->i_stype = 0;
*/
	l->i_shost = arpa_ether(p->ip_src.s_host);
	l->i_dhost = arpa_ether(p->ip_dst.s_host);
	l->i_type = IPTYPE;

	/* finish ip leader by calculating checksum and doing
	   necessary byte-swapping  */

	p->ip_sum = 0;
 	ip_bswap(p);
	p->ip_sum = cksum(m, sizeof(struct ip));     

	m->m_off -= L1822;              /* -> 1822 leader */
	m->m_len += L1822;

	return(imp_snd(m));             /* pass frag to 1822 */
}

ip_setup(up, m, len)            /* setup an ip header for raw write */
register struct ucb *up;
register struct mbuf *m;
int len;
{
	register struct ip *ip;
COUNT(IP_SETUP);

	m->m_off = MSIZE - sizeof(struct ip);
	m->m_len = sizeof(struct ip);

	ip = (struct ip *)((int)m + m->m_off);

	ip->ip_tos = 0;
	ip->ip_id = 0;
	ip->ip_off = 0;
	ip->ip_p = up->uc_lolink;
	ip->ip_len = len + sizeof(struct ip);

	ip->ip_src.s_addr = netcb.n_lhost.s_addr;
        ip->ip_dst.s_addr = up->uc_host->h_addr.s_addr;
}


/*
 * Convert logical host on imp to ethernet address.
 * (Primitive; use gateway table in next version.)
 *	0/78	arpavax		253
 *	1/78	ucb-c70		-
 *	-	csvax		252
 *	-	ucb-comet	223
 */
arpa_ether(n)
	int n;
{
COUNT(ARPA_ETHER);
	switch (n) {
	case 0:			/* arpavax */
		return 253;
	default:
		return n;
	}
}
