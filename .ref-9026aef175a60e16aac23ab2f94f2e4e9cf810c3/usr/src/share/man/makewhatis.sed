#!/usr/bin/sed -nf
#
# Copyright (c) 1988, 1993, 1994
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#
#	@(#)makewhatis.sed	8.4 (Berkeley) %G%
#

/^[a-zA-Z][a-zA-Z0-9\._+\-]*(\([a-zA-Z0-9\._+\-]*\).*/ {
	s;^[a-zA-Z0-9\._+\-]*(\([a-zA-Z0-9\._+\-]*\).*;\1;
	h
	d
}

/^NNAAMMEE/!d

:name
	s;.*;;
	N
	s;\n;;
	# some twits underline the command name
	s;_;;g
	/^[^	 ]/b print
	H
	b name

:print
	x
	s;\n;;g
	/-/!d
	s;.;;g
	s;\([a-z][A-z]\)-[	 ][	 ]*;\1;
	s;\([a-zA-Z0-9,\._+\-]\)[	 ][	 ]*;\1 ;g
	s;[^a-zA-Z0-9\._+\-]*\([a-zA-Z0-9\._+\-]*\)[^a-zA-Z0-9\._+\-]*\(.*\) - \(.*\);\2 (\1) - \3;
	p
	q
