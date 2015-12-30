#!/bin/sh

VCXPROJ=SmileLib.vcxproj

ALLSRC=`mktemp`

grep '<ClCompile Include' $VCXPROJ \
	| sed -e 's/^\s*<ClCompile Include=\"//; s/\"\s*\/\?>\s*$//; s/\\/\//g' \
	| sort \
	> $ALLSRC

printf '# This file is generated.  Do not edit!'

printf '\nGC_SRCS = \\\n'

egrep '^gc\/' $ALLSRC \
	| sed -e 's/^\(.*\)$/\t\1 \\/; $s/\\//;'

printf '\nDECIMAL_SRCS = \\\n'

egrep '^decimal\/' $ALLSRC \
	| sed -e 's/^\(.*\)$/\t\1 \\/; $s/\\//;'

printf '\nSMILE_SRCS = \\\n'

egrep '^src\/' $ALLSRC \
	| sed -e 's/^\(.*\)$/\t\1 \\/; $s/\\//;'

printf '\n# End of generated file.\n'

rm -f $ALLSRC

