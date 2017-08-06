#!/bin/sh

VCXPROJ=

while [ "$1" != "" ]; do
case $1 in
	-h | --help )	printf "Usage: getsrc.sh Project.vcxproj"
			exit 1
			;;
	* )		VCXPROJ=$1
esac
shift
done

PLATFORM=`uname -s`
if [ $PLATFORM == Darwin ]
then
	SED='sed -E'
else
	SED='sed -r'
fi

printf '# This file is generated.  Do not edit!'

printf '\nSRCS = \\\n'

grep '<ClCompile Include' $VCXPROJ \
	| $SED -e 's/^[[:space:]]*<ClCompile Include=\"//; s/\"[[:space:]]*\/*>[[:space:]]*$//; s/\\/\//g; s/^(.*)$/    \1 \\/; $s/\\//;' \
	| sort
printf '\n# End of generated file.\n'

