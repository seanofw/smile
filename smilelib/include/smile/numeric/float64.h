#ifndef __SMILE_NUMERIC_FLOAT64_H__
#define __SMILE_NUMERIC_FLOAT64_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#define FLOAT_KIND_POS_ZERO		0
#define FLOAT_KIND_NEG_ZERO		(0x80 | FLOAT_KIND_POS_ZERO)
#define FLOAT_KIND_POS_NUM		1
#define FLOAT_KIND_NEG_NUM		(0x80 | FLOAT_KIND_POS_NUM)
#define FLOAT_KIND_POS_INF		2
#define FLOAT_KIND_NEG_INF		(0x80 | FLOAT_KIND_POS_INF)
#define FLOAT_KIND_POS_QNAN		3
#define FLOAT_KIND_NEG_QNAN		(0x80 | FLOAT_KIND_POS_QNAN)
#define FLOAT_KIND_POS_SNAN		4
#define FLOAT_KIND_NEG_SNAN		(0x80 | FLOAT_KIND_POS_SNAN)

SMILE_API_FUNC String Float64_ToFixedString(Float64 float64, Int minIntDigits, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC String Float64_ToExpString(Float64 float64, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC String Float64_ToStringEx(Float64 float64, Int minIntDigits, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC Int32 Float64_Decompose(Byte *str, Int32 *exp, Int32 *kind, Float64 float64);

#endif