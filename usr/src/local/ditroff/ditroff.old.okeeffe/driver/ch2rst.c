/*	ch2rst.c	1.5	85/01/31
 *
 * Font translation to Imagen-style fonts (RST format) from character format.
 *
 *	Use:	ch2rst  [ -i  -s ]  charfile  > rstfile
 *
 *		Takes input from charfile (which must be in the format written
 *	by one of the xxx2ch programs), converts to rst format and writes to
 *	stdout.  If charfile is missing, stdin is read.  The -i flag tells
 *	ch2rst to ignore the character codes at the start of each glyph
 *	definition, and pack the glyphs in consecutive code positions starting
 *	with 0.  The -s flag forces ch2rst to NOT trim off any white space in
 *	the glyph map.  This is usefull to make stipples of fixed size.
 */

#include <stdio.h>
#include "rst.h"


#define PREAMBLE	44			/* size of preamble */
#define STRINGS		2			/* length of strings at pre. */
#define STARTGLYPH	PREAMBLE + STRINGS
#define MAXLINE		250
#define GLYPHSPACE	(MAXLINE * MAXLINE)
#define wr1(x)		putchar(x)


char *	malloc();

glyph_dir g[DIRSIZ];		/* directory of glyph definitions */
preamble p;			/* set of variables for preamble */
double widthtofix;		/* fix conversion factor */

int	code;
int	width, length, maxv, minv, maxh, minh, refv, refh;
int	bitwidth;

int	ignorecode = 0;
int	stipple = 0;
FILE *	filep;
char	ibuff[MAXLINE];
char	ebuff[MAXLINE];
char *	glyphs[DIRSIZ];
char	charbits[GLYPHSPACE];	/* place to store bits for a glyph */


main(argc, argv)
int argc;
char **argv;
{
    register int i;
    register int j;
    register int codeindex;
    register char *chp;
    register char *bitp;
    float par;

    while (argc > 1 && argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 'i':
		ignorecode = 1;
		break;

	case 's':
		stipple = 1;
		break;
	default:
		error("%s, unknown option flag", argv[1]);
	}
	argc--; argv++;
    }

    if (argc == 2) {
	if ((filep = fopen (argv[1], "r")) == NULL)
	    error("can't open file \"%s\"", argv[1]);
    } else filep = stdin;

    widthtofix = (1.0 / FIXPIX);	/* default fix conversion factor */
    codeindex = 0;
    for (i = 0; i < DIRSIZ; glyphs[i++] = (char *) 0);

    fgets(ibuff, MAXLINE, filep);
    if (strcmp(ibuff, "fontheader\n"))
	error("not a character font file");

    while (fgets(ibuff, MAXLINE, filep) != NULL) {
	if (index(ibuff, '\n') == 0)
	    error("input line too long");

	if (ibuff[0] != ':') {
	    sscanf (ibuff, "%s %f", ebuff, &par);
	    if (strcmp(ebuff, "size") == 0) p.p_size = par + 0.5;
	    else if (strcmp(ebuff, "version") == 0) {
		if (p.p_version = par + 0.5)
		    error("wrong version (%d) for Font file.", p.p_version);
	    } else if (strcmp(ebuff, "mag") == 0) {
		p.p_mag = par + 0.5;
		if (p.p_mag) widthtofix = 1000.0 / (FIXPIX * p.p_mag);
	    } else if (strcmp(ebuff, "desiz") == 0) p.p_desiz = par / FIX + 0.5;
	    else if (strcmp(ebuff, "linesp") == 0)
		p.p_linesp = par * widthtofix + 0.5;
	    else if (strcmp(ebuff, "wordsp") == 0)
		p.p_wordsp = par * widthtofix + 0.5;
	    else if (strcmp(ebuff, "rot") == 0) p.p_rot = par + 0.5;
	    else if (strcmp(ebuff, "cadv") == 0) p.p_cadv = par + 0.5;
	    else if (strcmp(ebuff, "ladv") == 0) p.p_ladv = par + 0.5;
	    else if (strcmp(ebuff, "id") == 0) p.p_id = par + 0.5;
	    else if (strcmp(ebuff, "res") == 0) {
		if ((p.p_res = par + 0.5) != RES)
		    fprintf(stderr, "ch2rst: Warning, wrong resolution (%d).\n",
				p.p_res);
	    } /* ignore unrecognized fields */
	} else {
	    if (sscanf (ibuff, ":%d, width = %f", &code, &par) != 2)
		error("bad glyph header, %s", ibuff);
	    if (ignorecode) codeindex++; else codeindex = code;
	    g[codeindex].g_pwidth = par * widthtofix + 0.5;

	    chp = &charbits[0];
	    if (fgets(chp, MAXLINE, filep) == NULL)
		error("unexpected end of input");
	    width = strlen(chp) - 1;
	    minh = width;
	    maxh = 0;
	    refv = minv = -1;

	    for (length = 0; *chp != '\n'; length++) {
		if ((length + 1) * width > GLYPHSPACE)
		    error ("out of glyph space");
		for (i = 0; i < width; i++, chp++) {
		    switch (*chp) {
			case '.':
				break;
			case 'x':
				refh = i;
				refv = length;
				*chp = '.';
				break;
			case 'X':
				refh = i;
				refv = length;
			case '@':
			case '*':
				maxv = length;
				if (minv < 0) minv = length;
				if (i < minh) minh = i;
				if (i > maxh) maxh = i;
				break;
			default:
				error("illegal character '%c' in map.", *chp);
		    } /* switch */
		} /* for i */
		if (fgets(chp, MAXLINE, filep) == NULL)
			error("unexpected end of input");
	    } /* for length */

	    if (stipple) {		/* use the whole box to make a */
		minv = 0;		/* stipple pattern. */
		minh = 0;
		maxv = length - 1;
		maxh = width - 1;
	    }

	    if (refv < 0) error("no reference point in glyph %d.", code);
	    if (minv < 0) {
		minv = maxv = refv;
		minh = maxh = refh;
	    }
	    g[codeindex].g_height = maxv + 1 - minv;
	    g[codeindex].g_width = maxh + 1 - minh;
	    g[codeindex].g_up = refv - minv;
	    g[codeindex].g_left = refh - minh;
	    g[codeindex].g_bitp =
		g[codeindex].g_height * ((g[codeindex].g_width + 7) / 8);

	    bitp = (glyphs[codeindex] = malloc(g[codeindex].g_bitp)) - 1;
	    for (i = minv; i <= maxv; i++) {
		chp = &charbits[0] + width * i + minh;
		bitwidth = 0;
		for (j = minh; j <= maxh; j++, chp++) {
		    if (--bitwidth < 0) {
			*++bitp = '\0';
			bitwidth = 7;
		    }
		    if (*chp != '.') *bitp |= 1 << bitwidth;
		}
	    } /* for i */
	} /* else */
    } /* while */

    if (ignorecode) {
	p.p_last = codeindex - 1;
	p.p_first = 0;
    } else {
	for (i = DIRSIZ - 1; glyphs[i] == (char *) 0; i--);
	p.p_last = i;
	for (i = 0; glyphs[i] == (char *) 0; i++);
	p.p_first = i;
    }
    bitwidth = STARTGLYPH + 15 * (1 + p.p_last - p.p_first);

    printf("Rast UCB");
    wr2(p.p_size);	 wr1(p.p_version);	wr3(STARTGLYPH);
    wr2(p.p_first);	 wr2(p.p_last);		wr4(p.p_mag);
    wr4(p.p_desiz);	 wr4(p.p_linesp);	wr4(p.p_wordsp);
    wr2(p.p_rot);	 wr1(p.p_cadv);		wr1(p.p_ladv);
    wr4(p.p_id);	 wr2(p.p_res);
    for (i = 0; i < STRINGS; i++) putchar('\0');

    for (i = p.p_first; i <= p.p_last; i++) {
	if (glyphs[i] == (char *) 0) {
	    for (j = 0; j < 15; j++) putchar('\0');
	} else {
	    wr2(g[i].g_height);
	    wr2(g[i].g_width);
	    wr2(g[i].g_up);
	    wr2(g[i].g_left);
	    wr4(g[i].g_pwidth);
	    wr3(bitwidth);
	    bitwidth += g[i].g_bitp;
	}
    } /* for i */
    fflush(stdout);

    for (i = p.p_first; i <= p.p_last; i++)
	if (glyphs[i] != (char *) 0)
	    vwrite(glyphs[i], g[i].g_bitp);
    exit(0);
}


