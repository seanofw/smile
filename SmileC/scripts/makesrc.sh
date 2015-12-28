#!/usr/bin/sh

VCXPROJ=SmileLib.vcxproj

grep '<ClCompile Include' $VCXPROJ \
	| sed -e 's/^\s*<ClCompile Include=\"//; s/\"\s*\/\?>\s*$//; s/\\/\//g' \
	| sort \
	> Makefile.allsrc

echo -e "# This file is generated.  Do not edit!"

echo -e "\nGC_SRCS = \\"

egrep '^gc\/' Makefile.allsrc \
	| sed -e 's/^\(.*\)$/\t\1 \\/; $s/\\//;'

echo -e "\nDECIMAL_SRCS = \\"

egrep '^decimal\/' Makefile.allsrc \
	| sed -e 's/^\(.*\)$/\t\1 \\/; $s/\\//;'

echo -e "\nSMILE_SRCS = \\"

egrep '^src\/' Makefile.allsrc \
	| sed -e 's/^\(.*\)$/\t\1 \\/; $s/\\//;'

echo -e "\n# End of generated file."

