/*
 * Copyright (c) 1989 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)state.c	5.2 (Berkeley) %G%";
#endif /* not lint */

#include "telnetd.h"

char	doopt[] = { IAC, DO, '%', 'c', 0 };
char	dont[] = { IAC, DONT, '%', 'c', 0 };
char	will[] = { IAC, WILL, '%', 'c', 0 };
char	wont[] = { IAC, WONT, '%', 'c', 0 };
int	not42 = 1;

/*
 * Buffer for sub-options, and macros
 * for suboptions buffer manipulations
 */
char	subbuffer[100], *subpointer= subbuffer, *subend= subbuffer;

#define	SB_CLEAR()	subpointer = subbuffer;
#define	SB_TERM()	{ subend = subpointer; SB_CLEAR(); }
#define	SB_ACCUM(c)	if (subpointer < (subbuffer+sizeof subbuffer)) { \
				*subpointer++ = (c); \
			}
#define	SB_GET()	((*subpointer++)&0xff)
#define	SB_EOF()	(subpointer >= subend)



/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_SB		3	/* throw away begin's... */
#define	TS_SE		4	/* ...end's (suboption negotiation) */
#define	TS_WILL		5	/* will option negotiation */
#define	TS_WONT		6	/* wont " */
#define	TS_DO		7	/* do " */
#define	TS_DONT		8	/* dont " */

telrcv()
{
	register int c;
	static int state = TS_DATA;
#ifdef	CRAY2
	char *opfrontp = pfrontp;
#endif

	while (ncc > 0) {
		if ((&ptyobuf[BUFSIZ] - pfrontp) < 2)
			break;
		c = *netip++ & 0377, ncc--;
		switch (state) {

		case TS_CR:
			state = TS_DATA;
			/* Strip off \n or \0 after a \r */
			if ((c == 0) || (c == '\n')) {
				break;
			}
			/* FALL THROUGH */

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
				break;
			}
			/*
			 * We now map \r\n ==> \r for pragmatic reasons.
			 * Many client implementations send \r\n when
			 * the user hits the CarriageReturn key.
			 *
			 * We USED to map \r\n ==> \n, since \r\n says
			 * that we want to be in column 1 of the next
			 * printable line, and \n is the standard
			 * unix way of saying that (\r is only good
			 * if CRMOD is set, which it normally is).
			 */
			if ((c == '\r') && (hisopts[TELOPT_BINARY] == OPT_NO)) {
				/*
				 * If we are operating in linemode,
				 * convert to local end-of-line.
				 */
				if ((linemode) && (ncc > 0)&&('\n' == *netip)) {
					netip++; ncc--;
					c = '\n';
				} else {
					state = TS_CR;
				}
			}
			*pfrontp++ = c;
			break;

		case TS_IAC:
gotiac:			switch (c) {

			/*
			 * Send the process on the pty side an
			 * interrupt.  Do this with a NULL or
			 * interrupt char; depending on the tty mode.
			 */
			case IP:
				interrupt();
				break;

			case BREAK:
				sendbrk();
				break;

			/*
			 * Are You There?
			 */
			case AYT:
				(void) strcpy(nfrontp, "\r\n[Yes]\r\n");
				nfrontp += 9;
				break;

			/*
			 * Abort Output
			 */
			case AO:
			    {
				ptyflush();	/* half-hearted */
				init_termbuf();

				if (slctab[SLC_AO].sptr &&
				    *slctab[SLC_AO].sptr != '\377') {
					*pfrontp++ = *slctab[SLC_AO].sptr;
				}

				netclear();	/* clear buffer back */
				*nfrontp++ = IAC;
				*nfrontp++ = DM;
				neturg = nfrontp-1; /* off by one XXX */
				break;
			    }

			/*
			 * Erase Character and
			 * Erase Line
			 */
			case EC:
			case EL:
			    {
				unsigned char ch;

				ptyflush();	/* half-hearted */
				init_termbuf();
				ch = (c == EC) ? *slctab[SLC_EC].sptr :
						 *slctab[SLC_EL].sptr;
				if (ch != '\377')
					*pfrontp++ = ch;
				break;
			    }

			/*
			 * Check for urgent data...
			 */
			case DM:
				SYNCHing = stilloob(net);
				settimer(gotDM);
				break;


			/*
			 * Begin option subnegotiation...
			 */
			case SB:
				state = TS_SB;
				SB_CLEAR();
				continue;

			case WILL:
				state = TS_WILL;
				continue;

			case WONT:
				state = TS_WONT;
				continue;

			case DO:
				state = TS_DO;
				continue;

			case DONT:
				state = TS_DONT;
				continue;
			case EOR:
				if (hisopts[TELOPT_EOR])
					doeof();
				break;

			/*
			 * Handle RFC 10xx Telnet linemode option additions
			 * to command stream (EOF, SUSP, ABORT).
			 */
			case xEOF:
				doeof();
				break;

			case SUSP:
				sendsusp();
				break;

			case ABORT:
				sendbrk();
				break;

			case IAC:
				*pfrontp++ = c;
				break;
			}
			state = TS_DATA;
			break;

		case TS_SB:
			if (c == IAC) {
				state = TS_SE;
			} else {
				SB_ACCUM(c);
			}
			break;

		case TS_SE:
			if (c != SE) {
				if (c != IAC) {
					/*
					 * bad form of suboption negotiation.
					 * handle it in such a way as to avoid
					 * damage to local state.  Parse
					 * suboption buffer found so far,
					 * then treat remaining stream as
					 * another command sequence.
					 */
					SB_TERM();
					suboption();
					state = TS_IAC;
					goto gotiac;
				}
				SB_ACCUM(c);
				state = TS_SB;
			} else {
				SB_TERM();
				suboption();	/* handle sub-option */
				state = TS_DATA;
			}
			break;

		case TS_WILL:
			willoption(c, 0);
			state = TS_DATA;
			continue;

		case TS_WONT:
			wontoption(c, 0);
			state = TS_DATA;
			continue;

		case TS_DO:
			dooption(c, 0);
			state = TS_DATA;
			continue;

		case TS_DONT:
			dontoption(c, 0);
			state = TS_DATA;
			continue;

		default:
			syslog(LOG_ERR, "telnetd: panic state=%d\n", state);
			printf("telnetd: panic state=%d\n", state);
			exit(1);
		}
	}
