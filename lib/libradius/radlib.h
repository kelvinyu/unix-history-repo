/*-
 * Copyright 1998 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD$
 */

#ifndef _RADLIB_H_
#define _RADLIB_H_

#include <sys/types.h>
#include <netinet/in.h>

/* Message types */
#define RAD_ACCESS_REQUEST		1
#define RAD_ACCESS_ACCEPT		2
#define RAD_ACCESS_REJECT		3
#define RAD_ACCESS_CHALLENGE		11

/* Attribute types and values */
#define RAD_USER_NAME			1	/* String */
#define RAD_USER_PASSWORD		2	/* String */
#define RAD_CHAP_PASSWORD		3	/* String */
#define RAD_NAS_IP_ADDRESS		4	/* IP address */
#define RAD_NAS_PORT			5	/* Integer */
#define RAD_SERVICE_TYPE		6	/* Integer */
	#define RAD_LOGIN			1
	#define RAD_FRAMED			2
	#define RAD_CALLBACK_LOGIN		3
	#define RAD_CALLBACK_FRAMED		4
	#define RAD_OUTBOUND			5
	#define RAD_ADMINISTRATIVE		6
	#define RAD_NAS_PROMPT			7
	#define RAD_AUTHENTICATE_ONLY		8
	#define RAD_CALLBACK_NAS_PROMPT		9
#define RAD_FRAMED_PROTOCOL		7	/* Integer */
	#define RAD_PPP				1
	#define RAD_SLIP			2
	#define RAD_ARAP			3	/* Appletalk */
	#define RAD_GANDALF			4
	#define RAD_XYLOGICS			5
#define RAD_FRAMED_IP_ADDRESS		8	/* IP address */
#define RAD_FRAMED_IP_NETMASK		9	/* IP address */
#define RAD_FRAMED_ROUTING		10	/* Integer */
#define RAD_FILTER_ID			11	/* String */
#define RAD_FRAMED_MTU			12	/* Integer */
#define RAD_FRAMED_COMPRESSION		13	/* Integer */
#define RAD_LOGIN_IP_HOST		14	/* IP address */
#define RAD_LOGIN_SERVICE		15	/* Integer */
#define RAD_LOGIN_TCP_PORT		16	/* Integer */
     /* unassiged			17 */
#define RAD_REPLY_MESSAGE		18	/* String */
#define RAD_CALLBACK_NUMBER		19	/* String */
#define RAD_CALLBACK_ID			20	/* String */
     /* unassiged			21 */
#define RAD_FRAMED_ROUTE		22	/* String */
#define RAD_FRAMED_IPX_NETWORK		23	/* IP address */
#define RAD_STATE			24	/* String */
#define RAD_CLASS			25	/* Integer */
#define RAD_VENDOR_SPECIFIC		26	/* Integer */
#define RAD_SESSION_TIMEOUT		27	/* Integer */
#define RAD_IDLE_TIMEOUT		28	/* Integer */
#define RAD_TERMINATION_ACTION		29	/* Integer */
#define RAD_CALLED_STATION_ID		30	/* String */
#define RAD_CALLING_STATION_ID		31	/* String */
#define RAD_NAS_IDENTIFIER		32	/* Integer */
#define RAD_PROXY_STATE			33	/* Integer */
#define RAD_LOGIN_LAT_SERVICE		34	/* Integer */
#define RAD_LOGIN_LAT_NODE		35	/* Integer */
#define RAD_LOGIN_LAT_GROUP		36	/* Integer */
#define RAD_FRAMED_APPLETALK_LINK	37	/* Integer */
#define RAD_FRAMED_APPLETALK_NETWORK	38	/* Integer */
#define RAD_FRAMED_APPLETALK_ZONE	39	/* Integer */
     /* reserved for accounting		40-59 */
#define RAD_CHAP_CHALLENGE		60	/* String */
#define RAD_NAS_PORT_TYPE		61	/* Integer */
#define RAD_PORT_LIMIT			62	/* Integer */
#define RAD_LOGIN_LAT_PORT		63	/* Integer */

struct rad_handle;

__BEGIN_DECLS
int			 rad_add_server(struct rad_handle *,
			    const char *, int, const char *, int, int);
void			 rad_close(struct rad_handle *);
int			 rad_config(struct rad_handle *, const char *);
int			 rad_create_request(struct rad_handle *, int);
struct in_addr		 rad_cvt_addr(const void *);
u_int32_t		 rad_cvt_int(const void *);
char			*rad_cvt_string(const void *, size_t);
int			 rad_get_attr(struct rad_handle *, const void **,
			    size_t *);
struct rad_handle	*rad_open(void);
int			 rad_put_addr(struct rad_handle *, int, struct in_addr);
int			 rad_put_attr(struct rad_handle *, int,
			    const void *, size_t);
int			 rad_put_int(struct rad_handle *, int, u_int32_t);
int			 rad_put_string(struct rad_handle *, int,
			    const char *);
int			 rad_send_request(struct rad_handle *);
const char		*rad_strerror(struct rad_handle *);
__END_DECLS

#endif /* _RADLIB_H_ */
