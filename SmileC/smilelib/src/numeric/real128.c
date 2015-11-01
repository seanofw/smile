//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <smile/numeric/real32.h>
#include <smile/numeric/real64.h>
#include <smile/numeric/real128.h>

#include "../decimal/bid_conf.h"
#include "../decimal/bid_functions.h"

SMILE_API Real128 Real128_Zero =	{ { 0x0000000000000000ULL, 0x3040000000000000ULL } };
SMILE_API Real128 Real128_One =		{ { 0x0000000000000001ULL, 0x3040000000000000ULL } };
SMILE_API Real128 Real128_NegOne =	{ { 0x0000000000000001ULL, 0xB040000000000000ULL } };

SMILE_API Real128 Real128_FromInt32(Int32 int32)
{
	Real128 result;
	bid128_from_int32((BID_UINT128 *)&result, &int32);
	return result;
}

SMILE_API Real128 Real128_FromInt64(Int64 int64)
{
	Real128 result;
	bid128_from_int64((BID_UINT128 *)&result, &int64);
	return result;
}

SMILE_API Real32 Real128_ToReal32(Real128 real128)
{
	Real32 result;
	bid128_to_bid32((BID_UINT32 *)&result, (BID_UINT128 *)&real128);
	return result;
}

SMILE_API Real64 Real128_ToReal64(Real128 real128)
{
	Real64 result;
	bid128_to_bid64((BID_UINT64 *)&result, (BID_UINT128 *)&real128);
	return result;
}

SMILE_API Real128 Real128_Add(Real128 a, Real128 b)
{
	Real128 result;
	bid128_add((BID_UINT128 *)&result, (BID_UINT128 *)&a, (BID_UINT128 *)&b);
	return result;
}

SMILE_API Real128 Real128_Sub(Real128 a, Real128 b)
{
	Real128 result;
	bid128_sub((BID_UINT128 *)&result, (BID_UINT128 *)&a, (BID_UINT128 *)&b);
	return result;
}

SMILE_API Real128 Real128_Mul(Real128 a, Real128 b)
{
	Real128 result;
	bid128_mul((BID_UINT128 *)&result, (BID_UINT128 *)&a, (BID_UINT128 *)&b);
	return result;
}

SMILE_API Real128 Real128_Div(Real128 a, Real128 b)
{
	Real128 result;
	bid128_div((BID_UINT128 *)&result, (BID_UINT128 *)&a, (BID_UINT128 *)&b);
	return result;
}

SMILE_API Real128 Real128_Rem(Real128 a, Real128 b)
{
	Real128 result;
	bid128_rem((BID_UINT128 *)&result, (BID_UINT128 *)&a, (BID_UINT128 *)&b);
	return result;
}

SMILE_API Real128 Real128_Neg(Real128 real128)
{
	Real128 result;
	bid128_negate((BID_UINT128 *)&result, (BID_UINT128 *)&real128);
	return result;
}

SMILE_API Real128 Real128_Abs(Real128 real128)
{
	Real128 result;
	bid128_abs((BID_UINT128 *)&result, (BID_UINT128 *)&real128);
	return result;
}
