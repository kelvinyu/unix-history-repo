/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)kerberos5.c	5.1 (Berkeley) %G%";
#endif /* not lint */

/*
 * Copyright (C) 1990 by the Massachusetts Institute of Technology
 *
 * Export of this software from the United States of America is assumed
 * to require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */


#ifdef	KRB5
#include <arpa/telnet.h>
#include <stdio.h>
#include <krb5/krb5.h>
#include <krb5/crc-32.h>
#include <krb5/libos-proto.h>
#include <netdb.h>
#include <ctype.h>

#ifdef	__STDC__
#include <stdlib.h>
#endif
#ifdef	NO_STRING_H
#include <strings.h>
#else
#include <string.h>
#endif

#include "encrypt.h"
#include "auth.h"
#include "misc.h"

extern auth_debug_mode;

char *malloc();

static unsigned char str_data[1024] = { IAC, SB, TELOPT_AUTHENTICATION, 0,
			  		AUTHTYPE_KERBEROS_V5, };

#define	KRB_AUTH	0		/* Authentication data follows */
#define	KRB_REJECT	1		/* Rejected (reason might follow) */
#define	KRB_ACCEPT	2		/* Accepted (name might follow) */
#define	KRB_NEWKEY	3		/* New key to use */
#define KRB_NAME        4               /* Name to authenticate for */

static	krb5_data auth;
	/* telnetd gets session key from here */
static	krb5_tkt_authent *authdat = NULL;

#if	defined(ENCRYPT)
Block	session_key;
#endif

	static int
Data(type, d, c)
	int type;
	void *d;
	int c;
{
        unsigned char *p = str_data + 6;
	unsigned char *cd = (unsigned char *)d;

	if (c == -1)
		c = strlen(d);

        if (auth_debug_mode) {
                printf("%s:%d: [%d] (%d)",
                        str_data[3] == TELQUAL_IS ? ">>>IS" : ">>>REPLY",
                        str_data[3],
                        type, c);
                printd(d, c);
                printf("\r\n");
        }
	*p++ = type;
        while (c-- > 0) {
                if ((*p++ = *cd++) == IAC)
                        *p++ = IAC;
        }
        *p++ = IAC;
        *p++ = SE;
	if (str_data[3] == TELQUAL_IS)
		printsub('>', &str_data[2], p - &str_data[2]);
        return(net_write(str_data, p - str_data));
}

	int
kerberos5_init(ap, server)
	Authenticator *ap;
	int server;
{
	if (server)
		str_data[3] = TELQUAL_REPLY;
	else
		str_data[3] = TELQUAL_IS;
	str_data[4] = ap->type;
	str_data[5] = ap->way;
        krb5_init_ets();
	return(1);
}

	int
