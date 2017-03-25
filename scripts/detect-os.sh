#!/bin/sh

NAME=`uname -s`

OS='Unknown'

if echo "$NAME" | egrep -q -i '^linux'; then
	OS='Linux'
elif echo "$NAME" | egrep -q -i '^cygwin'; then
	OS='Cygwin'
elif echo "$NAME" | egrep -q -i '^mingw'; then
	OS='MinGW'
elif echo "$NAME" | egrep -q -i '^qnx'; then
	OS='QNX'
elif echo "$NAME" | egrep -q -i '^darwin'; then
	OS='Darwin'
elif echo "$NAME" | egrep -q -i '^freebsd'; then
	OS='FreeBSD'
elif echo "$NAME" | egrep -q -i '^gnu'; then
	OS='GNU'
elif echo "$NAME" | egrep -q -i '^hp-ux'; then
	OS='HPUX'
elif echo "$NAME" | egrep -q -i '^aix'; then
	OS='AIX'
elif echo "$NAME" | egrep -q -i '^irix'; then
	OS='IRIX'
elif echo "$NAME" | egrep -q -i '^sunos'; then
	OS='SunOS'
elif echo "$NAME" | egrep -q -i '^ultrix'; then
	OS='Ultrix'
elif echo "$NAME" | egrep -q -i '^bsd'; then
	OS='BSD'
else
	OS='Unknown'
fi

echo $OS

