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

// A note about the tests in this file:  These are ported (with slight mutations) from the
// very exhaustive test suite Intel provides with its BID library.

//-------------------------------------------------------------------------------------------------
//  Helper functions and macros.

#define RI(__n__) Real128_FromInt32(__n__)
#define RL(__n__) Real128_FromInt64(__n__)
#define RF(__n__) Real128_FromFloat32(__n__)
#define RD(__n__) Real128_FromFloat64(__n__)

Inline Int ComputeByteValue(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	else return -1;
}

/// <summary>
/// Convert a string of hex characters in high-to-low order into a Real128.
/// </summary>
static Real128 RR(const char *str)
{
	// TODO: This only works properly on little-endian architectures.

	union {
		Byte buffer[16];
		Real128 real;
	} u;
	Byte *bufptr = u.buffer;
	Bool highNybble = False;
	const char *src = str;

	while (*src) src++;

	while (src >= str && bufptr < u.buffer + 16) {
		Int value = ComputeByteValue(*--src);
		if (value < 0) continue;

		if (highNybble) {
			*bufptr++ |= (Byte)value << 4;
			highNybble = False;
		}
		else {
			*bufptr = (Byte)value;
			highNybble = True;
		}
	}
	while (bufptr < u.buffer + 16) {
		if (highNybble) {
			*bufptr++;
			highNybble = False;
		}
		else {
			*bufptr = 0;
			highNybble = True;
		}
	}

	return u.real;
}

Inline Eq(Real128 a, Real128 b)
{
	return a.value[0] == b.value[0] && a.value[1] == b.value[1];
}

//-------------------------------------------------------------------------------------------------
//  Static constants.

// Small things involving fractions.
static const Real128 _onePointFive = { { 15ULL, 0x303E000000000000ULL } };
static const Real128 _negOnePointFive = { { 15ULL, 0xB03E000000000000ULL } };
static const Real128 _ohPointFive = { { 50ULL, 0x303C000000000000ULL } };
static const Real128 _ohPointTwoFive = { { 25ULL, 0x303C000000000000ULL } };
static const Real128 _ohPointSevenFive = { { 75ULL, 0x303C000000000000ULL } };
static const Real128 _ohPointOneTwoFive = { { 125ULL, 0x303A000000000000ULL } };
static const Real128 _onePointSixTwoFive = { { 1625ULL, 0x303A000000000000ULL } };

// Common limits.
static const Real128 _int32Max = { { 0x7FFFFFFFULL, 0x3040000000000000ULL } };
static const Real128 _int32Min = { { 0x80000000ULL, 0xB040000000000000ULL } };
static const Real128 _int64Max = { { 0x7FFFFFFFFFFFFFFFULL, 0x3040000000000000ULL } };
static const Real128 _int64Min = { { 0x8000000000000000ULL, 0xB040000000000000ULL } };

// Miscellaneous integers.
static const Real128 _three = { { 3ULL, 0x3040000000000000ULL } };
static const Real128 _fiveSevenNine = { { 579ULL, 0x3040000000000000ULL } };
static const Real128 _fiveSevenNineOhOh = { { 57900ULL, 0x303C000000000000ULL } };
static const Real128 _shortBigPi = { { 314159265ULL, 0x3040000000000000ULL } };
static const Real128 _longBigPi = { { 314159265358979323ULL, 0x3040000000000000ULL } };
static const Real128 _longMediumPi = { { 314159265358979323ULL, 0x302E000000000000ULL } };
static const Real128 _pi = { { 314159265358979323ULL, 0x301E000000000000ULL } };
static const Real128 _longNegMediumPi = { { 314159265358979323ULL, 0xB02E000000000000ULL } };
static const Real128 _negPi = { { 314159265358979323ULL, 0xB01E000000000000ULL } };

//-------------------------------------------------------------------------------------------------
//  Type-Conversion Tests.

