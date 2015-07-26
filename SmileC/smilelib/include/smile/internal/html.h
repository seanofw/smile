#ifndef __SMILE_INTERNAL_HTML_H__
#define __SMILE_INTERNAL_HTML_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

typedef struct {
	struct StringInt string;
	UInt value;
} HtmlEntity;

extern const HtmlEntity HtmlEntityTable[];
extern const Int HtmlEntityTableLength;

String HtmlEntityValueToName(Int32 value);
Int32 HtmlEntityNameToValue(String name);

#endif