/*	uu.c	4.4	83/06/01	*/

#include "uu.h"
#if NUU > 0
/*
 * TU58 DECtape II/DL11 device driver
 *
 *  ******   Warning: untested  ******
 *
 * The TU58 * is treated as a block device (only).  Error detection and
 * recovery is almost non-existant.  It is assumed that the
 * TU58 will follow the RSP protocol exactly, very few protocol
 * errors are checked for.  
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/time.h"
#include "../h/kernel.h"
#include "../h/errno.h"
#include "../h/uio.h"
#include "../h/file.h"

#include "../vax/cpu.h"
#include "../vax/nexus.h"

#include "../vaxuba/ubavar.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/uureg.h"

#define	printd	if (uudebug) printf
#ifdef	printd
int	uudebug = 0;		/* printd */
#endif	printd

#define	NTUBLK	512		/* number of blocks on a TU58 cassette */
#define	WRV     01              /* bit in minor dev => write w. read verify */
#define	NDPC	02		/* drives per controller */
#define	NUX	NDPC * NUU	/* number of drives */
#define	NTUQ	02		/* # of block which can be queued up */
#define	UMASK	01		/* unit number mask */
#define UUIPL	04		/* ipl level to use */

/*
 * Structure of a command packet
 */
struct packet {
	u_char	pk_flag;	/* indicates packet type (cmd, data, etc.) */
	u_char	pk_mcount;	/* length of packet (bytes) */
	u_char	pk_op;		/* operation to perform (read, write, etc.) */
	u_char	pk_mod;		/* modifier for op or returned status */
	u_char	pk_unit;	/* unit number */
	u_char	pk_sw;		/* switches */
	u_short	pk_seq;		/* sequence number, always zero */
	u_short	pk_count;	/* requested byte count for read or write */
	u_short	pk_block;	/* block number for read, write, or seek */
	u_short	pk_chksum;	/* checksum, by words with end around carry */
};

struct packet uucmd[NUU];	/* a command sent to the TU58 */
struct packet uudata[NUU];	/* a command or data returned from TU58 */
struct buf uitab[NUU];		/* buffer queue headers */

/*
 * per controller state information
 */
struct uu_ctlr {
	u_char	*uu_rbptr;	/* pointer to buffer for read */
	int	uu_rcnt;	/* how much to read */
	u_char	*uu_wbptr;	/* pointer to buffer for write */
	int	uu_wcnt;	/* how much to write */
	int	uu_state;	/* current uu_state of tansfer operation */
	int	uu_flag;	/* read in progress flag */
	char	*uu_addr;	/* real buffer data address */
	int	uu_count;	/* real requested count */
	int	uu_serrs;	/* count of soft errors */
	int	uu_cerrs;	/* count of checksum errors */
	int	uu_herrs;	/* count of hard errors */
	char    uu_dopen[NDPC];	/* drive is open */
} uu_ctlr[NUU];

/*
 * controller states
 */
#define	UUS_INIT1	0	/* sending nulls */
#define	UUS_INIT2	1	/* sending inits */
#define	UUS_IDLE	2	/* initialized, no transfer in progress */
#define	UUS_SENDH	3	/* sending header */
#define	UUS_SENDD	4	/* sending data */
#define	UUS_SENDC	5	/* sending checksum */
#define	UUS_SENDR	6	/* sending read command packet */
#define	UUS_SENDW	7	/* sending write command packet */
#define	UUS_GETH	8	/* reading header */
#define	UUS_GETD	9	/* reading data */
#define	UUS_GETC	10	/* reading checksum */
#define	UUS_GET		11	/* reading an entire packet */
#define	UUS_WAIT	12	/* waiting for continue */

#define	UUS_NSTATES	13
char *uustates[UUS_NSTATES] = {
	"INIT1", "INIT2", "IDLE", "SENDH", "SENDD", "SENDC", "SENDR",
	"SENDW", "GETH", "GETD", "GETC", "GET", "WAIT"
};

#define	UNIT(dev)	(minor(dev)>>1)
#define	printstate(state) \
	if ((state) < UUS_NSTATES) \
		printf("%s", uustates[(state)]); \
	else \
		printf("%d", (state));

/*
 * Packet Flags
 */
#define	TUF_DATA	1		/* data packet */
#define	TUF_CMD		2		/* command packet */
#define	TUF_INITF	4		/* initialize */
#define	TUF_CONT	020		/* continue */
#define	TUF_XOFF	023		/* flow control */

/*
 * Op Codes
 */
