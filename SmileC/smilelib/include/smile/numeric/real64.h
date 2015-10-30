#ifndef __SMILE_NUMERIC_REAL64_H__
#define __SMILE_NUMERIC_REAL64_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

SMILE_API Real64 Real64_Zero;
SMILE_API Real64 Real64_One;

SMILE_API Real64 Real64_FromInt32(Int32 int32);
SMILE_API Real64 Real64_FromInt64(Int64 int64);
SMILE_API Real32 Real64_ToReal32(Real64 real64);
SMILE_API Real128 Real64_ToReal128(Real64 real64);

#endif
