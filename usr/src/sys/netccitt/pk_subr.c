/*
 * Copyright (c) University of British Columbia, 1984
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Laboratory for Computation Vision and the Computer Science Department
 * of the University of British Columbia.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)pk_subr.c	7.3 (Berkeley) %G%
 */

#include "param.h"
#include "systm.h"
#include "mbuf.h"
#include "socket.h"
#include "protosw.h"
#include "socketvar.h"
#include "errno.h"
#include "time.h"
#include "kernel.h"

#include "x25.h"
#include "pk.h"
#include "pk_var.h"
#include "x25err.h"

int     pk_sendspace = 1024 * 2 + 8;
int     pk_recvspace = 1024 * 2 + 8;

struct	x25_packet *pk_template ();

/* 
 *  Attach X.25 protocol to socket, allocate logical channel descripter
 *  and buffer space, and enter LISTEN state if we are to accept
 *  IN-COMMING CALL packets.  
 *
 */

pk_attach (so)
struct socket *so;
{
	register struct pklcd *lcp;
	register struct mbuf *m;
	register int error;

	if (error = soreserve (so, pk_sendspace, pk_recvspace))
		return (error);

	/* Hopefully we can remove this when SEQ_PKT is available (4.3?) */
	so -> so_snd.sb_mbmax = pk_sendspace;

	if ((m = m_getclr (M_DONTWAIT, MT_PCB)) == 0)
		return (ENOBUFS);
	lcp = mtod (m, struct pklcd *);
	so -> so_pcb = (caddr_t) lcp;
	lcp -> lcd_so = so;

	if (so -> so_options & SO_ACCEPTCONN)
		lcp -> lcd_state = LISTEN;
	else
		lcp -> lcd_state = READY;

	return (0);
}

/* 
 *  Disconnect X.25 protocol from socket.
 */

pk_disconnect (lcp)
register struct pklcd *lcp;
{
	register struct socket *so = lcp -> lcd_so;
	register struct pklcd *l, *p;

	switch (lcp -> lcd_state) {
	case LISTEN: 
		for (p = 0, l = pk_listenhead; l && l != lcp; p = l, l = l -> lcd_listen);
		if (p == 0) {
			if (l != 0)
				pk_listenhead = l -> lcd_listen;
		}
		else
		if (l != 0)
			p -> lcd_listen = l -> lcd_listen;
		pk_close (lcp);
		break;

	case READY: 
		pk_acct (lcp);
		pk_close (lcp);
		break;

	case SENT_CLEAR: 
	case RECEIVED_CLEAR: 
		break;

	default: 
		pk_acct (lcp);
		if (so) {
			soisdisconnecting (so);
			sbflush (&so -> so_rcv);
		}
		pk_clear (lcp);

	}
}

/* 
 *  Close an X.25 Logical Channel. Discard all space held by the
 *  connection and internal descriptors. Wake up any sleepers.
 */

pk_close (lcp)
struct pklcd *lcp;
{
	register struct socket *so = lcp -> lcd_so;

	pk_freelcd (lcp);

	if (so == NULL)
		return;

	so -> so_pcb = 0;
	sbflush (&so -> so_snd);
	sbflush (&so -> so_rcv);
	soisdisconnected (so);
	sofree (so);	/* gak!!! you can't do that here */
}

/* 
 *  Create a template to be used to send X.25 packets on a logical
 *  channel. It allocates an mbuf and fills in a skeletal packet
 *  depending on its type. This packet is passed to pk_output where
 *  the remainer of the packet is filled in.
*/

struct x25_packet *
pk_template (lcn, type)
int lcn, type;
{
	register struct mbuf *m;
	register struct x25_packet *xp;

	MGET (m, M_DONTWAIT, MT_HEADER);
	if (m == 0)
		panic ("pk_template");
	m -> m_act = 0;

	/*
	 * Efficiency hack: leave a four byte gap at the beginning
	 * of the packet level header with the hope that this will
	 * be enough room for the link level to insert its header.
	 */
	m -> m_data += 4;
	m -> m_len = PKHEADERLN;

	xp = mtod (m, struct x25_packet *);
	*(long *)xp = 0;		/* ugly, but fast */
/*	xp -> q_bit = 0;*/
	xp -> fmt_identifier = 1;
/*	xp -> lc_group_number = 0;*/

	xp -> logical_channel_number = lcn;
	xp -> packet_type = type;

	return (xp);
}

