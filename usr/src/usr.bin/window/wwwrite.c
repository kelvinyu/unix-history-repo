#ifndef lint
static	char *sccsid = "@(#)wwwrite.c	3.18 84/03/03";
#endif

#include "ww.h"
#include "tt.h"

wwwrite(w, p, n)
register struct ww *w;
register char *p;
int n;
{
	char hascursor;
	char *savep = p;
	char *q = p + n;

	if (hascursor = w->ww_hascursor)
		wwcursor(w, 0);
	while (p < q && !w->ww_stopped && (!wwinterrupt() || w->ww_nointr)) {
		if (w->ww_wstate == 0 && !ISCTRL(*p)) {
			register i;
			register union ww_char *bp;
			int col, col1;

			if (w->ww_insert) {	/* this is very slow */
				if (*p == '\t') {
					p++;
					w->ww_cur.c += 8 -
						(w->ww_cur.c - w->ww_w.l & 7);
					goto chklf;
				}
				wwinschar(w, w->ww_cur.r, w->ww_cur.c,
					*p++ | w->ww_modes << WWC_MSHIFT);
				goto right;
			}

			bp = &w->ww_buf[w->ww_cur.r][w->ww_cur.c];
			i = w->ww_cur.c;
			while (i < w->ww_w.r && p < q && !ISCTRL(*p))
				if (*p == '\t') {
					register tmp = 8 - (i - w->ww_w.l & 7);
					p++;
					i += tmp;
					bp += tmp;
				} else {
					bp++->c_w = *p++
						| w->ww_modes << WWC_MSHIFT;
					i++;
				}

			col = MAX(w->ww_cur.c, w->ww_i.l);
			col1 = MIN(i, w->ww_i.r);
			w->ww_cur.c = i;
			if (w->ww_cur.r >= w->ww_i.t && w->ww_cur.r < w->ww_i.b)
			{
				register union ww_char *ns = wwns[w->ww_cur.r];
				register char *smap = &wwsmap[w->ww_cur.r][col];
				register char *win = w->ww_win[w->ww_cur.r];
				int nchanged = 0;

				bp = w->ww_buf[w->ww_cur.r];
				for (i = col; i < col1; i++)
					if (*smap++ == w->ww_index) {
						nchanged++;
						ns[i].c_w = bp[i].c_w
							^ win[i] << WWC_MSHIFT;
					}
				if (nchanged > 0) {
					wwtouched[w->ww_cur.r] |= WWU_TOUCHED;
					wwupdate1(w->ww_cur.r, w->ww_cur.r + 1);
				}
			}
			
		chklf:
			if (w->ww_cur.c >= w->ww_w.r)
				goto crlf;
		} else switch (w->ww_wstate) {
		case 0:
			switch (*p++) {
			case '\n':
				if (w->ww_mapnl)
		crlf:
					w->ww_cur.c = w->ww_w.l;
		lf:
				if (++w->ww_cur.r >= w->ww_w.b) {
					w->ww_cur.r = w->ww_w.b - 1;
					if (w->ww_w.b < w->ww_b.b) {
						(void) wwscroll1(w, w->ww_i.t,
							w->ww_i.b, 1, 0);
						w->ww_buf++;
						w->ww_b.t--;
						w->ww_b.b--;
					} else
						wwdelline(w, w->ww_b.t);
				}
				break;
			case '\b':
				if (--w->ww_cur.c < w->ww_w.l) {
					w->ww_cur.c = w->ww_w.r - 1;
					goto up;
				}
				break;
			case '\r':
				w->ww_cur.c = w->ww_w.l;
				break;
			case CTRL(g):
				ttputc(CTRL(g));
				break;
			case CTRL([):
				w->ww_wstate = 1;
				break;
			}
			break;
		case 1:
			w->ww_wstate = 0;
			switch (*p++) {
			case '@':
				w->ww_insert = 1;
				break;
			case 'A':
		up:
				if (--w->ww_cur.r < w->ww_w.t) {
					w->ww_cur.r = w->ww_w.t;
					if (w->ww_w.t > w->ww_b.t) {
						(void) wwscroll1(w, w->ww_i.t,
							w->ww_i.b, -1, 0);
						w->ww_buf--;
						w->ww_b.t++;
						w->ww_b.b++;
					} else
						wwinsline(w, w->ww_b.t);
				}
				break;
			case 'B':
				goto lf;
			case 'C':
		right:
				w->ww_cur.c++;
				goto chklf;
			case 'E':
				w->ww_buf -= w->ww_w.t - w->ww_b.t;
				w->ww_b.t = w->ww_w.t;
				w->ww_b.b = w->ww_b.t + w->ww_b.nr;
				w->ww_cur.r = w->ww_w.t;
				w->ww_cur.c = w->ww_w.l;
				wwclreos(w, w->ww_w.t, w->ww_w.l);
				break;
			case 'H':
				w->ww_cur.r = w->ww_w.t;
				w->ww_cur.c = w->ww_w.l;
				break;
			case 'J':
				wwclreos(w, w->ww_cur.r, w->ww_cur.c);
				break;
			case 'K':
				wwclreol(w, w->ww_cur.r, w->ww_cur.c);
				break;
			case 'L':
				wwinsline(w, w->ww_cur.r);
				break;
			case 'M':
				wwdelline(w, w->ww_cur.r);
				break;
			case 'N':
				wwdelchar(w, w->ww_cur.r, w->ww_cur.c);
				break;
			case 'O':
				w->ww_insert = 0;
				break;
			case 'Y':
				w->ww_wstate = 2;
				break;
			case 'p':
				w->ww_modes |= WWM_REV;
				break;
			case 'q':
				w->ww_modes &= ~WWM_REV;
				break;
			case 'r':
				w->ww_modes |= WWM_UL;
				break;
			case 's':
				w->ww_modes &= ~WWM_UL;
				break;
			case 'F':
				w->ww_modes |= WWM_GRP;
				break;
			case 'G':
				w->ww_modes &= ~WWM_GRP;
				break;
			}
			break;
		case 2:
			w->ww_cur.r = w->ww_w.t +
				(unsigned)(*p++ - ' ') % w->ww_w.nr;
			w->ww_wstate = 3;
			break;
		case 3:
			w->ww_cur.c = w->ww_w.l +
				(unsigned)(*p++ - ' ') % w->ww_w.nc;
			w->ww_wstate = 0;
			break;
		}
	}
	if (hascursor)
		wwcursor(w, 1);
	wwnwwr++;
	wwnwwra += n;
	n = p - savep;
	wwnwwrc += n;
	return n;
}
