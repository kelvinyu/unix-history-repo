/*
 * Copyright (c) 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <curses.h>
#include <err.h>

#include "gomoku.h"

#define USER	0		/* get input from standard input */
#define PROGRAM	1		/* get input from program */
#define INPUTF	2		/* get input from a file */

int	interactive = 1;	/* true if interactive */
int	debug;			/* true if debugging */
int	test;			/* both moves come from 1: input, 2: computer */
char	*prog;			/* name of program */
FILE	*debugfp;		/* file for debug output */
FILE	*inputfp;		/* file for debug input */

char	*color[]	= { "black", "white", "empty", "border" };
char	pdir[4]		= "-\\|/";
char	fmtbuf[128];

struct	spotstr	board[BAREA];		/* info for board */
struct	combostr frames[FAREA];		/* storage for all frames */
struct	combostr *sortframes[2];	/* sorted list of non-empty frames */
char	overlap[FAREA * FAREA];		/* true if frame [a][b] overlap */
short	intersect[FAREA * FAREA];	/* frame [a][b] intersection */
int	movelog[BSZ * BSZ];		/* log of all the moves */
int	movenum;			/* current move number */
char	*plyr[2];			/* who's who */

main(argc, argv)
	int argc;
	char **argv;
{
	char buf[128];
	int color, curmove, i, ch;
	int input[2];
	extern void whatsup(), quit();
	static char *fmt[2] = {
		"%3d %-6s",
		"%3d        %-6s"
	};

	prog = strrchr(argv[0], '/');
	if (prog)
		prog++;
	else
		prog = argv[0];

	while ((ch = getopt(argc, argv, "bcdD:u")) != EOF) {
		switch (ch) {
		case 'b':	/* background */
			interactive = 0;
			break;
		case 'd':	/* debugging */
			debug++;
			break;
		case 'D':	/* log debug output to file */
			if ((debugfp = fopen(optarg, "w")) == NULL)
				err(1, "%s", optarg);
			break;
		case 'u':	/* testing: user verses user */
			test = 1;
			break;
		case 'c':	/* testing: computer verses computer */
			test = 2;
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc) {
		if ((inputfp = fopen(*argv, "r")) == NULL)
			err(1, "%s", *argv);
		test = 3;
	}

	if (!debug)
#ifdef SVR4
		srandom(time(0));
#else
		srand(time(0));
#endif
	if (interactive)
		cursinit();		/* initialize curses */
again:
	bdinit(board);			/* initialize board contents */

	if (interactive) {
		plyr[BLACK] = plyr[WHITE] = "???";
		bdisp_init();		/* initialize display of board */
#ifdef DEBUG
		signal(SIGINT, whatsup);
#else
		signal(SIGINT, quit);
#endif

		if (test != 3) {
			for (;;) {
				ask("black or white? ");
				getline(buf, sizeof buf);
				if (buf[0] == 'b' || buf[0] == 'B') {
					color = BLACK;
					break;
				}
				if (buf[0] == 'w' || buf[0] == 'W') {
					color = WHITE;
					break;
				}
				move(22, 0);
				printw("Black moves first. Please enter `black' or `white'\n");
			}
			move(22, 0);
			clrtoeol();
		}
	} else {
		setbuf(stdout, 0);
		getline(buf, sizeof buf);
		if (strcmp(buf, "black") == 0)
			color = BLACK;
		else if (strcmp(buf, "white") == 0)
			color = WHITE;
		else {
			sprintf(fmtbuf,
			    "Huh?  Expected `black' or `white', got `%s'\n",
			    buf);
			panic(fmtbuf);
		}
	}

	switch (test) {
	case 0: /* user verses program */
		input[color] = USER;
		input[!color] = PROGRAM;
		break;

	case 1: /* user verses user */
		input[BLACK] = USER;
		input[WHITE] = USER;
		break;

	case 2: /* program verses program */
		input[BLACK] = PROGRAM;
		input[WHITE] = PROGRAM;
		break;

	case 3: /* user verses program */
		input[BLACK] = INPUTF;
		input[WHITE] = INPUTF;
	}
	if (interactive) {
		plyr[BLACK] = input[BLACK] == USER ? "you" : prog;
		plyr[WHITE] = input[WHITE] == USER ? "you" : prog;
		bdwho(1);
	}

	for (color = BLACK; ; color = !color) {
		switch (input[color]) {
		case INPUTF: /* input comes from a file */
			curmove = readinput(inputfp);
			if (curmove != ILLEGAL)
				break;
			input[color] = USER;
			input[!color] = PROGRAM;
			plyr[color] = "you";
			bdwho(1);
			/* FALLTHROUGH */

		case USER: /* input comes from standard input */
		getinput:
			if (interactive)
				ask("move? ");
			if (!getline(buf, sizeof buf)) {
				curmove = RESIGN;
				break;
			}
			if (buf[0] == '\0')
				goto getinput;
			curmove = ctos(buf);
			if (interactive) {
				if (curmove == SAVE) {
					FILE *fp;

					ask("save file name? ");
					(void)getline(buf, sizeof buf);
					if ((fp = fopen(buf, "w")) == NULL) {
						log("cannot create save file");
						goto getinput;
					}
					for (i = 0; i < movenum - 1; i++)
						fprintf(fp, "%s\n",
							stoc(movelog[i]));
					fclose(fp);
					goto getinput;
				}
				if (curmove != RESIGN &&
				    board[curmove].s_occ != EMPTY) {
					log("Illegal move");
					goto getinput;
				}
			}
			break;

		case PROGRAM: /* input comes from the program */
			curmove = pickmove(color);
			break;
		}
		if (interactive) {
			sprintf(fmtbuf, fmt[color], movenum, stoc(curmove));
			log(fmtbuf);
		}
		if ((i = makemove(color, curmove)) != MOVEOK)
			break;
		if (interactive)
			bdisp();
	}
	if (interactive) {
		move(22, 0);
		switch (i) {
		case WIN:
			if (input[color] == PROGRAM)
				addstr("Ha ha, I won");
			else
				addstr("Rats! you won");
			break;
		case TIE:
			addstr("Wow! its a tie");
			break;
		case ILLEGAL:
			addstr("Illegal move");
			break;
		}
		clrtoeol();
		bdisp();
		if (i != RESIGN) {
			ask("replay? ");
			if (getline(buf, sizeof buf) &&
			    buf[0] == 'y' || buf[0] == 'Y')
				goto again;
		}
	}
	quit();
}

readinput(fp)
	FILE *fp;
{
	char *cp;
	int c;

	cp = fmtbuf;
	while ((c = getc(fp)) != EOF && c != '\n')
		*cp++ = c;
	*cp = '\0';
	return (ctos(fmtbuf));
}

#ifdef DEBUG
/*
 * Handle strange situations.
 */
void
whatsup(signum)
	int signum;
{
	int i, pnum, n;
	struct spotstr *sp;
	FILE *fp;
	char *str;
	struct elist *ep;
	struct combostr *cbp;

	if (!interactive)
		quit();
top:
	ask("cmd? ");
	if (!getline(fmtbuf, 128))
		quit();
	switch (*fmtbuf) {
	case 'q':		/* conservative quit */
		quit();
	case 'd':		/* set debug level */
		debug = fmtbuf[1] - '0';
		sprintf(fmtbuf, "Debug set to %d", debug);
		dlog(fmtbuf);
		sleep(1);
	case 'c':
		break;
	case 'b':		/* back up a move */
		if (movenum > 1) {
			movenum--;
			board[movelog[movenum - 1]].s_occ = EMPTY;
			bdisp();
		}
		goto top;
	case 'f':		/* go forward a move */
		board[movelog[movenum - 1]].s_occ = movenum & 1 ? BLACK : WHITE;
		movenum++;
		bdisp();
		goto top;
	case 'l':		/* print move history */
		if (fmtbuf[1] == '\0') {
			for (i = 0; i < movenum - 1; i++)
				dlog(stoc(movelog[i]));
			goto top;
		}
		if ((fp = fopen(fmtbuf + 1, "w")) == NULL)
			goto top;
		for (i = 0; i < movenum - 1; i++) {
			fprintf(fp, "%s", stoc(movelog[i]));
			if (++i < movenum - 1)
				fprintf(fp, " %s\n", stoc(movelog[i]));
			else
				fputc('\n', fp);
		}
		bdump(fp);
		fclose(fp);
		goto top;
	case 'p':
		sp = &board[i = ctos(fmtbuf + 1)];
		sprintf(fmtbuf, "V %s %x/%d %d %x/%d %d %d %x", stoc(i),
			sp->s_combo[BLACK].s, sp->s_level[BLACK],
			sp->s_nforce[BLACK],
			sp->s_combo[WHITE].s, sp->s_level[WHITE],
			sp->s_nforce[WHITE], sp->s_wval, sp->s_flg);
		dlog(fmtbuf);
		sprintf(fmtbuf, "FB %s %x %x %x %x", stoc(i),
			sp->s_fval[BLACK][0].s, sp->s_fval[BLACK][1].s,
			sp->s_fval[BLACK][2].s, sp->s_fval[BLACK][3].s);
		dlog(fmtbuf);
		sprintf(fmtbuf, "FW %s %x %x %x %x", stoc(i),
			sp->s_fval[WHITE][0].s, sp->s_fval[WHITE][1].s,
			sp->s_fval[WHITE][2].s, sp->s_fval[WHITE][3].s);
		dlog(fmtbuf);
		goto top;
	case 'P':
		sp = &board[i = ctos(fmtbuf + 1)];
		for (pnum = BLACK; pnum <= WHITE; pnum++) {
			for (ep = sp->s_empty[pnum]; ep; ep = ep->e_next) {
				cbp = ep->e_combo;
				str = fmtbuf;
				sprintf(str, "C%c %s", "BW"[pnum], stoc(i));
				str += strlen(str);
				if (cbp->c_nframes == 2) {
					sprintf(str, " %s%c",
						stoc(cbp->c_link[0]->c_vertex),
						pdir[cbp->c_link[0]->c_dir]);
					str += strlen(str);
					sprintf(str, " %s%c %x/%d",
						stoc(cbp->c_link[1]->c_vertex),
						pdir[cbp->c_link[1]->c_dir],
						cbp->c_combo.s, cbp->c_nframes);
				} else {
					sprintf(str, " %s%c %x/%d",
						stoc(cbp->c_vertex),
						pdir[ep->e_frame->c_dir],
						cbp->c_combo.s, cbp->c_nframes);
				}
				dlog(fmtbuf);
			}
		}
		goto top;
	default:
syntax:
		dlog("Options are:");
		dlog("q    - quit");
		dlog("c    - continue");
		dlog("d#   - set debug level to #");
		dlog("p#   - print values at #");
		goto top;
	}
}
#endif /* DEBUG */

/*
 * Display debug info.
 */
dlog(str)
	char *str;
{

	if (debugfp)
		fprintf(debugfp, "%s\n", str);
	else if (interactive)
		dislog(str);
	else
		fprintf(stderr, "%s\n", str);
}

log(str)
	char *str;
{

	if (debugfp)
		fprintf(debugfp, "%s\n", str);
	if (interactive)
		dislog(str);
	else
		printf("%s\n", str);
}

void
quit()
{
	if (interactive) {
		bdisp();		/* show final board */
		cursfini();
	}
	exit(0);
}

/*
 * Die gracefully.
 */
panic(str)
	char *str;
{
	fprintf(stderr, "%s: %s\n", prog, str);
	fputs("resign\n", stdout);
	quit();
}