/* 
 *  This routine restarts all the virtual circuits. Actually,
 *  the virtual circuits are not "restarted" as such. Instead,
 *  any active switched circuit is simply returned to READY
 *  state.
 */

pk_restart (pkp, restart_cause)
register struct pkcb *pkp;
int restart_cause;
{
	register struct x25_packet *xp;
	register struct pklcd *lcp;
	register int i;

	/* Restart all logical channels. */
	if (pkp->pk_chan == 0)
		return;
	for (i = 1; i <= pkp->pk_maxlcn; ++i)
		if ((lcp = pkp->pk_chan[i]) != NULL) {
			if (lcp -> lcd_so)
				lcp->lcd_so -> so_error = ENETRESET;
			pk_close (lcp);
		}

	if (restart_cause < 0)
		return;

	pkp->pk_state = DTE_SENT_RESTART;
	lcp = pkp->pk_chan[0];
	xp = lcp -> lcd_template = pk_template (lcp -> lcd_lcn, X25_RESTART);
	(dtom (xp)) -> m_len++;
	xp -> packet_data = 0;	/* DTE only */
	pk_output (lcp);
}


/* 
 *  This procedure frees up the Logical Channel Descripter.
 */

static
pk_freelcd (lcp)
register struct pklcd *lcp;
{
	if (lcp == NULL)
		return;

	if (lcp -> lcd_template)
		m_freem (dtom (lcp -> lcd_template));

	if (lcp -> lcd_craddr)
		m_freem (dtom (lcp -> lcd_craddr));

	if (lcp -> lcd_ceaddr)
		m_freem (dtom (lcp -> lcd_ceaddr));

	if (lcp -> lcd_lcn > 0)
		lcp -> lcd_pkp -> pk_chan[lcp -> lcd_lcn] = NULL;

	m_freem (dtom (lcp));
}


/* 
 *  Bind a address and protocol value to a socket.  The important
 *  part is the protocol value - the first four characters of the 
 *  Call User Data field.
 */

pk_bind (lcp, nam)
struct pklcd *lcp;
struct mbuf *nam;
{
	register struct sockaddr_x25 *sa;
	register struct pkcb *pkp;
	register struct mbuf *m;
	register struct pklcd *pp;

	if (nam == NULL)
		return (EADDRNOTAVAIL);
	if (lcp -> lcd_ceaddr)				/* XXX */
		return (EADDRINUSE);
	if (checksockaddr (nam))
		return (EINVAL);
	sa = mtod (nam, struct sockaddr_x25 *);

	/*
	 * If the user wishes to accept calls only from a particular
	 * net (net != 0), make sure the net is known
	 */

	if (sa -> x25_net)
		for (pkp = pkcbhead; ; pkp = pkp -> pk_next) {
			if (pkp == 0)
				return (ENETUNREACH);
			if (pkp -> pk_xcp -> xc_net == sa -> x25_net)
				break;
		}

	for (pp = pk_listenhead; pp; pp = pp -> lcd_listen)
		if (bcmp (pp -> lcd_ceaddr -> x25_udata, sa -> x25_udata,
			min (pp->lcd_ceaddr->x25_udlen, sa->x25_udlen)) == 0)
			return (EADDRINUSE);

	if ((m = m_copy (nam, 0, (int)M_COPYALL)) == 0)
		return (ENOBUFS);
	lcp -> lcd_ceaddr = mtod (m, struct sockaddr_x25 *);
	return (0);
}

/*
 * Associate a logical channel descriptor with a network.
 * Fill in the default network specific parameters and then
 * set any parameters explicitly specified by the user or
 * by the remote DTE.
 */

