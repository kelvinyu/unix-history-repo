#!/bin/sh

# $FreeBSD$

cd `dirname $0`
. ../../common.sh

# Description
DESC="Variable expansion with M modifier"

eval_cmd $*
