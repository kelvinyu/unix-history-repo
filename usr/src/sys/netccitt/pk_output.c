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
 *	@(#)pk_output.c	7.2 (Berkeley) %G%
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/protosw.h"
#include "../h/errno.h"

#include "../net/if.h"

#include "../netccitt/pk.h"
#include "../netccitt/pk_var.h"
#include "../netccitt/x25.h"

struct	mbuf *nextpk ();

pk_output (lcp)
register struct pklcd *lcp;
{
	register struct x25_packet *xp;
	register struct mbuf *m;
	register struct pkcb *pkp = lcp -> lcd_pkp;

	if (lcp == 0 || pkp == 0) {
		printf ("pk_output: zero arg\n");
		return;
	}

	while ((m = nextpk (lcp)) != NULL) {
		xp = mtod (m, struct x25_packet *);

		switch (pk_decode (xp) + lcp -> lcd_state) {
		/* 
		 *  All the work is already done - just set the state and
		 *  pass to peer.
		 */
		case CALL + READY: 
			lcp -> lcd_state = SENT_CALL;
			lcp -> lcd_timer = pk_t21;
			break;

		/*
		 *  Just set the state to allow packet to flow and send the
		 *  confirmation.
		 */
		case CALL_ACCEPTED + RECEIVED_CALL: 
			lcp -> lcd_state = DATA_TRANSFER;
			break;

		/* 
		 *  Just set the state. Keep the LCD around till the clear
		 *  confirmation is returned.
		 */
		case CLEAR + RECEIVED_CALL: 
		case CLEAR + SENT_CALL: 
		case CLEAR + DATA_TRANSFER: 
			lcp -> lcd_state = SENT_CLEAR;
			lcp -> lcd_retry = 0;
			/* fall through */

		case CLEAR + SENT_CLEAR:
			lcp -> lcd_timer = pk_t23;
			lcp -> lcd_retry++;
			break;

		case CLEAR_CONF + RECEIVED_CLEAR: 
		case CLEAR_CONF + SENT_CLEAR: 
		case CLEAR_CONF + READY: 
			lcp -> lcd_state = READY;
			break;

		case DATA + DATA_TRANSFER: 
			PS(xp) = lcp -> lcd_ssn;
			PR(xp) = lcp -> lcd_input_window;
			lcp -> lcd_last_transmitted_pr = lcp -> lcd_input_window;
			lcp -> lcd_ssn = (lcp -> lcd_ssn + 1) % MODULUS;
			if (lcp -> lcd_ssn == ((lcp -> lcd_output_window + pkp->pk_xcp->xc_pwsize) % MODULUS))
				lcp -> lcd_window_condition = TRUE;
			break;

		case INTERRUPT + DATA_TRANSFER: 
			xp -> packet_data = 0;
			lcp -> lcd_intrconf_pending = TRUE;
			break;

		case INTERRUPT_CONF + DATA_TRANSFER: 
			break;

		case RR + DATA_TRANSFER: 
			lcp -> lcd_input_window = (lcp -> lcd_input_window + 1) % MODULUS;
			PR(xp) = lcp -> lcd_input_window;
			lcp -> lcd_last_transmitted_pr = lcp -> lcd_input_window;
			break;

		case RNR + DATA_TRANSFER: 
			PR(xp) = lcp -> lcd_input_window;
			lcp -> lcd_last_transmitted_pr = lcp -> lcd_input_window;
			break;

		case RESET + DATA_TRANSFER: 
			lcp -> lcd_reset_condition = TRUE;
			break;

		case RESET_CONF + DATA_TRANSFER: 
			lcp -> lcd_reset_condition = FALSE;
			break;

		/* 
		 *  A restart should be only generated internally. Therefore
		 *  all logic for restart is in the pk_restart routine.
		 */
		case RESTART + READY: 
			lcp -> lcd_timer = pk_t20;
			break;

		/* 
		 *  Restarts are all  handled internally.  Therefore all the
		 *  logic for the incoming restart packet is handled in  the
		 *  pk_input routine.
		 */
		case RESTART_CONF + READY: 
			break;

		default: 
			m_freem (m);
			return;
		}

		/* Trace the packet. */
		pk_trace (pkp -> pk_xcp, xp, "P-Out");

		/* Pass the packet on down to the link layer */
		(*pkp -> pk_output) (m, pkp -> pk_xcp);
	}
}

/* 
 *  This procedure returns the next packet to send or null. A
 *  packet is composed of one or more mbufs.
 */

struct mbuf *
nextpk (lcp)
struct pklcd *lcp;
{
	register struct socket *so = lcp -> lcd_so;
	register struct mbuf *m = 0, *n;

	if (lcp -> lcd_template) {
		m = dtom (lcp -> lcd_template);
		lcp -> lcd_template = NULL;
	} else {
		if (lcp -> lcd_rnr_condition || lcp -> lcd_window_condition ||
				lcp -> lcd_reset_condition)
			return (NULL);

		if (so == 0)
			return (NULL);

		if ((m = so -> so_snd.sb_mb) == 0)
			return (NULL);

		n = m;
		while (n) {
			sbfree (&so -> so_snd, n);
#ifndef BSD4_3
			if ((int) n -> m_act == 1)
				break;
#endif
			n = n -> m_next;
		}

#ifdef BSD4_3
 		so->so_snd.sb_mb = m->m_act;
 		m->m_act = 0;
#else
		if (n) {
			so -> so_snd.sb_mb = n -> m_next;
			n -> m_next = 0;
		}
#endif
	}

	return (m);
}
