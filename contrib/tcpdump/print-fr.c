/*
 * Copyright (c) 1990, 1991, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $FreeBSD$
 */

#ifndef lint
static const char rcsid[] _U_ =
	"@(#)$Header: /tcpdump/master/tcpdump/print-fr.c,v 1.32 2005/04/06 21:32:39 mcr Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <string.h>
#include <pcap.h>

#include "addrtoname.h"
#include "interface.h"
#include "ethertype.h"
#include "nlpid.h"
#include "extract.h"
#include "oui.h"

static void frf15_print(const u_char *, u_int);

/*
 * the frame relay header has a variable length
 *
 * the EA bit determines if there is another byte
 * in the header
 *
 * minimum header length is 2 bytes
 * maximum header length is 4 bytes
 *
 *      7    6    5    4    3    2    1    0
 *    +----+----+----+----+----+----+----+----+
 *    |        DLCI (6 bits)        | CR | EA |
 *    +----+----+----+----+----+----+----+----+
 *    |   DLCI (4 bits)   |FECN|BECN| DE | EA |
 *    +----+----+----+----+----+----+----+----+
 *    |           DLCI (7 bits)          | EA |
 *    +----+----+----+----+----+----+----+----+
 *    |        DLCI (6 bits)        |SDLC| EA |
 *    +----+----+----+----+----+----+----+----+
 */

#define FR_EA_BIT	0x01

#define FR_CR_BIT       0x02000000
#define FR_DE_BIT	0x00020000
#define FR_BECN_BIT	0x00040000
#define FR_FECN_BIT	0x00080000
#define FR_SDLC_BIT	0x00000002


struct tok fr_header_flag_values[] = {
    { FR_CR_BIT, "C!" },
    { FR_DE_BIT, "DE" },
    { FR_BECN_BIT, "BECN" },
    { FR_FECN_BIT, "FECN" },
    { FR_SDLC_BIT, "sdlcore" },
    { 0, NULL }
};


/* Finds out Q.922 address length, DLCI and flags. Returns 0 on success
 * save the flags dep. on address length
 */
static int parse_q922_addr(const u_char *p, u_int *dlci, u_int *sdlcore,
                           u_int *addr_len, u_int8_t *flags)
{
	if ((p[0] & FR_EA_BIT))
		return -1;

	*addr_len = 2;
	*dlci = ((p[0] & 0xFC) << 2) | ((p[1] & 0xF0) >> 4);

        flags[0] = p[0] & 0x02; /* populate the first flag fields */
        flags[1] = p[1] & 0x0c;

	if (p[1] & FR_EA_BIT)
		return 0;	/* 2-byte Q.922 address */

	p += 2;
	(*addr_len)++;		/* 3- or 4-byte Q.922 address */
	if ((p[0] & FR_EA_BIT) == 0) {
		*dlci = (*dlci << 7) | (p[0] >> 1);
		(*addr_len)++;	/* 4-byte Q.922 address */
		p++;
	}

	if ((p[0] & FR_EA_BIT) == 0)
		return -1; /* more than 4 bytes of Q.922 address? */

        flags[3] = p[0] & 0x02;

	if (p[0] & 0x02)
                *sdlcore =  p[0] >> 2;
	else
		*dlci = (*dlci << 6) | (p[0] >> 2);

	return 0;
}

/* Frame Relay packet structure, with flags and CRC removed

                  +---------------------------+
                  |       Q.922 Address*      |
                  +--                       --+
                  |                           |
                  +---------------------------+
                  | Control (UI = 0x03)       |
                  +---------------------------+
                  | Optional Pad      (0x00)  |
                  +---------------------------+
                  | NLPID                     |
                  +---------------------------+
                  |             .             |
                  |             .             |
                  |             .             |
                  |           Data            |
                  |             .             |
                  |             .             |
                  +---------------------------+

           * Q.922 addresses, as presently defined, are two octets and
             contain a 10-bit DLCI.  In some networks Q.922 addresses
             may optionally be increased to three or four octets.
*/

static u_int
fr_hdrlen(const u_char *p, u_int addr_len, u_int caplen)
{
	if ((caplen > addr_len + 1 /* UI */ + 1 /* pad */) &&
	    !p[addr_len + 1] /* pad exist */)
		return addr_len + 1 /* UI */ + 1 /* pad */ + 1 /* NLPID */;
	else 
		return addr_len + 1 /* UI */ + 1 /* NLPID */;
}

