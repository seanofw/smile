//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

#include "../stdafx.h"

TEST_SUITE(Real128Tests)

//-------------------------------------------------------------------------------------------------
//  Type-Conversion Tests.

START_TEST(CanConvertInt32ToReal128)
{
	Real128 real;

	Real128 weirdPi		= { { 314159265ULL, 0x3040000000000000ULL } };
	Real128 int32Max	= { { 0x7FFFFFFFULL, 0x3040000000000000ULL } };
	Real128 int32Min	= { { 0x80000000ULL, 0xB040000000000000ULL } };

	real = Real128_FromInt32(0);
	ASSERT(!MemCmp(&real, &Real128_Zero, sizeof(Real128)));

	real = Real128_FromInt32(1);
	ASSERT(!MemCmp(&real, &Real128_One, sizeof(Real128)));

	real = Real128_FromInt32(-1);
	ASSERT(!MemCmp(&real, &Real128_NegOne, sizeof(Real128)));

	real = Real128_FromInt32(314159265);
	ASSERT(!MemCmp(&real, &weirdPi, sizeof(Real128)));

	real = Real128_FromInt32(Int32Max);
	ASSERT(!MemCmp(&real, &int32Max, sizeof(Real128)));

	real = Real128_FromInt32(Int32Min);
	ASSERT(!MemCmp(&real, &int32Min, sizeof(Real128)));
}
END_TEST

START_TEST(CanConvertInt64ToReal128)
{
	Real128 real;

	Real128 weirdPi		= { { 314159265358979323ULL, 0x3040000000000000ULL } };
	Real128 int64Max	= { { 0x7FFFFFFFFFFFFFFFULL, 0x3040000000000000ULL } };
	Real128 int64Min	= { { 0x8000000000000000ULL, 0xB040000000000000ULL } };

	real = Real128_FromInt64(0);
	ASSERT(!MemCmp(&real, &Real128_Zero, sizeof(Real128)));

	real = Real128_FromInt64(1);
	ASSERT(!MemCmp(&real, &Real128_One, sizeof(Real128)));

	real = Real128_FromInt64(-1);
	ASSERT(!MemCmp(&real, &Real128_NegOne, sizeof(Real128)));

	real = Real128_FromInt64(314159265358979323LL);
	ASSERT(!MemCmp(&real, &weirdPi, sizeof(Real128)));

	real = Real128_FromInt64(Int64Max);
	ASSERT(!MemCmp(&real, &int64Max, sizeof(Real128)));

	real = Real128_FromInt64(Int64Min);
	ASSERT(!MemCmp(&real, &int64Min, sizeof(Real128)));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Arithmetic Tests.

START_TEST(CanAddReal128)
{
	Real128 a, b, c, d;

	Real128 fiveSevenNine = { { 579ULL, 0x3040000000000000ULL } };

	a = Real128_FromInt64(123);
	b = Real128_FromInt64(456);
	c = Real128_Add(a, b);
	ASSERT(!MemCmp(&c, &fiveSevenNine, sizeof(Real128)));
}
END_TEST

#include "real128_tests.generated.inc"
