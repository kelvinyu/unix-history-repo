#ifndef lint
static char sccsid[] = "@(#)main.c	1.6 (Lucasfilm) %G%";
#endif

#include "systat.h"

static struct nlist nlst[] = {
#define X_CCPU          0
        { "_ccpu" },
#define X_AVENRUN       1
        { "_avenrun" },
        { "" }
};

int     kmem = -1;
int     mem = -1;
int     swap = -1;
int	naptime = 5;

int     die();
int     display();
int     suspend();

static	WINDOW *wload;			/* one line window for load average */

main(argc, argv)
        int argc;
        char **argv;
{

	argc--, argv++;
	while (argc > 0) {
		if (argv[0][0] == '-') {
			struct cmdtab *p;

			p = lookup(&argv[0][1]);
			if (p == (struct cmdtab *)-1) {
				fprintf(stderr, "%s: unknown request\n",
				    &argv[0][1]);
				exit(1);
			}
			curcmd = p;
		} else {
			naptime = atoi(argv[1]);
			if (naptime < 5)
				naptime = 5;
		}
		argc--, argv++;
	}
        nlist("/vmunix", nlst);
	if (nlst[X_CCPU].n_type == 0) {
		fprintf(stderr, "Couldn't namelist /vmunix.\n");
		exit(1);
	}
	kmemf = "/dev/kmem";
	kmem = open(kmemf, O_RDONLY);
	if (kmem < 0) {
		perror(kmemf);
		exit(1);
	}
	memf = "/dev/mem";
	mem = open(memf, O_RDONLY);
	if (mem < 0) {
		perror(memf);
		exit(1);
	}
	swapf = "/dev/drum";
	swap = open(swapf, O_RDONLY);
	if (swap < 0) {
		perror(swapf);
		exit(1);
	}
        signal(SIGINT, die);
        signal(SIGQUIT, die);
        signal(SIGTERM, die);

        /*
	 * Initialize display.  Load average appears in a one line
	 * window of its own.  Current command's display appears in
	 * an overlapping sub-window of stdscr configured by the display
	 * routines to minimize update work by curses.
	 */
        initscr();
	wnd = (*curcmd->c_open)();
	if (wnd == NULL) {
		fprintf(stderr, "Couldn't initialize display.\n");
		die();
	}
	wload = newwin(1, 0, 3, 20);
	if (wload == NULL) {
		fprintf(stderr, "Couldn't set up load average window.\n");
		die();
	}

        gethostname(hostname, sizeof (hostname));
        lseek(kmem, nlst[X_CCPU].n_value, 0);
        read(kmem, &ccpu, sizeof (ccpu));
        lccpu = log(ccpu);
	(*curcmd->c_init)();
	curcmd->c_flags = 1;
        labels();

        known[0].k_uid = -1;
	known[0].k_name[0] = '\0';
        numknown = 1;
	procs[0].pid = -1;
	strcpy(procs[0].cmd, "<idle>");
	numprocs = 1;
        dellave = 0.0;

        signal(SIGALRM, display);
        signal(SIGTSTP, suspend);
        display();
        noecho();
        crmode();
	keyboard();
	/*NOTREACHED*/
}

labels()
{

        mvaddstr(2, 20,
                "/0   /1   /2   /3   /4   /5   /6   /7   /8   /9   /10");
        mvaddstr(3, 5, "Load Average");
        (*curcmd->c_label)();
#ifdef notdef
        mvprintw(21, 25, "CPU usage on %s", hostname);
#endif
        refresh();
}

display()
{
        register int i, j;

        /* Get the load average over the last minute. */
        lseek(kmem, nlst[X_AVENRUN].n_value, L_SET);
        read(kmem, &lave, sizeof (lave));
        (*curcmd->c_fetch)();
        j = 5.0*lave + 0.5;
        dellave -= lave;
        if (dellave >= 0.0)
                c = '<';
        else {
                c = '>';
                dellave = -dellave;
        }
        if (dellave < 0.1)
                c = '|';
        dellave = lave;
        wmove(wload, 0, 0); wclrtoeol(wload);
        for (i = (j > 50) ? 50 : j; i > 0; i--)
                waddch(wload, c);
        if (j > 50)
                wprintw(wload, " %4.1f", lave);
        (*curcmd->c_refresh)();
	wrefresh(wload);
        wrefresh(wnd);
        move(22, col);
        refresh();
        alarm(naptime);
}

load()
{

	lseek(kmem, nlst[X_AVENRUN].n_value, L_SET);
	read(kmem, &lave, sizeof (lave));
	mvprintw(22, 0, "%4.1f", lave);
	clrtoeol();
}

die()
{

        endwin();
        exit(0);
}

error(fmt, a1, a2, a3)
{

	mvprintw(22, 0, fmt, a1, a2, a3);
	clrtoeol();
	refresh();
}