#define	TUOP_NOOP	0		/* no operation */
#define	TUOP_INIT	1		/* initialize */
#define	TUOP_READ	2		/* read block */
#define	TUOP_WRITE	3		/* write block */
#define	TUOP_SEEK	5		/* seek to block */
#define	TUOP_DIAGNOSE	7		/* run micro-diagnostics */
#define	TUOP_END	0100		/* end packet */

/*
 * Mod Flags
 */
#define TUMD_WRV        1               /* write with read verify */

/*
 * Switches
 */

u_char	uunull[2] = { 0, 0 };	/* nulls to send for initialization */
u_char	uuinit[2] = { TUF_INITF, TUF_INITF };	/* inits to send */

struct	uba_device	*uudinfo[NUU];

int uuprobe(), uuattach(), uurintr(), uuxintr(), uuwatch();
u_short uustd[] = { 0176500 };
struct uba_driver uudriver =
    { uuprobe, 0, uuattach, 0, uustd, "uu", uudinfo };

int	uuwstart;
static char pcnt[NUX];			/* pee/vee counters */

/*ARGSUSED*/
uuprobe(reg)
	caddr_t reg;
{
	register int br, cvec;			/* value result */
	struct uudevice *uuaddr = (struct uudevice *)reg;
	int i;

#ifdef lint
	br = 0; cvec = br; br = cvec;
	uurintr(0); uuxintr(0);
#endif
	uuaddr->tcs = UUCS_INTR;
	DELAY(1000);
	uuaddr->tcs = 0;
	cvec -= 4;		/* since we are using the xmitter intrpt */
	return(sizeof (*uuaddr));
}

uuattach(ui)
	register struct uba_device *ui;
{
}

/*ARGSUSED1*/
uuopen(dev, flag)
	dev_t dev;
	int flag;	
{
	register struct uba_device *ui;
	register struct uu_ctlr *uuc;
	register struct uudevice *uuaddr;
	int ctlr, unit = UNIT(dev), s;

	ctlr = unit / NDPC;
	if (unit >= NUX || (ui = uudinfo[ctlr]) == 0 || ui->ui_alive == 0)
		return (ENXIO);
	uuc = &uu_ctlr[ctlr];
	if (uuc->uu_dopen[unit&UMASK])
		return (EBUSY);
	if (uuwstart++ == 0)
		timeout(uuwatch, (caddr_t)0, hz);

	uuc->uu_dopen[unit&UMASK]++;
	uuaddr = (struct uudevice *)ui->ui_addr;
	s = splx(UUIPL);
	/*
	 * If the unit already initialized,
	 * just enable interrupts and return.
	 */
	if (uuc->uu_state == UUS_IDLE) {
		uuaddr->rcs = UUCS_INTR;
		(void) splx(s);
		return (0);
	}

	/* 
	 * Must initialize, reset the cassette
	 * and wait for things to settle down.
	 */
	uureset(ctlr);
	sleep((caddr_t)uuc, PZERO+1);
	uitab[ctlr].b_active = NULL;
	if (uuc->uu_state != UUS_IDLE) {
		uuc->uu_state = UUS_INIT1;
		uuc->uu_dopen[unit&UMASK] = 0;
		uuc->uu_rcnt = uuc->uu_wcnt = 0;
		uuaddr->rcs = 0;
		uuaddr->tcs = 0;
		splx(s);
		return (ENXIO);
	}
	splx(s);
	return (0);
}

uuclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct uu_ctlr *uuc;
	int unit = UNIT(dev);

	uuwstart--;
	uuc = &uu_ctlr[unit%NDPC];
	if (uuc->uu_serrs + uuc->uu_cerrs + uuc->uu_herrs != 0) {
		/*
		 * A tu58 is like nothing ever seen before;
		 * I guess this is appropriate then...
		 */
		uprintf(
		   "uu%d: %d soft errors, %d checksum errors, %d hard errors\n",
		    unit, uuc->uu_serrs, uuc->uu_cerrs, uuc->uu_herrs);
		    uuc->uu_serrs = uuc->uu_cerrs = uuc->uu_herrs = 0;
	}
	uuc->uu_dopen[unit%NDPC] = 0;
}

