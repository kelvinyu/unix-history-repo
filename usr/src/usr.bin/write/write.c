/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jef Poskanzer and Craig Leres of the Lawrence Berkeley Laboratory.
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
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)write.c	4.16 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <utmp.h>
#include <ctype.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <strings.h>

#define STRCPY(s1, s2) \
    { (void)strncpy(s1, s2, sizeof(s1)); s1[sizeof(s1) - 1] = '\0'; }

int uid;					/* myuid */

main(argc, argv)
	int argc;
	char **argv;
{
	extern int errno;
	register char *cp;
	char tty[MAXPATHLEN];
	int msgsok, myttyfd;
	time_t atime;
	char *mytty, *getlogin(), *ttyname();
	void done();

	/* check that sender has write enabled. */
	if (isatty(fileno(stdin)))
		myttyfd = fileno(stdin);
	else if (isatty(fileno(stdout)))
		myttyfd = fileno(stdout);
	else if (isatty(fileno(stderr)))
		myttyfd = fileno(stderr);
	else {
		(void)fprintf(stderr, "write: can't find your tty\n");
		exit(1);
	}
	if (!(mytty = ttyname(myttyfd))) {
		(void)fprintf(stderr, "write: can't find your tty's name\n");
		exit(1);
	}
	if (cp = rindex(mytty, '/'))
		mytty = cp + 1;
	if (term_chk(mytty, &msgsok, &atime, 1))
		exit(1);
	if (!msgsok) {
		(void)fprintf(stderr,
		    "write: you have write permission turned off.\n");
		exit(1);
	}

	uid = getuid();

	/* check args */
	switch (argc) {
	case 2:
		search_utmp(argv[1], tty, mytty);
		do_write(tty, mytty);
		break;
	case 3:
		if (!strncmp(argv[2], "/dev/", 5))
			argv[2] += 5;
		if (utmp_chk(argv[1], argv[2])) {
			(void)fprintf(stderr,
			    "write: %s is not logged in on %s.\n",
			    argv[1], argv[2]);
			exit(1);
		}
		if (term_chk(argv[2], &msgsok, &atime, 1))
			exit(1);
		if (uid && !msgsok) {
			(void)fprintf(stderr,
			    "write: %s has messages disabled on %s\n",
			    argv[1], argv[2]);
			exit(1);
		}
		do_write(argv[2], mytty);
		break;
	default:
		(void)fprintf(stderr, "usage: write user [tty]\n");
		exit(1);
	}
	done();
	/* NOTREACHED */
}

/*
 * utmp_chk - checks that the given user is actually logged in on
 *     the given tty
 */
utmp_chk(user, tty)
	char *user, *tty;
{
	struct utmp u;
	int ufd;

	if ((ufd = open(_PATH_UTMP, O_RDONLY)) < 0)
		return(0);	/* ignore error, shouldn't happen anyway */

	while (read(ufd, (char *) &u, sizeof(u)) == sizeof(u))
		if (strncmp(user, u.ut_name, sizeof(u.ut_name)) == 0 &&
		    strncmp(tty, u.ut_line, sizeof(u.ut_line)) == 0) {
			(void)close(ufd);
			return(0);
		}

	(void)close(ufd);
	return(1);
}

/*
 * search_utmp - search utmp for the "best" terminal to write to
 *
 * Ignores terminals with messages disabled, and of the rest, returns
 * the one with the most recent access time.  Returns as value the number
 * of the user's terminals with messages enabled, or -1 if the user is
 * not logged in at all.
 *
 * Special case for writing to yourself - ignore the terminal you're
 * writing from, unless that's the only terminal with messages enabled.
 */
