PUSHDIVERT(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988 The Regents of the University of California.
# All rights reserved.
#
# %sccs.include.redist.sh%
#
POPDIVERT

##################################################
###   Local and Program Mailer specification   ###
##################################################

VERSIONID(@(#)local.m4	2.2 (Berkeley) %G%)

Mlocal,	P=ifdef(`LOCAL_MAILER', `LOCAL_MAILER', /bin/mail), F=rlsDFMmn, S=10, R=20, A=mail -d $u
Mprog,	P=ifdef(`LOCAL_SHELL', `LOCAL_SHELL', /bin/sh),   F=lsDFMe,   S=10, R=20, A=sh -c $u

S10
R@			$n			errors to mailer-daemon
