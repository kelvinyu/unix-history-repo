/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)kerberos.c	5.2 (Berkeley) %G%";
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

#ifdef	KRB4
#include <sys/types.h>
#include <arpa/telnet.h>
#include <stdio.h>
#if	defined(ENCRYPT)
#define	__NEED_ENCRYPT__
#undef	ENCRYPT
#endif
#include <des.h>        /* BSD wont include this in krb.h, so we do it here */
#include <krb.h>
#if	defined(__NEED_ENCRYPT__) && !defined(ENCRYPT)
#define	ENCRYPT
#undef	__NEED_ENCRYPT__
#endif
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

int cksum P((unsigned char *, int));
int krb_mk_req P((KTEXT, char *, char *, char *, u_long));
int krb_rd_req P((KTEXT, char *, char *, u_long, AUTH_DAT *, char *));
int krb_kntoln P((AUTH_DAT *, char *));
int krb_get_cred P((char *, char *, char *, CREDENTIALS *));
int krb_get_lrealm P((char *, int));
int kuserok P((AUTH_DAT *, char *));

extern auth_debug_mode;

static unsigned char str_data[1024] = { IAC, SB, TELOPT_AUTHENTICATION, 0,
			  		AUTHTYPE_KERBEROS_V4, };
static unsigned char str_name[1024] = { IAC, SB, TELOPT_AUTHENTICATION,
					TELQUAL_NAME, };

#define	KRB_AUTH	0		/* Authentication data follows */
#define	KRB_REJECT	1		/* Rejected (reason might follow) */
#define	KRB_ACCEPT	2		/* Accepted */
#define	KRB_CHALLANGE	3		/* Challange for mutual auth. */
#define	KRB_RESPONSE	4		/* Response for mutual auth. */

static	KTEXT_ST auth;
static	char name[ANAME_SZ];
static	AUTH_DAT adat = { 0 };
#if	defined(ENCRYPT)
static Block	session_key	= { 0 };
#endif
static Schedule sched;
static Block	challange	= { 0 };

	static int
Data(ap, type, d, c)
	Authenticator *ap;
	int type;
	void *d;
	int c;
{
        unsigned char *p = str_data + 4;
	unsigned char *cd = (unsigned char *)d;

	if (c == -1)
		c = strlen((char *)cd);

        if (auth_debug_mode) {
                printf("%s:%d: [%d] (%d)",
                        str_data[3] == TELQUAL_IS ? ">>>IS" : ">>>REPLY",
                        str_data[3],
                        type, c);
                printd(d, c);
                printf("\r\n");
        }
	*p++ = ap->type;
	*p++ = ap->way;
	*p++ = type;
        while (c-- > 0) {
                if ((*p++ = *cd++) == IAC)
                        *p++ = IAC;
        }
        *p++ = IAC;
        *p++ = SE;
	if (str_data[3] == TELQUAL_IS)
		printsub('>', &str_data[2], p - (&str_data[2]));
        return(net_write(str_data, p - str_data));
}

	int
kerberos4_init(ap, server)
	Authenticator *ap;
	int server;
{
	if (server)
		str_data[3] = TELQUAL_REPLY;
	else
		str_data[3] = TELQUAL_IS;
	return(1);
}

char dst_realm_buf[REALM_SZ], *dest_realm = NULL;
int dst_realm_sz = REALM_SZ;

	int
