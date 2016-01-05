#!/bin/sh

MACHINE=`uname -m`

PROC='Unknown'

if echo "$MACHINE" | egrep -q -i '^[ix][3456]?86[-_]?64'; then
	PROC='x64'
elif echo "$MACHINE" | egrep -q -i '^[ix]64'; then
	PROC='x64'
elif echo "$MACHINE" | egrep -q -i '^amd64'; then
	PROC='x64'
elif echo "$MACHINE" | egrep -q -i '^[ix][3456]?86'; then
	PROC='x86'
elif echo "$MACHINE" | egrep -q -i '^ia64'; then
	PROC='IA64'
elif echo "$MACHINE" | egrep -q -i '^armv?6'; then
	PROC='ARM6'
elif echo "$MACHINE" | egrep -q -i '^armv?7'; then
	PROC='ARM7'
elif echo "$MACHINE" | egrep -q -i '^armv?8'; then
	PROC='ARM8'
elif echo "$MACHINE" | egrep -q -i '^arm'; then
	PROC='ARM'
elif echo "$MACHINE" | egrep -q -i '^sparc64'; then
	PROC='SPARC64'
elif echo "$MACHINE" | egrep -q -i '^sparc'; then
	PROC='SPARC'
elif echo "$MACHINE" | egrep -q -i '^sun4u'; then
	PROC='SPARC'
elif echo "$MACHINE" | egrep -q -i '^p(ower)?pc64'; then
	PROC='PPC64'
elif echo "$MACHINE" | egrep -q -i '^p(ower)?pc'; then
	PROC='PPC'
elif echo "$MACHINE" | egrep -q -i '^mips'; then
	PROC='MIPS'
fi

echo $PROC

