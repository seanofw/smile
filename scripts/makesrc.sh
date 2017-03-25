#!/bin/sh

VCXPROJ=SmileLib.vcxproj

ALLSRC=`mktemp`

PLATFORM=`uname -s`
if [ $PLATFORM == Darwin ]
then
	SED='sed -E'
else
	SED='sed -r'
fi

grep '<ClCompile Include' $VCXPROJ \
	| $SED -e 's/^[[:space:]]*<ClCompile Include=\"//; s/\"[[:space:]]*\/*>[[:space:]]*$//; s/\\/\//g' \
	| sort \
	> $ALLSRC

printf '# This file is generated.  Do not edit!'

printf '\nGC_SRCS = \\\n'

egrep '^gc\/' $ALLSRC \
	| $SED -e 's/^(.*)$/    \1 \\/; $s/\\//;'

printf '\nDECIMAL_SRCS = \\\n'

egrep '^decimal\/' $ALLSRC \
	| $SED -e 's/^(.*)$/    \1 \\/; $s/\\//;'

printf '\nSMILE_SRCS = \\\n'

egrep '^src\/' $ALLSRC \
	| $SED -e 's/^(.*)$/    \1 \\/; $s/\\//;'

printf '\n# End of generated file.\n'

rm -f $ALLSRC

