/*	if_enp.c	1.2	86/11/29	*/

#include "enp.h"
#if NENP > 0
/*
 * Modified 3Com Ethernet Controller interface
 * enp modifications added S. F. Holmgren
 *
 * UNTESTED WITH 4.3
 */
#include "param.h"
#include "systm.h"
#include "mbuf.h"
#include "buf.h"
#include "protosw.h"
#include "socket.h"
#include "vmmac.h"
#include "ioctl.h"
#include "errno.h"
#include "vmparam.h"
#include "syslog.h"
#include "uio.h"

#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"
#ifdef INET
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/in_var.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"
#include "../netinet/if_ether.h"
#endif
#ifdef NS
#include "../netns/ns.h"
#include "../netns/ns_if.h"
#endif

#include "../tahoe/cpu.h"
#include "../tahoe/pte.h"
#include "../tahoe/mtpr.h"

#include "../tahoevba/vbavar.h"
#include "../tahoeif/if_enpreg.h"

#define	ENPVEC	0xc1
#define ENPSTART	0xf02000	/* standard enp start addr */
#define	ENPUNIT(dev)	(minor(dev))	/* for enp ram devices */

int	enpprobe(), enpattach(), enpintr();
long	enpstd[] = { 0xf41000, 0xf61000, 0 };
struct  vba_device *enpinfo[NENP];
struct  vba_driver enpdriver = 
    { enpprobe, 0, enpattach, 0, enpstd, "enp", enpinfo, "enp-20", 0 };

int	enpinit(), enpioctl(), enpreset(), enpoutput();
struct  mbuf *enpget();

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * es_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
struct  enp_softc {
	struct  arpcom es_ac;           /* common ethernet structures */
#define es_if		es_ac.ac_if
#define es_enaddr	es_ac.ac_enaddr
	short	es_flags;		/* flags for devices */
	short	es_ivec;		/* interrupt vector */
	struct	pte *es_map;		/* map for dual ported memory */
	caddr_t	es_ram;			/* virtual address of mapped memory */
} enp_softc[NENP]; 
extern	struct ifnet loif;

