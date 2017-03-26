#ifndef __SMILE_INTERNAL_HTML_H__
#define __SMILE_INTERNAL_HTML_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

typedef struct {
	String string;
	UInt value;
} HtmlEntity;

SMILE_INTERNAL_DATA extern const HtmlEntity HtmlEntityTable[];
SMILE_INTERNAL_DATA extern const Int HtmlEntityTableLength;

SMILE_INTERNAL_FUNC String HtmlEntityValueToName(Int32 value);
SMILE_INTERNAL_FUNC Int32 HtmlEntityNameToValue(const Byte *src, Int length);

#endif