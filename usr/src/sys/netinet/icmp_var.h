/*	icmp_var.h	4.1	83/03/10	*/

/*
 * Variables related to this implementation
 * of the internet control message protocol.
 */
struct	icmpstat {
/* statistics related to icmp packets generated */
	int	icps_error;		/* # of calls to icmp_error */
	int	icps_oldshort;		/* no error 'cuz old ip too short */
	int	icps_oldicmp;		/* no error 'cuz old was icmp */
	int	icps_outhist[ICMP_IREQREPLY + 1];
/* statistics related to input messages processed */
	int	icps_tooshort;		/* packet < ICMP_MINLEN */
	int	icps_checksum;		/* bad checksum */
	int	icps_badlen;		/* calculated bound mismatch */
	int	icps_reflect;		/* number of responses */
	int	icps_inhist[ICMP_IREQREPLY + 1];
};

#ifdef KERNEL
struct	impcstat icmpstat;
#endif
