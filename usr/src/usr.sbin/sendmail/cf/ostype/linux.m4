divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#

divert(0)
VERSIONID(`@(#)linux.m4	8.1 (Berkeley) %G%')
define(`LOCAL_MAILER_PATH', /usr/bin/lmail)dnl
define(`LOCAL_MAILER_ARGS', mail $u)dnl
define(`LOCAL_MAILER_FLAGS', `rm')dnl
