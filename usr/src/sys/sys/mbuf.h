/* mbuf.h 4.7 81/11/29 */

/*
 * Constants related to memory allocator.
 */
#define	MSIZE		128			/* size of an mbuf */
#define	MMINOFF		12			/* mbuf header length */
#define	MTAIL		4
#define	MMAXOFF		(MSIZE-MTAIL)		/* offset where data ends */
#define	MLEN		(MSIZE-MMINOFF-MTAIL)	/* mbuf data length */
#define	NMBCLUSTERS	256

/*
 * Macros for type conversion
 *
 * CONSTANTS HERE ARE A CROCK
 */

/* network cluster number to virtual address, and back */
#define	cltom(x) ((struct mbuf *)((int)mbutl + ((x) << CLSHIFT)))
#define	mtocl(x) (((int)x - (int)mbutl) >> CLSHIFT)

/* address in mbuf to mbuf head */
#define	dtom(x)		((struct mbuf *)((int)x & ~(MSIZE-1)))

/* mbuf head, to typed data */
#define	mtod(x,t)	((t)((int)(x) + (x)->m_off))

struct mbuf {
	struct	mbuf *m_next;		/* next buffer in chain */
	u_long	m_off;			/* offset of data */
	short	m_len;			/* amount of data in this mbuf */
	short	m_cnt;			/* reference count */
	u_char	m_dat[MLEN];		/* data storage */
	struct	mbuf *m_act;		/* link in higher-level mbuf list */
};

#define	M_WAIT	1

#define	MGET(m, i) \
	{ int ms = splimp(); \
	  if ((m)=mfree) \
		{ mbstat.m_bufs--; mfree = (m)->m_next; (m)->m_next = 0; } \
	  else \
		(m) = m_more(i); \
	  splx(ms); }
#define	MCLGET(m, i) \
	{ int ms = splimp(); \
	  if ((m)=mclfree) \
	      { ++mclrefcnt[mtocl(m)]; nmclfree--; mclfree = (m)->m_next; } \
	  splx(ms); }
#define	MFREE(m, n) \
	{ int ms = splimp(); \
	  if ((m)->m_off > MSIZE) { \
		(n) = (struct mbuf *)(mtod(m, int)&~0x3ff); \
		if (--mclrefcnt[mtocl(n)] == 0) \
		    { (n)->m_next = mclfree; mclfree = (n); nmclfree++; } \
	  } \
	  (n) = (m)->m_next; (m)->m_next = mfree; \
	  (m)->m_off = 0; (m)->m_act = 0; mfree = (m); mbstat.m_bufs++; \
	  splx(ms); }

struct mbstat {
	short	m_bufs;			/* # free msg buffers */
	short	m_hiwat;		/* # free mbufs allocated */
	short	m_lowat;		/* min. # free mbufs */
	short	m_pages;		/* # pages owned by network */
	short	m_drops;		/* times failed to find space */
};

#ifdef	KERNEL
extern	struct mbuf mbutl[];		/* virtual address of net free mem */
extern	struct pte Mbmap[];		/* page tables to map Netutl */
struct	mbstat mbstat;
int	nmbclusters;
struct	mbuf *mfree, *mclfree;
int	nmclfree;
char	mclrefcnt[NMBCLUSTERS];
struct	mbuf *m_get(), *m_getclr(), *m_free(), *m_more(), *m_copy();
#endif
