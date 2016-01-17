#!/bin/sh

VCXPROJ=

while [ "$1" != "" ]; do
case $1 in
	-h | --help )	printf "Usage: makesrc.sh Project.vcxproj"
			exit 1
			;;
	* )		VCXPROJ=$1
esac
shift
done

printf '# This file is generated.  Do not edit!'

printf '\nSRCS = \\\n'

PLATFORM=`uname -s`
if [ $PLATFORM == Darwin ]
then
	grep '<ClCompile Include' $VCXPROJ \
		| sed -E -e 's/^[[:space:]]*<ClCompile Include=\"//; s/\"[[:space:]]*\/*>[[:space:]]*$//; s/\\/\//g; s/^(.*)$/    \1 \\/; $s/\\//;' \
		| sort
else
	grep '<ClCompile Include' $VCXPROJ \
		| sed -e 's/^\s*<ClCompile Include=\"//; s/\"\s*\/\?>\s*$//; s/\\/\//g; s/^\(.*\)$/\t\1 \\/; $s/\\//;' \
		| sort
fi
printf '\n# End of generated file.\n'

