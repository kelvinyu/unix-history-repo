PUSHDIVERT(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#

ifdef(`UUCP_MAILER_PATH',, `define(`UUCP_MAILER_PATH', /usr/bin/uux)')
ifdef(`UUCP_MAILER_ARGS',, `define(`UUCP_MAILER_ARGS', `uux - -r -a$g -gC $h!rmail ($u)')')
ifdef(`UUCP_MAILER_FLAGS',, `define(`UUCP_MAILER_FLAGS', `')')
ifdef(`UUCP_MAX_SIZE',, `define(`UUCP_MAX_SIZE', 100000)')
POPDIVERT
#####################################
###   UUCP Mailer specification   ###
#####################################

VERSIONID(`@(#)uucp.m4	8.18 (Berkeley) %G%')

#
#  There are innumerable variations on the UUCP mailer.  It really
#  is rather absurd.
#

# old UUCP mailer (two names)
Muucp,		P=UUCP_MAILER_PATH, F=CONCAT(DFMhuU, UUCP_MAILER_FLAGS), S=12, R=22/42, M=UUCP_MAX_SIZE, T=X-UUCP,
		A=UUCP_MAILER_ARGS
Muucp-old,	P=UUCP_MAILER_PATH, F=CONCAT(DFMhuU, UUCP_MAILER_FLAGS), S=12, R=22/42, M=UUCP_MAX_SIZE, T=X-UUCP,
		A=UUCP_MAILER_ARGS

# smart UUCP mailer (handles multiple addresses) (two names)
Msuucp,		P=UUCP_MAILER_PATH, F=CONCAT(mDFMhuU, UUCP_MAILER_FLAGS), S=12, R=22/42, M=UUCP_MAX_SIZE, T=X-UUCP,
		A=UUCP_MAILER_ARGS
Muucp-new,	P=UUCP_MAILER_PATH, F=CONCAT(mDFMhuU, UUCP_MAILER_FLAGS), S=12, R=22/42, M=UUCP_MAX_SIZE, T=X-UUCP,
		A=UUCP_MAILER_ARGS

ifdef(`_MAILER_smtp_',
`# domain-ized UUCP mailer
Muucp-dom,	P=UUCP_MAILER_PATH, F=CONCAT(mDFMhu, UUCP_MAILER_FLAGS), S=52/31, R=ifdef(`_ALL_MASQUERADE_', `11/31', `21'), M=UUCP_MAX_SIZE, T=X-UUCP,
		A=UUCP_MAILER_ARGS

# domain-ized UUCP mailer with UUCP-style sender envelope
Muucp-uudom,	P=UUCP_MAILER_PATH, F=CONCAT(mDFMhu, UUCP_MAILER_FLAGS), S=72/31, R=ifdef(`_ALL_MASQUERADE_', `11/31', `21'), M=UUCP_MAX_SIZE, T=X-UUCP,
		A=UUCP_MAILER_ARGS')


#
#  envelope and header sender rewriting
#
S12

# handle error address as a special case
R<@>				$n			errors to mailer-daemon

# do not qualify list:; syntax
R$* :; <@>			$@ $1 :;

R$* < @ $* . >			$1 < @ $2 >		strip trailing dots
R$* < @ $=w >			$1			strip local name
R<@ $- . UUCP > : $+		$1 ! $2			convert to UUCP format
R<@ $+ > : $+			$1 ! $2			convert to UUCP format
R$* < @ $- . UUCP >		$2 ! $1			convert to UUCP format
R$* < @ $+ >			$2 ! $1			convert to UUCP format
R$&h ! $+ ! $+			$@ $1 ! $2		$h!...!user => ...!user
R$&h ! $+			$@ $&h ! $1		$h!user => $h!user
R$+				$: $U ! $1		prepend our name
R! $+				$: $k ! $1		in case $U undefined

#
#  envelope recipient rewriting
#
S22

# don't touch list:; syntax
R$* :; <@>			$@ $1 :;

R$* < @ $* . >			$1 < @ $2 >		strip trailing dots
R$* < @ $j >			$1			strip local name
R<@ $- . UUCP > : $+		$1 ! $2			convert to UUCP format
R<@ $+ > : $+			$1 ! $2			convert to UUCP format
R$* < @ $- . UUCP >		$2 ! $1			convert to UUCP format
R$* < @ $+ >			$2 ! $1			convert to UUCP format

#
#  header recipient rewriting
#
S42

# don't touch list:; syntax
R$* :; <@>			$@ $1 :;

R$* < @ $* . >			$1 < @ $2 >		strip trailing dots
R$* < @ $j >			$1			strip local name
R<@ $- . UUCP > : $+		$1 ! $2			convert to UUCP format
R<@ $+ > : $+			$1 ! $2			convert to UUCP format
R$* < @ $- . UUCP >		$2 ! $1			convert to UUCP format
R$* < @ $+ >			$2 ! $1			convert to UUCP format
R$&h ! $+ ! $+			$@ $1 ! $2		$h!...!user => ...!user
R$&h ! $+			$@ $&h ! $1		$h!user => $h!user
R$+				$: $U ! $1		prepend our name
R! $+				$: $k ! $1		in case $U undefined


ifdef(`_MAILER_smtp_',
`#
#  envelope sender rewriting for uucp-dom mailer
#
S52

# handle error address as a special case
R<@>				$n			errors to mailer-daemon

# pass everything to standard SMTP mailer rewriting
R$*				$@ $>11 $1

#
#  envelope sender rewriting for uucp-uudom mailer
#
S72

# handle error address as a special case
R<@>				$n			errors to mailer-daemon

# do not qualify list:; syntax
R$* :; <@>			$@ $1 :;

R$* < @ $* . >			$1 < @ $2 >		strip trailing dots
R$* < @ $=w >			$1			strip local name
R<@ $- . UUCP > : $+		$1 ! $2			convert to UUCP format
R<@ $+ > : $+			$1 ! $2			convert to UUCP format
R$* < @ $- . UUCP >		$2 ! $1			convert to UUCP format
R$* < @ $+ >			$@ $2 ! $1		convert to UUCP format

R$&h ! $+ ! $+			$@ $1 ! $2		$h!...!user => ...!user
R$&h ! $+			$@ $&h ! $1		$h!user => $h!user
R$+				$: $M ! $1		prepend masquerade name
R! $+				$: $j ! $1		in case $M undefined')


PUSHDIVERT(4)
# resolve locally connected UUCP links
R$* < @ $=Z . UUCP. > $*	$#uucp-uudom $@ $2 $: $1 < @ $2 .UUCP. > $3
R$* < @ $=Y . UUCP. > $*	$#uucp-new $@ $2 $: $1 < @ $2 .UUCP. > $3
R$* < @ $=U . UUCP. > $*	$#uucp-old $@ $2 $: $1 < @ $2 .UUCP. > $3
POPDIVERT
