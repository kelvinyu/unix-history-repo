#!/bin/sh
#	dumpall.sh	1.1	%G%
#	shell script to do all pending dumps
#	Asks for confirmation before proceeding
list=`dump w|sed -e '/^ /!d
	/^ /s/^  //
	s/	.*$//'`
for fi in $list
do
	echo "File system to dump is $fi"
	echo -n "Do you wish to continue"
	read ans
	if [ "$ans" = "yes" -o "$ans" = "y" -o "$ans" = "Y" ]
	then
		doadump $fi
	else
		exit 1
	fi
done