kerberos5_send(ap)
	Authenticator *ap;
{
	char **realms;
	char *name;
	char *p1, *p2;
	krb5_checksum ksum;
	krb5_octet sum[CRC32_CKSUM_LENGTH];
	krb5_data *server[4];
	krb5_data srvdata[3];
	krb5_error_code r;
	krb5_ccache ccache;
	krb5_creds creds;		/* telnet gets session key from here */
	extern krb5_flags krb5_kdc_default_options;

	ksum.checksum_type = CKSUMTYPE_CRC32;
	ksum.contents = sum;
	ksum.length = sizeof(sum);
	bzero((void *)sum, sizeof(sum));
	
        if (!UserNameRequested) {
                if (auth_debug_mode) {
                        printf("Kerberos V5: no user name supplied\r\n");
                }
                return(0);
        }

	if (r = krb5_cc_default(&ccache)) {
		if (auth_debug_mode) {
			printf("Kerberos V5: could not get default ccache\r\n");
		}
		return(0);
	}

	if ((name = malloc(strlen(RemoteHostName)+1)) == NULL) {
		if (auth_debug_mode)
			printf("Out of memory for hostname in Kerberos V5\r\n");
		return(0);
	}

	if (r = krb5_get_host_realm(RemoteHostName, &realms)) {
		if (auth_debug_mode)
			printf("Kerberos V5: no realm for %s\r\n", RemoteHostName);
		free(name);
		return(0);
	}

	p1 = RemoteHostName;
	p2 = name;

	while (*p2 = *p1++) {
		if (isupper(*p2))
			*p2 |= 040;
		++p2;
	}

	srvdata[0].data = realms[0];
	srvdata[0].length = strlen(realms[0]);
	srvdata[1].data = "rcmd";
	srvdata[1].length = 4;
	srvdata[2].data = name;
	srvdata[2].length = p2 - name;

	server[0] = &srvdata[0];
	server[1] = &srvdata[1];
	server[2] = &srvdata[2];
	server[3] = 0;

	bzero((char *)&creds, sizeof(creds));
	creds.server = (krb5_principal)server;

	if (r = krb5_cc_get_principal(ccache, &creds.client)) {
		if (auth_debug_mode) {
			printf("Keberos V5: failure on principal (%d)\r\n",
				error_message(r));
		}
		free(name);
		krb5_free_host_realm(realms);
		return(0);
	}

	if (r = krb5_get_credentials(krb5_kdc_default_options, ccache, &creds)) {
		if (auth_debug_mode) {
			printf("Keberos V5: failure on credentials(%d)\r\n",r);
		}
		free(name);
		krb5_free_host_realm(realms);
		return(0);
	}

	r = krb5_mk_req_extended(0, &ksum, &creds.times,
				 krb5_kdc_default_options,
				 ccache, &creds, 0, &auth);

	free(name);
	krb5_free_host_realm(realms);
	if (r) {
		if (auth_debug_mode) {
			printf("Keberos V5: mk_req failed\r\n");
		}
		return(0);
	}

        if (!Data(KRB_NAME, (void *)UserNameRequested, -1)) {
                if (auth_debug_mode)
                        printf("Not enough room for user name\r\n");
                return(0);
        }
	if (!Data(KRB_AUTH, auth.data, auth.length)) {
		if (auth_debug_mode)
			printf("Not enough room for authentication data\r\n");
		return(0);
	}
#if     defined(ENCRYPT)
	if (creds.keyblock.keytype == KEYTYPE_DES) {
		Schedule krb_sched;
		Block enckey;

		des_key_sched(creds.keyblock.contents, krb_sched);
		des_set_random_generator_seed(creds.keyblock.contents);
		des_new_random_key(session_key);
		des_ecb_encrypt(session_key, enckey, krb_sched, 1);
		Data(KRB_NEWKEY, (void *)enckey, sizeof(enckey));
	}
#endif
	if (auth_debug_mode) {
		printf("Sent Kerberos V5 credentials to server\r\n");
	}
	return(1);
}

	void
kerberos5_is(ap, data, cnt)
	Authenticator *ap;
	unsigned char *data;
	int cnt;
{
	int r;
	struct hostent *hp;
	char *p1, *p2;
	static char *realm = NULL;
	krb5_data *server[4];
	krb5_data srvdata[3];
        Schedule sched;
        Session_Key skey;
	char *name;
	char *getenv();

	if (cnt-- < 1)
		return;
	switch (*data++) {
        case KRB_NAME: {
                char user[256];

                if (cnt > 255)
                        cnt = 255;
                strncpy(user, data, cnt);
                user[cnt] = 0;
                auth_encrypt_user(user);
                return;
            }
	case KRB_AUTH:
		auth.data = (char *)data;
		auth.length = cnt;

		if (!(hp = gethostbyname(LocalHostName))) {
			if (auth_debug_mode)
				printf("Cannot resolve local host name\r\n");
			Data(KRB_REJECT, "Unknown local hostname.", -1);
			auth_finished(ap, AUTH_REJECT);
			return;
		}

		if (!realm && (krb5_get_default_realm(&realm))) {
			if (auth_debug_mode)
				printf("Could not get defualt realm\r\n");
			Data(KRB_REJECT, "Could not get default realm.", -1);
			auth_finished(ap, AUTH_REJECT);
			return;
		}

		if ((name = malloc(strlen(hp->h_name)+1)) == NULL) {
			if (auth_debug_mode)
				printf("Out of memory for hostname in Kerberos V5\r\n");
			Data(KRB_REJECT, "Out of memory.", -1);
			auth_finished(ap, AUTH_REJECT);
			return;
		}

		p1 = hp->h_name;
		p2 = name;

		while (*p2 = *p1++) {
			if (isupper(*p2))
				*p2 |= 040;
			++p2;
		}

		srvdata[0].data = realm;
		srvdata[0].length = strlen(realm);
		srvdata[1].data = "rcmd";
		srvdata[1].length = 4;
		srvdata[2].data = name;
		srvdata[2].length = p2 - name;

		server[0] = &srvdata[0];
		server[1] = &srvdata[1];
		server[2] = &srvdata[2];
		server[3] = 0;

		if (authdat)
			krb5_free_tkt_authent(authdat);
		if (r = krb5_rd_req_simple(&auth, server, 0, &authdat)) {
			char errbuf[128];

			authdat = 0;
			(void) strcpy(errbuf, "Read req failed: ");
			(void) strcat(errbuf, error_message(r));
			Data(KRB_REJECT, errbuf, -1);
			if (auth_debug_mode)
				printf("%s\r\n", errbuf);
			return;
		}
		free(name);
		if (krb5_unparse_name(authdat->ticket->enc_part2 ->client,
				      					&name))
			name = 0;
		Data(KRB_ACCEPT, name, name ? -1 : 0);
		if (auth_debug_mode) {
			printf("Kerberos5 accepting him as ``%s''\r\n",
							name ? name : "");
		}
                auth_finished(ap, AUTH_USER);
                return;
        case KRB_NEWKEY:
#if     defined(ENCRYPT)
		if (authdat && authdat->ticket->enc_part2->session->keytype
				== KEYTYPE_DES)
		{
			des_key_sched(authdat->ticket->enc_part2->session
							->contents, sched);
			des_ecb_encrypt(data, session_key, sched, 0);
			skey.type = SK_DES;
			skey.length = 8;
			skey.data = session_key;
			encrypt_session_key(&skey, 1);
		}
#endif
                return;
	default:
		if (auth_debug_mode)
			printf("Unknown Kerberos option %d\r\n", data[-1]);
		Data(KRB_REJECT, 0, 0);
		return;
	}
}

	void
