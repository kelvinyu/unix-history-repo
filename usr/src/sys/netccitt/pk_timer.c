/* Copyright (c) University of British Columbia, 1984 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/protosw.h"
#include "../h/socketvar.h"
#include "../h/errno.h"

#include "../net/if.h"

#include "../netccitt/pk.h"
#include "../netccitt/pk_var.h"

/*
 * Various timer values.  They can be adjusted
 * by patching the binary with adb if necessary.
 */
int	pk_t20 = 18 * PR_SLOWHZ;	/* restart timer */
int	pk_t21 = 20 * PR_SLOWHZ;	/* call timer */
/* XXX pk_t22 is never used */
int	pk_t22 = 18 * PR_SLOWHZ;	/* reset timer */
int	pk_t23 = 18 * PR_SLOWHZ;	/* clear timer */

pk_timer ()
{
	register struct pkcb *pkp;
	register struct pklcd *lcp, **pp;
	register int lcns_jammed, cant_restart;

	for (pkp = pkcbhead; pkp; pkp = pkp->pk_next) {
		switch (pkp -> pk_state) {
		case DTE_SENT_RESTART:
			lcp = pkp -> pk_chan[0];
			/*
			 * If restart failures are common, a link level
			 * reset should be initiated here.
			 */
			if (lcp -> lcd_timer && --lcp -> lcd_timer == 0)
				pk_message (0, pkp -> pk_xcp,
					"packet level restart failed");
			break;

		case DTE_READY:
			lcns_jammed = cant_restart = 0;
			for (pp = &pkp -> pk_chan[1]; pp <= &pkp -> pk_chan[pkp -> pk_maxlcn]; pp++) {
				if ((lcp = *pp) == 0)
					continue;
				switch (lcp -> lcd_state) {
				case SENT_CALL: 
					if (--lcp -> lcd_timer == 0) {
						lcp -> lcd_so -> so_error = ETIMEDOUT;
						pk_clear (lcp);
					}
					break;

				case SENT_CLEAR: 
					if (lcp -> lcd_retry >= 3)
						lcns_jammed++;
					else
						if (--lcp -> lcd_timer == 0)
							pk_clear (lcp);
					break;

				case DATA_TRANSFER:	/* lcn active */
					cant_restart++;
					break;
				}
			}
			if (lcns_jammed > pkp -> pk_maxlcn / 2 && cant_restart == 0) {
				pk_message (0, pkp -> pk_xcp, "%d lcns jammed: attempting restart", lcns_jammed);
				pk_restart (pkp, 0);
			}
		}
	}
}