#if	CRAY2
	if (!linemode) {
		char	xptyobuf[BUFSIZ+NETSLOP];
		char	xbuf2[BUFSIZ];
		register char *cp;
		int n = pfrontp - opfrontp, oc;
		bcopy(opfrontp, xptyobuf, n);
		pfrontp = opfrontp;
		pfrontp += term_input(xptyobuf, pfrontp, n, BUFSIZ+NETSLOP,
					xbuf2, &oc, BUFSIZ);
		for (cp = xbuf2; oc > 0; --oc)
			if ((*nfrontp++ = *cp++) == IAC)
				*nfrontp++ = IAC;
	}
#endif	/* CRAY2 */
}  /* end of telrcv */

/*
 * The will/wont/do/dont state machines are based on Dave Borman's
 * Telnet option processing state machine.  We keep track of the full
 * state of the option negotiation with the following state variables
 *	myopts, hisopts - The last fully negotiated state for each
 *			side of the connection.
 *	mywants, hiswants - The state we wish to be in after a completed
 *			negotiation.  (hiswants is slightly misleading,
 *			this is more precisely the state I want him to
 *			be in.
 *	resp - We count the number of requests we have sent out.
 *
 * These correspond to the following states:
 *	my_state = the last negotiated state
 *	want_state = what I want the state to go to
 *	want_resp = how many requests I have sent
 * All state defaults are negative, and resp defaults to 0.
 *
 * When initiating a request to change state to new_state:
 * 
 * if ((want_resp == 0 && new_state == my_state) || want_state == new_state) {
 *	do nothing;
 * } else {
 *	want_state = new_state;
 *	send new_state;
 *	want_resp++;
 * }
 *
 * When receiving new_state:
 *
 * if (want_resp) {
 *	want_resp--;
 *	if (want_resp && (new_state == my_state))
 *		want_resp--;
 * }
 * if ((want_resp == 0) && (new_state != want_state)) {
 *	if (ok_to_switch_to new_state)
 *		want_state = new_state;
 *	else
 *		want_resp++;
 *	send want_state;
 * }
 * my_state = new_state;
 *
 * Note that new_state is implied in these functions by the function itself.
 * will and do imply positive new_state, wont and dont imply negative.
 *
 * Finally, there is one catch.  If we send a negative response to a
 * positive request, my_state will be the positive while want_state will
 * remain negative.  my_state will revert to negative when the negative
 * acknowlegment arrives from the peer.  Thus, my_state generally tells
 * us not only the last negotiated state, but also tells us what the peer
 * wants to be doing as well.  It is important to understand this difference
 * as we may wish to be processing data streams based on our desired state
 * (want_state) or based on what the peer thinks the state is (my_state).
 *
 * This all works fine because if the peer sends a positive request, the data
 * that we receive prior to negative acknowlegment will probably be affected
 * by the positive state, and we can process it as such (if we can; if we
 * can't then it really doesn't matter).  If it is that important, then the
 * peer probably should be buffering until this option state negotiation
 * is complete.
 *
 * In processing options, request signifies whether this is a request
 * to send or a response.  request is true if this is a request to 
 * send generated locally.
 */
