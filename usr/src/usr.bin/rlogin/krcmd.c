/*
 *	$Source: /mit/kerberos/ucb/mit/kcmd/RCS/krcmd.c,v $
 *	$Header: /mit/kerberos/ucb/mit/kcmd/RCS/krcmd.c,v 5.1 89/07/25 15:38:44 kfall Exp Locker: kfall $
 */

#ifndef lint
static char *rcsid_kcmd_c =
"$Header: /mit/kerberos/ucb/mit/kcmd/RCS/krcmd.c,v 5.1 89/07/25 15:38:44 kfall Exp Locker: kfall $";
#endif lint
#define LIBC_SCCS

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)krcmd.c	1.3 (Berkeley) 2/10/89";
#endif /* not lint */

#include <sys/types.h>
#include <stdio.h>
#include <kerberosIV/des.h>
#include <kerberosIV/krb.h>

#define	SERVICE_NAME	"rcmd"

/*
 * krcmd: simplified version of Athena's "kcmd"
 *	returns a socket attached to the destination, -1 or krb error on error 
 *	if fd2p is non-NULL, another socket is filled in for it
 */

int
krcmd(ahost, rport, remuser, cmd, fd2p, realm)
	char	**ahost;
	u_short	rport;
	char	*remuser, *cmd;
	int	*fd2p;
	char	*realm;
{
	int		sock = -1, err = 0;
	KTEXT_ST	ticket;
	long		authopts = 0L;

	err = kcmd(
		&sock,
		ahost,
		rport,
		NULL,	/* locuser not used */
		remuser,
		cmd,
		fd2p,
		&ticket,
		SERVICE_NAME,
		realm,
		(CREDENTIALS *)  NULL,		/* credentials not used */
		(bit_64 *) NULL,		/* key schedule not used */
		(MSG_DAT *) NULL,		/* MSG_DAT not used */
		(struct sockaddr_in *) NULL,	/* local addr not used */
		(struct sockaddr_in *) NULL,	/* foreign addr not used */
		authopts
	);

	if (err > KSUCCESS && err < MAX_KRB_ERRORS) {
		fprintf(stderr, "krcmd: %s\n", krb_err_txt[err]);
		return(-1);
	}
	if (err < 0)
		return(-1);
	return(sock);
}

#include <sys/socket.h>
#include <netinet/in.h>

int
krcmd_mutual(ahost, rport, remuser, cmd, fd2p, realm, cred, sched)
	char		**ahost;
	u_short		rport;
	char		*remuser, *cmd;
	int		*fd2p;
	char		*realm;
	CREDENTIALS	*cred;
	Key_schedule	sched;
{
	int		sock, err;
	KTEXT_ST	ticket;
	MSG_DAT		msg_dat;
	struct sockaddr_in	laddr, faddr;
	long authopts = KOPT_DO_MUTUAL;

	err = kcmd(
		&sock,
		ahost,
		rport,
		NULL,	/* locuser not used */
		remuser,
		cmd,
		fd2p,
		&ticket,
		SERVICE_NAME,
		realm,
		cred,		/* filled in */
		sched,		/* filled in */
		&msg_dat,	/* filled in */
		&laddr,		/* filled in */
		&faddr,		/* filled in */
		authopts
	);

	if (err > KSUCCESS && err < MAX_KRB_ERRORS) {
		fprintf(stderr, "krcmd_mutual: %s\n", krb_err_txt[err]);
		return(-1);
	}

	if (err < 0)
		return (-1);
	return(sock);
}