uureset(ctlr)
	int ctlr;
{
	register struct uu_ctlr *uuc = &uu_ctlr[ctlr];
	register struct packet *cmd = &uucmd[ctlr];
	struct uba_device *ui = uudinfo[ctlr];
	register struct uudevice *uuaddr = (struct uudevice *)ui->ui_addr;

	printf ("uureset\n");
	uitab[ctlr].b_active++;
	uuc->uu_state = UUS_INIT1;
	uuc->uu_wbptr = uunull;
	uuc->uu_wcnt = sizeof (uunull);
	cmd->pk_flag = TUF_CMD;
	cmd->pk_mcount = sizeof (*cmd) - 4;
	cmd->pk_mod = 0;
	cmd->pk_seq = 0;
	cmd->pk_sw = 0;
	uuaddr->rcs = 0;
	uuaddr->tcs = UUCS_INTR | UUCS_BREAK;
	uuxintr(ctlr);				/* start output */
}

/*
 * Strategy routine for block I/O
 */
uustrategy(bp)
	register struct buf *bp;
{
	register struct buf *uutab;
	struct uba_device *ui;
	int s, unit = UNIT(minor(bp->b_dev));

	if (unit > NUX)
		goto bad;
	if (bp->b_blkno >= NTUBLK)
		goto bad;
	ui = uudinfo[unit/NDPC];
	if (ui == 0 || ui->ui_alive == 0)
		goto bad;
	uutab = &uitab[unit/NDPC];	/* one request queue per controller */
	if ((bp->b_flags&B_READ) == 0)
		uu_pee(&pcnt[unit]);
	printf("uustrat: unit=%d, bp=%x\n", unit, bp);
	s = splx(UUIPL);
	bp->b_forw = NULL;
	if (uutab->b_actf == NULL)
		uutab->b_actf = bp;
	else
		uutab->b_actl->b_forw = bp;
	uutab->b_actl = bp;
	if (uutab->b_active == 0)
		uustart(ui);
	splx(s);
	return;

bad:
	bp->b_flags |= B_ERROR;
	bp->b_error = ENXIO;
	iodone(bp);
	return;
}

/*
 * Start the transfer
 */
uustart(ui)
	register struct uba_device *ui;
{
	register struct buf *bp;
	register struct uu_ctlr *uuc;
	struct packet *cmd;
	int ctlr = ui->ui_unit;

	if ((bp = uitab[ctlr].b_actf) == NULL)
		return;
	uuc = &uu_ctlr[ctlr];
	cmd = &uucmd[ctlr];
	if (uuc->uu_state != UUS_IDLE) {
		uureset(ctlr);
		return;
	}
	uitab[ctlr].b_active++;
	cmd->pk_op = bp->b_flags&B_READ ? TUOP_READ : TUOP_WRITE;
	cmd->pk_mod = ((bp->b_flags&B_READ) == 0 && (minor(bp->b_dev)&WRV)) ?
	    TUMD_WRV : 0;
	cmd->pk_unit = UNIT(minor(bp->b_dev));
	cmd->pk_sw = 0;
	cmd->pk_count = uuc->uu_count = bp->b_bcount;
	cmd->pk_block = bp->b_blkno;
	cmd->pk_chksum =
	    uuchk(*((short *)cmd), (u_short *)&cmd->pk_op,
		(int)cmd->pk_mcount);
	uuc->uu_state = bp->b_flags&B_READ ? UUS_SENDR : UUS_SENDW;
	uuc->uu_addr = bp->b_un.b_addr;
	uuc->uu_count = bp->b_bcount;
	uuc->uu_wbptr = (u_char *)cmd;
	uuc->uu_wcnt = sizeof (*cmd);
	uuxintr(ctlr);
}

/*
 * TU58 receiver interrupt
 */