kerberos4_send(ap)
	Authenticator *ap;
{
	KTEXT_ST auth;
	Block enckey;
	char instance[INST_SZ];
	char *realm;
	char *krb_realmofhost();
	char *krb_get_phost();
	CREDENTIALS cred;
	int r;
	
	if (!UserNameRequested) {
		if (auth_debug_mode) {
			printf("Kerberos V4: no user name supplied\r\n");
		}
		return(0);
	}

	bzero(instance, sizeof(instance));
	if (realm = krb_get_phost(RemoteHostName))
		strncpy(instance, realm, sizeof(instance));
	instance[sizeof(instance)-1] = '\0';

	realm = dest_realm ? dest_realm : krb_realmofhost(RemoteHostName);
	if (!realm) {
		if (auth_debug_mode) {
			printf("Kerberos V4: no realm for %s\r\n", RemoteHostName);
		}
		return(0);
	}
	if (r = krb_mk_req(&auth, "rcmd", instance, realm, 0L)) {
		if (auth_debug_mode) {
			printf("mk_req failed: %s\r\n", krb_err_txt[r]);
		}
		return(0);
	}
	if (r = krb_get_cred("rcmd", instance, realm, &cred)) {
		if (auth_debug_mode) {
			printf("get_cred failed: %s\r\n", krb_err_txt[r]);
		}
		return(0);
	}
	if (!auth_sendname(UserNameRequested, strlen(UserNameRequested))) {
		if (auth_debug_mode)
			printf("Not enough room for user name\r\n");
		return(0);
	}
	if (auth_debug_mode)
		printf("Sent %d bytes of authentication data\r\n", auth.length);
	if (!Data(ap, KRB_AUTH, (void *)auth.dat, auth.length)) {
		if (auth_debug_mode)
			printf("Not enough room for authentication data\r\n");
		return(0);
	}
	/*
	 * If we are doing mutual authentication, get set up to send
	 * the challange, and verify it when the response comes back.
	 */
	if ((ap->way & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL) {
		register int i;

		des_key_sched(cred.session, sched);
		des_set_random_generator_seed(cred.session);
		des_new_random_key(challange);
		des_ecb_encrypt(challange, session_key, sched, 1);
		/*
		 * Increment the challange by 1, and encrypt it for
		 * later comparison.
		 */
		for (i = 7; i >= 0; --i) {
			register int x;
			x = (unsigned int)challange[i] + 1;
			challange[i] = x;	/* ignore overflow */
			if (x < 256)		/* if no overflow, all done */
				break;
		}
		des_ecb_encrypt(challange, challange, sched, 1);
	}
	
	if (auth_debug_mode) {
		printf("CK: %d:", cksum(auth.dat, auth.length));
		printd(auth.dat, auth.length);
		printf("\r\n");
		printf("Sent Kerberos V4 credentials to server\r\n");
	}
	return(1);
}

	void
kerberos4_is(ap, data, cnt)
	Authenticator *ap;
	unsigned char *data;
	int cnt;
{
	Session_Key skey;
	Block datablock;
	char realm[REALM_SZ];
	char instance[INST_SZ];
	int r;

	if (cnt-- < 1)
		return;
	switch (*data++) {
	case KRB_AUTH:
		if (krb_get_lrealm(realm, 1) != KSUCCESS) {
			Data(ap, KRB_REJECT, (void *)"No local V4 Realm.", -1);
			auth_finished(ap, AUTH_REJECT);
			if (auth_debug_mode)
				printf("No local realm\r\n");
			return;
		}
		bcopy((void *)data, (void *)auth.dat, auth.length = cnt);
		if (auth_debug_mode) {
			printf("Got %d bytes of authentication data\r\n", cnt);
			printf("CK: %d:", cksum(auth.dat, auth.length));
			printd(auth.dat, auth.length);
			printf("\r\n");
		}
		instance[0] = '*'; instance[1] = 0;
		if (r = krb_rd_req(&auth, "rcmd", instance, 0, &adat, "")) {
			if (auth_debug_mode)
				printf("Kerberos failed him as %s\r\n", name);
			Data(ap, KRB_REJECT, (void *)krb_err_txt[r], -1);
			auth_finished(ap, AUTH_REJECT);
			return;
		}
		bcopy((void *)adat.session, (void *)session_key, sizeof(Block));
		krb_kntoln(&adat, name);
		Data(ap, KRB_ACCEPT, (void *)0, 0);
		auth_finished(ap, AUTH_USER);
		if (auth_debug_mode) {
			printf("Kerberos accepting him as %s\r\n", name);
		}
		break;

	case KRB_CHALLANGE:
		if (!VALIDKEY(session_key)) {
			/*
			 * We don't have a valid session key, so just
			 * send back a response with an empty session
			 * key.
			 */
			Data(ap, KRB_RESPONSE, (void *)0, 0);
			break;
		}

		des_key_sched(session_key, sched);
		bcopy((void *)data, (void *)datablock, sizeof(Block));
		/*
		 * Take the received encrypted challange, and encrypt
		 * it again to get a unique session_key for the
		 * ENCRYPT option.
		 */
		des_ecb_encrypt(datablock, session_key, sched, 1);
		skey.type = SK_DES;
		skey.length = 8;
		skey.data = session_key;
		encrypt_session_key(&skey, 1);
		/*
		 * Now decrypt the received encrypted challange,
		 * increment by one, re-encrypt it and send it back.
		 */
		des_ecb_encrypt(datablock, challange, sched, 0);
		for (r = 7; r >= 0; r++) {
			register int t;
			t = (unsigned int)challange[r] + 1;
			challange[r] = t;	/* ignore overflow */
			if (t < 256)		/* if no overflow, all done */
				break;
		}
		des_ecb_encrypt(challange, challange, sched, 1);
		Data(ap, KRB_RESPONSE, (void *)challange, sizeof(challange));
		break;

	default:
		if (auth_debug_mode)
			printf("Unknown Kerberos option %d\r\n", data[-1]);
		Data(ap, KRB_REJECT, 0, 0);
		break;
	}
}

	void
kerberos4_reply(ap, data, cnt)
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
			printf("[ Kerberos V4 refuses authentication because %.*s ]\r\n",
				cnt, data);
		} else
			printf("[ Kerberos V4 refuses authentication ]\r\n");
		auth_send_retry();
		return;
	case KRB_ACCEPT:
		printf("[ Kerberos V4 accepts you ]\n");
		if ((ap->way & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL) {
			/*
			 * Send over the encrypted challange.
		 	 */
			Data(ap, KRB_CHALLANGE, (void *)session_key,
						sizeof(session_key));
#if	defined(ENCRYPT)
			des_ecb_encrypt(session_key, session_key, sched, 1);
			skey.type = SK_DES;
			skey.length = 8;
			skey.data = session_key;
			encrypt_session_key(&skey, 0);
#endif
			return;
		}
		auth_finished(ap, AUTH_USER);
		return;
	case KRB_RESPONSE:
		/*
		 * Verify that the response to the challange is correct.
		 */
		if ((cnt != sizeof(Block)) ||
		    (0 != memcmp((void *)data, (void *)challange,
						sizeof(challange))))
		{
			printf("[ Kerberos V4 challange failed!!! ]\r\n");
			auth_send_retry();
			return;
		}
		printf("[ Kerberos V4 challange successful ]\r\n");
		auth_finished(ap, AUTH_USER);
		break;
	default:
		if (auth_debug_mode)
			printf("Unknown Kerberos option %d\r\n", data[-1]);
		return;
	}
}

	int
