divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988 The Regents of the University of California.
# All rights reserved.
#
# %sccs.include.redist.sh%
#

include(`../m4/cf.m4')
VERSIONID(@(#)mail.eecs.mc	2.2 (Berkeley) %G%)
OSTYPE(hpux)dnl
DOMAIN(cs.exposed)dnl
MAILER(local)dnl
MAILER(smtp)dnl
define(`NEWSENDMAIL')dnl
DDBerkeley.EDU

# hosts for which we accept and forward mail (must be in .Berkeley.EDU)
CF CS ucbarpa arpa ucbernie ernie renoir

LOCAL_RULE_0
R< @ $=F . $D . > : $*		$@ $>7 $1		@here:... -> ...
R$* $=O $* < @ $=F . $D . >	$@ $>7 $1 $2 $3		...@here -> ...

R$* < @ $=F . $D . >		$#local $: $1		use UDB