static void
fr_hdr_print(int length, u_int addr_len, u_int dlci, u_int8_t *flags, u_int16_t nlpid)
{
    if (qflag) {
        (void)printf("Q.922, DLCI %u, length %u: ",
                     dlci,
                     length);
    } else {
        if (nlpid <= 0xff) /* if its smaller than 256 then its a NLPID */
            (void)printf("Q.922, hdr-len %u, DLCI %u, Flags [%s], NLPID %s (0x%02x), length %u: ",
                         addr_len,
                         dlci,
                         bittok2str(fr_header_flag_values, "none", EXTRACT_32BITS(flags)),
                         tok2str(nlpid_values,"unknown", nlpid),
                         nlpid,
                         length);
        else /* must be an ethertype */
            (void)printf("Q.922, hdr-len %u, DLCI %u, Flags [%s], cisco-ethertype %s (0x%04x), length %u: ",
                         addr_len,
                         dlci,
                         bittok2str(fr_header_flag_values, "none", EXTRACT_32BITS(flags)),
                         tok2str(ethertype_values, "unknown", nlpid),
                         nlpid,
                         length);        
    }
}

u_int
fr_if_print(const struct pcap_pkthdr *h, register const u_char *p)
{
	register u_int length = h->len;
	register u_int caplen = h->caplen;
	u_int16_t extracted_ethertype;
	u_int32_t orgcode;
	register u_short et;
	u_int dlci;
        u_int sdlcore;
	u_int addr_len;
	u_int16_t nlpid;
	u_int hdr_len;
	u_int8_t flags[4];

	if (caplen < 4) {	/* minimum frame header length */
		printf("[|fr]");
		return caplen;
	}

	if (parse_q922_addr(p, &dlci, &sdlcore, &addr_len, flags)) {
		printf("Q.922, invalid address");
		return caplen;
	}

	hdr_len = fr_hdrlen(p, addr_len, caplen);

	if (caplen < hdr_len) {
		printf("[|fr]");
		return caplen;
	}

	if (p[addr_len] != 0x03 && dlci != 0) {

                /* lets figure out if we have cisco style encapsulation: */
                extracted_ethertype = EXTRACT_16BITS(p+addr_len);

                if (eflag)
                    fr_hdr_print(length, addr_len, dlci, flags, extracted_ethertype);

                if (ether_encap_print(extracted_ethertype,
                                      p+addr_len+ETHERTYPE_LEN,
                                      length-addr_len-ETHERTYPE_LEN,
                                      caplen-addr_len-ETHERTYPE_LEN,
                                      &extracted_ethertype) == 0)
                    /* ether_type not known, probably it wasn't one */
                    printf("UI %02x! ", p[addr_len]);
                else
                    return hdr_len;
        }

	if (!p[addr_len + 1]) {	/* pad byte should be used with 3-byte Q.922 */
		if (addr_len != 3)
			printf("Pad! ");
	} else if (addr_len == 3)
		printf("No pad! ");

	nlpid = p[hdr_len - 1];

	if (eflag)
		fr_hdr_print(length, addr_len, dlci, flags, nlpid);

	p += hdr_len;
	length -= hdr_len;
	caplen -= hdr_len;

	switch (nlpid) {
	case NLPID_IP:
	        ip_print(gndo, p, length);
		break;

#ifdef INET6
	case NLPID_IP6:
		ip6_print(p, length);
		break;
#endif
	case NLPID_CLNP:
	case NLPID_ESIS:
	case NLPID_ISIS:
                isoclns_print(p-1, length+1, caplen+1); /* OSI printers need the NLPID field */
		break;

	case NLPID_SNAP:
		orgcode = EXTRACT_24BITS(p);
		et = EXTRACT_16BITS(p + 3);

                if (eflag)
                    (void)printf("SNAP, oui %s (0x%06x), ethertype %s (0x%04x): ",
                                 tok2str(oui_values,"Unknown",orgcode),
                                 orgcode,
                                 tok2str(ethertype_values,"Unknown", et),
                                 et);

		if (snap_print((const u_char *)(p + 5), length - 5,
			   caplen - 5, &extracted_ethertype, orgcode, et,
			   0) == 0) {
			/* ether_type not known, print raw packet */
                        if (!eflag)
                            fr_hdr_print(length + hdr_len, hdr_len,
                                         dlci, flags, nlpid);
			if (!xflag && !qflag)
                            default_print(p - hdr_len, caplen + hdr_len);
		}
		break;

        case NLPID_Q933:
		q933_print(p, length);
		break;

        case NLPID_MFR:
                frf15_print(p, length);
                break;

	default:
		if (!eflag)
                    fr_hdr_print(length + hdr_len, addr_len,
				     dlci, flags, nlpid);
		if (!xflag)
			default_print(p, caplen);
	}

	return hdr_len;
}