pk_assoc (pkp, lcp, sa)
register struct pkcb *pkp;
register struct pklcd *lcp;
register struct sockaddr_x25 *sa;
{

	lcp -> lcd_pkp = pkp;
	lcp -> lcd_packetsize = pkp -> pk_xcp -> xc_psize;
	lcp -> lcd_windowsize = pkp -> pk_xcp -> xc_pwsize;
	lcp -> lcd_rsn = MODULUS - 1;
	pkp -> pk_chan[lcp -> lcd_lcn] = lcp;

	if (sa -> x25_opts.op_psize)
		lcp -> lcd_packetsize = sa -> x25_opts.op_psize;
	else
		sa -> x25_opts.op_psize = lcp -> lcd_packetsize;
	if (sa -> x25_opts.op_wsize)
		lcp -> lcd_windowsize = sa -> x25_opts.op_wsize;
	else
		sa -> x25_opts.op_wsize = lcp -> lcd_windowsize;
	sa -> x25_net = pkp -> pk_xcp -> xc_net;
	lcp -> lcd_flags = sa -> x25_opts.op_flags;
	lcp -> lcd_stime = time.tv_sec;
}

pk_connect (lcp, nam)
register struct pklcd *lcp;
struct mbuf *nam;
{
	register struct pkcb *pkp;
	register struct sockaddr_x25 *sa;
	register struct mbuf *m;

	if (checksockaddr (nam))
		return (EINVAL);
	sa = mtod (nam, struct sockaddr_x25 *);
	if (sa -> x25_addr[0] == '\0')
		return (EDESTADDRREQ);
	for (pkp = pkcbhead; ; pkp = pkp->pk_next) {
		if (pkp == 0)
			return (ENETUNREACH);
		/*
		 * use first net configured (last in list
		 * headed by pkcbhead) if net is zero
		 */
		if (sa -> x25_net == 0 && pkp -> pk_next == 0)
			break;
		if (sa -> x25_net == pkp -> pk_xcp -> xc_net)
			break;
	}

	if (pkp -> pk_state != DTE_READY)
		return (ENETDOWN);
	if ((lcp -> lcd_lcn = pk_getlcn (pkp)) == 0)
		return (EMFILE);
	if ((m = m_copy (nam, 0, (int)M_COPYALL)) == 0)
		return (ENOBUFS);
	lcp -> lcd_ceaddr = mtod (m, struct sockaddr_x25 *);
	pk_assoc (pkp, lcp, lcp -> lcd_ceaddr);
	if (lcp -> so)
		soisconnecting (lcp -> lcd_so);
	lcp -> lcd_template = pk_template (lcp -> lcd_lcn, X25_CALL);
	pk_callrequest (lcp, m, pkp -> pk_xcp);
	pk_output (lcp);
	return (0);
}

/* 
 *  Build the rest of the CALL REQUEST packet. Fill in calling
 *  address, facilities fields and the user data field.
 */

pk_callrequest (lcp, nam, xcp)
struct pklcd *lcp;
struct mbuf *nam;
register struct x25config *xcp;
{
	register struct x25_calladdr *a;
	register struct sockaddr_x25 *sa = mtod (nam, struct sockaddr_x25 *);
	register struct mbuf *m = dtom (lcp -> lcd_template);
	unsigned posn = 0;
	octet *cp;
	char addr[sizeof (xcp -> xc_ntn) * 2];

	a = (struct x25_calladdr *) &lcp -> lcd_template -> packet_data;
	a -> calling_addrlen = xcp -> xc_ntnlen;
	cp = (octet *) xcp -> xc_ntn;
	from_bcd (addr, &cp, xcp -> xc_ntnlen);
	a -> called_addrlen = strlen (sa -> x25_addr);
	cp = (octet *) a -> address_field;
	to_bcd (&cp, (int)a -> called_addrlen, sa -> x25_addr, &posn);
	to_bcd (&cp, (int)a -> calling_addrlen, addr, &posn);
	if (posn & 0x01)
		*cp++ &= 0xf0;

	build_facilities (&cp, sa, (int)xcp -> xc_type);

	bcopy (sa -> x25_udata, (caddr_t)cp, (unsigned)sa -> x25_udlen);
	cp += sa -> x25_udlen;

	m -> m_len += cp - (octet *) a;

#ifdef ANDREW
	printf ("call: ");
	for (cp = mtod (m, octet *), posn = 0; posn < m->m_len; ++posn)
		printf ("%x ", *cp++);
	printf ("\n");
#endif
}