kerberos5_reply(ap, data, cnt)
	Authenticator *ap;
	unsigned char *data;
	int cnt;
{
        Session_Key skey;

	if (cnt-- < 1)
		return;
	switch (*data++) {
	case KRB_REJECT:
		if (cnt > 0) {
			printf("[ Kerberos V5 refuses authentication because %.*s ]\r\n",
				cnt, data);
		} else
			printf("[ Kerberos V5 refuses authentication ]\r\n");
		auth_send_retry();
		return;
	case KRB_ACCEPT:
		if (cnt > 0) {
			printf("[ Kerberos V5 accepts you as %.*s ]\n", cnt, data);
		} else
			printf("[ Kerberos V5 accepts you ]\n", cnt, data);
#if	defined(ENCRYPT)
                skey.type = SK_DES;
                skey.length = 8;
                skey.data = session_key;
                encrypt_session_key(&skey, 0);
#endif
                auth_finished(ap, AUTH_USER);
		return;
	default:
		if (auth_debug_mode)
			printf("Unknown Kerberos option %d\r\n", data[-1]);
		return;
	}
}

	int
kerberos5_status(ap, name, level)
	Authenticator *ap;
	char *name;
	int level;
{
	if (level < AUTH_USER)
		return(level);

	if (UserNameRequested &&
	    krb5_kuserok(authdat->ticket->enc_part2->client, UserNameRequested))
	{
		strcpy(name, UserNameRequested);
		return(AUTH_VALID);
	} else
		return(AUTH_USER);
}

#define	BUMP(buf, len)		while (*(buf)) {++(buf), --(len);}
#define	ADDC(buf, len, c)	if ((len) > 0) {*(buf)++ = (c); --(len));}

	void
kerberos5_printsub(data, cnt, buf, buflen)
	unsigned char *data, *buf;
	int cnt, buflen;
{
	char lbuf[32];
	register int i;

	buf[buflen-1] = '\0';		/* make sure its NULL terminated */
	buflen -= 1;

	switch(data[3]) {
	case KRB_NAME:			/* Name to authenticate for */
		strncpy(buf, " NAME ", buflen);
		goto common;

	case KRB_REJECT:		/* Rejected (reason might follow) */
		strncpy(buf, " REJECT ", buflen);
		goto common;

	case KRB_ACCEPT:		/* Accepted (name might follow) */
		strncpy(buf, " ACCEPT ", buflen);
	common:
		BUMP(buf, buflen);
		if (cnt <= 4)
			break;
		ADDC(buf, buflen, '"');
		for (i = 4; i < cnt; i++)
			ADDC(buf, buflen, data[i]);
		ADDC(buf, buflen, '"');
		ADDC(buf, buflen, '\0');
		break;

	case KRB_AUTH:			/* Authentication data follows */
		strncpy(buf, " AUTH", buflen);
		goto common2;

	case KRB_NEWKEY:		/* A new session key follows */
		strncpy(buf, " NEWKEY", buflen);
		goto common2;

	default:
		sprintf(lbuf, " %d (unknown)", data[3]);
		strncpy(buf, lbuf, buflen);
	common2:
		BUMP(buf, buflen);
		for (i = 4; i < cnt; i++) {
			sprintf(lbuf, " %d", data[i]);
			strncpy(buf, lbuf, buflen);
			BUMP(buf, buflen);
		}
		break;
	}
}
#endif