/* an NLPID of 0xb1 indicates a 2-byte
 * FRF.15 header
 * 
 *      7    6    5    4    3    2    1    0
 *    +----+----+----+----+----+----+----+----+
 *    ~              Q.922 header             ~
 *    +----+----+----+----+----+----+----+----+
 *    |             NLPID (8 bits)            | NLPID=0xb1
 *    +----+----+----+----+----+----+----+----+
 *    | B  | E  | C  |seq. (high 4 bits) | R  |
 *    +----+----+----+----+----+----+----+----+
 *    |        sequence  (low 8 bits)         |
 *    +----+----+----+----+----+----+----+----+
 */

struct tok frf15_flag_values[] = {
    { 0x80, "Begin" },
    { 0x40, "End" },
    { 0x20, "Control" },
    { 0, NULL }
};

#define FR_FRF15_FRAGTYPE 0x01

static void
frf15_print (const u_char *p, u_int length) {
    
    u_int16_t sequence_num, flags;

    flags = p[0]&0xe0;
    sequence_num = (p[0]&0x1e)<<7 | p[1];

    printf("FRF.15, seq 0x%03x, Flags [%s],%s Fragmentation, length %u",
           sequence_num,
           bittok2str(frf15_flag_values,"none",flags),
           flags&FR_FRF15_FRAGTYPE ? "Interface" : "End-to-End",
           length);

/* TODO:
 * depending on all permutations of the B, E and C bit
 * dig as deep as we can - e.g. on the first (B) fragment
 * there is enough payload to print the IP header
 * on non (B) fragments it depends if the fragmentation
 * model is end-to-end or interface based wether we want to print
 * another Q.922 header
 */

}

/*
 * Q.933 decoding portion for framerelay specific.
 */

/* Q.933 packet format
                      Format of Other Protocols   
                          using Q.933 NLPID
                  +-------------------------------+            
                  |        Q.922 Address          | 
                  +---------------+---------------+
                  |Control  0x03  | NLPID   0x08  |        
                  +---------------+---------------+        
                  |          L2 Protocol ID       |
                  | octet 1       |  octet 2      |
                  +-------------------------------+
                  |          L3 Protocol ID       |
                  | octet 2       |  octet 2      |
                  +-------------------------------+
                  |         Protocol Data         |
                  +-------------------------------+
                  | FCS                           |
                  +-------------------------------+
 */

/* L2 (Octet 1)- Call Reference Usually is 0x0 */

/*
 * L2 (Octet 2)- Message Types definition 1 byte long.
 */
/* Call Establish */
#define MSG_TYPE_ESC_TO_NATIONAL  0x00
#define MSG_TYPE_ALERT            0x01
#define MSG_TYPE_CALL_PROCEEDING  0x02
#define MSG_TYPE_CONNECT          0x07
#define MSG_TYPE_CONNECT_ACK      0x0F
#define MSG_TYPE_PROGRESS         0x03
#define MSG_TYPE_SETUP            0x05
/* Call Clear */
#define MSG_TYPE_DISCONNECT       0x45
#define MSG_TYPE_RELEASE          0x4D
#define MSG_TYPE_RELEASE_COMPLETE 0x5A
#define MSG_TYPE_RESTART          0x46
#define MSG_TYPE_RESTART_ACK      0x4E
/* Status */
#define MSG_TYPE_STATUS           0x7D
#define MSG_TYPE_STATUS_ENQ       0x75