START_TEST(CanConvertInt32ToReal128)
{
	ASSERT(Eq(RI(-1250716900), RR("b040000000000000000000004a8c6ce4")));
	ASSERT(Eq(RI(-1375453304), RR("b0400000000000000000000051fbc078")));
	ASSERT(Eq(RI(1450740614), RR("30400000000000000000000056788b86")));
	ASSERT(Eq(RI(-1834423832), RR("b040000000000000000000006d571618")));
	ASSERT(Eq(RI(1930174213), RR("304000000000000000000000730c1f05")));
	ASSERT(Eq(RI(1947153827), RR("304000000000000000000000740f35a3")));
	ASSERT(Eq(RI(206409795), RR("3040000000000000000000000c4d9043")));
	ASSERT(Eq(RI(2103044481), RR("3040000000000000000000007d59e981")));
	ASSERT(Eq(RI(-244968231), RR("b040000000000000000000000e99eb27")));
	ASSERT(Eq(RI(-328472007), RR("b04000000000000000000000139415c7")));
	ASSERT(Eq(RI(-430827320), RR("b0400000000000000000000019ade738")));
	ASSERT(Eq(RI(472411624), RR("3040000000000000000000001c286de8")));
	ASSERT(Eq(RI(-503478515), RR("b040000000000000000000001e0278f3")));
	ASSERT(Eq(RI(620564199), RR("30400000000000000000000024fd0ee7")));
	ASSERT(Eq(RI(648685195), RR("30400000000000000000000026aa268b")));
	ASSERT(Eq(RI(734426893), RR("3040000000000000000000002bc6770d")));
	ASSERT(Eq(RI(774990435), RR("3040000000000000000000002e316a63")));
	ASSERT(Eq(RI(797130594), RR("3040000000000000000000002f833f62")));
	ASSERT(Eq(RI(-893748152), RR("b04000000000000000000000354583b8")));
	ASSERT(Eq(RI(-958871488), RR("b04000000000000000000000392737c0")));
}
END_TEST

