divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#

divert(0)
VERSIONID(`@(#)redirect.m4	8.3 (Berkeley) %G%')
divert(-1)


PUSHDIVERT(3)
# addresses sent to foo@host.REDIRECT will give a 551 error code
R$* < @ $+ .REDIRECT. >	$# error $@ 5.1.1 $: "551 User not local; please try " <$1@$2>
POPDIVERT

PUSHDIVERT(6)
CPREDIRECT
POPDIVERT
