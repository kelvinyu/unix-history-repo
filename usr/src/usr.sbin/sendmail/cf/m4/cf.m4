divert(0)dnl
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988 The Regents of the University of California.
# All rights reserved.
#
# %sccs.include.redist.sh%
#


######################################################################
######################################################################
#####
#####		SENDMAIL CONFIGURATION FILE
#####
define(`TEMPFILE', maketemp(/tmp/cfXXXXXX))dnl
syscmd(sh ../sh/makeinfo.sh > TEMPFILE)dnl
include(TEMPFILE)dnl
syscmd(rm -f TEMPFILE)dnl
#####
######################################################################
######################################################################

divert(-1)

changecom()
ifdef(`pushdef', `',
	`errprint(`You need a newer version of M4, at least as new as
System V or GNU')
	include(NoSuchFile)')
define(`PUSHDIVERT', `pushdef(`__D__', divnum)divert($1)')
define(`POPDIVERT', `divert(__D__)popdef(`__D__')')
define(`OSTYPE', `include(../ostype/$1.m4)')
define(`MAILER',
`ifdef(`_MAILER_$1_', `dnl`'',
`define(`_MAILER_$1_', `')PUSHDIVERT(7)include(../mailer/$1.m4)POPDIVERT`'')')
define(`DOMAIN', `include(../domain/$1.m4)')
define(`FEATURE', `include(../feature/$1.m4)')
define(`HACK', `include(../hack/$1.m4)')
define(`OLDSENDMAIL', `define(`_OLD_SENDMAIL_', `')')
define(`VERSIONID', ``#####  $1  #####'')
define(`LOCAL_RULE_3', `divert(2)')
define(`LOCAL_RULE_0', `divert(3)')
define(`LOCAL_CONFIG', `divert(6)')
define(`LOCAL_NET_CONFIG', `define(`_LOCAL_RULES_', 1)divert(1)')
define(`UUCPSMTP', `R DOL(*) < @ $1 .UUCP > DOL(*)	DOL(1) < @ $2 > DOL(2)')
define(`CONCAT', `$1$2$3$4$5$6$7')
define(`DOL', ``$'$1')
define(`SITECONFIG',
`CONCAT(D, $3, $2)
define(`_CLASS_$3_', `')dnl
ifelse($3, U, Cw$2, `dnl')
define(`SITE', `ifelse(CONCAT($'2`, $3), SU,
		CONCAT(CY, $'1`),
		CONCAT(C, $3, $'1`))')
sinclude(../siteconfig/$1.m4)')
define(`EXPOSED_USER', `PUSHDIVERT(5)CE$1
POPDIVERT`'dnl')
define(`LOCAL_USER', `PUSHDIVERT(5)CL$1
POPDIVERT`'dnl')
define(`MASQUERADE_AS', `define(`MASQUERADE_NAME', $1)')

m4wrap(`include(`../m4/proto.m4')')

# define default values for options
define(`confMAILER_NAME', ``MAILER-DAEMON'')
define(`confFROM_LINE', `From $?<$<$|$g$.  $d')
define(`confOPERATORS', `.:%@!^/[]')
define(`confSMTP_LOGIN_MSG', `$j Sendmail $v/$Z ready at $b')
define(`confEIGHT_BIT_INPUT', `False')
define(`confALIAS_WAIT', `10')
define(`confMIN_FREE_BLOCKS', `4')
define(`confBLANK_SUB', `.')
define(`confCON_EXPENSIVE', `False')
define(`confCHECKPOINT_INTERVAL', `10')
define(`confDELIVERY_MODE', `background')
define(`confAUTO_REBUILD', `False')
define(`confSAVE_FROM_LINES', `False')
define(`confTEMP_FILE_MODE', `0600')
define(`confMATCH_GECOS', `False')
define(`confDEF_GROUP_ID', `1')
define(`confMAX_HOP', `17')
define(`confIGNORE_DOTS', `False')
define(`confBIND_OPTS', `')
define(`confMCI_CACHE_SIZE', `2')
define(`confMCI_CACHE_TIMEOUT', `5m')
define(`confLOG_LEVEL', `9')
define(`confME_TOO', `False')
define(`confCHECK_ALIASES', `True')
define(`confOLD_STYLE_HEADERS', `True')
define(`confPRIVACY_FLAGS', `public')
define(`confSAFE_QUEUE', `True')
define(`confMESSAGE_TIMEOUT', `5d')
define(`confTIME_ZONE', `USE_SYSTEM')
define(`confDEF_USER_ID', `1')
define(`confQUEUE_LA', `8')
define(`confREFUSE_LA', `12')
define(`confSEPARATE_PROC', `False')

divert(0)dnl
VERSIONID(`@(#)cf.m4	6.6 (Berkeley) %G%')
