/*	defs.h		*/

/*
 */
#include <sys/types.h>
#include <sys/socket.h>

#include <net/route.h>
#include <netxns/xn.h>
#include <netxns/idp.h>

#include <stdio.h>

#include "protocol.h"
#include "trace.h"
#include "interface.h"
#include "table.h"
#include "af.h"


/*
 * When we find any interfaces marked down we rescan the
 * kernel every CHECK_INTERVAL seconds to see if they've
 * come up.
 */
#define	CHECK_INTERVAL	(1*60)

#define equal(a1, a2) \
	(bcmp((caddr_t)(a1), (caddr_t)(a2), sizeof (struct sockaddr)) == 0)
#define	min(a,b)	((a)>(b)?(b):(a))

int	kmem;
int	supplier;		/* process should supply updates */
int	install;		/* if 1 call kernel */
int	lookforinterfaces;	/* if 1 probe kernel for new up interfaces */
int	performnlist;		/* if 1 check if /vmunix has changed */
int	externalinterfaces;	/* # of remote and local interfaces */
int	timeval;		/* local idea of time */

char	packet[MAXPACKETSIZE+sizeof(struct idp)+1];
struct	rip *msg;

char	**argv0;

extern	char *sys_errlist[];
extern	int errno;

char	*malloc();
int	exit();
int	sendmsg();
int	supply();
int	timer();
int	cleanup();
