#ifndef __SMILE_NUMERIC_REAL32_H__
#define __SMILE_NUMERIC_REAL32_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

SMILE_API Real32 Real32_Zero;
SMILE_API Real32 Real32_One;

SMILE_API Real32 Real32_FromInt32(Int32 int32);
SMILE_API Real32 Real32_FromInt64(Int64 int64);
SMILE_API Real64 Real32_ToReal64(Real32 real32);
SMILE_API Real128 Real32_ToReal128(Real32 real32);

#endif