uurintr(ctlr)
	int ctlr;
{
	struct uba_device *ui = uudinfo[ctlr];
	register struct uu_ctlr *uuc = &uu_ctlr[ctlr];
	register struct uudevice *uuaddr = (struct uudevice *)ui->ui_addr;
	register struct buf *uutab = &uitab[ctlr];
	register struct buf *bp = uutab->b_actf;
	struct packet *data, *cmd;
	int c, unit = UNIT(minor(bp->b_dev));

	c = uuaddr->rdb;
	/*
	 * We may have a stray interrupt, but read
	 * the data anyway, otherwise we'll get an overrun
	 * next time
	 */
	if (uutab->b_active == 0)
		return;
	cmd = &uucmd[ctlr];
	if (c & UURDB_ERROR) {
		if (c & UURDB_ORUN) {
			printf("uu%d: data overrun (ignored)\n", unit);
		} else {
			printf("uu%d: break received, device reset, state=", 
				unit);
			printstate(uuc->uu_state);
			uureset(ctlr);
			printf("\n");
			return;
		}
	}
	data = &uudata[ctlr];
top:
	c &= 0xff;
	if (uuc->uu_rcnt) {		/* still waiting for data? */
		*uuc->uu_rbptr++ = c;	/* yup, put it there */
		if (--uuc->uu_rcnt)	/* decrement count, any left? */
			return;		/* get some more */
	}

	/*
	 * We got all the data we were expecting for now,
	 * switch on the uu_state of the transfer.
	 */
	switch(uuc->uu_state) {

	/*
	 * If we get an unexpected "continue",
	 * start all over again...
	 */
	case UUS_INIT2:
		uuc->uu_state = c == TUF_CONT ? UUS_IDLE : UUS_INIT1;
		uuc->uu_flag = 0;
		wakeup((caddr_t)uuc);
		uustart(ui);
		break;

	/*
	 * Only transition from this state
	 * is on a "continue", so if we don't
	 * get it, reset the world.
	 */
	case UUS_WAIT:			/* waiting for continue */
		switch(c) {
		case TUF_CONT:  /* got the expected continue */
			uuc->uu_flag = 0;
			data->pk_flag = TUF_DATA;
			data->pk_mcount = MIN(128, uuc->uu_count);
			data->pk_chksum =
			    tuchk(*((short *)data), (caddr_t)uuc->uu_addr,
				(int)data->pk_mcount);
			uuc->uu_state = UUS_SENDH;
			uuc->uu_wbptr = (u_char *)data;
			uuc->uu_wcnt = 2;
			uuxintr(ctlr);
			break;

		case TUF_CMD:   /* sending us an END packet...error */
			uuc->uu_state = UUS_GET;
			uuc->uu_rbptr = (u_char *) data;
			uuc->uu_rcnt = sizeof (*data);
			uuc->uu_flag = 1;
			uuaddr->tcs = 0;
			goto top;

		case TUF_INITF:
			uureset(ctlr);
			break;

		default:        /* something random...bad news */
			uuc->uu_state = UUS_INIT1;
			break;
		}
		break;

	case UUS_SENDW:
		if (c != TUF_CONT)
			goto bad;
		uureset(ctlr);
		break;

	/*
	 * Got header, now get data; amount to
	 * fetch is included in packet.
	 */
	case UUS_GETH:
		if (data->pk_flag == TUF_DATA)
			uuc->uu_rbptr = (u_char *)uuc->uu_addr;
		uuc->uu_rcnt = data->pk_mcount;
		uuc->uu_state = UUS_GETD;
		break;

	/*
	 * Got the data, now fetch the checksum.
	 */
	case UUS_GETD:
		uuc->uu_rbptr = (u_char *)&data->pk_chksum;
		uuc->uu_rcnt = sizeof (data->pk_chksum);
		uuc->uu_state = UUS_GETC;
		break;

	case UUS_GET:
	case UUS_GETC:
		/* got entire packet */
#ifdef notdef
		if (data->pk_chksum !=
		    uuchk(*((short *)data), (u_short *)
		     (data->pk_flag == TUF_DATA ? uuc->uu_addr : &data->pk_op),
		     (int)data->pk_mcount))
			uuc->uu_cerrs++;
#endif
		if (data->pk_flag == TUF_DATA) {
			/* data packet, advance to next */
			uuc->uu_addr += data->pk_mcount;
			uuc->uu_count -= data->pk_mcount;
			uuc->uu_state = UUS_GETH;
			uuc->uu_rbptr = (u_char *)data;	/* next packet */
			uuc->uu_rcnt = 2;
		} else if (data->pk_flag==TUF_CMD && data->pk_op==TUOP_END) {
			/* end packet, idle and reenable transmitter */
			uuc->uu_state = UUS_IDLE;
			uuc->uu_flag = 0;
			uuaddr->rcs = UUCS_INTR;
			printd("ON ");
			if (bp == NULL) {
				printf("uu(%d): no bp, active %d\n", 
					uitab[ctlr].b_active);
				uustart(ui);
				return;
			}
			if (data->pk_mod > 1) {        /* hard error */
				bp->b_flags |= B_ERROR;
				uuc->uu_herrs++;
				harderr(bp, "uu");
				printf(" pk_mod %o\n", data->pk_mod&0xff);
			} else if (data->pk_mod != 0)	/* soft error */
				uuc->uu_serrs++;
			uutab->b_active = NULL;
			uutab->b_actf = bp->b_forw;
			bp->b_resid = uuc->uu_count;
			if ((bp->b_flags&B_READ) == 0)
				uu_vee(&pcnt[unit]);
			iodone(bp);
			uustart(ui);
		} else {
			printf("neither data nor end: %o %o\n",
			    data->pk_flag&0xff, data->pk_op&0xff);
			uuaddr->rcs = 0;		/* flush the rest */
			uuc->uu_state = UUS_INIT1;
		}
		break;

	case UUS_IDLE:
	case UUS_INIT1:
		break;

	default:
bad:
		if (c == TUF_INITF) {
			printf("uu%d protocol error, state=", unit);
			printstate(uuc->uu_state);
			printf(", op=%x, cnt=%d, block=%d\n",
			    cmd->pk_op, cmd->pk_count, cmd->pk_block);
			uutab->b_active = NULL;
			if (bp = uutab->b_actf) {
				bp->b_flags |= B_ERROR;
				uutab->b_actf = bp->b_forw;
				if ((bp->b_flags&B_READ) == 0)
					uu_vee(&pcnt[unit]);
				iodone(bp);
			}
			uuc->uu_state = UUS_INIT1;
		} else {
			printf("uu%d receive state error, state=", 
				unit);
			printstate(uuc->uu_state);
			printf(", byte=%x\n", c);
#ifdef notdef
			uuc->uu_state = UUS_INIT1;
#endif
			wakeup((caddr_t)uuc);
		}
	}
}

