/*	subr_prf.c	4.8	%G%	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/seg.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/mtpr.h"
#include "../h/reboot.h"
#include "../h/vm.h"
#include "../h/msgbuf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */

char	*panicstr;

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 */
/*VARARGS1*/
printf(fmt, x1)
register char *fmt;
unsigned x1;
{

	prf(fmt, &x1, 0);
}

/*
 * print to the current users terminal,
 * guarantee not to sleep (so can be called by intr routine)
 * no watermark checking - so no verbose messages
 */
/*VARARGS1*/
uprintf(fmt, x1)
	char	*fmt;
	unsigned x1;
{

	prf(fmt, &x1, 2);
}

/* THIS CODE IS VAX DEPENDENT */
prf(fmt, adx, touser)
register char *fmt;
register u_int *adx;
{
	register int b, c;
	char *s;

loop:
	while ((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c, touser);
	}
again:
	c = *fmt++;
	switch (c) {

	case 'l':
		goto again;
	case 'x': case 'X':
		b = 16;
		goto number;
	case 'd': case 'D':
	case 'u':		/* what a joke */
		b = 10;
		goto number;
	case 'o': case 'O':
		b = 8;
number:
		printn(*adx, b, touser);
		break;
	case 'c':
		c = *adx;
		while (c) {
			putchar(c&0x7f, touser);
			c >>= 8;
		}
		break;
	case 's':
		s = (char *)*adx;
		while (c = *s++)
			putchar(c, touser);
		break;
	}
	adx++;
	goto loop;
}
/* END VAX DEPENDENT CODE */

printn(n, b, touser)
	unsigned long n;
{
	char buf[11];
	register char *cp;

	if (b == 10 && (int)n < 0) {
		putchar('-', touser);
		n = (unsigned)(-(int)n);
	}
	cp = buf;
	do {
		*cp++ = "0123456789abcdef"[n%b];
		n /= b;
	} while (n);
	do
		putchar(*--cp, touser);
	while (cp > buf);
}

/*
 * Panic is called on unresolvable fatal errors.
 * It syncs, prints "panic: mesg", and then reboots.
 */
panic(s)
char *s;
{

	panicstr = s;
	printf("panic: %s\n", s);
	(void) spl0();
	for(;;)
		boot(RB_PANIC, RB_AUTOBOOT);
}

/*
 * prdev prints a warning message of the
 * form "mesg on dev x/y".
 * x and y are the major and minor parts of
 * the device argument.
 */
prdev(str, dev)
	char *str;
	dev_t dev;
{

	printf("%s on dev %d/%d\n", str, major(dev), minor(dev));
}

/*
 * deverr prints a diagnostic from
 * a device driver.
 * It prints the device, block number,
 * and an octal word (usually some error
 * status register) passed as argument.
 */
deverror(bp, o1, o2)
	register struct buf *bp;
{

	printf("bn=%d er=%x,%x", bp->b_blkno, o1,o2);
	prdev("", bp->b_dev);
}

/*
 * Print a character on console or users terminal.
 * If destination is console then the last MSGBUFS characters
 * are saved in msgbuf for inspection later.
 */
/*ARGSUSED*/
putchar(c, touser)
	register int c;
{

	if (touser) {
		register struct tty *tp = u.u_ttyp;

		if (tp && (tp->t_state&CARR_ON)) {
			register s = spl6();
			if (c == '\n')
				ttyoutput('\r', tp);
			ttyoutput(c, tp);
			ttstart(tp);
			splx(s);
		}
		return;
	}
	if (c != '\0' && c != '\r' && c != 0177 && mfpr(MAPEN)) {
		if (msgbuf.msg_magic != MSG_MAGIC) {
			msgbuf.msg_bufx = 0;
			msgbuf.msg_magic = MSG_MAGIC;
		}
		if (msgbuf.msg_bufx < 0 || msgbuf.msg_bufx >= MSG_BSIZE)
			msgbuf.msg_bufx = 0;
		msgbuf.msg_bufc[msgbuf.msg_bufx++] = c;
	}
	if (c == 0)
		return;
	cnputc(c);
}