enpprobe(reg, vi)
	caddr_t reg;
	struct vba_device *vi;
{
	register br, cvec;		/* must be r12, r11 */
	register struct enpdevice *addr = (struct enpdevice *)reg;
	struct enp_softc *es = &enp_softc[vi->ui_unit];

#ifdef lint
	enpintr(0);
#endif
	if (badaddr(addr, 2) || badaddr(&addr->enp_ram[0], 2))
		return (0);
	es->es_ivec = --vi->ui_hd->vh_lastiv;
	addr->enp_state = S_ENPRESET;		/* reset by VERSAbus reset */
	br = 0x14, cvec = es->es_ivec;		/* XXX */
	return (sizeof (struct enpdevice));
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets. 
 */
enpattach(ui)
	register struct vba_device *ui;
{
	struct enp_softc *es = &enp_softc[ui->ui_unit];
	register struct ifnet *ifp = &es->es_if;
	register struct enpdevice *addr = (struct enpdevice *)ui->ui_addr;

	ifp->if_unit = ui->ui_unit;
	ifp->if_name = "enp";
	ifp->if_mtu = ETHERMTU;
	/*
	 * Get station's addresses.
	 */
	enpcopy(&addr->enp_addr.e_baseaddr, es->es_enaddr,
	    sizeof (es->es_enaddr));
	printf("enp%d: hardware address %s\n", ui->ui_unit,
	    ether_sprintf(es->es_enaddr));
	/*
	 * Allocate and map ram.
	 */
	vbmemalloc(128, ((caddr_t)addr)+0x1000, &es->es_map, &es->es_ram);

	ifp->if_init = enpinit;
	ifp->if_ioctl = enpioctl;
	ifp->if_output = enpoutput;
	ifp->if_reset = enpreset;
	ifp->if_flags = IFF_BROADCAST;
	if_attach(ifp);
}

/*
 * Reset of interface after "system" reset.
 */
enpreset(unit, vban)
	int unit, vban;
{
	register struct vba_device *ui;

	if (unit >= NENP || (ui = enpinfo[unit]) == 0 || ui->ui_alive == 0 ||
	    ui->ui_vbanum != vban)
		return;
	printf(" enp%d", unit);
	enpinit(unit);
}

/*
 * Initialization of interface; clear recorded pending operations.
 */
enpinit(unit)
	int unit;
{
	struct enp_softc *es = &enp_softc[unit];
	register struct vba_device *ui = enpinfo[unit];
	struct enpdevice *addr;
	register struct ifnet *ifp = &es->es_if;
	int s;

	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;
	if ((ifp->if_flags & IFF_RUNNING) == 0) {
		addr = (struct enpdevice *)ui->ui_addr;
		s = splimp();
		RESET_ENP(addr);
		DELAY(200000);
		addr->enp_intrvec = es->es_ivec;
		es->es_if.if_flags |= IFF_RUNNING;
		splx(s);
	}
}

/*
 * Ethernet interface interrupt.
 */
enpintr(unit)
	int unit;
{
	register struct enpdevice *addr;
	register BCB *bcbp;

	addr = (struct enpdevice *)enpinfo[unit]->ui_addr;
	if (!IS_ENP_INTR(addr))
		return;
	ACK_ENP_INTR(addr);
	while ((bcbp = (BCB *)ringget(&addr->enp_tohost )) != 0) {
		(void) enpread(&enp_softc[unit], bcbp, unit);
		ringput(&addr->enp_enpfree, bcbp); 
	}
}

/*
 * Read input packet, examine its packet type, and enqueue it.
 */
enpread(es, bcbp, unit)
	struct enp_softc *es;
	register BCB *bcbp;
	int unit;
{
	register struct ether_header *enp;
	struct mbuf *m;
	long int s;
	int len, off, resid, enptype;
	register struct ifqueue *inq;

	es->es_if.if_ipackets++; 
	/*
	 * Get input data length.
	 * Get pointer to ethernet header (in input buffer).
	 * Deal with trailer protocol: if type is PUP trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
	len = bcbp->b_msglen - sizeof (struct ether_header);
	enp = (struct ether_header *)bcbp->b_addr;
#define enpdataaddr(enp, off, type) \
    ((type)(((caddr_t)(((char *)enp)+sizeof (struct ether_header))+(off))))
	enp->ether_type = ntohs((u_short)enp->ether_type);
	if (enp->ether_type >= ETHERTYPE_TRAIL &&
	    enp->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
		off = (enp->ether_type - ETHERTYPE_TRAIL) * 512;
		if (off >= ETHERMTU)
			goto setup;
		enp->ether_type = ntohs(*enpdataaddr(enp, off, u_short *));
		resid = ntohs(*(enpdataaddr(enp, off+2, u_short *)));
		if (off + resid > len)
			goto setup;
		len = off + resid;
	} else
		off = 0;
	if (len == 0)
		goto setup;

	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; enpget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	m = enpget(bcbp->b_addr, len, off, &es->es_if);
	if (m == 0)
		goto setup;
	if (off) {
		struct ifnet *ifp;

		ifp = *(mtod(m, struct ifnet **));
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
		*(mtod(m, struct ifnet **)) = ifp;
	}
	switch (enp->ether_type) {

#ifdef INET
	case ETHERTYPE_IP:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;
#endif
	case ETHERTYPE_ARP:
		arpinput(&es->es_ac, m);
		goto setup;

#ifdef NS
	case ETHERTYPE_NS:
		schednetisr(NETISR_NS);
		inq = &nsintrq;
		break;
#endif
	default:
		m_freem(m);
		goto setup;
	}
	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
		goto setup;
	}
	s = splimp();
	IF_ENQUEUE(inq, m);
	splx(s);
setup:
	return (0);
}

/*
 * Ethernet output routine. (called by user)
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 * If destination is this address or broadcast, send packet to
 * loop device to kludge around the fact that 3com interfaces can't
 * talk to themselves.
 */
enpoutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	register struct enp_softc *es = &enp_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *enp;
	register int off, i;
	struct mbuf *mcopy = (struct mbuf *)0;
	int type, s, error, usetrailers;
	u_char edst[6];
	struct in_addr idst;

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		goto bad;
	}
	switch (dst->sa_family) {
#ifdef INET
	case AF_INET:
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&es->es_ac, m, &idst, edst, &usetrailers))
			return (0);	/* if not yet resolved */
		if (!bcmp((caddr_t)edst, (caddr_t)etherbroadcastaddr,
		    sizeof (edst)))
			mcopy = m_copy(m, 0, (int)M_COPYALL);
		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
		if (usetrailers && off > 0 && (off & 0x1ff) == 0 &&
		    m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
			type = ETHERTYPE_TRAIL + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = ETHERTYPE_IP;
			*(mtod(m, u_short *) + 1) = m->m_len;
			goto gottrailertype;
		}
		type = ETHERTYPE_IP;
		off = 0;
		goto gottype;