build_facilities (cp, sa, type)
register octet **cp;
struct sockaddr_x25 *sa;
{
	register octet *fcp;
	register int revcharge;

	fcp = *cp + 1;
	revcharge = sa -> x25_opts.op_flags & X25_REVERSE_CHARGE ? 1 : 0;
	/*
	 * This is specific to Datapac X.25(1976) DTEs.  International
	 * calls must have the "hi priority" bit on.
	 */
	if (type == X25_1976 && sa -> x25_opts.op_psize == X25_PS128)
		revcharge |= 02;
	if (revcharge) {
		*fcp++ = FACILITIES_REVERSE_CHARGE;
		*fcp++ = revcharge;
	}
	switch (type) {
	case X25_1980:
	case X25_1984:
		*fcp++ = FACILITIES_PACKETSIZE;
		*fcp++ = sa -> x25_opts.op_psize;
		*fcp++ = sa -> x25_opts.op_psize;

		*fcp++ = FACILITIES_WINDOWSIZE;
		*fcp++ = sa -> x25_opts.op_wsize;
		*fcp++ = sa -> x25_opts.op_wsize;
	}
	**cp = fcp - *cp - 1;
	*cp = fcp;
}

to_bcd (a, len, x, posn)
register octet **a;
register char *x;
register int len;
register unsigned *posn;
{
	while (--len >= 0)
		if ((*posn)++ & 0x01)
			*(*a)++ |= *x++ & 0x0F;
		else
			**a = *x++ << 4;
}

/* 
 *  This routine gets the  first available logical channel number.  The
 *  search is from the highest number to lowest number (DTE).
 */

pk_getlcn (pkp)
register struct pkcb *pkp;
{
	register int i;

	if (pkp->pk_chan == 0)
		return (0);
	for (i = pkp -> pk_maxlcn; i > 0; --i)
		if (pkp -> pk_chan[i] == NULL)
			break;
	return (i);

}

static
checksockaddr (m)
struct mbuf *m;
{
	register struct sockaddr_x25 *sa = mtod (m, struct sockaddr_x25 *);
	register char *cp;

	if (m -> m_len != sizeof (struct sockaddr_x25))
		return (1);
	if (sa -> x25_family != AF_CCITT || sa -> x25_udlen == 0 ||
		sa -> x25_udlen > sizeof (sa -> x25_udata))
		return (1);
	for (cp = sa -> x25_addr; *cp; cp++) {
		if (*cp < '0' || *cp > '9' ||
			cp >= &sa -> x25_addr[sizeof (sa -> x25_addr) - 1])
			return (1);
	}
	return (0);
}

/* 
 *  This procedure sends a CLEAR request packet. The lc state is
 *  set to "SENT_CLEAR". 
 */

pk_clear (lcp)
struct pklcd *lcp;
{
	register struct x25_packet *xp;

	xp = lcp -> lcd_template = pk_template (lcp -> lcd_lcn, X25_CLEAR);
	(dtom (xp)) -> m_len++;
	xp -> packet_data = 0;

	pk_output (lcp);

}

/* 
 *  This procedure sends a RESET request packet. It re-intializes 
 *  virtual circuit.
 */

