.\" Copyright (c) 1986 Regents of the University of California.
.\" All rights reserved.  The Berkeley software License Agreement
.\" specifies the terms and conditions for redistribution.
.\"
.\"	@(#)dgramread.c	6.1 (Berkeley) %G%
.\"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

/*
 * In the included file <netinet/in.h> a sockaddr_in is defined as follows:
 * struct sockaddr_in {
 *	short	sin_family;
 *	u_short	sin_port;
 *	struct in_addr sin_addr;
 *	char	sin_zero[8];
 * }; 
 */

/*
 * This program creates a datagram socket, binds a name to it, then reads
 * from the socket.  
 */

main()
{
	int             sock, length;
	struct sockaddr_in name;
	char            buf[1024];

	/* Create socket from which to read. */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("opening datagram socket");
		exit(-1);
	}
	/* Create name with wildcards. */
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = 0;
	if (bind(sock, &name, sizeof(name))) {
		close(sock);
		perror("binding datagram socket");
	}
	/* Find assigned port value and print it out. */
	length = sizeof(name);
	if (getsockname(sock, &name, &length)) {
		close(sock);
		perror("getting socket name");
	}
	printf("Socket has port #%d\en", ntohs(name.sin_port));
	/* Read from the socket */
	if (read(sock, buf, 1024, 0) < 0)
		perror("receiving datagram packet");
	printf("-->%s\en", buf);
	close(sock);
}
