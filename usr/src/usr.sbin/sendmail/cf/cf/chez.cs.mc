divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#

include(`../m4/cf.m4')
VERSIONID(`@(#)chez.cs.mc	8.3 (Berkeley) %G%')
OSTYPE(bsd4.4)dnl
DOMAIN(cs.exposed)dnl
define(`LOCAL_RELAY', vangogh.CS.Berkeley.EDU)dnl
define(`MASQUERADE_NAME', vangogh.CS.Berkeley.EDU)dnl
FEATURE(use_cw_file)dnl
MAILER(local)dnl
MAILER(smtp)dnl