#endif
#ifdef NS
	case AF_NS:
		bcopy((caddr_t)&(((struct sockaddr_ns *)dst)->sns_addr.x_host),
		    (caddr_t)edst, sizeof (edst));
		if (!bcmp((caddr_t)edst, (caddr_t)&ns_broadhost, sizeof (edst)))
			mcopy = m_copy(m, 0, (int)M_COPYALL);
		else if (!bcmp((caddr_t)edst, (caddr_t)&ns_thishost,
		    sizeof (edst)))
			return (looutput(&loif, m, dst));
		type = ETHERTYPE_NS;
		off = 0;
		goto gottype;
#endif
	case AF_UNSPEC:
		enp = (struct ether_header *)dst->sa_data;
		bcopy((caddr_t)enp->ether_dhost, (caddr_t)edst, sizeof (edst));
		type = enp->ether_type;
		goto gottype;

	default:
		log(LOG_ERR, "enp%d: can't handle af%d\n",
		    ifp->if_unit, dst->sa_family);
		error = EAFNOSUPPORT;
		goto bad;
	}

gottrailertype:
	/*
	 * Packet to be sent as trailer: move first packet
	 * (control information) to end of chain.
	 */
	while (m->m_next)
		m = m->m_next;
	m->m_next = m0;
	m = m0->m_next;
	m0->m_next = 0;
	m0 = m;

gottype:
	/*
         * Add local net header.  If no space in first mbuf,
         * allocate another.
         */
	if (m->m_off > MMAXOFF ||
	    MMINOFF + sizeof (struct ether_header) > m->m_off) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct ether_header);
	} else {
		m->m_off -= sizeof (struct ether_header);
		m->m_len += sizeof (struct ether_header);
	}
	enp = mtod(m, struct ether_header *);
	bcopy((caddr_t)edst, (caddr_t)enp->ether_dhost, sizeof (edst));
	bcopy((caddr_t)es->es_enaddr, (caddr_t)enp->ether_shost,
	    sizeof (es->es_enaddr));
	enp->ether_type = htons((u_short)type);

	/*
	 * Queue message on interface if possible 
	 */
	s = splimp();	
	if (enpput(ifp->if_unit, m)) {
		error = ENOBUFS;
		goto qfull;
	}
	splx(s);	
	es->es_if.if_opackets++; 
	return (mcopy ? looutput(&loif, mcopy, dst) : 0);
qfull:
	splx(s);	
	m0 = m;
bad:
	m_freem(m0);
	if (mcopy)
		m_freem(mcopy);
	return (error);
}

/*
 * Routine to copy from mbuf chain to transmitter buffer on the VERSAbus.
 */
enpput(unit, m)
	int unit;
	struct mbuf *m;
{
	register BCB *bcbp;
	register struct enpdevice *addr;
	register struct mbuf *mp;
	register u_char *bp;
	register u_int len;
	u_char *mcp;

	addr = (struct enpdevice *)enpinfo[unit]->ui_addr;
	if (ringempty(&addr->enp_hostfree)) 
		return (1);	
	bcbp = (BCB *)ringget(&addr->enp_hostfree);
	bcbp->b_len = 0;
	bp = (u_char *)bcbp->b_addr;
	for (mp = m; mp; mp = mp->m_next) {
		len = mp->m_len;
		if (len == 0)
			continue;
		mcp = mtod(mp, u_char *);
		enpcopy(mcp, bp, len);
		bp += len;
		bcbp->b_len += len;
	}
	bcbp->b_len = max(ETHERMIN, bcbp->b_len);
	bcbp->b_reserved = 0;
	if (ringput(&addr->enp_toenp, bcbp) == 1)
		INTR_ENP(addr);
	m_freem(m);
	return (0);
}

/*
 * Routine to copy from VERSAbus memory into mbufs.
 *
 * Warning: This makes the fairly safe assumption that
 * mbufs have even lengths.
 */
struct mbuf *
enpget(rxbuf, totlen, off0, ifp)
	u_char *rxbuf;
	int totlen, off0;
	struct ifnet *ifp;
{
	register u_char *cp, *mcp;
	register struct mbuf *m;
	struct mbuf *top = 0, **mp = &top;
	int len, off = off0;

