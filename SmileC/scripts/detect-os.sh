#!/bin/sh

RAWNAME=`uname -s`
LOWERNAME=`echo "$RAWNAME" | sed -e 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/;'`

OS='Unknown'

if [[ $LOWERNAME =~ ^linux ]]; then
	OS='Linux'
elif [[ $LOWERNAME =~ ^cygwin ]]; then
	OS='Cygwin'
elif [[ $LOWERNAME =~ ^mingw ]]; then
	OS='MinGW'
elif [[ $LOWERNAME =~ ^qnx ]]; then
	OS='QNX'
elif [[ $LOWERNAME =~ ^darwin ]]; then
	OS='Darwin'
elif [[ $LOWERNAME =~ ^freebsd ]]; then
	OS='FreeBSD'
elif [[ $LOWERNAME =~ ^gnu ]]; then
	OS='GNU'
elif [[ $LOWERNAME =~ ^hp-ux ]]; then
	OS='HPUX'
elif [[ $LOWERNAME =~ ^aix ]]; then
	OS='AIX'
elif [[ $LOWERNAME =~ ^irix ]]; then
	OS='IRIX'
elif [[ $LOWERNAME =~ ^sunos ]]; then
	OS='SunOS'
elif [[ $LOWERNAME =~ ^ultrix ]]; then
	OS='Ultrix'
elif [[ $LOWERNAME =~ ^bsd ]]; then
	OS='BSD'
else
	OS='Unknown'
fi

echo $OS