willoption(option, request)
	int option, request;
{
	int changeok = 0;
	char *fmt = (char *)0;

    /*
     * process input from peer.
     */
    if (request == 0) {
		switch (option) {

		case TELOPT_BINARY:
			init_termbuf();
			tty_binaryin(1);
			set_termbuf();
			changeok++;
			break;

		case TELOPT_ECHO:
			not42 = 0;		/* looks like a 4.2 system */
			/*
			 * Now, in a 4.2 system, to break them out of ECHOing
			 * (to the terminal) mode, we need to send a
			 * "WILL ECHO".  Kludge upon kludge!
			 */
			if (myopts[TELOPT_ECHO] == OPT_YES) {
				dooption(TELOPT_ECHO, 1);
			}
			/*
			 * Fool the state machine into sending a don't.
			 * This also allows the initial echo sending code to
			 * break out of the loop that it is
			 * in.  (Look in telnet())
			 */
			hiswants[TELOPT_ECHO] = OPT_NO;
			break;

		case TELOPT_TM:
#if	defined(LINEMODE) && defined(KLUDGELINEMODE)
			/*
		 	 * This telnetd implementation does not really support
			 * timing marks, it just uses them to support the kludge
			 * linemode stuff.  If we receive a will or wont TM in
			 * response to our do TM request that may have been sent
			 * to determine kludge linemode support, process it,
			 * otherwise TM should get a negative response back.
			 */
			/*
			 * Handle the linemode kludge stuff.
			 * If we are not currently supporting any
			 * linemode at all, then we assume that this
			 * is the client telling us to use kludge
			 * linemode in response to our query.  Set the
			 * linemode type that is to be supported, note
			 * that the client wishes to use linemode, and
			 * eat the will TM as though it never arrived.
			 */
			if (lmodetype < KLUDGE_LINEMODE) {
				lmodetype = KLUDGE_LINEMODE;
				clientstat(TELOPT_LINEMODE, WILL, 0);
				dontoption(TELOPT_SGA, 0);
			}
#endif	/* defined(LINEMODE) && defined(KLUDGELINEMODE) */
			/*
			 * cheat the state machine so that it
			 * looks like we never sent the TM at
			 * all.  The bad part of this is that
			 * if the client sends a will TM on his
			 * own to turn on linemode, then he
			 * won't get a response.
			 */
			hiswants[TELOPT_TM] = OPT_NO;
			resp[TELOPT_TM]--;
			return;

		case TELOPT_LFLOW:
			/*
			 * If we are going to support flow control option,
			 * then don't worry peer that we can't change the
			 * flow control characters.
			 */
			slctab[SLC_XON].defset.flag &= ~SLC_LEVELBITS;
			slctab[SLC_XON].defset.flag |= SLC_DEFAULT;
			slctab[SLC_XOFF].defset.flag &= ~SLC_LEVELBITS;
			slctab[SLC_XOFF].defset.flag |= SLC_DEFAULT;
		case TELOPT_TTYPE:
		case TELOPT_SGA:
		case TELOPT_NAWS:
		case TELOPT_TSPEED:
#ifdef	LINEMODE
		case TELOPT_LINEMODE:
#endif	LINEMODE
			changeok++;
			break;

		default:
			break;
		}

	}

	if (request) {
		if (!((resp[option] == 0 && hisopts[option] == OPT_YES) ||
		    hiswants[option] == OPT_YES)) {
			hiswants[option] = OPT_YES;
			fmt = doopt;
			resp[option]++;
		}
	} else {
		if (resp[option]) {
			resp[option]--;
			if (resp[option] && hisopts[option] == OPT_YES)
				resp[option]--;
		}
		if ((resp[option] == 0) && (hiswants[option] != OPT_YES)) {
			if (changeok)
				hiswants[option] = OPT_YES;
			else
				resp[option]++;
			fmt = (hiswants[option] ? doopt : dont);
		}
		hisopts[option] = OPT_YES;
	}

	if (fmt) {
		(void) sprintf(nfrontp, fmt, option);
		nfrontp += sizeof (dont) - 2;
	}

	/*
	 * Handle other processing that should occur after we have
	 * responded to client input.
	 */
	if (!request) {
		switch (option) {
#ifdef	LINEMODE
		case TELOPT_LINEMODE:
# ifdef	KLUDGELINEMODE
			/*
			 * Note client's desire to use linemode.
			 */
			lmodetype = REAL_LINEMODE;
# endif	/* KLUDGELINEMODE */
			clientstat(TELOPT_LINEMODE, WILL, 0);
			break;
#endif	LINEMODE
		
		default:
			break;
		}
	}

}  /* end of willoption */

