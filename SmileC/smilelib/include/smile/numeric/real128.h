#ifndef __SMILE_NUMERIC_REAL128_H__
#define __SMILE_NUMERIC_REAL128_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

SMILE_API Real128 Real128_Zero;
SMILE_API Real128 Real128_One;
SMILE_API Real128 Real128_NegOne;

SMILE_API Real128 Real128_FromInt32(Int32 int32);
SMILE_API Real128 Real128_FromInt64(Int64 int64);
SMILE_API Real128 Real128_FromFloat32(Float32 float32);
SMILE_API Real128 Real128_FromFloat64(Float64 float64);
SMILE_API Float32 Real128_ToFloat32(Real128 real128);
SMILE_API Float64 Real128_ToFloat64(Real128 real128);
SMILE_API Real32 Real128_ToReal32(Real128 real128);
SMILE_API Real64 Real128_ToReal64(Real128 real128);

SMILE_API Real128 Real128_Add(Real128 a, Real128 b);
SMILE_API Real128 Real128_Sub(Real128 a, Real128 b);
SMILE_API Real128 Real128_Mul(Real128 a, Real128 b);
SMILE_API Real128 Real128_Div(Real128 a, Real128 b);
SMILE_API Real128 Real128_Rem(Real128 a, Real128 b);

SMILE_API Real128 Real128_Neg(Real128 real128);
SMILE_API Real128 Real128_Abs(Real128 real128);

#endif
