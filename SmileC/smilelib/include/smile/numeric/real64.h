#ifndef __SMILE_NUMERIC_REAL64_H__
#define __SMILE_NUMERIC_REAL64_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

SMILE_API Real64 Real64_Zero;
SMILE_API Real64 Real64_One;
SMILE_API Real64 Real64_NegOne;

SMILE_API Real64 Real64_FromInt32(Int32 int32);
SMILE_API Real64 Real64_FromInt64(Int64 int64);
SMILE_API Real64 Real64_FromFloat32(Float32 float32);
SMILE_API Real64 Real64_FromFloat64(Float64 float64);

SMILE_API Real32 Real64_ToReal32(Real64 real64);
SMILE_API Real128 Real64_ToReal128(Real64 real64);
SMILE_API Float32 Real64_ToFloat32(Real64 real64);
SMILE_API Float64 Real64_ToFloat64(Real64 real64);

SMILE_API Real64 Real64_Add(Real64 a, Real64 b);
SMILE_API Real64 Real64_Sub(Real64 a, Real64 b);
SMILE_API Real64 Real64_Mul(Real64 a, Real64 b);
SMILE_API Real64 Real64_Div(Real64 a, Real64 b);
SMILE_API Real64 Real64_Rem(Real64 a, Real64 b);

SMILE_API Real64 Real64_Neg(Real64 real64);
SMILE_API Real64 Real64_Abs(Real64 real64);

#endif
