/*	cy.c	7.4	87/02/17	*/

/*
 * Cypher tape driver. Stand alone version.
 */
#include "../machine/pte.h"
#include "../machine/mtpr.h"

#include "param.h"
#include "inode.h"
#include "fs.h"

#include "saio.h"

#define CYERROR
#include "../tahoevba/cyreg.h"
#include "../tahoevba/vbaparam.h"

/*
 * NB: this driver assumes unit 0 throughout.
 */
long	cystd[] = { 0xf4000, 0 };
#define	CYADDR(i)	(cystd[i] + (int)VBIOBASE)

struct	cyscp *cyscp[] = { (struct cyscp *)0xc06, (struct cyscp *)0 };
#define	CYSCP(i)	(cyscp[i])

struct	cyscp *SCP;
struct	cyscb scb;
struct	cyccb ccb;
struct	cytpb tpb;
struct	cytpb cycool;		/* tape parameter block to clear interrupts */
#ifdef notdef
int	cyblocksize = 1024;	/* foreign tape size as found in open routine */
#endif
int	cybufsize;		/* controller buffer size */
long	cyblock;		/* next block number for i/o */

/*
 * Reset the controller.
 */
cyopen(io)
	register struct iob *io;
{
	register ctlradr = CYADDR(0);

	if (io->i_unit != 0)
		_stop("Only 1 cypher supported\n");
	SCP = CYSCP(0);				/* absolute - for setup */
	CY_RESET(ctlradr);			/* reset the controller */
	/*
	 * Initialize the system configuration pointer
	 */
	SCP->csp_buswidth = 1;			/* system width = 16 bits. */
	SCP->csp_unused = 0;
	/* initialize the pointer to the system configuration block */
	cyldmba(SCP->csp_scb, (caddr_t)&scb);
	/*
	 * Initialize the system configuration block.
	 */
	scb.csb_fixed = CSB_FIXED;		/* fixed value */
	/* initialize the pointer to the channel control block */
	cyldmba(scb.csb_ccb, (caddr_t)&ccb);
	/*
	 * Initialize the channel control block.
	 */
	ccb.cbcw = CBCW_IE;		/* normal interrupts */
	/* initialize the pointer to the tape parameter block */
	cyldmba(ccb.cbtpb, (caddr_t)&tpb);
	/*
	 * set the command to be CY_NOP.
	 */
	tpb.tpcmd = CY_NOP;
	/*
	 * TPB not used on first attention
	 */
	tpb.tpcontrol = CYCW_LOCK | CYCW_16BITS;
	ccb.cbgate = GATE_CLOSED;	
	CY_GO(ctlradr);			/* execute! */
	cywait(10*1000);
	/*
	 * set the command to be CY_CONFIGURE.
	 * NO interrupt on completion.
	 */
	tpb.tpcmd = CY_CONFIG;
	tpb.tpcontrol = CYCW_LOCK | CYCW_16BITS;
	tpb.tpstatus = 0;
	ccb.cbgate = GATE_CLOSED;	
	CY_GO(ctlradr);			/* execute! */
	cywait(10*1000);
	uncache(&tpb.tpstatus);
	if (tpb.tpstatus & CYS_ERR) {
		printf("Cypher initialization error!\n");
		cy_print_error(tpb.tpstatus);
		_stop("");
	}
	uncache(&tpb.tpcount);
	cybufsize = tpb.tpcount;
	if (cycmd(io, CY_REW) == -1)
		_stop("Rewind failed!\n");
	while (io->i_boff > 0) {
		if (cycmd(io, CY_FSF) == -1)
			_stop("cy: seek failure!\n");
		io->i_boff--;
	}
#ifdef notdef
#ifdef NOBLOCK
	if (io->i_flgs & F_READ) {
		cyblocksize = cycmd(io, CY_READFORN);
		if (cyblocksize == -1)
			_stop("Read foreign tape failed\n");
		cyblock++;		/* XXX force backspace record */
		if (cycmd(io, CY_SFORW) == -1)
			_stop("Backspace after read foreign failed\n");
	}
#endif
#endif
}

cyclose(io)
	register struct iob *io;
{

	if (io->i_flgs & F_WRITE) {	/* if writing, write file marks */
		cycmd(io, CY_WEOF);
		cycmd(io, CY_WEOF);
	}
	cycmd(io, CY_REW);
}

