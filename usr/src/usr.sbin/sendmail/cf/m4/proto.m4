divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988 The Regents of the University of California.
# All rights reserved.
#
# %sccs.include.redist.sh%
#
divert(0)

VERSIONID(@(#)proto.m4	2.19 (Berkeley) %G%)

MAILER(local)dnl

##################
#   local info   #
##################

Cwlocalhost
ifdef(`USE_CW_FILE',
`# file containing names of hosts for which we receive email
Fw/etc/sendmail.cw', `dnl')

ifdef(`UUCP_RELAY',
`# UUCP relay host
CONCAT(DY, UUCP_RELAY)
')dnl
ifdef(`BITNET_RELAY',
`#  BITNET relay host
CONCAT(DB, BITNET_RELAY)
')dnl
ifdef(`CSNET_RELAY',
`# CSNET relay host
CONCAT(DC, CSNET_RELAY)
')dnl
# my official hostname ($w or $w.$D)
CONCAT(Dj$w, ifdef(`NEED_DOMAIN', .$D))

# who I masquerade as (can be $j)
CONCAT(DM, ifdef(`MASQUERADE_NAME', MASQUERADE_NAME, $j))

# who I send unqualified names to (null means deliver locally)
CONCAT(DR, ifdef(`LOCAL_RELAY', LOCAL_RELAY))

# names that should be delivered locally, even if we have a relay
CLroot
undivert(5)dnl
undivert(6)dnl

ifdef(`UUCP_NAME',
`# uucp hostnames
UUCP_NAME
UUCP_ALIASES

# local UUCP connections
include(UUCP_HOSTS_FILE)',
`dnl')

# operators that cannot be in local usernames (i.e., network indicators)
CO @ % !

# a class with just dot (for identifying canonical names)
C..

# list of locations of user database file (null means no lookup)
OU`'ifdef(`USERDB_SPEC', `USERDB_SPEC')

# set if we can guarantee no wildcard MX records matching our domain
ifdef(`_NO_WILDCARD_MX_', `', `#')Ow

ifdef(`_OLD_SENDMAIL_', `dnl',
`# level 2 config file format
O=2')

include(`../m4/version.m4')

ifdef(`INTERNET_RELAY',
`include(../sitedep/nicregistered.m4)',
`dnl')
include(`../m4/boilerplate.m4')
#
######################################################################
######################################################################
#####
#####			REWRITING RULES
#####
######################################################################
######################################################################


###########################################
###  Rulset 3 -- Name Canonicalization  ###
###########################################
S3

# handle "from:<>" special case
R$*<>$*			$@@				turn into magic token

# basic textual canonicalization -- note RFC733 heuristic here
R$*<$*<$*<$+>$*>$*>$*	$4				3-level <> nesting
R$*<$*<$+>$*>$*		$3				2-level <> nesting
R$*<$+>$*		$2				basic RFC821/822 parsing

# make sure <@a,@b,@c:user@d> syntax is easy to parse -- undone later
R@$+,$+			@$1:$2				change all "," to ":"

# localize and dispose of route-based addresses
R@$+:$+			$@$>6<@$1>:$2			handle <route-addr>

# find focus for list syntax
R$+:$*;@$+		$@$>6$1:$2;<@$3>		list syntax
R$+:$*;			$@$1:$2;			list syntax

# find focus for @ syntax addresses
R$+@$+			$:$1<@$2>			focus on domain
R$+<$+@$+>		$1$2<@$3>			move gaze right
R$+<@$+>		$@$>6$1<@$2>			already canonical

# convert old-style addresses to a domain-based address
R$-!$+			$@$>6$2<@$1.UUCP>		resolve uucp names
R$+.$-!$+		$@$>6$3<@$1.$2>			domain uucps
R$+!$+			$@$>6$2<@$1.UUCP>		uucp subdomains

# if we have % signs, take the rightmost one
R$*%$*			$1@$2				First make them all @s.
R$*@$*@$*		$1%$2@$3			Undo all but the last.
R$*@$*			$@$>6$1<@$2>			Insert < > and finish

# else we must be a local name


###############################################
###  Ruleset 6 -- bottom half of ruleset 3  ###
###############################################

#  At this point, everything should be in a local_part@domain format.

S6

# handle special cases for local names
R$* < @ $=w > $*		$: $1 < @ $j . > $3		no domain at all
R$* < @ $=w .UUCP> $*		$: $1 < @ $j . > $3		.UUCP domain
undivert(2)dnl

ifdef(`UUCP_RELAY',
`# pass UUCP addresses straight through
R$* < @ $+ . UUCP > $*		$@ $1 < @ $2 .UUCP > $3',
`# if really UUCP, handle it immediately
ifdef(`_CLASS_U_',
`R$* < @ $=U . UUCP > $*		$@ $1 < @ $2 .UUCP > $3', `dnl')
ifdef(`_CLASS_V_',
`R$* < @ $=V . UUCP > $*		$@ $1 < @ $2 .UUCP > $3', `dnl')
ifdef(`_CLASS_W_',
`R$* < @ $=W . UUCP > $*		$@ $1 < @ $2 .UUCP > $3', `dnl')
ifdef(`_CLASS_X_',
`R$* < @ $=X . UUCP > $*		$@ $1 < @ $2 .UUCP > $3', `dnl')
ifdef(`_CLASS_Y_',
`R$* < @ $=Y . UUCP > $*		$@ $1 < @ $2 .UUCP > $3', `dnl')

# try UUCP traffic as a local address
R$* < @ $+ .UUCP > $*			$: $1 < @ $[ $2 $] .UUCP > $3
ifdef(`_OLD_SENDMAIL_',
`R$* < @ $+ . $+ .UUCP > $*		$@ $1 < @ $2 . $3 > $4',
`R$* < @ $+ . .UUCP > $*		$@ $1 < @ $2 . > $3')')

# pass to name server to make hostname canonical
R$* < @ $* $~. > $*		$: $1 < @ $[ $2 $3 $] > $4

# handle possible alternate names
R$* < @ $=w . $m . > $*		$: $1 < @ $j . > $3
R$* < @ $=w . $m > $*		$: $1 < @ $j . > $3
undivert(8)dnl

# if this is the local hostname, make sure we treat is as canonical
R$* < @ $j > $*			$: $1 < @ $j . > $2


##################################################
###  Ruleset 4 -- Final Output Post-rewriting  ###
##################################################
S4

R@			$@				handle <> error addr

# resolve numeric addresses to name if possible
R$* < @ [ $+ ] > $*	$: $1 < @ $[ [$2] $] > $3	lookup numeric internet addr

# strip trailing dot off possibly canonical name
R$* < @ $+ . > $*	$1 < @ $2 > $3

# externalize local domain info
R$* < $+ > $*		$1 $2 $3			defocus
R@ $+ : @ $+ : $+	@ $1 , @ $2 : $3		<route-addr> canonical

# UUCP must always be presented in old form
R$+ @ $- . UUCP		$2!$1				u@h.UUCP => h!u

# delete duplicate local names
R$+ % $=w @ $=w		$1 @ $j				u%host@host => u@host



#############################################################
###   Ruleset 7 -- recanonicalize and call ruleset zero   ###
###		   (used for recursive calls)		  ###
#############################################################

S7
R$*			$: $>3 $1
R$*			$@ $>0 $1


######################################
###   Ruleset 0 -- Parse Address   ###
######################################

S0

# handle numeric address spec
R$* < @ [ $+ ] > $*	$: $1 < @ $[ [$2] $] > $3	numeric internet addr
R$* < @ [ $+ ] > $*	$#smtp $@ [$2] $: $1 @ [$2] $3	numeric internet spec

#R@			$#error$:Invalid address	handle <> form

# now delete the local info -- note $=O to find characters that cause forwarding
R< @ $j . > : $*	$@ $>7 $1			@here:... -> ...
R$* $=O $* < @ $j . >	$@ $>7 $1 $2 $3			...@here -> ...

# short circuit local delivery so forwarded email works
ifdef(`_OLD_SENDMAIL_',
`R$+ < @ $j . >		$#local $: $1			local address',
`R$+ < @ $j . >		$#local $: @ $1			local address')
undivert(3)dnl
undivert(4)dnl

# resolve remotely connected UUCP links
ifdef(`_CLASS_V_',
`R$* < @ $=V . UUCP > $*		$#smtp $@ $V $: $1<@$2.UUCP>$3', `dnl')
ifdef(`_CLASS_W_',
`R$* < @ $=W . UUCP > $*		$#smtp $@ $W $: $1<@$2.UUCP>$3', `dnl')
ifdef(`_CLASS_X_',
`R$* < @ $=X . UUCP > $*		$#smtp $@ $X $: $1<@$2.UUCP>$3', `dnl')

# resolve fake top level domains by forwarding to other hosts
ifdef(`BITNET_RELAY',
`R$*<@$+.BITNET>$*	$#smtp$@$B$:$1<@$2.BITNET>$3		user@host.BITNET',
	`dnl')
ifdef(`CSNET_RELAY',
`R$*<@$+.CSNET>$*	$#smtp$@$C$:$1<@$2.CSNET>$3		user@host.CSNET',
	`dnl')

ifdef(`UUCP_RELAY',
`# forward non-local UUCP traffic to our UUCP relay
R$*<@$*.UUCP>$*		$#smtp$@$Y$:$1<@$2.UUCP>		uucp mail',
`ifdef(`_UUCP_LINKED_',
`# forward other UUCP traffic straight to UUCP
R< @ $+ .UUCP > : $+	$#uucp $@ $1 $: $1:$2			@host.UUCP:...
R$+ < @ $+ .UUCP >	$#uucp $@ $2 $: $1			user@host.UUCP',
	`dnl')')

# deal with other remote names
R$* < @ $* > $*		$# smtp $@ $2 $: $1 < @ $2 > $3		user@host.domain

ifdef(`_OLD_SENDMAIL_',
`# forward remaining names to local relay, if any
R$+			$: $1 < @ $R >
R$+ < @ >		$# local $: $1
R$+ < @ $+ >		$# smtp $@ $2 $: $1',
`# handle locally delivered names
R$=L			$# local $: @ $1		special local names
R$+			$# local $: $1			regular local names')

ifdef(`_OLD_SENDMAIL_', `dnl',
`#
# special rewriting after aliases have been expanded
#

S5

R$+			$: $1 < @ $R >
R$+ < @ $+ >		$# smtp $@ $2 $: $1		send to relay')
#
######################################################################
######################################################################
#####
#####			MAILER DEFINITIONS
#####
######################################################################
######################################################################
undivert(7)dnl