/*
 * TU58 transmitter interrupt
 */
uuxintr(ctlr)
	int ctlr;
{
	register struct uu_ctlr *uuc = &uu_ctlr[ctlr];
	register struct uudevice *uuaddr;
	register struct packet *data;
	struct uba_device *ui = uudinfo[ctlr];
	int c;

	data = &uudata[ctlr];
	uuaddr = (struct uudevice *) ui->ui_addr;
top:
	if (uuc->uu_wcnt) {
		/* still stuff to send, send one byte */
		while ((uuaddr->tcs & UUCS_READY) == 0)
			;
		uuaddr->tdb = *uuc->uu_wbptr++;
		uuc->uu_wcnt--;
		return;
	}

	/*
	 * Last message byte was sent out.
	 * Switch on uu_state of transfer.
	 */
	if (uudebug) {
		printf("uuxintr: state=");
		printstate(uuc->uu_state);
	}
	switch(uuc->uu_state) {

	/*
	 * Two nulls have been sent, remove break, and send inits
	 */
	case UUS_INIT1:	
		uuaddr->tcs = UUCS_INTR;
		printd("ON2 ");
		uuc->uu_state = UUS_INIT2;
		uuc->uu_wbptr = uuinit;
		uuc->uu_wcnt = sizeof (uuinit);
		goto top;

	/*
	 * Inits have been sent, wait for a continue msg.
	 */
	case UUS_INIT2:	
		c = uuaddr->rdb;	/* prevent overrun error */
		uuaddr->rcs = UUCS_INTR;
		uuc->uu_flag = 1;
		break;

	case UUS_IDLE:		/* stray interrupt? */
		break;

	/*
	 * Read cmd packet sent, get ready for data
	 */
	case UUS_SENDR:
		uuc->uu_state = UUS_GETH;
		uuc->uu_rbptr = (u_char *)data;
		uuc->uu_rcnt = 2;
		uuc->uu_flag = 1;
		uuaddr->tcs = 0;	/* disable transmitter interrupts */
		printd("OFF ");
		break;

	/*
	 * Write cmd packet sent, wait for continue
	 */
	case UUS_SENDW:	
		uuc->uu_state = UUS_WAIT;
		uuc->uu_flag = 1;
		if ((uuaddr->rcs&UUCS_INTR) == 0) {
			printf("NO IE\n");
			uuaddr->rcs = UUCS_INTR;
		}
		break;

	/*
	 * Header sent, send data.
	 */
	case UUS_SENDH:
		uuc->uu_state = UUS_SENDD;
		uuc->uu_wbptr = (u_char *)uuc->uu_addr;
		uuc->uu_wcnt = data->pk_mcount;
		goto top;

	/*
	 * Data sent, follow with checksum.
	 */
	case UUS_SENDD:	
		uuc->uu_state = UUS_SENDC;
		uuc->uu_wbptr = (u_char *)&data->pk_chksum;
		uuc->uu_wcnt = sizeof (data->pk_chksum);
		goto top;

	/* 
	 * Checksum sent, wait for continue.
	 */
	case UUS_SENDC:
		/*
		 * Updata buffer address and count.
		 */
		uuc->uu_addr += data->pk_mcount;
		uuc->uu_count -= data->pk_mcount;
		if (uuc->uu_count) {
			uuc->uu_state = UUS_WAIT;
			uuc->uu_flag = 1;
			break;
		}

		/*
		 * End of transmission, get ready for end packet.
		 */
		uuc->uu_state = UUS_GET;
		uuc->uu_rbptr = (u_char *)data;
		uuc->uu_rcnt = sizeof (*data);
		uuc->uu_flag = 1;
		uuaddr->tcs = 0;		/* disable transm. interrupts */
		printd("OFF2 ");
		break;

	/*
	 * Random interrupt
	 */
	default:
		break;
	}
	if (uudebug) {
		printd("  new uu_state=");
		printstate(uuc->uu_state);
		printf("\n");
	}
}

