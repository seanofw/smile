#!/bin/sh

RAWMACHINE=`uname -m`
LOWERMACHINE=`echo "$RAWMACHINE" | sed -e 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/;'`

PROC='Unknown'

if [[ $LOWERMACHINE =~ ^[ix][3456]?86[-_]?64 ]]; then
	PROC='x64'
elif [[ $LOWERMACHINE =~ ^[ix]64 ]]; then
	PROC='x64'
elif [[ $LOWERMACHINE =~ ^amd64 ]]; then
	PROC='x64'
elif [[ $LOWERMACHINE =~ ^[ix][3456]?86 ]]; then
	PROC='x86'
elif [[ $LOWERMACHINE =~ ^ia64 ]]; then
	PROC='IA64'
elif [[ $LOWERMACHINE =~ ^armv?6 ]]; then
	PROC='ARM6'
elif [[ $LOWERMACHINE =~ ^armv?7 ]]; then
	PROC='ARM7'
elif [[ $LOWERMACHINE =~ ^armv?8 ]]; then
	PROC='ARM8'
elif [[ $LOWERMACHINE =~ ^arm ]]; then
	PROC='ARM'
elif [[ $LOWERMACHINE =~ ^sparc64 ]]; then
	PROC='SPARC64'
elif [[ $LOWERMACHINE =~ ^sparc ]]; then
	PROC='SPARC'
elif [[ $LOWERMACHINE =~ ^sun4u ]]; then
	PROC='SPARC'
elif [[ $LOWERMACHINE =~ ^p(ower)?pc64 ]]; then
	PROC='PowerPC64'
elif [[ $LOWERMACHINE =~ ^p(ower)?pc ]]; then
	PROC='PowerPC'
elif [[ $LOWERMACHINE =~ ^mips ]]; then
	PROC='MIPS'
fi

echo $PROC

