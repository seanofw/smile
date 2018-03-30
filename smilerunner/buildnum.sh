#!/bin/sh

/usr/bin/printf \
"// THIS IS A GENERATED FILE.  DO NOT EDIT!\r\n\
\r\n\
#define BUILD_STR_HELPER(x) #x\r\n\
#define BUILD_STR(x) BUILD_STR_HELPER(x)\r\n\
\r\n\
#define BUILDNUM %d\r\n\
#define BUILDDATE \"%s\"\r\n\
#define BUILDSTRING (\"r\" BUILD_STR(BUILDNUM) \" / \" BUILDDATE)\r\n\
" \
`git rev-list --count HEAD` \
`date -I` > buildnum.h

