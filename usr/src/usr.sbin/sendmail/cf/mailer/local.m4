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

VERSIONID(`@(#)local.m4	6.7 (Berkeley) %G%')

ifdef(`LOCAL_MAILER_FLAGS',, `define(`LOCAL_MAILER_FLAGS', `rn')')dnl
Mlocal,		P=ifdef(`LOCAL_MAILER_PATH', `LOCAL_MAILER_PATH', /bin/mail), F=CONCAT(`lsDFMm', LOCAL_MAILER_FLAGS), S=10, R=20,
		A=mail -d $u
Mprog,		P=ifdef(`LOCAL_SHELL_PATH', `LOCAL_SHELL_PATH', /bin/sh), F=lsDFMeu, S=10, R=20, D=$z:/,
		A=sh -c $u

S10
R<@>			$n			errors to mailer-daemon
