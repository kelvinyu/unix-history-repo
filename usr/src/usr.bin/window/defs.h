/*
 *	@(#)defs.h	3.7 83/11/30	
 */

#include "ww.h"
#include <signal.h>
#ifndef O_4_1A
#include <sys/time.h>
#include <sys/resource.h>
#endif

#define NWINDOW 9

int nread;
int nreade;
int nreadz;
int nreadc;
#ifndef O_4_1A
struct timeval starttime;
#endif

	/* things for handling input */
char ibuf[512];
char *ibufp;
int ibufc;
#define bgetc()		(ibufc ? ibufc--, *ibufp++&0x7f : -1)
#define bpeekc()	(ibufc ? *ibufp&0x7f : -1)
#define bungetc(c)	(ibufp > ibuf ? ibufc++, *--ibufp = (c) : -1)

struct ww *window[NWINDOW];	/* the windows */
struct ww *selwin;		/* the selected window */
struct ww *lastselwin;		/* the last selected window */
struct ww *cmdwin;		/* the command window */
struct ww *framewin;		/* the window for framing */
struct ww *boxwin;		/* the window for the box */

char *shell;			/* the shell program */
char *shellname;		/* the shell program name (for argv[0]) */

int nbufline;			/* number of lines in the buffer */

	/* flags */
char quit;
char terse;
char debug;
char incmd;			/* in command mode */
char escapec;			/* escape character */

struct ww *getwin();
struct ww *openwin();
struct ww *vtowin();
struct ww *openiwin();
