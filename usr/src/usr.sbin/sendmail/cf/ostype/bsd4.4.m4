divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988 The Regents of the University of California.
# All rights reserved.
#
# %sccs.include.redist.sh%
#
VERSIONID(@(#)bsd4.4.m4	2.1 (Berkeley) %G%)
#

define(`ALIAS_FILE', /etc/aliases)dnl
define(`HELP_FILE', /usr/share/misc/sendmail.hf)dnl
define(`QUEUE_DIR', /var/spool/mqueue)dnl
define(`STATUS_FILE', /var/log/sendmail.st)dnl
define(`LOCAL_MAILER', /usr/libexec/mail.local)dnl

divert(0)