kerberos4_status(ap, name, level)
	Authenticator *ap;
	char *name;
	int level;
{
	if (level < AUTH_USER)
		return(level);

	if (UserNameRequested && !kuserok(&adat, UserNameRequested)) {
		strcpy(name, UserNameRequested);
		return(AUTH_VALID);
	} else
		return(AUTH_USER);
}

#define	BUMP(buf, len)		while (*(buf)) {++(buf), --(len);}
#define	ADDC(buf, len, c)	if ((len) > 0) {*(buf)++ = (c); --(len);}

	void
kerberos4_printsub(data, cnt, buf, buflen)
	unsigned char *data, *buf;
	int cnt, buflen;
{
	char lbuf[32];
	register int i;

	buf[buflen-1] = '\0';		/* make sure its NULL terminated */
	buflen -= 1;

	switch(data[3]) {
	case KRB_REJECT:		/* Rejected (reason might follow) */
		strncpy((char *)buf, " REJECT ", buflen);
		goto common;

	case KRB_ACCEPT:		/* Accepted (name might follow) */
		strncpy((char *)buf, " ACCEPT ", buflen);
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
		strncpy((char *)buf, " AUTH", buflen);
		goto common2;

	case KRB_CHALLANGE:
		strncpy((char *)buf, " CHALLANGE", buflen);
		goto common2;

	case KRB_RESPONSE:
		strncpy((char *)buf, " RESPONSE", buflen);
		goto common2;

	default:
		sprintf(lbuf, " %d (unknown)", data[3]);
		strncpy((char *)buf, lbuf, buflen);
	common2:
		BUMP(buf, buflen);
		for (i = 4; i < cnt; i++) {
			sprintf(lbuf, " %d", data[i]);
			strncpy((char *)buf, lbuf, buflen);
			BUMP(buf, buflen);
		}
		break;
	}
}

	int
cksum(d, n)
	unsigned char *d;
	int n;
{
	int ck = 0;

	switch (n&03)
	while (n > 0) {
	case 0:
		ck ^= *d++ << 24;
		--n;
	case 3:
		ck ^= *d++ << 16;
		--n;
	case 2:
		ck ^= *d++ << 8;
		--n;
	case 1:
		ck ^= *d++;
		--n;
	}
	return(ck);
}
#endif

#ifdef notdef

prkey(msg, key)
	char *msg;
	unsigned char *key;
{
	register int i;
	printf("%s:", msg);
	for (i = 0; i < 8; i++)
		printf(" %3d", key[i]);
	printf("\r\n");
}
#endif