	cp = rxbuf + sizeof (struct ether_header);
	while (totlen > 0) {
		MGET(m, M_DONTWAIT, MT_DATA);
		if (m == 0) 
			goto bad;
		if (off) {
			len = totlen - off;
			cp = rxbuf + sizeof (struct ether_header) + off;
		} else
			len = totlen;
		if (len >= NBPG) {
			struct mbuf *p;

			MCLGET(m);
			if (m->m_len == CLBYTES)
				m->m_len = len = MIN(len, CLBYTES);
			else
				m->m_len = len = MIN(MLEN, len);
		} else {
			m->m_len = len = MIN(MLEN, len);
			m->m_off = MMINOFF;
		}
		mcp = mtod(m, u_char *);
		if (ifp) {
			/*
			 * Prepend interface pointer to first mbuf.
			 */
			*(mtod(m, struct ifnet **)) = ifp;
			mcp += sizeof (ifp);
			len -= sizeof (ifp);
			ifp = (struct ifnet *)0;
		}
		enpcopy(cp, mcp, len);
		cp += len;
		*mp = m;
		mp = &m->m_next;
		if (off == 0) {
			totlen -= len;
			continue;
		}
		off += len;
		if (off == totlen) {
			cp = rxbuf + sizeof (struct ether_header);
			off = 0;
			totlen = off0;
		}
	}
	return (top);
bad:
	m_freem(top);
	return (0);
}

enpcopy(from, to, cnt)
	register char *from, *to;
	register cnt;
{
	register c;
	register short *f, *t;

	if (((int)from&01) && ((int)to&01)) {
		/* source & dest at odd addresses */
		*to++ = *from++;
		--cnt;
	}
	if (cnt > 1 && (((int)to&01) == 0) && (((int)from&01) == 0)) {
		t = (short *)to;
		f = (short *)from;
		for (c = cnt>>1; c; --c)	/* even address copy */
			*t++ = *f++;
		cnt &= 1;
		if (cnt) {			/* odd len */
			from = (char *)f;
			to = (char *)t;
			*to = *from;
		}
	}
	while (cnt-- > 0)	/* one of the address(es) must be odd */
		*to++ = *from++;
}

/*
 * Process an ioctl request.
 */
enpioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	struct enpdevice *addr;
	int s = splimp(), error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		switch (ifa->ifa_addr.sa_family) {
#ifdef INET
		case AF_INET:
			enpinit(ifp->if_unit);
			((struct arpcom *)ifp)->ac_ipaddr =
			    IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif
#ifdef NS
		case AF_NS: {
			struct ns_addr *ina = &IA_SNS(ifa)->sns_addr;
			struct enp_softc *es = &enp_softc[ifp->if_unit];

			if (!ns_nullhost(*ina)) {
				ifp->if_flags &= ~IFF_RUNNING;
				addr = (struct enpdevice *)
				    enpinfo[ifp->if_unit]->ui_addr;
				enpsetaddr(ifp->if_unit, addr,
				    ina->x_host.c_host);
			} else
				ina->x_host = *(union ns_host *)es->es_enaddr;
			enpinit(ifp->if_unit);
			break;
		}
#endif
		default:
			enpinit(ifp->if_unit);
			break;
		}
		break;

	case SIOCSIFFLAGS:
		if ((ifp->if_flags&IFF_UP) == 0 && ifp->if_flags&IFF_RUNNING) {
			enpinit(ifp->if_unit);		/* reset board */
			ifp->if_flags &= ~IFF_RUNNING;
		} else if (ifp->if_flags&IFF_UP &&
		     (ifp->if_flags&IFF_RUNNING) == 0)
			enpinit(ifp->if_unit);
		break;

	default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}

enpsetaddr(unit, addr, enaddr)
	int unit;
	struct enpdevice *addr;
	u_char *enaddr;
{
	u_char *cp;
	int i, code;

	cp = &addr->enp_addr.e_baseaddr.ea_addr[0];
	for (i = 0; i < 6; i++)
		*cp++ = ~*enaddr++;
	enpcopy(&addr->enp_addr.e_listsize, &code, sizeof (code)); 
	code |= E_ADDR_SUPP;
	enpcopy(&code, &addr->enp_addr.e_listsize, sizeof (code)); 
	enpinit(unit);
}

/* 
 * Routines to synchronize enp and host.
 */
static
ringinit(rp, size)
	register RING *rp;
{
	register int i;
	register short *sp; 

	rp->r_rdidx = rp->r_wrtidx = 0;
	rp->r_size = size;
}