wontoption(option, request)
	int option, request;
{
	char *fmt = (char *)0;

	/*
	 * Process client input.
	 */
	if (!request) {
		switch (option) {
		case TELOPT_ECHO:
			not42 = 1;	/* doesn't seem to be a 4.2 system */
			break;

		case TELOPT_BINARY:
			init_termbuf();
			tty_binaryin(0);
			set_termbuf();
			break;

#ifdef	LINEMODE
		case TELOPT_LINEMODE:
# ifdef	KLUDGELINEMODE
			/*
			 * If real linemode is supported, then client is
			 * asking to turn linemode off.
			 */
			if (lmodetype == REAL_LINEMODE)
# endif	/* KLUDGELINEMODE */
				clientstat(TELOPT_LINEMODE, WONT, 0);
			break;
#endif	LINEMODE

		case TELOPT_TM:
#if	defined(LINEMODE) && defined(KLUDGELINEMODE)
			if (lmodetype < REAL_LINEMODE) {
				lmodetype = NO_LINEMODE;
				clientstat(TELOPT_LINEMODE, WONT, 0);
				dooption(TELOPT_SGA, 0);
			}
#endif	/* defined(LINEMODE) && defined(KLUDGELINEMODE) */
			/*
			 * If we get a WONT TM, and had sent a DO TM, don't
			 * respond with a DONT TM, just leave it as is.
			 * Short circut the state machine to achive this.
			 * The bad part of this is that if the client sends
			 * a WONT TM on his own to turn off linemode, then he
			 * won't get a response.
			 */
			hiswants[TELOPT_TM] = OPT_NO;
			resp[TELOPT_TM]--;
			return;

		case TELOPT_LFLOW:
			/*
			 * If we are not going to support flow control option,
			 * then let peer know that we can't change the
			 * flow control characters.
			 */
			slctab[SLC_XON].defset.flag &= ~SLC_LEVELBITS;
			slctab[SLC_XON].defset.flag |= SLC_CANTCHANGE;
			slctab[SLC_XOFF].defset.flag &= ~SLC_LEVELBITS;
			slctab[SLC_XOFF].defset.flag |= SLC_CANTCHANGE;
			break;

		default:
			break;
		}
	}


	if (request) {
		if (!((resp[option] == 0 && hisopts[option] == OPT_NO) ||
			hiswants[option] == OPT_NO)) {
			hiswants[option] = OPT_NO;
			fmt = dont;
			resp[option]++;
		}
	} else {
		if (resp[option]) {
			resp[option]--;
			if (resp[option] && hisopts[option] == OPT_NO)
				resp[option]--;
		}
		if ((resp[option] == 0) && (hiswants[option] != OPT_NO)) {
			/* it is always ok to change to negative state */
			hiswants[option] = OPT_NO;
			fmt = dont;
		}
		hisopts[option] = OPT_NO;
	}

	if (fmt) {
		(void) sprintf(nfrontp, fmt, option);
		nfrontp += sizeof (doopt) - 2;
	}

}  /* end of wontoption */

