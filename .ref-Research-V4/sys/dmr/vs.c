#
/*
 * Screw Works interface via DC-11
 */

#include "/sys/nsys/tty.h"

#define	VSADDR	0174150
#define	CDLEAD	01
#define	B1200	030
#define	STOP1	0400
#define	CLSEND	02
#define	RQSEND	01

struct {
	int	vsrcsr;
	int	vsrbuf;
	int	vsxcsr;
	int	vsxbuf;
};

struct {
	struct	clist	iq;
	struct	clist	oq;
} vs;

char	vsmap[] "?0*#?546?213?879?";

vsopen(dev)
{
	VSADDR->vsrcsr = IENABLE|B1200|CDLEAD;
	VSADDR->vsxcsr = STOP1|IENABLE|B1200;
	vschar(0);
}

vsclose(dev)
{
	VSADDR->vsrcsr =& ~IENABLE;
	while (getc(&vs.iq) >= 0);
	while (getc(&vs.oq) >= 0);
}

vswrite(dev)
{
	int count, c;

	count = 0;
	while (cpass(&c) >= 0) {
		if (--count <= 0) {
			count = 60;
			vschar(0);
		}
		vschar(c);
	}
	vschar(0);
}

vschar(c)
{

	c =^ 0277;		/* magic map */
	spl5();
	while (vs.oq.c_cc > 60) {
		vsxintr();
		sleep(&vs.oq, TTIPRI);
	}
	putc(c, &vs.oq);
	vsxintr();
	spl0();
}

vsxintr()
{
	static lchar;
	register c;
	register int *xcsr;

	xcsr = &VSADDR->vsxcsr;
	if (*xcsr&DONE) {
		if (lchar==0277) {
			*xcsr =& ~RQSEND;
			lchar = 0;
			if (vs.oq.c_cc==0)
				goto wake;
		}
		if ((*xcsr&CLSEND) == 0) {
			*xcsr =& ~RQSEND;
			*xcsr =| RQSEND;
			if ((*xcsr&CLSEND) == 0)
				goto wake;
		}
		if ((c = getc(&vs.oq)) >= 0)
			VSADDR->vsxbuf = lchar = c;
		if (vs.oq.c_cc <= 15)
	    wake:
			wakeup(&vs.oq);
	}
}

vsread(dev)
{
	register int c;

	spl5();
	while ((c = getc(&vs.iq)) < 0)
		sleep(&vs.iq, TTIPRI);
	spl0();
	passc(vsmap[c&017]);
}

vsrintr()
{
	register int c;

	c = VSADDR->vsrbuf;
	if (vs.iq.c_cc<=10)
		putc(c, &vs.iq);
	wakeup(&vs.iq);
}
