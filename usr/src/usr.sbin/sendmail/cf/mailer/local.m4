PUSHDIVERT(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#
ifdef(`LOCAL_MAILER_FLAGS',, `define(`LOCAL_MAILER_FLAGS', `rn')')
ifdef(`LOCAL_MAILER_PATH',, `define(`LOCAL_MAILER_PATH', /bin/mail)')
ifdef(`LOCAL_MAILER_ARGS',, `define(`LOCAL_MAILER_ARGS', `mail -d $u')')
ifdef(`LOCAL_SHELL_FLAGS',, `define(`LOCAL_SHELL_FLAGS', `eu')')
ifdef(`LOCAL_SHELL_PATH',, `define(`LOCAL_SHELL_PATH', /bin/sh)')
ifdef(`LOCAL_SHELL_ARGS',, `define(`LOCAL_SHELL_ARGS', `sh -c $u')')
POPDIVERT

##################################################
###   Local and Program Mailer specification   ###
##################################################

VERSIONID(`@(#)local.m4	8.3 (Berkeley) %G%')

Mlocal,		P=LOCAL_MAILER_PATH, F=CONCAT(`lsDFMm', LOCAL_MAILER_FLAGS), S=10, R=20,
		A=LOCAL_MAILER_ARGS
Mprog,		P=LOCAL_SHELL_PATH, F=CONCAT(`lsDFM', LOCAL_SHELL_FLAGS), S=10, R=20, D=$z:/,
		A=LOCAL_SHELL_ARGS

S10
R<@>			$n			errors to mailer-daemon
ifdef(`_ALWAYS_ADD_DOMAIN_',
`R$* < @ $* > $*		$@ $1 < @ $2 > $3	already fully qualified
R$*			$: $1 @ $M		add local qualification
R$* @			$: $1 @ $j		if $M not defined',
`dnl')