dooption(option, request)
	int option, request;
{
	int changeok = 0;
	char *fmt = (char *)0;

	/*
	 * Process client input.
	 */
	if (!request) {
		switch (option) {
		case TELOPT_ECHO:
#ifdef	LINEMODE
			if (hisopts[TELOPT_LINEMODE] == OPT_NO) {
#endif
				init_termbuf();
				tty_setecho(1);
				set_termbuf();
#ifdef	LINEMODE
			}
#endif
			changeok++;
			break;

		case TELOPT_BINARY:
			init_termbuf();
			tty_binaryout(1);
			set_termbuf();
			changeok++;
			break;

		case TELOPT_SGA:
#if	defined(LINEMODE) && defined(KLUDGELINEMODE)
			/*
			 * If kludge linemode is in use, then we must process
			 * an incoming do SGA for linemode purposes.
			 */
			if (lmodetype == KLUDGE_LINEMODE) {
				/*
				 * Receipt of "do SGA" in kludge linemode
				 * is the peer asking us to turn off linemode.
				 * Make note of the request.
				 */
				clientstat(TELOPT_LINEMODE, WONT, 0);
				/*
				 * If linemode did not get turned off then
				 * don't tell peer that we did.  Breaking
				 * here forces a wont SGA to be returned.
				 */
				if (linemode)
					break;
			}
#endif	/* defined(LINEMODE) && defined(KLUDGELINEMODE) */
			changeok++;
			break;

		case TELOPT_STATUS:
			changeok++;
			break;

		case TELOPT_TM:
		case TELOPT_LINEMODE:
		case TELOPT_TTYPE:
		case TELOPT_NAWS:
		case TELOPT_TSPEED:
		case TELOPT_LFLOW:
		default:
			break;
		}
	}

	if (request) {
		if (!((resp[option] == 0 && myopts[option] == OPT_YES) ||
		    mywants[option] == OPT_YES)) {
			mywants[option] = OPT_YES;
			fmt = will;
			resp[option]++;
		}
	} else {
		if (resp[option]) {
			resp[option]--;
			if (resp[option] && myopts[option] == OPT_YES)
				resp[option]--;
		}
		if ((resp[option] == 0) && (mywants[option] != OPT_YES)) {
			if (changeok)
				mywants[option] = OPT_YES;
			else
				resp[option]++;
			fmt = (mywants[option] ? will : wont);
		}
		myopts[option] = OPT_YES;
	}

	if (fmt) {
		(void) sprintf(nfrontp, fmt, option);
		nfrontp += sizeof (doopt) - 2;
	}

}  /* end of dooption */