search_utmp(user, tty, mytty)
	char *user, *tty, *mytty;
{
	struct utmp u;
	time_t bestatime, atime;
	int ufd, nloggedttys, nttys, msgsok, user_is_me;
	char atty[UT_LINESIZE + 1];

	if ((ufd = open(_PATH_UTMP, O_RDONLY)) < 0) {
		perror("utmp");
		exit(1);
	}

	nloggedttys = nttys = 0;
	bestatime = 0;
	user_is_me = 0;
	while (read(ufd, (char *) &u, sizeof(u)) == sizeof(u))
		if (strncmp(user, u.ut_name, sizeof(u.ut_name)) == 0) {
			++nloggedttys;
			STRCPY(atty, u.ut_line);
			if (term_chk(atty, &msgsok, &atime, 0))
				continue;	/* bad term? skip */
			if (uid && !msgsok)
				continue;	/* skip ttys with msgs off */
			if (strcmp(atty, mytty) == 0) {
				user_is_me = 1;
				continue;	/* don't write to yourself */
			}
			++nttys;
			if (atime > bestatime) {
				bestatime = atime;
				(void)strcpy(tty, atty);
			}
		}

	(void)close(ufd);
	if (nloggedttys == 0) {
		(void)fprintf(stderr, "write: %s is not logged in\n", user);
		exit(1);
	}
	if (nttys == 0) {
		if (user_is_me) {		/* ok, so write to yourself! */
			(void)strcpy(tty, mytty);
			return;
		}
		(void)fprintf(stderr,
		    "write: %s has messages disabled\n", user);
		exit(1);
	} else if (nttys > 1) {
		(void)fprintf(stderr,
		    "write: %s is logged in more than once; writing to %s\n",
		    user, tty);
	}
}

/*
 * term_chk - check that a terminal exists, and get the message bit
 *     and the access time
 */
term_chk(tty, msgsokP, atimeP, showerror)
	char *tty;
	int *msgsokP, showerror;
	time_t *atimeP;
{
	struct stat s;
	char path[MAXPATHLEN];

	(void)sprintf(path, "/dev/%s", tty);
	if (stat(path, &s) < 0) {
		if (showerror)
			(void)fprintf(stderr, "write: %s: %s\n",
			    path, strerror(errno));
		return(1);
	}
	*msgsokP = (s.st_mode & (S_IWRITE >> 3)) != 0;	/* group write bit */
	*atimeP = s.st_atime;
	return(0);
}

/*
 * do_write - actually make the connection
 */
do_write(tty, mytty)
	char *tty, *mytty;
{
	register char *login, *nows;
	register struct passwd *pwd;
	time_t now, time();
	char path[MAXPATHLEN], host[MAXHOSTNAMELEN], line[512];
	void done();

	(void)sprintf(path, "/dev/%s", tty);
	if ((freopen(path, "w", stdout)) == NULL) {
		(void)fprintf(stderr,
		    "write: %s: %s\n", path, strerror(errno));
		exit(1);
	}

	/* catch ^C. */
	(void)signal(SIGINT, done);

	/* print greeting. */
	if ((login = getlogin()) == NULL)
		if (pwd = getpwuid(getuid()))
			login = pwd->pw_name;
		else
			login = "???";
	if (gethostname(host, sizeof(host)) < 0)
		(void)strcpy(host, "???");
	now = time((time_t *)NULL);
	nows = ctime(&now);
	nows[16] = '\0';
	(void)printf("\r\n\007\007\007Message from %s@%s on %s at %s ...\r\n",
	    login, host, mytty, nows  + 11);

	while (fgets(line, sizeof(line), stdin) != NULL)
		massage_fputs(line);
}

/*
 * done - cleanup and exit
 */
void
done()
{
	(void)printf("EOF\r\n");
	exit(0);
}

/*
 * massage_fputs - like fputs(), but makes control characters visible and
 *     turns \n into \r\n
 */
massage_fputs(s)
	register char *s;
{
	register char c;

#define	PUTC(c)	if (putchar(c) == EOF) goto err;

	for (; *s != '\0'; ++s) {
		c = toascii(*s);
		if (c == '\n') {
			PUTC('\r');
			PUTC('\n');
		} else if (!isprint(c) && !isspace(c) && c != '\007') {
			PUTC('^');
			PUTC(c^0x40);	/* DEL to ?, others to alpha */
		} else
			PUTC(c);
	}
	return;

err:	(void)fprintf(stderr, "write: %s\n", strerror(errno));
	exit(1);
#undef PUTC
}
