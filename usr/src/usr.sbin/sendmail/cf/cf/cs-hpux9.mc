divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#

#
#  This is a Berkeley-specific configuration file for HP-UX 9.x.
#  It applies only the the Computer Science Division at Berkeley,
#  and should not be used elsewhere.   It is provided on the sendmail
#  distribution as a sample only.  To create your own configuration
#  file, create an appropriate domain file in ../domain, change the
#  `DOMAIN' macro below to reference that file, and copy the result
#  to a name of your own choosing.
#

include(`../m4/cf.m4')
VERSIONID(`@(#)cs-hpux9.mc	8.3 (Berkeley) %G%')
OSTYPE(hpux9)dnl
DOMAIN(CS.Berkeley.EDU)dnl
define(`MAIL_HUB', mailspool.CS.Berkeley.EDU)dnl
MAILER(local)dnl
MAILER(smtp)dnl
