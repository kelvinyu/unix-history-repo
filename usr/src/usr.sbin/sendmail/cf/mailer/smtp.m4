PUSHDIVERT(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#
ifdef(`SMTP_MAILER_FLAGS',,
	`define(`SMTP_MAILER_FLAGS',
		`ifdef(`_OLD_SENDMAIL_', `L', `')')')
POPDIVERT
#####################################
###   SMTP Mailer specification   ###
#####################################

VERSIONID(`@(#)smtp.m4	8.5 (Berkeley) %G%')

Msmtp,		P=[IPC], F=CONCAT(mDFMuX, SMTP_MAILER_FLAGS), S=11, R=ifdef(`_ALL_MASQUERADE_', `11', `21'), E=\r\n,
		ifdef(`_OLD_SENDMAIL_',, `L=990, ')A=IPC $h
Mesmtp,		P=[IPC], F=CONCAT(mDFMuXa, SMTP_MAILER_FLAGS), S=11, R=ifdef(`_ALL_MASQUERADE_', `11', `21'), E=\r\n,
		ifdef(`_OLD_SENDMAIL_',, `L=990, ')A=IPC $h
Mrelay,		P=[IPC], F=CONCAT(mDFMuXa, SMTP_MAILER_FLAGS), S=11, R=19, E=\r\n,
		ifdef(`_OLD_SENDMAIL_',, `L=2040, ')A=IPC $h

S11

# do sender/recipient common rewriting
R$+			$: $>19 $1

# if already @ qualified, we are done
R$* < @ $* > $*		$@ $1 < @ $2 > $3		already qualified

# do not qualify list:; syntax
R$* :; <@>		$@ $1 :;

# unqualified names (e.g., "eric") "come from" $M
R$=E			$@ $1 < @ $j>			show exposed names
R$+			$: $1 < @ $M >			user w/o host
R$+ <@>			$: $1 < @ $j >			in case $M undefined

ifdef(`_ALL_MASQUERADE_', `dnl',
`S21

# do sender/recipient common rewriting
R$+			$: $>19 $1

# if already @ qualified, we are done
R$* < @ $* > $*		$@ $1 < @ $2 > $3		already qualified

# do not qualify list:; syntax
R$* :; <@>		$@ $1 :;

# unqualified names (e.g., "eric") are qualified by local host
R$+			$: $1 < @ $j >')

S19

# pass <route-addr>s through
R< @ $+ > $*		$@ < @ $1 > $2			resolve <route-addr>

# output fake domains as user%fake@relay
ifdef(`BITNET_RELAY',
`R$+ <@ $+ . BITNET >	$: $1 % $2 .BITNET < @ $B >	user@host.BITNET
R$+.BITNET <@ $+:$+ >	$: $1 .BITNET < @ $3 >		strip mailer: part',
	`dnl')
ifdef(`CSNET_RELAY',
`R$+ <@ $+ . CSNET >	$: $1 % $2 .CSNET < @ $C >	user@host.CSNET
R$+.CSNET <@ $+:$+ >	$: $1 .CSNET < @ $3 >		strip mailer: part',
	`dnl')
ifdef(`_NO_UUCP_', `dnl',
`R$+ <@ $+ . UUCP >	$: $2 ! $1 < @ $j >		user@host.UUCP')