struct tok fr_q933_msg_values[] = {
    { MSG_TYPE_ESC_TO_NATIONAL, "ESC to National" },
    { MSG_TYPE_ALERT, "Alert" },
    { MSG_TYPE_CALL_PROCEEDING, "Call proceeding" },
    { MSG_TYPE_CONNECT, "Connect" },
    { MSG_TYPE_CONNECT_ACK, "Connect ACK" },
    { MSG_TYPE_PROGRESS, "Progress" },
    { MSG_TYPE_SETUP, "Setup" },
    { MSG_TYPE_DISCONNECT, "Disconnect" },
    { MSG_TYPE_RELEASE, "Release" },
    { MSG_TYPE_RELEASE_COMPLETE, "Release Complete" },
    { MSG_TYPE_RESTART, "Restart" },
    { MSG_TYPE_RESTART_ACK, "Restart ACK" },
    { MSG_TYPE_STATUS, "Status Reply" },
    { MSG_TYPE_STATUS_ENQ, "Status Enquiry" },
    { 0, NULL }
};

#define MSG_ANSI_LOCKING_SHIFT	0x95

#define FR_LMI_ANSI_REPORT_TYPE_IE	0x01
#define FR_LMI_ANSI_LINK_VERIFY_IE_91	0x19 /* details? */
#define FR_LMI_ANSI_LINK_VERIFY_IE	0x03
#define FR_LMI_ANSI_PVC_STATUS_IE	0x07

#define FR_LMI_CCITT_REPORT_TYPE_IE	0x51
#define FR_LMI_CCITT_LINK_VERIFY_IE	0x53
#define FR_LMI_CCITT_PVC_STATUS_IE	0x57

struct tok fr_q933_ie_values_codeset5[] = {
    { FR_LMI_ANSI_REPORT_TYPE_IE, "ANSI Report Type" },
    { FR_LMI_ANSI_LINK_VERIFY_IE_91, "ANSI Link Verify" },
    { FR_LMI_ANSI_LINK_VERIFY_IE, "ANSI Link Verify" },
    { FR_LMI_ANSI_PVC_STATUS_IE, "ANSI PVC Status" },
    { FR_LMI_CCITT_REPORT_TYPE_IE, "CCITT Report Type" },
    { FR_LMI_CCITT_LINK_VERIFY_IE, "CCITT Link Verify" },
    { FR_LMI_CCITT_PVC_STATUS_IE, "CCITT PVC Status" },
    { 0, NULL }
};

#define FR_LMI_REPORT_TYPE_IE_FULL_STATUS 0
#define FR_LMI_REPORT_TYPE_IE_LINK_VERIFY 1
#define FR_LMI_REPORT_TYPE_IE_ASYNC_PVC   2

struct tok fr_lmi_report_type_ie_values[] = {
    { FR_LMI_REPORT_TYPE_IE_FULL_STATUS, "Full Status" },
    { FR_LMI_REPORT_TYPE_IE_LINK_VERIFY, "Link verify" },
    { FR_LMI_REPORT_TYPE_IE_ASYNC_PVC, "Async PVC Status" },
    { 0, NULL }
};