static
ringempty(rp)
	register RING *rp;
{

	return (rp->r_rdidx == rp->r_wrtidx);
}

static
ringfull(rp)
	register RING *rp;
{
	register short idx;

	idx = (rp->r_wrtidx + 1) & (rp->r_size-1);
	return (idx == rp->r_rdidx);
}

static
ringput(rp, v)
	register RING *rp;
{
	register int idx;

	idx = (rp->r_wrtidx + 1) & (rp->r_size-1);
	if (idx != rp->r_rdidx) {
		rp->r_slot[rp->r_wrtidx] = v;
		rp->r_wrtidx = idx;
		if ((idx -= rp->r_rdidx) < 0)
			idx += rp->r_size;
		return (idx);			/* num ring entries */
	}
	return (0);
}

static
ringget(rp)
	register RING *rp;
{
	register int i = 0;

	if (rp->r_rdidx != rp->r_wrtidx) {
		i = rp->r_slot[rp->r_rdidx];
		rp->r_rdidx = (++rp->r_rdidx) & (rp->r_size-1);
	}
	return (i);
}

static
fir(rp)
	register RING *rp;
{

	return (rp->r_rdidx != rp->r_wrtidx ? rp->r_slot[rp->r_rdidx] : 0);
}

/*
 * ENP Ram device.
 */
enpr_open(dev)
	dev_t dev;
{
	register int unit = ENPUNIT(dev);
	struct vba_device *ui;
	struct enpdevice *addr;

	if (unit >= NENP || (ui = enpinfo[unit]) == 0 || ui->ui_alive == 0 ||
	    (addr = (struct enpdevice *)ui->ui_addr) == 0)
		return (ENODEV);
	if (addr->enp_state != S_ENPRESET)
		return (EACCES);  /* enp is not in reset state, don't open  */
	return (0);
}

enpr_close(dev)
	dev_t dev;
{

	return (0);
}

enpr_read(dev, uio)
	dev_t dev;
	register struct uio *uio;
{
	register struct iovec *iov;
	struct enpdevice *addr;
	int error;

	if (uio->uio_offset > RAM_SIZE)
		return (ENODEV);
	if (uio->uio_offset + iov->iov_len > RAM_SIZE)
		iov->iov_len = RAM_SIZE - uio->uio_offset;
	addr = (struct enpdevice *)enpinfo[ENPUNIT(dev)]->ui_addr;
	iov = uio->uio_iov;
	error = useracc(iov->iov_base, iov->iov_len, 0);
	if (error)
		return (error);
	enpcopy(&addr->enp_ram[uio->uio_offset], iov->iov_base, iov->iov_len);
	uio->uio_resid -= iov->iov_len;
	iov->iov_len = 0;
	return (0);
}

enpr_write(dev, uio)
	dev_t dev;
	register struct uio *uio;
{
	register struct enpdevice *addr;
	register struct iovec *iov;
	register error;

	addr = (struct enpdevice *)enpinfo[ENPUNIT(dev)]->ui_addr;
	iov = uio->uio_iov;
	if (uio->uio_offset > RAM_SIZE)
		return (ENODEV);
	if (uio->uio_offset + iov->iov_len > RAM_SIZE)
		iov->iov_len = RAM_SIZE - uio->uio_offset;
	error =  useracc(iov->iov_base, iov->iov_len, 1);
	if (error)
		return (error);
	enpcopy(iov->iov_base, &addr->enp_ram[uio->uio_offset], iov->iov_len);
	uio->uio_resid -= iov->iov_len;
	iov->iov_len = 0;
	return (0);
}

enpr_ioctl(dev, cmd, data)
	dev_t dev;
	caddr_t data;
{
	register struct enpdevice *addr;
	register unit = ENPUNIT(dev);
	register struct vba_device *ui;

	addr = (struct enpdevice *)enpinfo[unit]->ui_addr;
	switch(cmd) {

	case ENPIOGO:
/* not needed if prom based version */
		addr->enp_base = (int)addr;
		addr->enp_intrvec = enp_softc[unit].es_ivec;
		ENP_GO(addr, ENPSTART);
		DELAY(200000);
		enpinit(unit);
		addr->enp_state = S_ENPRUN;  /* it is running now */
/* end of not needed */
		break;

	case ENPIORESET:
		RESET_ENP(addr);
		addr->enp_state = S_ENPRESET;  /* it is reset now */
		DELAY(100000);
		break;
	}
	return (0);
}
#endif