dontoption(option, request)
	int option, request;
{
	char *fmt = (char *)0;

	/*
	 * Process client input.
	 */
	if (!request) {
		switch (option) {
		case TELOPT_BINARY:
			init_termbuf();
			tty_binaryout(0);
			set_termbuf();
			break;

		case TELOPT_ECHO:		/* we should stop echoing */
#ifdef	LINEMODE
			if (hisopts[TELOPT_LINEMODE] == OPT_NO) {
#endif
				init_termbuf();
				tty_setecho(0);
				set_termbuf();
#ifdef	LINEMODE
			}
#endif
			break;

		case TELOPT_SGA:
#if	defined(LINEMODE) && defined(KLUDGELINEMODE)
			/*
			 * If kludge linemode is in use, then we must process an
			 * incoming do SGA for linemode purposes.
			 */
			if (lmodetype == KLUDGE_LINEMODE) {
				/*
				 * The client is asking us to turn linemode
				 * on.
				 */
				clientstat(TELOPT_LINEMODE, WILL, 0);
				/*
				 * If we did not turn line mode on, then what do
				 * we say?  Will SGA?  This violates design of
				 * telnet.  Gross.  Very Gross.
				 */
			}
#endif	/* defined(LINEMODE) && defined(KLUDGELINEMODE) */

		default:
			break;
		}
	}

	if (request) {
		if (!((resp[option] == 0 && myopts[option] == OPT_NO) ||
		    mywants[option] == OPT_NO)) {
			mywants[option] = OPT_NO;
			fmt = wont;
			resp[option]++;
		}
	} else {
		if (resp[option]) {
			resp[option]--;
			if (resp[option] && myopts[option] == OPT_NO)
				resp[option]--;
		}
		if ((resp[option] == 0) && (mywants[option] != OPT_NO)) {
			mywants[option] = OPT_NO;
			fmt = wont;
		}
		myopts[option] = OPT_NO;
	}

	if (fmt) {
	    (void) sprintf(nfrontp, fmt, option);
	    nfrontp += sizeof (wont) - 2;
	}

}  /* end of dontoption */

/*
 * suboption()
 *
 *	Look at the sub-option buffer, and try to be helpful to the other
 * side.
 *
 *	Currently we recognize:
 *
 *	Terminal type is
 *	Linemode
 *	Window size
 *	Terminal speed
 */
