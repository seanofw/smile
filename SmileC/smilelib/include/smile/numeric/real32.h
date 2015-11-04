#ifndef __SMILE_NUMERIC_REAL32_H__
#define __SMILE_NUMERIC_REAL32_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

SMILE_API Real32 Real32_Zero;
SMILE_API Real32 Real32_One;
SMILE_API Real32 Real32_NegOne;

SMILE_API Real32 Real32_FromInt32(Int32 int32);
SMILE_API Real32 Real32_FromInt64(Int64 int64);
SMILE_API Real32 Real32_FromFloat32(Float32 float32);
SMILE_API Real32 Real32_FromFloat64(Float64 float64);

SMILE_API Real64 Real32_ToReal64(Real32 real32);
SMILE_API Real128 Real32_ToReal128(Real32 real32);
SMILE_API Float32 Real32_ToFloat32(Real32 real32);
SMILE_API Float64 Real32_ToFloat64(Real32 real32);

SMILE_API Real32 Real32_Add(Real32 a, Real32 b);
SMILE_API Real32 Real32_Sub(Real32 a, Real32 b);
SMILE_API Real32 Real32_Mul(Real32 a, Real32 b);
SMILE_API Real32 Real32_Div(Real32 a, Real32 b);
SMILE_API Real32 Real32_Rem(Real32 a, Real32 b);

SMILE_API Real32 Real32_Neg(Real32 real32);
SMILE_API Real32 Real32_Abs(Real32 real32);

#endif
