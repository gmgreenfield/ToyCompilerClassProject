#!/bin/sh
#
#	Usage:	tc basename
#
#	e.g. "tc tst"  to compile tst.ty, 
#
#	First make sure source file exists
#
if [ ! -f "$1.ty" ] 
then
	echo "Cannot open \"$1.ty\""
	exit 1
fi
#
./toy $1 <$1.ty 