suboption()
{
    register int subchar;

    subchar = SB_GET();
    switch (subchar) {
    case TELOPT_TSPEED: {
	register int xspeed, rspeed;

	if (hisopts[TELOPT_TSPEED] == OPT_NO)	/* Ignore if option disabled */
		break;

	settimer(tspeedsubopt);

	if (SB_EOF() || SB_GET() != TELQUAL_IS)
		return;

	xspeed = atoi(subpointer);

	while (SB_GET() != ',' && !SB_EOF());
	if (SB_EOF())
		return;

	rspeed = atoi(subpointer);
	clientstat(TELOPT_TSPEED, xspeed, rspeed);

	break;

    }  /* end of case TELOPT_TSPEED */

    case TELOPT_TTYPE: {		/* Yaaaay! */
	static char terminalname[5+41] = "TERM=";

	if (hisopts[TELOPT_TTYPE] == OPT_NO)	/* Ignore if option disabled */
		break;
	settimer(ttypesubopt);

	if (SB_GET() != TELQUAL_IS) {
	    return;		/* ??? XXX but, this is the most robust */
	}

	terminaltype = terminalname+sizeof("TERM=")-1;

	while ((terminaltype < (terminalname + sizeof terminalname-1)) &&
								    !SB_EOF()) {
	    register int c;

	    c = SB_GET();
	    if (isupper(c)) {
		c = tolower(c);
	    }
	    *terminaltype++ = c;    /* accumulate name */
	}
	*terminaltype = 0;
	terminaltype = terminalname;
	break;
    }  /* end of case TELOPT_TTYPE */

    case TELOPT_NAWS: {
	register int xwinsize, ywinsize;

	if (hisopts[TELOPT_NAWS] == OPT_NO)	/* Ignore if option disabled */
		break;

	if (SB_EOF())
		return;
	xwinsize = SB_GET() << 8;
	if (SB_EOF())
		return;
	xwinsize |= SB_GET();
	if (SB_EOF())
		return;
	ywinsize = SB_GET() << 8;
	if (SB_EOF())
		return;
	ywinsize |= SB_GET();
	clientstat(TELOPT_NAWS, xwinsize, ywinsize);

	break;

    }  /* end of case TELOPT_NAWS */

#ifdef	LINEMODE
    case TELOPT_LINEMODE: {
	register int request;

	if (hisopts[TELOPT_LINEMODE] == OPT_NO)	/* Ignore if option disabled */
		break;
	/*
	 * Process linemode suboptions.
	 */
	if (SB_EOF()) break;  /* garbage was sent */
	request = SB_GET();  /* get will/wont */
	if (SB_EOF()) break;  /* another garbage check */

	if (request == LM_SLC) {  /* SLC is not preceeded by WILL or WONT */
		/*
		 * Process suboption buffer of slc's
		 */
		start_slc(1);
		do_opt_slc(subpointer, subend - subpointer);
		end_slc(0);

	} else if (request == LM_MODE) {
		useeditmode = SB_GET();  /* get mode flag */
		clientstat(LM_MODE, 0, 0);
	}

	switch (SB_GET()) {  /* what suboption? */
	case LM_FORWARDMASK:
		/*
		 * According to spec, only server can send request for
		 * forwardmask, and client can only return a positive response.
		 * So don't worry about it.
		 */

	default:
		break;
	}

    }  /* end of case TELOPT_LINEMODE */
#endif
    case TELOPT_STATUS: {
	int mode;

	mode = SB_GET();
	switch (mode) {
	case TELQUAL_SEND:
	    if (myopts[TELOPT_STATUS] == OPT_YES)
		send_status();
	    break;

	case TELQUAL_IS:
	    break;

	default:
	    break;
	}
    }

    default:
	break;
    }  /* end of switch */

}  /* end of suboption */

#define	ADD(c)	 *ncp++ = c;
#define	ADD_DATA(c) { *ncp++ = c; if (c == SE) *ncp++ = c; }
send_status()
{
	char statusbuf[256];
	register char *ncp;
	register int i;

	ncp = statusbuf;

	netflush();	/* get rid of anything waiting to go out */

	ADD(IAC);
	ADD(SB);
	ADD(TELOPT_STATUS);
	ADD(TELQUAL_IS);

	for (i = 0; i < NTELOPTS; i++) {
		if (myopts[i] == OPT_YES) {
			ADD(WILL);
			ADD_DATA(i);
			if (i == IAC)
				ADD(IAC);
		}
		if (hisopts[i] == OPT_YES) {
			ADD(DO);
			ADD_DATA(i);
			if (i == IAC)
				ADD(IAC);
		}
	}

#ifdef	LINEMODE
	if (hisopts[TELOPT_LINEMODE] == OPT_YES) {
		char *cp, *cpe;
		int len;

		ADD(SB);
		ADD(TELOPT_LINEMODE);
		ADD(LM_MODE);
		ADD_DATA(editmode);
		if (editmode == IAC)
			ADD(IAC);
		ADD(SE);

		ADD(SB);
		ADD(TELOPT_LINEMODE);
		ADD(LM_SLC);
		start_slc(0);
		send_slc();
		len = end_slc(&cp);
		for (cpe = cp + len; cp < cpe; cp++)
			ADD_DATA(*cp);
		ADD(SE);
	}
#endif	/* LINEMODE */

	ADD(IAC);
	ADD(SE);

	writenet(statusbuf, ncp - statusbuf);
	netflush();	/* Send it on its way */
}