static
pk_reset (lcp)
register struct pklcd *lcp;
{
	register struct x25_packet *xp;
	register struct socket *so;

	if (lcp -> lcd_state != DATA_TRANSFER)
		return;

	lcp -> lcd_reset_condition = TRUE;

	/* Reset all the control variables for the channel. */
	lcp -> lcd_window_condition = lcp -> lcd_rnr_condition =
		lcp -> lcd_intrconf_pending = FALSE;
	lcp -> lcd_rsn = MODULUS - 1;
	lcp -> lcd_ssn = 0;
	lcp -> lcd_output_window = lcp -> lcd_input_window =
		lcp -> lcd_last_transmitted_pr = 0;
	if (so = lcp -> lcd_so)  {
		so -> so_error = ECONNRESET;
		sbflush (&so -> so_rcv);
		sbflush (&so -> so_snd);
	}
	xp = lcp -> lcd_template = pk_template (lcp -> lcd_lcn, X25_RESET);
	(dtom (xp)) -> m_len += 2;
	xp -> packet_data = 0;
	pk_output (lcp);

}


/* 
 *  This procedure handles all local protocol procedure errors.
 */

pk_procerror (error, lcp, errstr)
register struct pklcd *lcp;
char *errstr;
{

	pk_message (lcp -> lcd_lcn, lcp -> lcd_pkp -> pk_xcp, errstr);

	switch (error) {
	case CLEAR: 
		if (lcp->lcd_so) {
			lcp->lcd_so -> so_error = ECONNABORTED;
			soisdisconnecting (lcp->lcd_so);
		}
		pk_clear (lcp);
		break;

	case RESET: 
		pk_reset (lcp);
	}
}

/* 
 *  This procedure is called during the DATA TRANSFER state to check 
 *  and  process  the P(R) values  received  in the DATA,  RR OR RNR
 *  packets.
 */

pk_ack (lcp, pr)
struct pklcd *lcp;
unsigned pr;
{
	register struct socket *so = lcp -> lcd_so;

	if (lcp -> lcd_output_window == pr)
		return (PACKET_OK);
	if (lcp -> lcd_output_window < lcp -> lcd_ssn) {
		if (pr < lcp -> lcd_output_window || pr > lcp -> lcd_ssn) {
			pk_procerror (RESET, lcp, "p(r) flow control error");
			return (ERROR_PACKET);
		}
	}
	else {
		if (pr < lcp -> lcd_output_window && pr > lcp -> lcd_ssn) {
			pk_procerror (RESET, lcp, "p(r) flow control error");
			return (ERROR_PACKET);
		}
	}

	lcp -> lcd_output_window = pr;		/* Rotate window. */
	if (lcp -> lcd_window_condition == TRUE)
		lcp -> lcd_window_condition = FALSE;

	if (so && ((so -> so_snd.sb_flags & SB_WAIT) || so -> so_snd.sb_sel))
		sowwakeup (so);
	if (lcp -> lcd_downq.pq_unblock)
		(*lcp -> lcd_downq.pq_unblock)(lcp);

	return (PACKET_OK);
}

/* 
 *  This procedure decodes the X.25 level 3 packet returning a 
 *  code to be used in switchs or arrays.
 */

pk_decode (xp)
register struct x25_packet *xp;
{
	register int type;

	if (xp -> fmt_identifier != 1)
		return (INVALID_PACKET);

	/* 
	 *  Make sure that the logical channel group number is 0.
	 *  This restriction may be removed at some later date.
	 */
	if (xp -> lc_group_number != 0)
		return (INVALID_PACKET);

	/* 
	 *  Test for data packet first.
	 */
	if (!(xp -> packet_type & DATA_PACKET_DESIGNATOR))
		return (DATA);

	/* 
	 *  Test if flow control packet (RR or RNR).
	 */
	if (!(xp -> packet_type & RR_OR_RNR_PACKET_DESIGNATOR))
		if (!(xp -> packet_type & RR_PACKET_DESIGNATOR))
			return (RR);
		else
			return (RNR);

	/* 
	 *  Determine the rest of the packet types.
	 */
	switch (xp -> packet_type) {
	case X25_CALL: 
		type = CALL;
		break;

	case X25_CALL_ACCEPTED: 
		type = CALL_ACCEPTED;
		break;

	case X25_CLEAR: 
		type = CLEAR;
		break;

	case X25_CLEAR_CONFIRM: 
		type = CLEAR_CONF;
		break;

	case X25_INTERRUPT: 
		type = INTERRUPT;
		break;

	case X25_INTERRUPT_CONFIRM: 
		type = INTERRUPT_CONF;
		break;

	case X25_RESET: 
		type = RESET;
		break;

	case X25_RESET_CONFIRM: 
		type = RESET_CONF;
		break;

	case X25_RESTART: 
		type = RESTART;
		break;

	case X25_RESTART_CONFIRM: 
		type = RESTART_CONF;
		break;

	default: 
		type = INVALID_PACKET;
	}
	return (type);
}

