/*	tcp_fsm.h	4.6	81/11/18	*/

/*
 * TCP FSM state definitions.
 *
 * This TCP machine has two more states than suggested in RFC 793,
 * the extra states being L_SYN_RCVD and RCV_WAIT
 *
 * EXPLAIN THE EXTRA STATES.
 */

/*
 * States
 */
#define	TCP_NSTATES	14

#define	EFAILEC		-1		/* new state for failure, internally */
#define	SAME		0		/* no state change, internally */
#define	LISTEN		1		/* listening for connection */
#define	SYN_SENT	2		/* active, have sent syn */
#define	SYN_RCVD	3
#define	L_SYN_RCVD	4
#define	ESTAB		5		/* established */
#define	FIN_W1		6		/* have closed and sent fin */
#define	FIN_W2		7		/* have closed and rcvd ack of fin */
#define	TIME_WAIT	8		/* in 2*msl quiet wait after close */
#define	CLOSE_WAIT	9		/* rcvd fin, waiting for close */
#define	CLOSING		10		/* closed xchd FIN; await FIN ACK */
#define	LAST_ACK	11		/* had fin and close; await FIN ACK */
#define	RCV_WAIT	12		/* waiting for user to drain data */
#define	CLOSED		13		/* closed */

#ifdef KPROF
int	tcp_acounts[TCP_NSTATES][PRU_NREQ];
#endif

#ifdef TCPSTATES
char *tcpstates[] = {
	"SAME",		"LISTEN",	"SYN_SENT",	"SYN_RCVD",
	"L_SYN_RCVD",	"ESTAB",	"FIN_W1",	"FIN_W2",
	"TIME_WAIT",	"CLOSE_WAIT",	"CLOSING",	"LAST_ACK",
	"RCV_WAIT",	"CLOSED"
};
char *tcptimers[] = { "INIT", "REXMT", "REXMTTL", "PERSIST", "FINACK" };
#endif