/*
 * Compute checksum TU58 fashion
 */
#ifdef lint
uuchk(word, cp, n)
	register word;
	register unsigned short *cp;
	int n;
{
	register int c = n >> 1;
	register long temp;

	do {
		temp = *cp++;	/* temp, only because vax cc won't *r++ */
		word += temp;
	} while (--c > 0);
	if (n & 1)
		word += *(unsigned char *)cp;
	while (word & 0xffff0000)
		word = (word & 0xffff) + ((word >> 16) & 0xffff);
	return (word);
}
#else
uuchk(word0, wp, n)
	register int word0;			/* r11 */
	register char *wp;			/* r10 */
	register int n;				/* r9 */
{
	asm("loop:");
	asm("	addw2	(r10)+,r11");		/* add a word to sum */
	asm("	adwc	$0,r11");		/* add in carry, end-around */
	asm("	acbl	$2,$-2,r9,loop");	/* done yet? */
	asm("	blbc	r9,ok");		/* odd byte count? */
	asm("	movzbw	(r10),r10");		/* yes, get last byte */
	asm("	addw2	r10,r11");		/* add it in */
	asm("	adwc	$0,r11");		/* and the carry */
	asm("ok:");
	asm("	movl	r11,r0");		/* return sum */
}
#endif

uuwatch()
{
	register struct uu_ctlr *uuc;
	register struct uudevice *uuaddr;
	struct uba_device *ui;
	struct buf *bp, *uutab;
	int s;

	if (uuwstart == 0)
		return;
	for (s=0; s<NUU; s++) {
		int i;

		uuc = &uu_ctlr[s];
		ui = uudinfo[s];
		if (uuc->uu_flag)
			uuc->uu_flag++;
		if (uuc->uu_flag <= 40)
			continue;
		printf("uu%d: read stalled\n", s);
		printf("%X %X %X %X %X %X %X %X\n", uuc->uu_rbptr, uuc->uu_rcnt,
		       uuc->uu_wbptr, uuc->uu_wcnt, uuc->uu_state, uuc->uu_flag,
		       uuc->uu_addr, uuc->uu_count);
		uuc->uu_flag = 0;
		uuaddr = (struct uudevice *)ui->ui_addr;
		uutab = &uitab[s];
		i = uuaddr->rdb;		/* dummy */
		uuaddr->rcs = UUCS_INTR;	/* in case we were flushing */
		uuaddr->tcs = UUCS_INTR;
		uuc->uu_state = UUS_IDLE;
		if (!uutab->b_active) {
			wakeup((caddr_t)uuc);
			continue;
		}
		if (++uutab->b_errcnt <= 1) {
			uustart(ui);
			continue;
		}
		if (bp = uutab->b_actf) {
			bp->b_flags |= B_ERROR;
			if ((bp->b_flags&B_READ) == 0)
				uu_vee(&pcnt[UNIT(minor(bp->b_dev))]);
			iodone(bp);
		}
	}
	timeout(uuwatch, (caddr_t)0, hz);
	return;
}


uuioctl(dev, cmd, data, flag)
	dev_t dev;
	caddr_t data;
{
	/*
	 * to be added later
	 */
	return (ENXIO);
}

uu_pee(cp)
char *cp;
{
	register int s;

	s = splx(UUIPL);
	if (++(*cp) > NTUQ) {
		sleep(cp, PRIBIO);
	}
	splx(s);
}

uu_vee(cp)
char *cp;
{
	register int s;

	s = splx(UUIPL);
	if (--(*cp) <= NTUQ) {
		wakeup(cp);
	}
	splx(s);
}
#endif