/*----------------------------------------------------------------------------*
 | Routine:	vwrite (buffer, buffer_size)
 |
 | Results:	writes out character array "buffer" of size "buffer_size"
 |		to standard output in small enough chunks that a pipe could
 |		handle them.
 |
 | Bugs:	this routine shouldn't be needed
 *----------------------------------------------------------------------------*/

vwrite(buf, bufsize)
char *buf;
int bufsize;
{
	int tsize = 0;

	while (bufsize) {
		buf += tsize;
		tsize = bufsize > BUFSIZ ? BUFSIZ : bufsize;
		if ((tsize = write(1, buf, tsize)) < 0) {
			perror("ch2rst: write failed");
			exit(-1);
		}
		bufsize -= tsize;
	}
}


/*----------------------------------------------------------------------------*
 | Routine:	error (format_string, argument1, argument2.... )
 |
 | Results:	fprints a message to standard error, then exits with error
 |		code 1
 |
 | Side Efct:	This routine does NOT return
 *----------------------------------------------------------------------------*/

/*VARARGS1*/
error(string, a1, a2, a3, a4)
char *string;
{ 
	fprintf(stderr, "ch2rst: ");
	fprintf(stderr, string, a1, a2, a3, a4);
	fprintf(stderr, "\n");
	exit(1);
}


/*----------------------------------------------------------------------------*
 | Routine:	wr2, wr3, wr4 (and wr1)
 |
 | Results:	writes out 2, 3, or 4 byte integers in RST byte order, using
 |		the wr1() routine, which writes one byte to standard output.
 *----------------------------------------------------------------------------*/

wr2(i)
unsigned int i;
{
    wr1((i >> 8) & 255);
    wr1(i & 255);
}

wr3(i)
unsigned int i;
{
    wr1((i >> 16) & 255);
    wr1(( i >> 8) & 255);
    wr1(i & 255);
}

wr4(i)
unsigned int i;
{
    wr1((i >> 24) & 255);
    wr1((i >> 16) & 255);
    wr1((i >> 8) & 255);
    wr1(i & 255);
}
