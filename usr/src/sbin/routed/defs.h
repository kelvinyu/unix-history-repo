/*	defs.h	4.6	82/06/05	*/

/*
 * Internal data structure definitions for
 * user routing process.  Based on Xerox NS
 * protocol specs with mods relevant to more
 * general addressing scheme.
 */
#include <net/route.h>

/*
 * Routing table structure; differs a bit from kernel tables.
 *
 * Note: the union below must agree in the first 4 members
 * so the ioctl's will work.
 */
struct rthash {
	struct	rt_entry *rt_forw;
	struct	rt_entry *rt_back;
};

struct rt_entry {
	struct	rt_entry *rt_forw;
	struct	rt_entry *rt_back;
	union {
		struct	rtentry rtu_rt;
		struct {
			u_long	rtu_hash;
			struct	sockaddr rtu_dst;
			struct	sockaddr rtu_gateway;
			short	rtu_flags;
			short	rtu_retry;
			short	rtu_timer;
			short	rtu_state;
			int	rtu_metric;
			struct	ifnet *rtu_ifp;
		} rtu_entry;
	} rt_rtu;
};

#define	rt_rt		rt_rtu.rtu_rt			/* pass to ioctl */
#define	rt_hash		rt_rtu.rtu_entry.rtu_hash	/* for net or host */
#define	rt_dst		rt_rtu.rtu_entry.rtu_dst	/* match value */
#define	rt_gateway	rt_rtu.rtu_entry.rtu_gateway	/* who to forward to */
#define	rt_flags	rt_rtu.rtu_entry.rtu_flags	/* kernel flags */
#define	rt_retry	rt_rtu.rtu_entry.rtu_retry	/* retries of ioctl */
#define	rt_timer	rt_rtu.rtu_entry.rtu_timer	/* for invalidation */
#define	rt_state	rt_rtu.rtu_entry.rtu_state	/* see below */
#define	rt_metric	rt_rtu.rtu_entry.rtu_metric	/* cost of route */
#define	rt_ifp		rt_rtu.rtu_entry.rtu_ifp	/* interface to take */

#define	ROUTEHASHSIZ	19

/*
 * "State" of routing table entry.
 */
#define	RTS_DELRT	0x1		/* delete pending */
#define	RTS_CHGRT	0x2		/* change command pending */
#define	RTS_ADDRT	0x4		/* add command pending */
#define	RTS_HIDDEN	0x8		/* don't send to router */

struct	rthash nethash[ROUTEHASHSIZ], hosthash[ROUTEHASHSIZ];
struct	rt_entry *rtlookup(), *rtfind();

/*
 * Per address family routines.
 */
struct afswitch {
	int	(*af_hash)();		/* returns keys based on address */
	int	(*af_netmatch)();	/* verifies net # matching */
	int	(*af_output)();		/* interprets address for sending */
	int	(*af_portmatch)();	/* packet from some other router? */
	int	(*af_portcheck)();	/* packet from priviledged peer? */
	int	(*af_checkhost)();	/* tells if address for host or net */
	int	(*af_canon)();		/* canonicalize address for compares */
};

/*
 * Structure returned by af_hash routines.
 */
struct afhash {
	u_int	afh_hosthash;		/* host based hash */
	u_int	afh_nethash;		/* network based hash */
};

struct	afswitch afswitch[AF_MAX];	/* table proper */