START_TEST(CanConvertInt64ToReal128)
{
	ASSERT(Eq(RL(-1297714641311568497LL), RR("b0400000000000001202689737331271")));
	ASSERT(Eq(RL(-1382533918512158604LL), RR("b040000000000000132fbf461ed8978c")));
	ASSERT(Eq(RL(1991200058281483048LL), RR("30400000000000001ba229e7369adb28")));
	ASSERT(Eq(RL(-1998782831704824207LL), RR("b0400000000000001bbd1a653aea598f")));
	ASSERT(Eq(RL(2081344241858484150LL), RR("30400000000000001ce26b8f7f4797b6")));
	ASSERT(Eq(RL(-3494168784785967941LL), RR("b040000000000000307dc7ff326d0b45")));
	ASSERT(Eq(RL(-3623475514305027548LL), RR("b04000000000000032492bc8427a1ddc")));
	ASSERT(Eq(RL(-3719337100671242603LL), RR("b040000000000000339dbd631d70d16b")));
	ASSERT(Eq(RL(4134085355817711631LL), RR("3040000000000000395f38ba50a8b80f")));
	ASSERT(Eq(RL(-4543384426972292615LL), RR("b0400000000000003f0d58107fb9a607")));
	ASSERT(Eq(RL(-456313642285820771LL), RR("b040000000000000065526d70a9c6b63")));
	ASSERT(Eq(RL(572333114425411975LL), RR("304000000000000007f155ef6a850987")));
	ASSERT(Eq(RL(-5896812445967530814LL), RR("b04000000000000051d5afad4cff3b3e")));
	ASSERT(Eq(RL(6084829525942336602LL), RR("30400000000000005471a8370c99fc5a")));
	ASSERT(Eq(RL(6343298205297819548LL), RR("30400000000000005807ec1c07263f9c")));
	ASSERT(Eq(RL(-640590259490231037LL), RR("b04000000000000008e3d5726f7d92fd")));
	ASSERT(Eq(RL(7071067091834809443LL), RR("304000000000000062217a0e4fc4ac63")));
	ASSERT(Eq(RL(7575924962073745274LL), RR("30400000000000006923179d316b977a")));
	ASSERT(Eq(RL(-8534495841839544214LL), RR("b04000000000000076709ec01de4f796")));
	ASSERT(Eq(RL(945634496444636363LL), RR("30400000000000000d1f919077f01ccb")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Parsing Tests.

START_TEST(CanParseReal128)
{
	Real128 result;

	ASSERT(Real128_TryParse(String_FromC("579"), &result));
	ASSERT(Eq(result, _fiveSevenNine));

	ASSERT(Real128_TryParse(String_FromC("579.00"), &result));
	ASSERT(Eq(result, _fiveSevenNineOhOh));

	ASSERT(Real128_TryParse(String_FromC("314159265358979323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real128_TryParse(String_FromC("314_159_265_358_979_323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real128_TryParse(String_FromC("314'159'265'358'979'323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real128_TryParse(String_FromC("314'159\"265'358\"979'323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real128_TryParse(String_FromC("-314'159'265.358'979'323"), &result));
	ASSERT(Eq(result, _longNegMediumPi));

	ASSERT(Real128_TryParse(String_FromC("3.14159'26535'89793'23"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real128_TryParse(String_FromC("-3.14159'26535'89793'23"), &result));
	ASSERT(Eq(result, _negPi));

	ASSERT(Real128_TryParse(String_FromC("+3.14159'26535'89793'23"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real128_TryParse(String_FromC("+3.14159'26535'89793'23E+8"), &result));
	ASSERT(Eq(result, _longMediumPi));

	ASSERT(Real128_TryParse(String_FromC("-3.14159'26535'89793'23E+8"), &result));
	ASSERT(Eq(result, _longNegMediumPi));

	ASSERT(Real128_TryParse(String_FromC("+314'159'265.358'979'323E-8"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real128_TryParse(String_FromC("-314'159'265.358'979'323E-8"), &result));
	ASSERT(Eq(result, _negPi));

	ASSERT(Real128_TryParse(String_FromC("+inf"), &result));
	ASSERT(Eq(result, Real128_Inf));

	ASSERT(Real128_TryParse(String_FromC("inf"), &result));
	ASSERT(Eq(result, Real128_Inf));

	ASSERT(Real128_TryParse(String_FromC("-inf"), &result));
	ASSERT(Eq(result, Real128_NegInf));

	ASSERT(Real128_TryParse(String_FromC("+nan"), &result));
	ASSERT(Real128_IsNaN(result));
	ASSERT(!Real128_IsNeg(result));

	ASSERT(Real128_TryParse(String_FromC("nan"), &result));
	ASSERT(Real128_IsNaN(result));
	ASSERT(!Real128_IsNeg(result));

	ASSERT(Real128_TryParse(String_FromC("-nan"), &result));
	ASSERT(Real128_IsNaN(result));
	ASSERT(Real128_IsNeg(result));
}
END_TEST

START_TEST(CanStringifyReal128InExponentialForm)
{
	ASSERT_STRING(Real128_ToExpString(_pi, 1, False), "3.14159265358979323e+0", 22);
	ASSERT_STRING(Real128_ToExpString(_pi, 1, True), "+3.14159265358979323e+0", 23);
	ASSERT_STRING(Real128_ToExpString(_negPi, 1, False), "-3.14159265358979323e+0", 23);

	ASSERT_STRING(Real128_ToExpString(_longMediumPi, 1, False), "3.14159265358979323e+8", 22);
	ASSERT_STRING(Real128_ToExpString(_longNegMediumPi, 1, False), "-3.14159265358979323e+8", 23);

	ASSERT_STRING(Real128_ToExpString(RD(12345000000), 1, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real128_ToExpString(RD(-12345000000), 1, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real128_ToExpString(RD(12345000000), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real128_ToExpString(RD(-12345000000), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real128_ToExpString(Real128_ParseC("12345000000"), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real128_ToExpString(Real128_ParseC("-12345000000"), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real128_ToExpString(Real128_ParseC("1.2345E+10"), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real128_ToExpString(Real128_ParseC("-1.2345E+10"), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real128_ToExpString(Real128_ParseC("1.2345E+10"), 1, False), "1.2345e+10", 10);
	ASSERT_STRING(Real128_ToExpString(Real128_ParseC("-1.2345E+10"), 1, False), "-1.2345e+10", 11);

	ASSERT_STRING(Real128_ToExpString(RD(0.125), 1, False), "1.25e-1", 7);
	ASSERT_STRING(Real128_ToExpString(RD(-0.125), 1, False), "-1.25e-1", 8);

	ASSERT_STRING(Real128_ToExpString(Real128_ParseC("0.125000"), 1, False), "1.25000e-1", 10);
	ASSERT_STRING(Real128_ToExpString(Real128_ParseC("-0.125000"), 1, False), "-1.25000e-1", 11);

	ASSERT_STRING(Real128_ToExpString(_fiveSevenNineOhOh, 1, False), "5.7900e+2", 9);
	ASSERT_STRING(Real128_ToExpString(_fiveSevenNineOhOh, 2, False), "5.7900e+2", 9);
	ASSERT_STRING(Real128_ToExpString(_fiveSevenNineOhOh, 3, False), "5.7900e+2", 9);
	ASSERT_STRING(Real128_ToExpString(_fiveSevenNineOhOh, 4, False), "5.7900e+2", 9);
	ASSERT_STRING(Real128_ToExpString(_fiveSevenNineOhOh, 5, False), "5.79000e+2", 10);
	ASSERT_STRING(Real128_ToExpString(_fiveSevenNineOhOh, 6, False), "5.790000e+2", 11);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Arithmetic Tests.

START_TEST(CanAddReal128)
{
	ASSERT(Eq(Real128_Add(RI(123), RI(456)), _fiveSevenNine));
	ASSERT(Eq(Real128_Add(RD(1.5), RD(0.125)), _onePointSixTwoFive));
	ASSERT(Eq(Real128_Add(RD(1.5), RD(-3.0)), _negOnePointFive));
}
END_TEST

#include "real128_tests.generated.inc"
