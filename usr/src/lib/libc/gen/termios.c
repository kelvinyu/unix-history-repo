/*-
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)termios.c	5.11 (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#define KERNEL			/* XXX - FREAD and FWRITE ifdef'd KERNEL*/
#include <sys/fcntl.h>
#undef KERNEL
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

int
tcgetattr(fd, t)
	int fd;
	struct termios *t;
{

	return (ioctl(fd, TIOCGETA, t));
}

int
tcsetattr(fd, opt, t)
	int fd, opt;
	const struct termios *t;
{
	struct termios localterm;

	if (opt & TCSASOFT) {
		localterm = *t;
		localterm.c_cflag |= CIGNORE;
		t = &localterm;
	}
	switch (opt & ~TCSASOFT) {
	case TCSANOW:
		return (ioctl(fd, TIOCSETA, t));
	case TCSADRAIN:
		return (ioctl(fd, TIOCSETAW, t));
	case TCSAFLUSH:
		return (ioctl(fd, TIOCSETAF, t));
	default:
		errno = EINVAL;
		return (-1);
	}
}

int
#if __STDC__
tcsetpgrp(int fd, pid_t pgrp)
#else
tcsetpgrp(fd, pgrp)
	int fd;
	pid_t pgrp;
#endif
{
	int s;

	s = pgrp;
	return (ioctl(fd, TIOCSPGRP, &s));
}

pid_t
tcgetpgrp(fd)
{
	int s;

	if (ioctl(fd, TIOCGPGRP, &s) < 0)
		return ((pid_t)-1);

	return ((pid_t)s);
}

speed_t
cfgetospeed(t)
	const struct termios *t;
{

	return (t->c_ospeed);
}

speed_t
cfgetispeed(t)
	const struct termios *t;
{

	return (t->c_ispeed);
}

int
cfsetospeed(t, speed)
	struct termios *t;
	speed_t speed;
{
	t->c_ospeed = speed;
	return (0);
}

int
cfsetispeed(t, speed)
	struct termios *t;
	speed_t speed;
{
	t->c_ispeed = speed;
	return (0);
}

int
cfsetspeed(t, speed)
	struct termios *t;
	speed_t speed;
{
	t->c_ispeed = t->c_ospeed = speed;
	return (0);
}

/*
 * Make a pre-existing termios structure into "raw" mode: character-at-a-time
 * mode with no characters interpreted, 8-bit data path.
 */
void
cfmakeraw(t)
	struct termios *t;
{
	t->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	t->c_oflag &= ~OPOST;
	t->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	t->c_cflag &= ~(CSIZE|PARENB);
	t->c_cflag |= CS8;
	/* XXX set MIN/TIME */
}

tcsendbreak(fd, len)
	int fd, len;
{
	struct timeval sleepytime;

	sleepytime.tv_sec = 0;
	sleepytime.tv_usec = 400000;
	if (ioctl(fd, TIOCSBRK, 0) == -1)
		return (-1);
	(void)select(0, 0, 0, 0, &sleepytime);
	if (ioctl(fd, TIOCCBRK, 0) == -1)
		return (-1);
	return (0);
}

tcdrain(fd)
	int fd;
{
	return (ioctl(fd, TIOCDRAIN, 0) == -1 ? -1 : 0);
}

tcflush(fd, which)
	int fd, which;
{
	int com;

	switch (which) {
	case TCIFLUSH:
		com = FREAD;
		break;
	case TCOFLUSH:
		com = FWRITE;
		break;
	case TCIOFLUSH:
		com = FREAD | FWRITE;
		break;
	default:
		errno = EINVAL;
		return (-1);
	}
	return (ioctl(fd, TIOCFLUSH, &com) == -1 ? -1 : 0);
}

tcflow(fd, action)
	int fd, action;
{
	struct termios term;
	u_char c;

	switch (action) {
	case TCOOFF:
		return (ioctl(fd, TIOCSTOP, 0) == -1 ? -1 : 0);
	case TCOON:
		return (ioctl(fd, TIOCSTART, 0) == -1 ? -1 : 0);
	case TCION:
	case TCIOFF:
		if (tcgetattr(fd, &term) == -1)
			return (-1);
		c = term.c_cc[action == TCIOFF ? VSTOP : VSTART];
		if (c != _POSIX_VDISABLE && write(fd, &c, sizeof(c)) == -1)
			return (-1);
		return (0);
	default:
		errno = EINVAL;
		return (-1);
	}
	/* NOTREACHED */
}