/* array of 16 codepages - currently we only support codepage 5 */
static struct tok *fr_q933_ie_codesets[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    fr_q933_ie_values_codeset5,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


struct common_ie_header {
    u_int8_t ie_id;
    u_int8_t ie_len;
};

void
q933_print(const u_char *p, u_int length)
{
	const u_char *ptemp = p;
	struct common_ie_header *ie_p;
        int olen;
	int is_ansi = 0;
        u_int dlci,codeset;

	if (length < 9) {	/* shortest: Q.933a LINK VERIFY */
		printf("[|q.933]");
		return;
	}

        codeset = p[2]&0x0f;   /* extract the codeset */

	if (p[2] == MSG_ANSI_LOCKING_SHIFT)
		is_ansi = 1;
    
        printf("%s", eflag ? "" : "Q.933, ");

	/* printing out header part */
	printf(is_ansi ? "ANSI" : "CCITT ");

	if (p[0])
		printf(", Call Ref: 0x%02x", p[0]);

        if (vflag)
            printf(", %s (0x%02x), length %u",
                   tok2str(fr_q933_msg_values,"unknown message",p[1]),
                   p[1],
                   length);
        else
            printf(", %s",
                   tok2str(fr_q933_msg_values,"unknown message 0x%02x",p[1]));            

        olen = length; /* preserve the original length for non verbose mode */

	if (length < (u_int)(2 - is_ansi)) {
		printf("[|q.933]");
		return;
	}
	length -= 2 - is_ansi;
	ptemp += 2 + is_ansi;
	
	/* Loop through the rest of IE */
	while (length > sizeof(struct common_ie_header)) {
		ie_p = (struct common_ie_header *)ptemp;
		if (length < sizeof(struct common_ie_header) ||
		    length < sizeof(struct common_ie_header) + ie_p->ie_len) {
                    if (vflag) /* not bark if there is just a trailer */
                        printf("\n[|q.933]");
                    else
                        printf(", length %u",olen);
                    return;
		}

                /* lets do the full IE parsing only in verbose mode
                 * however some IEs (DLCI Status, Link Verify)
                 * are also intereststing in non-verbose mode */
                if (vflag)
                    printf("\n\t%s IE (%u), length %u: ",
                           tok2str(fr_q933_ie_codesets[codeset],"unknown",ie_p->ie_id),
                           ie_p->ie_id,
                           ie_p->ie_len);
                    
                switch (ie_p->ie_id) {

                case FR_LMI_ANSI_REPORT_TYPE_IE: /* fall through */
                case FR_LMI_CCITT_REPORT_TYPE_IE:
                    if (vflag)
                        printf("%s (%u)",
                               tok2str(fr_lmi_report_type_ie_values,"unknown",ptemp[2]),
                               ptemp[2]);
                    break;

                case FR_LMI_ANSI_LINK_VERIFY_IE: /* fall through */
                case FR_LMI_CCITT_LINK_VERIFY_IE:
                case FR_LMI_ANSI_LINK_VERIFY_IE_91:
                    if (!vflag)
                        printf(", ");
                    printf("TX Seq: %3d, RX Seq: %3d", ptemp[2], ptemp[3]);
                    break;
                case FR_LMI_ANSI_PVC_STATUS_IE: /* fall through */
                case FR_LMI_CCITT_PVC_STATUS_IE:
                    if (!vflag)
                        printf(", ");
                    /* now parse the DLCI information element. */                    
                    if ((ie_p->ie_len < 3) ||
                        (ptemp[2] & 0x80) ||
                        ((ie_p->ie_len == 3) && !(ptemp[3] & 0x80)) ||
                        ((ie_p->ie_len == 4) && ((ptemp[3] & 0x80) || !(ptemp[4] & 0x80))) ||
                        ((ie_p->ie_len == 5) && ((ptemp[3] & 0x80) || (ptemp[4] & 0x80) ||
                                           !(ptemp[5] & 0x80))) ||
                        (ie_p->ie_len > 5) ||
                        !(ptemp[ie_p->ie_len + 1] & 0x80))
                        printf("Invalid DLCI IE");
                    
                    dlci = ((ptemp[2] & 0x3F) << 4) | ((ptemp[3] & 0x78) >> 3);
                    if (ie_p->ie_len == 4)
                        dlci = (dlci << 6) | ((ptemp[4] & 0x7E) >> 1);
                    else if (ie_p->ie_len == 5)
                        dlci = (dlci << 13) | (ptemp[4] & 0x7F) | ((ptemp[5] & 0x7E) >> 1);

                    printf("DLCI %u: status %s%s", dlci,
                             ptemp[ie_p->ie_len + 1] & 0x8 ? "New, " : "",
                             ptemp[ie_p->ie_len + 1] & 0x2 ? "Active" : "Inactive");
                    break;
                    
                default:
                    if (vflag <= 1)
                        print_unknown_data(ptemp+2,"\n\t",ie_p->ie_len);
                    break;
                }

                /* do we want to see a hexdump of the IE ? */
                if (vflag> 1)
                    print_unknown_data(ptemp+2,"\n\t  ",ie_p->ie_len);

		length = length - ie_p->ie_len - 2;
		ptemp = ptemp + ie_p->ie_len + 2;
	}
        if (!vflag)
            printf(", length %u",olen);
}