cystrategy(io, func)
	register struct iob *io;
	register func;
{

#ifndef NOBLOCK
	if (func != CY_SFORW && func != CY_REW && io->i_bn != cyblock) {
		cycmd(io, CY_SFORW);
		tpb.tprec = 0;
	}
	if (func == READ || func == WRITE) {
		struct iob liob;
		register struct iob *lio = &liob;
		register count;

		liob = *io;
		while (lio->i_cc > 0) {
			if ((count = cycmd(lio, func)) == 0)
				return (-1);
			lio->i_cc -= count;
			lio->i_ma += count;
		}
		return (io->i_cc);
	}
#endif
	return (cycmd(io, func));
}

cycmd(io, func)
	register struct iob *io;
	long func;
{
	register ctlradr = CYADDR(0);
	int timeout = 0;
	int err;
	short j;

	cywait(9000); 			/* shouldn't be needed */
	tpb.tpcontrol = CYCW_LOCK | CYCW_16BITS;
	tpb.tpstatus = 0;
	tpb.tpcount = 0;
	cyldmba(ccb.cbtpb, (caddr_t)&tpb);
	tpb.tpcmd = func;
	switch (func) {
	case READ:
#ifdef notdef
		if (io->i_cc > cyblocksize)
			tpb.tpsize = htoms(cyblocksize);
		else
#endif
		tpb.tpsize = htoms(io->i_cc);
		cyldmba(tpb.tpdata, io->i_ma);
		tpb.tpcmd = CY_RCOM;
		cyblock++;
		break;
	case WRITE:
		tpb.tpcmd = CY_WCOM;
		tpb.tpsize = htoms(io->i_cc);
		cyldmba(tpb.tpdata, io->i_ma);
		cyblock++;
		break;
	case CY_SFORW:
		if ((j = io->i_bn - cyblock) < 0) {
			j = -j;
			tpb.tpcontrol |= CYCW_REV;
			cyblock -= j;
		} else
			cyblock += j;
		tpb.tprec = htoms(j);
		timeout = 60*5;
		break;
	case CY_FSF:
		tpb.tprec = htoms(1);
		/* fall thru... */
	case CY_REW:
		cyblock = 0;
		timeout = 60*5;
		break;
	}
	ccb.cbgate = GATE_CLOSED;
	CY_GO(ctlradr);			/* execute! */
	if (timeout == 0)
		timeout = 10;
	cywait(timeout*1000);
	/*
	 * First we clear the interrupt and close the gate.
	 */
	mtpr(PADC, 0);
	ccb.cbgate = GATE_CLOSED;
	cyldmba(ccb.cbtpb, (caddr_t)&cycool);
	cycool.tpcontrol = CYCW_LOCK;	/* No INTERRUPTS */
	CY_GO(ctlradr);
	cywait(20000); 
	uncache(&tpb.tpstatus); 
	if (err = (tpb.tpstatus & CYS_ERR) &&
	    err != CYER_FM && (err != CYER_STROBE || tpb.tpcmd != CY_RCOM)) {
		cy_print_error(tpb.tpstatus);
		io->i_error = EIO;
		return (-1);
	}
	uncache(&tpb.tpcount);
	return ((int)htoms(tpb.tpcount));
}
	
cy_print_error(status)
	int status;
{
	register char *message;

	if ((status & CYS_ERR) < NCYERROR)
		message = cyerror[status & CYS_ERR];
	else
		message = "unknown error";
	printf("cy0: %s, status=%b.\n", message, status, CYS_BITS);
}

cywait(timeout)
	register timeout;
{

	do {
		DELAY(1000);
		uncache(&ccb.cbgate);
	} while (ccb.cbgate != GATE_OPEN && --timeout > 0);
	if (timeout <= 0)
		_stop("cy: Transfer timeout");
}

/*
 * Load a 20 bit pointer into a Tapemaster pointer.
 */
cyldmba(reg, value)
	register caddr_t reg;
	caddr_t value;
{
	register int v = (int)value;

	*reg++ = v;
	*reg++ = v >> 8;
	*reg++ = 0;
	*reg = (v&0xf0000) >> 12;
}