/* 
 *  A restart packet has been received. Print out the reason
 *  for the restart.
 */

pk_restartcause (pkp, xp)
struct pkcb *pkp;
register struct x25_packet *xp;
{
	register struct x25config *xcp = pkp -> pk_xcp;
	register int lcn = xp -> logical_channel_number;

	switch (xp -> packet_data) {
	case X25_RESTART_LOCAL_PROCEDURE_ERROR: 
		pk_message (lcn, xcp, "restart: local procedure error");
		break;

	case X25_RESTART_NETWORK_CONGESTION: 
		pk_message (lcn, xcp, "restart: network congestion");
		break;

	case X25_RESTART_NETWORK_OPERATIONAL: 
		pk_message (lcn, xcp, "restart: network operational");
		break;

	default: 
		pk_message (lcn, xcp, "restart: unknown cause");
	}
}

#define MAXRESETCAUSE	7

int     Reset_cause[] = {
	EXRESET, EXROUT, 0, EXRRPE, 0, EXRLPE, 0, EXRNCG
};

/* 
 *  A reset packet has arrived. Return the cause to the user.
 */

pk_resetcause (pkp, xp)
struct pkcb *pkp;
register struct x25_packet *xp;
{
	register struct pklcd *lcp = pkp->pk_chan[xp -> logical_channel_number];
	register int code = xp -> packet_data;

	if (code > MAXRESETCAUSE)
		code = 7;	/* EXRNCG */

	lcp->lcd_so -> so_error = Reset_cause[code];
}

#define MAXCLEARCAUSE	25

int     Clear_cause[] = {
	EXCLEAR, EXCBUSY, 0, EXCINV, 0, EXCNCG, 0,
	0, 0, EXCOUT, 0, EXCAB, 0, EXCNOB, 0, 0, 0, EXCRPE,
	0, EXCLPE, 0, 0, 0, 0, 0, EXCRRC
};

/* 
 *  A clear packet has arrived. Return the cause to the user.
 */

pk_clearcause (pkp, xp)
struct pkcb *pkp;
register struct x25_packet *xp;
{
	register struct pklcd *lcp = pkp->pk_chan[xp -> logical_channel_number];
	register int code = xp -> packet_data;

	if (code > MAXCLEARCAUSE)
		code = 5;	/* EXRNCG */
	lcp->lcd_so -> so_error = Clear_cause[code];
}

char *
format_ntn (xcp)
register struct x25config *xcp;
{
	register int i;
	register char *src, *dest;
	static char ntn[12];

	src = xcp->xc_ntn;
	dest = ntn;
	for (i = 0; i < xcp->xc_ntnlen / 2; i++) {
		*dest++ = ((*src & 0xf0) >> 4) + '0';
		*dest++ = (*src++ & 0xf) + '0';
	}
	if (xcp->xc_ntnlen & 01)
		dest[-1] = 0;
	else
		*dest = 0;
	return (ntn);
}

/* VARARGS1 */
pk_message (lcn, xcp, fmt, a1, a2, a3, a4, a5, a6)
struct x25config *xcp;
char *fmt;
{

	if (lcn)
		if (pkcbhead -> pk_next)
			printf ("X.25(%s): lcn %d: ", format_ntn (xcp), lcn);
		else
			printf ("X.25: lcn %d: ", lcn);
	else
		if (pkcbhead -> pk_next)
			printf ("X.25(%s): ", format_ntn (xcp));
		else
			printf ("X.25: ");

	printf (fmt, a1, a2, a3, a4, a5, a6);
	printf ("\n");
}
