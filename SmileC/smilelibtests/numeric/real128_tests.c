//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2017 Sean Werkema
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

Inline Bool Eq(Real128 a, Real128 b)
{
	return a.value[0] == b.value[0] && a.value[1] == b.value[1];
}

//-------------------------------------------------------------------------------------------------
//  Static constants.

// Common limits.
static const Real128 _int32Max = { { 0x7FFFFFFFULL, 0x3040000000000000ULL } };
static const Real128 _int32Min = { { 0x80000000ULL, 0xB040000000000000ULL } };
static const Real128 _int64Max = { { 0x7FFFFFFFFFFFFFFFULL, 0x3040000000000000ULL } };
static const Real128 _int64Min = { { 0x8000000000000000ULL, 0xB040000000000000ULL } };

// Miscellaneous integers.
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
//
//  Some of these are ganked straight from Intel's tests included with their BID library.

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
	ASSERT(Real128_TryParse(String_FromC("INF"), &result));
	ASSERT(Eq(result, Real128_Inf));
	ASSERT(Real128_TryParse(String_FromC("Inf"), &result));
	ASSERT(Eq(result, Real128_Inf));

	ASSERT(Real128_TryParse(String_FromC("-inf"), &result));
	ASSERT(Eq(result, Real128_NegInf));

	ASSERT(Real128_TryParse(String_FromC("+nan"), &result));
	ASSERT(Real128_IsNaN(result));
	ASSERT(!Real128_IsNeg(result));

	ASSERT(Real128_TryParse(String_FromC("nan"), &result));
	ASSERT(Real128_IsNaN(result));
	ASSERT(!Real128_IsNeg(result));
	ASSERT(Real128_TryParse(String_FromC("NAN"), &result));
	ASSERT(Real128_IsNaN(result));
	ASSERT(!Real128_IsNeg(result));
	ASSERT(Real128_TryParse(String_FromC("NaN"), &result));
	ASSERT(Real128_IsNaN(result));
	ASSERT(!Real128_IsNeg(result));

	ASSERT(Real128_TryParse(String_FromC("-nan"), &result));
	ASSERT(Real128_IsNaN(result));
	ASSERT(Real128_IsNeg(result));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Stringification Tests.

START_TEST(CanStringifyReal128NumbersInExponentialForm)
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

START_TEST(CanStringifySpecialReal128ValuesInExponentialForm)
{
	ASSERT_STRING(Real128_ToExpString(Real128_Zero, 0, False), "0", 1);
	ASSERT_STRING(Real128_ToExpString(Real128_Zero, 0, True), "+0", 2);
	ASSERT_STRING(Real128_ToExpString(Real128_Zero, 5, False), "0.00000", 7);
	ASSERT_STRING(Real128_ToExpString(Real128_Zero, 5, True), "+0.00000", 8);

	ASSERT_STRING(Real128_ToExpString(Real128_One, 0, False), "1e+0", 4);
	ASSERT_STRING(Real128_ToExpString(Real128_One, 0, True), "+1e+0", 5);
	ASSERT_STRING(Real128_ToExpString(Real128_One, 5, False), "1.00000e+0", 10);
	ASSERT_STRING(Real128_ToExpString(Real128_One, 5, True), "+1.00000e+0", 11);

	ASSERT_STRING(Real128_ToExpString(Real128_Inf, 1, False), "inf", 3);
	ASSERT_STRING(Real128_ToExpString(Real128_Inf, 1, True), "+inf", 4);
	ASSERT_STRING(Real128_ToExpString(Real128_NegInf, 1, False), "-inf", 4);

	ASSERT_STRING(Real128_ToExpString(Real128_NaN, 1, False), "NaN", 3);
	ASSERT_STRING(Real128_ToExpString(Real128_NaN, 1, True), "+NaN", 4);
	ASSERT_STRING(Real128_ToExpString(Real128_NegNaN, 1, False), "-NaN", 4);
}
END_TEST

START_TEST(CanStringifyReal128NumbersInFixedForm)
{
	// Things split across the decimal point.
	ASSERT_STRING(Real128_ToFixedString(_pi, 1, 1, False), "3.14159265358979323", 19);
	ASSERT_STRING(Real128_ToFixedString(_pi, 1, 1, True), "+3.14159265358979323", 20);
	ASSERT_STRING(Real128_ToFixedString(_negPi, 1, 1, False), "-3.14159265358979323", 20);

	ASSERT_STRING(Real128_ToFixedString(_longMediumPi, 1, 1, False), "314159265.358979323", 19);
	ASSERT_STRING(Real128_ToFixedString(_longMediumPi, 1, 1, True), "+314159265.358979323", 20);
	ASSERT_STRING(Real128_ToFixedString(_longNegMediumPi, 1, 1, False), "-314159265.358979323", 20);

	// Things that only exist after the decimal point.
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("0.125000"), 1, 1, False), "0.125000", 8);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("-0.125000"), 1, 1, False), "-0.125000", 9);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("125e-5"), 1, 1, False), "0.00125", 7);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("-125e-5"), 1, 1, False), "-0.00125", 8);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("125e-5"), 0, 0, False), ".00125", 6);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("-125e-5"), 0, 0, False), "-.00125", 7);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("125e-5"), 3, 7, False), "000.0012500", 11);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("-125e-5"), 3, 7, False), "-000.0012500", 12);

	// Things that only exist before the decimal point.
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("125000"), 1, 1, False), "125000.0", 8);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("-125000"), 1, 1, False), "-125000.0", 9);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("125e+3"), 1, 1, False), "125000.0", 8);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("-125e+3"), 1, 1, False), "-125000.0", 9);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("125e+3"), 0, 0, False), "125000", 6);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("-125e+3"), 0, 0, False), "-125000", 7);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("125e+3"), 10, 3, False), "0000125000.000", 14);
	ASSERT_STRING(Real128_ToFixedString(Real128_ParseC("-125e+3"), 10, 3, False), "-0000125000.000", 15);
}
END_TEST

START_TEST(CanStringifySpecialReal128ValuesInFixedForm)
{
	ASSERT_STRING(Real128_ToFixedString(Real128_Zero, 0, 0, False), "0", 1);
	ASSERT_STRING(Real128_ToFixedString(Real128_Zero, 0, 0, True), "+0", 2);
	ASSERT_STRING(Real128_ToFixedString(Real128_Zero, 5, 5, False), "00000.00000", 11);
	ASSERT_STRING(Real128_ToFixedString(Real128_Zero, 5, 5, True), "+00000.00000", 12);

	ASSERT_STRING(Real128_ToFixedString(Real128_One, 0, 0, False), "1", 1);
	ASSERT_STRING(Real128_ToFixedString(Real128_One, 0, 0, True), "+1", 2);
	ASSERT_STRING(Real128_ToFixedString(Real128_One, 5, 5, False), "00001.00000", 11);
	ASSERT_STRING(Real128_ToFixedString(Real128_One, 5, 5, True), "+00001.00000", 12);

	ASSERT_STRING(Real128_ToFixedString(Real128_Inf, 0, 0, False), "inf", 3);
	ASSERT_STRING(Real128_ToFixedString(Real128_Inf, 0, 0, True), "+inf", 4);
	ASSERT_STRING(Real128_ToFixedString(Real128_NegInf, 0, 0, False), "-inf", 4);

	ASSERT_STRING(Real128_ToFixedString(Real128_NaN, 0, 0, False), "NaN", 3);
	ASSERT_STRING(Real128_ToFixedString(Real128_NaN, 0, 0, True), "+NaN", 4);
	ASSERT_STRING(Real128_ToFixedString(Real128_NegNaN, 0, 0, False), "-NaN", 4);
}
END_TEST

START_TEST(CanStringifyReal128NumbersGenerically)
{
	// Things split across the decimal point.
	ASSERT_STRING(Real128_ToString(_pi), "3.14159265358979323", 19);
	ASSERT_STRING(Real128_ToString(_negPi), "-3.14159265358979323", 20);

	ASSERT_STRING(Real128_ToString(_longMediumPi), "314159265.358979323", 19);
	ASSERT_STRING(Real128_ToString(_longNegMediumPi), "-314159265.358979323", 20);

	// Things that only exist after the decimal point.
	ASSERT_STRING(Real128_ToString(Real128_ParseC("0.125000")), ".125000", 7);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("-0.125000")), "-.125000", 8);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("125e-5")), ".00125", 6);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("-125e-5")), "-.00125", 7);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("125e-10")), "1.25e-8", 7);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("-125e-10")), "-1.25e-8", 8);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("1250e-11")), "1.250e-8", 8);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("-1250e-11")), "-1.250e-8", 9);

	// Things that only exist before the decimal point.
	ASSERT_STRING(Real128_ToString(Real128_ParseC("125000")), "125000", 6);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("-125000")), "-125000", 7);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("125e+3")), "125000", 6);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("-125e+3")), "-125000", 7);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("125e+10")), "1.25e+12", 8);
	ASSERT_STRING(Real128_ToString(Real128_ParseC("-125e+10")), "-1.25e+12", 9);
}
END_TEST

START_TEST(CanStringifySpecialReal128ValuesGenerically)
{
	ASSERT_STRING(Real128_ToString(Real128_Zero), "0", 1);
	ASSERT_STRING(Real128_ToString(Real128_One), "1", 1);

	ASSERT_STRING(Real128_ToString(Real128_Inf), "inf", 3);
	ASSERT_STRING(Real128_ToString(Real128_NegInf), "-inf", 4);

	ASSERT_STRING(Real128_ToString(Real128_NaN), "NaN", 3);
	ASSERT_STRING(Real128_ToString(Real128_NegNaN), "-NaN", 4);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Arithmetic Tests.

START_TEST(CanAddReal128)
{
	ASSERT(Eq(Real128_Add(RI(123), RI(456)), RI(579)));
	ASSERT(Eq(Real128_Add(RD(1.5), RD(0.125)), RD(1.625)));
	ASSERT(Eq(Real128_Add(RD(1.5), RD(-3.0)), RD(-1.5)));
}
END_TEST

START_TEST(CanMultiplyReal128)
{
	ASSERT(Eq(Real128_Mul(RI(123), RI(456)), RI(56088)));
	ASSERT(Eq(Real128_Mul(RD(1.5), RD(0.125)), RD(0.1875)));
	ASSERT(Eq(Real128_Mul(RD(1.5), RD(-3.0)), RD(-4.5)));
}
END_TEST

START_TEST(CanSubtractReal128)
{
	ASSERT(Eq(Real128_Sub(RI(123), RI(456)), RI(-333)));
	ASSERT(Eq(Real128_Sub(RD(1.5), RD(0.125)), RD(1.375)));
	ASSERT(Eq(Real128_Sub(RD(1.5), RD(-3.0)), RD(4.5)));
}
END_TEST

START_TEST(CanDivideReal128)
{
	ASSERT(Eq(Real128_Div(RI(123), RI(456)), Real128_ParseC(".2697368421052631578947368421052632")));
	ASSERT(Eq(Real128_Div(RD(1.5), RD(0.125)), RI(12)));
	ASSERT(Eq(Real128_Div(RD(1.5), RD(-3.0)), RD(-0.5)));
}
END_TEST

START_TEST(CanModulusReal128)
{
	ASSERT(Eq(Real128_Mod(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real128_Mod(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real128_Mod(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real128_Mod(RI(-456), RI(123)), RI(87)));
	ASSERT(Eq(Real128_Mod(RI(456), RI(-123)), RI(-87)));
	ASSERT(Eq(Real128_Mod(RI(-456), RI(-123)), RI(-87)));
}
END_TEST

START_TEST(CanRemainderReal128)
{
	ASSERT(Eq(Real128_Rem(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real128_Rem(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real128_Rem(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real128_Rem(RI(-456), RI(123)), RI(-87)));
	ASSERT(Eq(Real128_Rem(RI(456), RI(-123)), RI(87)));
	ASSERT(Eq(Real128_Rem(RI(-456), RI(-123)), RI(-87)));
}
END_TEST

START_TEST(CanIeeeRemainderReal128)
{
	ASSERT(Eq(Real128_IeeeRem(RI(456), RI(123)), RI(-36)));
	ASSERT(Eq(Real128_IeeeRem(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real128_IeeeRem(RI(456), RI(123)), RI(-36)));
	ASSERT(Eq(Real128_IeeeRem(RI(-456), RI(123)), RI(36)));
	ASSERT(Eq(Real128_IeeeRem(RI(456), RI(-123)), RI(-36)));
	ASSERT(Eq(Real128_IeeeRem(RI(-456), RI(-123)), RI(36)));
}
END_TEST

START_TEST(CanNegateReal128)
{
	ASSERT(Eq(Real128_Neg(RI(123)), RI(-123)));
	ASSERT(Eq(Real128_Neg(RI(-123)), RI(123)));

	ASSERT(Eq(Real128_Neg(Real128_Zero), Real128_NegZero));
	ASSERT(Eq(Real128_Neg(Real128_NegZero), Real128_Zero));

	ASSERT(Eq(Real128_Neg(Real128_Inf), Real128_NegInf));
	ASSERT(Eq(Real128_Neg(Real128_NegInf), Real128_Inf));
}
END_TEST

START_TEST(CanAbsoluteReal128)
{
	ASSERT(Eq(Real128_Abs(RI(123)), RI(123)));
	ASSERT(Eq(Real128_Abs(RI(-123)), RI(123)));

	ASSERT(Eq(Real128_Abs(Real128_Zero), Real128_Zero));
	ASSERT(Eq(Real128_Abs(Real128_NegZero), Real128_Zero));

	ASSERT(Eq(Real128_Abs(Real128_Inf), Real128_Inf));
	ASSERT(Eq(Real128_Abs(Real128_NegInf), Real128_Inf));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Powers and logarithms.

START_TEST(CanSqrtReal128)
{
	ASSERT(Eq(Real128_Sqrt(Real128_Zero), Real128_Zero));
	ASSERT(Eq(Real128_Sqrt(Real128_NegZero), Real128_NegZero));		// Weird, but correct.
	ASSERT(Eq(Real128_Sqrt(Real128_One), Real128_One));
	ASSERT(Eq(Real128_Sqrt(Real128_Inf), Real128_Inf));

	ASSERT(Eq(Real128_Sqrt(RI(256)), RI(16)));
	ASSERT(Eq(Real128_Sqrt(RI(25)), RI(5)));
	ASSERT(Eq(Real128_Sqrt(Real128_ParseC("18'446'744'065'119'617'025")), Real128_ParseC("4'294'967'295")));
	ASSERT(Eq(Real128_Sqrt(RI(123)), Real128_ParseC("11.09053'65064'09417'16205'16001'02609'93")));

	ASSERT(Eq(Real128_Sqrt(Real128_NegOne), Real128_NaN));
	ASSERT(Eq(Real128_Sqrt(RI(-123)), Real128_NaN));
	ASSERT(Eq(Real128_Sqrt(Real128_NegInf), Real128_NaN));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Rounding Tests.

START_TEST(CanFloorReal128)
{
	ASSERT(Eq(Real128_Floor(RI(123)), RI(123)));
	ASSERT(Eq(Real128_Floor(RI(-123)), RI(-123)));

	ASSERT(Eq(Real128_Floor(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real128_Floor(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real128_Floor(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real128_Floor(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real128_Floor(RD(-1.2)), RD(-2.0)));
	ASSERT(Eq(Real128_Floor(RD(1.8)), RD(1.0)));
	ASSERT(Eq(Real128_Floor(RD(-1.8)), RD(-2.0)));
	ASSERT(Eq(Real128_Floor(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real128_Floor(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanCeilReal128)
{
	ASSERT(Eq(Real128_Ceil(RI(123)), RI(123)));
	ASSERT(Eq(Real128_Ceil(RI(-123)), RI(-123)));

	ASSERT(Eq(Real128_Ceil(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real128_Ceil(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real128_Ceil(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real128_Ceil(RD(1.2)), RD(2.0)));
	ASSERT(Eq(Real128_Ceil(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real128_Ceil(RD(1.8)), RD(2.0)));
	ASSERT(Eq(Real128_Ceil(RD(-1.8)), RD(-1.0)));
	ASSERT(Eq(Real128_Ceil(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real128_Ceil(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanTruncReal128)
{
	ASSERT(Eq(Real128_Trunc(RI(123)), RI(123)));
	ASSERT(Eq(Real128_Trunc(RI(-123)), RI(-123)));

	ASSERT(Eq(Real128_Trunc(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real128_Trunc(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real128_Trunc(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real128_Trunc(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real128_Trunc(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real128_Trunc(RD(1.8)), RD(1.0)));
	ASSERT(Eq(Real128_Trunc(RD(-1.8)), RD(-1.0)));
	ASSERT(Eq(Real128_Trunc(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real128_Trunc(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanSplitReal128WithModf)
{
	Real128 intPart;

	ASSERT(Eq(Real128_Modf(RI(123), &intPart), Real128_Zero));
	ASSERT(Eq(intPart, RI(123)));
	ASSERT(Eq(Real128_Modf(RI(-123), &intPart), Real128_NegZero));
	ASSERT(Eq(intPart, RI(-123)));

	ASSERT(Eq(Real128_Modf(RD(0.0), &intPart), Real128_Zero));
	ASSERT(Eq(intPart, RI(0)));
	ASSERT(Eq(Real128_Modf(RD(1.0), &intPart), Real128_Zero));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real128_Modf(RD(-1.0), &intPart), Real128_NegZero));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real128_Modf(Real128_ParseC("1.2"), &intPart), Real128_ParseC(".2")));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real128_Modf(Real128_ParseC("-1.2"), &intPart), Real128_ParseC("-.2")));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real128_Modf(Real128_ParseC("1.8"), &intPart), Real128_ParseC(".8")));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real128_Modf(Real128_ParseC("-1.8"), &intPart), Real128_ParseC("-.8")));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real128_Modf(RD(2.0), &intPart), Real128_Zero));
	ASSERT(Eq(intPart, RI(2)));
	ASSERT(Eq(Real128_Modf(RD(-2.0), &intPart), Real128_NegZero));
	ASSERT(Eq(intPart, RI(-2)));
}
END_TEST

START_TEST(CanRoundReal128)
{
	ASSERT(Eq(Real128_Round(RI(123)), RI(123)));
	ASSERT(Eq(Real128_Round(RI(-123)), RI(-123)));

	ASSERT(Eq(Real128_Round(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real128_Round(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real128_Round(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real128_Round(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real128_Round(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real128_Round(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real128_Round(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real128_Round(RD(1.8)), RD(2.0)));
	ASSERT(Eq(Real128_Round(RD(-1.8)), RD(-2.0)));
	ASSERT(Eq(Real128_Round(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real128_Round(RD(-2.0)), RD(-2.0)));

	ASSERT(Eq(Real128_Round(RD(4.5)), RD(5.0)));
	ASSERT(Eq(Real128_Round(RD(4.0)), RD(4.0)));
	ASSERT(Eq(Real128_Round(RD(3.5)), RD(4.0)));
	ASSERT(Eq(Real128_Round(RD(3.0)), RD(3.0)));
	ASSERT(Eq(Real128_Round(RD(2.5)), RD(3.0)));
	ASSERT(Eq(Real128_Round(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real128_Round(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real128_Round(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real128_Round(RD(0.5)), RD(1.0)));
	ASSERT(Eq(Real128_Round(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real128_Round(RD(-0.5)), RD(-1.0)));
	ASSERT(Eq(Real128_Round(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real128_Round(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real128_Round(RD(-2.0)), RD(-2.0)));
	ASSERT(Eq(Real128_Round(RD(-2.5)), RD(-3.0)));
	ASSERT(Eq(Real128_Round(RD(-3.0)), RD(-3.0)));
	ASSERT(Eq(Real128_Round(RD(-3.5)), RD(-4.0)));
	ASSERT(Eq(Real128_Round(RD(-4.0)), RD(-4.0)));
	ASSERT(Eq(Real128_Round(RD(-4.5)), RD(-5.0)));
}
END_TEST

START_TEST(CanBankRoundReal128)
{
	ASSERT(Eq(Real128_BankRound(RD(4.5)), RD(4.0)));
	ASSERT(Eq(Real128_BankRound(RD(4.0)), RD(4.0)));
	ASSERT(Eq(Real128_BankRound(RD(3.5)), RD(4.0)));
	ASSERT(Eq(Real128_BankRound(RD(3.0)), RD(3.0)));
	ASSERT(Eq(Real128_BankRound(RD(2.5)), RD(2.0)));
	ASSERT(Eq(Real128_BankRound(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real128_BankRound(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real128_BankRound(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real128_BankRound(RD(0.5)), RD(0.0)));
	ASSERT(Eq(Real128_BankRound(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real128_BankRound(RD(-0.5)), Real128_NegZero));
	ASSERT(Eq(Real128_BankRound(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real128_BankRound(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real128_BankRound(RD(-2.0)), RD(-2.0)));
	ASSERT(Eq(Real128_BankRound(RD(-2.5)), RD(-2.0)));
	ASSERT(Eq(Real128_BankRound(RD(-3.0)), RD(-3.0)));
	ASSERT(Eq(Real128_BankRound(RD(-3.5)), RD(-4.0)));
	ASSERT(Eq(Real128_BankRound(RD(-4.0)), RD(-4.0)));
	ASSERT(Eq(Real128_BankRound(RD(-4.5)), RD(-4.0)));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Test-Function Tests.

START_TEST(CanTestForInfinityReal128)
{
	ASSERT(!Real128_IsInf(Real128_NegNaN));
	ASSERT( Real128_IsInf(Real128_NegInf));
	ASSERT(!Real128_IsInf(Real128_NegSixteen));
	ASSERT(!Real128_IsInf(Real128_NegTen));
	ASSERT(!Real128_IsInf(Real128_NegTwo));
	ASSERT(!Real128_IsInf(Real128_NegOne));
	ASSERT(!Real128_IsInf(Real128_NegZero));
	ASSERT(!Real128_IsInf(Real128_Zero));
	ASSERT(!Real128_IsInf(Real128_One));
	ASSERT(!Real128_IsInf(Real128_Two));
	ASSERT(!Real128_IsInf(Real128_Ten));
	ASSERT(!Real128_IsInf(Real128_Sixteen));
	ASSERT( Real128_IsInf(Real128_Inf));
	ASSERT(!Real128_IsInf(Real128_NaN));
}
END_TEST

START_TEST(CanTestForNaNReal128)
{
	ASSERT( Real128_IsNaN(Real128_NegNaN));
	ASSERT(!Real128_IsNaN(Real128_NegInf));
	ASSERT(!Real128_IsNaN(Real128_NegSixteen));
	ASSERT(!Real128_IsNaN(Real128_NegTen));
	ASSERT(!Real128_IsNaN(Real128_NegTwo));
	ASSERT(!Real128_IsNaN(Real128_NegOne));
	ASSERT(!Real128_IsNaN(Real128_NegZero));
	ASSERT(!Real128_IsNaN(Real128_Zero));
	ASSERT(!Real128_IsNaN(Real128_One));
	ASSERT(!Real128_IsNaN(Real128_Two));
	ASSERT(!Real128_IsNaN(Real128_Ten));
	ASSERT(!Real128_IsNaN(Real128_Sixteen));
	ASSERT(!Real128_IsNaN(Real128_Inf));
	ASSERT( Real128_IsNaN(Real128_NaN));
}
END_TEST

START_TEST(CanTestForNegativeReal128)
{
	ASSERT( Real128_IsNeg(Real128_NegNaN));
	ASSERT( Real128_IsNeg(Real128_NegInf));
	ASSERT( Real128_IsNeg(Real128_NegSixteen));
	ASSERT( Real128_IsNeg(Real128_NegTen));
	ASSERT( Real128_IsNeg(Real128_NegTwo));
	ASSERT( Real128_IsNeg(Real128_NegOne));
	ASSERT( Real128_IsNeg(Real128_NegZero));
	ASSERT(!Real128_IsNeg(Real128_Zero));
	ASSERT(!Real128_IsNeg(Real128_One));
	ASSERT(!Real128_IsNeg(Real128_Two));
	ASSERT(!Real128_IsNeg(Real128_Ten));
	ASSERT(!Real128_IsNeg(Real128_Sixteen));
	ASSERT(!Real128_IsNeg(Real128_Inf));
	ASSERT(!Real128_IsNeg(Real128_NaN));
}
END_TEST

START_TEST(CanTestForZeroReal128)
{
	ASSERT(!Real128_IsZero(Real128_NegNaN));
	ASSERT(!Real128_IsZero(Real128_NegInf));
	ASSERT(!Real128_IsZero(Real128_NegSixteen));
	ASSERT(!Real128_IsZero(Real128_NegTen));
	ASSERT(!Real128_IsZero(Real128_NegTwo));
	ASSERT(!Real128_IsZero(Real128_NegOne));
	ASSERT( Real128_IsZero(Real128_NegZero));
	ASSERT( Real128_IsZero(Real128_Zero));
	ASSERT(!Real128_IsZero(Real128_One));
	ASSERT(!Real128_IsZero(Real128_Two));
	ASSERT(!Real128_IsZero(Real128_Ten));
	ASSERT(!Real128_IsZero(Real128_Sixteen));
	ASSERT(!Real128_IsZero(Real128_Inf));
	ASSERT(!Real128_IsZero(Real128_NaN));
}
END_TEST

START_TEST(CanTestForFiniteReal128)
{
	ASSERT(!Real128_IsFinite(Real128_NegNaN));
	ASSERT(!Real128_IsFinite(Real128_NegInf));
	ASSERT( Real128_IsFinite(Real128_NegSixteen));
	ASSERT( Real128_IsFinite(Real128_NegTen));
	ASSERT( Real128_IsFinite(Real128_NegTwo));
	ASSERT( Real128_IsFinite(Real128_NegOne));
	ASSERT( Real128_IsFinite(Real128_NegZero));
	ASSERT( Real128_IsFinite(Real128_Zero));
	ASSERT( Real128_IsFinite(Real128_One));
	ASSERT( Real128_IsFinite(Real128_Two));
	ASSERT( Real128_IsFinite(Real128_Ten));
	ASSERT( Real128_IsFinite(Real128_Sixteen));
	ASSERT(!Real128_IsFinite(Real128_Inf));
	ASSERT(!Real128_IsFinite(Real128_NaN));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Comparison Tests.

START_TEST(CanCompareEqualReal128)
{
	ASSERT(!Real128_Eq(Real128_NegNaN, Real128_NegNaN));
	ASSERT(!Real128_Eq(Real128_NaN, Real128_NaN));
	ASSERT(!Real128_Eq(Real128_NegNaN, Real128_NaN));
	ASSERT(!Real128_Eq(Real128_NaN, Real128_NegNaN));

	ASSERT(Real128_Eq(Real128_NegZero, Real128_NegZero));
	ASSERT(Real128_Eq(Real128_Zero, Real128_Zero));
	ASSERT(Real128_Eq(Real128_NegZero, Real128_Zero));
	ASSERT(Real128_Eq(Real128_Zero, Real128_NegZero));

	ASSERT(Real128_Eq(Real128_NegInf, Real128_NegInf));
	ASSERT(Real128_Eq(Real128_NegOne, Real128_NegOne));
	ASSERT(Real128_Eq(Real128_One, Real128_One));
	ASSERT(Real128_Eq(Real128_Inf, Real128_Inf));

	ASSERT(!Real128_Eq(Real128_NegInf, Real128_Inf));
	ASSERT(!Real128_Eq(Real128_NegOne, Real128_One));
	ASSERT(!Real128_Eq(Real128_One, Real128_Two));
	ASSERT(!Real128_Eq(Real128_Inf, Real128_NegInf));
}
END_TEST

START_TEST(CanCompareNotEqualReal128)
{
	ASSERT(Real128_Ne(Real128_NegNaN, Real128_NegNaN));
	ASSERT(Real128_Ne(Real128_NaN, Real128_NaN));
	ASSERT(Real128_Ne(Real128_NegNaN, Real128_NaN));
	ASSERT(Real128_Ne(Real128_NaN, Real128_NegNaN));

	ASSERT(!Real128_Ne(Real128_NegZero, Real128_NegZero));
	ASSERT(!Real128_Ne(Real128_Zero, Real128_Zero));
	ASSERT(!Real128_Ne(Real128_NegZero, Real128_Zero));
	ASSERT(!Real128_Ne(Real128_Zero, Real128_NegZero));

	ASSERT(!Real128_Ne(Real128_NegInf, Real128_NegInf));
	ASSERT(!Real128_Ne(Real128_NegOne, Real128_NegOne));
	ASSERT(!Real128_Ne(Real128_One, Real128_One));
	ASSERT(!Real128_Ne(Real128_Inf, Real128_Inf));

	ASSERT(Real128_Ne(Real128_NegInf, Real128_Inf));
	ASSERT(Real128_Ne(Real128_NegOne, Real128_One));
	ASSERT(Real128_Ne(Real128_One, Real128_Two));
	ASSERT(Real128_Ne(Real128_Inf, Real128_NegInf));
}
END_TEST

START_TEST(CanCompareLessThanReal128)
{
	ASSERT(!Real128_Lt(Real128_NegNaN, Real128_NegNaN));
	ASSERT(!Real128_Lt(Real128_NaN, Real128_NaN));
	ASSERT(!Real128_Lt(Real128_NegNaN, Real128_NaN));
	ASSERT(!Real128_Lt(Real128_NaN, Real128_NegNaN));
	ASSERT(!Real128_Lt(Real128_One, Real128_NaN));
	ASSERT(!Real128_Lt(Real128_NaN, Real128_One));

	ASSERT(!Real128_Lt(Real128_NegZero, Real128_NegZero));
	ASSERT(!Real128_Lt(Real128_Zero, Real128_Zero));
	ASSERT(!Real128_Lt(Real128_NegZero, Real128_Zero));
	ASSERT(!Real128_Lt(Real128_Zero, Real128_NegZero));

	ASSERT(!Real128_Lt(Real128_NegInf, Real128_NegInf));
	ASSERT(!Real128_Lt(Real128_NegOne, Real128_NegOne));
	ASSERT(!Real128_Lt(Real128_One, Real128_One));
	ASSERT(!Real128_Lt(Real128_Inf, Real128_Inf));

	ASSERT(Real128_Lt(Real128_NegInf, Real128_Inf));
	ASSERT(!Real128_Lt(Real128_Inf, Real128_NegInf));
	ASSERT(Real128_Lt(Real128_NegOne, Real128_One));
	ASSERT(!Real128_Lt(Real128_One, Real128_NegOne));
	ASSERT(Real128_Lt(Real128_One, Real128_Two));
	ASSERT(!Real128_Lt(Real128_Two, Real128_One));
	ASSERT(Real128_Lt(Real128_Zero, Real128_Inf));
	ASSERT(!Real128_Lt(Real128_Inf, Real128_Zero));
	ASSERT(Real128_Lt(Real128_NegInf, Real128_Zero));
	ASSERT(!Real128_Lt(Real128_Zero, Real128_NegInf));
}
END_TEST

START_TEST(CanCompareGreaterThanReal128)
{
	ASSERT(!Real128_Gt(Real128_NegNaN, Real128_NegNaN));
	ASSERT(!Real128_Gt(Real128_NaN, Real128_NaN));
	ASSERT(!Real128_Gt(Real128_NegNaN, Real128_NaN));
	ASSERT(!Real128_Gt(Real128_NaN, Real128_NegNaN));
	ASSERT(!Real128_Gt(Real128_One, Real128_NaN));
	ASSERT(!Real128_Gt(Real128_NaN, Real128_One));

	ASSERT(!Real128_Gt(Real128_NegZero, Real128_NegZero));
	ASSERT(!Real128_Gt(Real128_Zero, Real128_Zero));
	ASSERT(!Real128_Gt(Real128_NegZero, Real128_Zero));
	ASSERT(!Real128_Gt(Real128_Zero, Real128_NegZero));

	ASSERT(!Real128_Gt(Real128_NegInf, Real128_NegInf));
	ASSERT(!Real128_Gt(Real128_NegOne, Real128_NegOne));
	ASSERT(!Real128_Gt(Real128_One, Real128_One));
	ASSERT(!Real128_Gt(Real128_Inf, Real128_Inf));

	ASSERT(!Real128_Gt(Real128_NegInf, Real128_Inf));
	ASSERT(Real128_Gt(Real128_Inf, Real128_NegInf));
	ASSERT(!Real128_Gt(Real128_NegOne, Real128_One));
	ASSERT(Real128_Gt(Real128_One, Real128_NegOne));
	ASSERT(!Real128_Gt(Real128_One, Real128_Two));
	ASSERT(Real128_Gt(Real128_Two, Real128_One));
	ASSERT(!Real128_Gt(Real128_Zero, Real128_Inf));
	ASSERT(Real128_Gt(Real128_Inf, Real128_Zero));
	ASSERT(!Real128_Gt(Real128_NegInf, Real128_Zero));
	ASSERT(Real128_Gt(Real128_Zero, Real128_NegInf));
}
END_TEST

START_TEST(CanCompareLessThanOrEqualReal128)
{
	ASSERT(!Real128_Le(Real128_NegNaN, Real128_NegNaN));
	ASSERT(!Real128_Le(Real128_NaN, Real128_NaN));
	ASSERT(!Real128_Le(Real128_NegNaN, Real128_NaN));
	ASSERT(!Real128_Le(Real128_NaN, Real128_NegNaN));
	ASSERT(!Real128_Le(Real128_One, Real128_NaN));
	ASSERT(!Real128_Le(Real128_NaN, Real128_One));

	ASSERT(Real128_Le(Real128_NegZero, Real128_NegZero));
	ASSERT(Real128_Le(Real128_Zero, Real128_Zero));
	ASSERT(Real128_Le(Real128_NegZero, Real128_Zero));
	ASSERT(Real128_Le(Real128_Zero, Real128_NegZero));

	ASSERT(Real128_Le(Real128_NegInf, Real128_NegInf));
	ASSERT(Real128_Le(Real128_NegOne, Real128_NegOne));
	ASSERT(Real128_Le(Real128_One, Real128_One));
	ASSERT(Real128_Le(Real128_Inf, Real128_Inf));

	ASSERT(Real128_Le(Real128_NegInf, Real128_Inf));
	ASSERT(!Real128_Le(Real128_Inf, Real128_NegInf));
	ASSERT(Real128_Le(Real128_NegOne, Real128_One));
	ASSERT(!Real128_Le(Real128_One, Real128_NegOne));
	ASSERT(Real128_Le(Real128_One, Real128_Two));
	ASSERT(!Real128_Le(Real128_Two, Real128_One));
	ASSERT(Real128_Le(Real128_Zero, Real128_Inf));
	ASSERT(!Real128_Le(Real128_Inf, Real128_Zero));
	ASSERT(Real128_Le(Real128_NegInf, Real128_Zero));
	ASSERT(!Real128_Le(Real128_Zero, Real128_NegInf));
}
END_TEST

START_TEST(CanCompareGreaterThanOrEqualReal128)
{
	ASSERT(!Real128_Ge(Real128_NegNaN, Real128_NegNaN));
	ASSERT(!Real128_Ge(Real128_NaN, Real128_NaN));
	ASSERT(!Real128_Ge(Real128_NegNaN, Real128_NaN));
	ASSERT(!Real128_Ge(Real128_NaN, Real128_NegNaN));
	ASSERT(!Real128_Ge(Real128_One, Real128_NaN));
	ASSERT(!Real128_Ge(Real128_NaN, Real128_One));

	ASSERT(Real128_Ge(Real128_NegZero, Real128_NegZero));
	ASSERT(Real128_Ge(Real128_Zero, Real128_Zero));
	ASSERT(Real128_Ge(Real128_NegZero, Real128_Zero));
	ASSERT(Real128_Ge(Real128_Zero, Real128_NegZero));

	ASSERT(Real128_Ge(Real128_NegInf, Real128_NegInf));
	ASSERT(Real128_Ge(Real128_NegOne, Real128_NegOne));
	ASSERT(Real128_Ge(Real128_One, Real128_One));
	ASSERT(Real128_Ge(Real128_Inf, Real128_Inf));

	ASSERT(!Real128_Ge(Real128_NegInf, Real128_Inf));
	ASSERT(Real128_Ge(Real128_Inf, Real128_NegInf));
	ASSERT(!Real128_Ge(Real128_NegOne, Real128_One));
	ASSERT(Real128_Ge(Real128_One, Real128_NegOne));
	ASSERT(!Real128_Ge(Real128_One, Real128_Two));
	ASSERT(Real128_Ge(Real128_Two, Real128_One));
	ASSERT(!Real128_Ge(Real128_Zero, Real128_Inf));
	ASSERT(Real128_Ge(Real128_Inf, Real128_Zero));
	ASSERT(!Real128_Ge(Real128_NegInf, Real128_Zero));
	ASSERT(Real128_Ge(Real128_Zero, Real128_NegInf));
}
END_TEST

START_TEST(CanCompareOrderedReal128)
{
	ASSERT(Real128_Compare(Real128_NegNaN, Real128_NegNaN) == 0);
	ASSERT(Real128_Compare(Real128_NegNaN, Real128_NegInf) < 0);
	ASSERT(Real128_Compare(Real128_NegNaN, Real128_NegOne) < 0);
	ASSERT(Real128_Compare(Real128_NegNaN, Real128_Zero) < 0);
	ASSERT(Real128_Compare(Real128_NegNaN, Real128_One) < 0);
	ASSERT(Real128_Compare(Real128_NegNaN, Real128_Inf) < 0);
	ASSERT(Real128_Compare(Real128_NegNaN, Real128_NaN) < 0);

	ASSERT(Real128_Compare(Real128_NegInf, Real128_NegNaN) > 0);
	ASSERT(Real128_Compare(Real128_NegInf, Real128_NegInf) == 0);
	ASSERT(Real128_Compare(Real128_NegInf, Real128_NegOne) < 0);
	ASSERT(Real128_Compare(Real128_NegInf, Real128_Zero) < 0);
	ASSERT(Real128_Compare(Real128_NegInf, Real128_One) < 0);
	ASSERT(Real128_Compare(Real128_NegInf, Real128_Inf) < 0);
	ASSERT(Real128_Compare(Real128_NegInf, Real128_NaN) < 0);

	ASSERT(Real128_Compare(Real128_NegOne, Real128_NegNaN) > 0);
	ASSERT(Real128_Compare(Real128_NegOne, Real128_NegInf) > 0);
	ASSERT(Real128_Compare(Real128_NegOne, Real128_NegOne) == 0);
	ASSERT(Real128_Compare(Real128_NegOne, Real128_Zero) < 0);
	ASSERT(Real128_Compare(Real128_NegOne, Real128_One) < 0);
	ASSERT(Real128_Compare(Real128_NegOne, Real128_Inf) < 0);
	ASSERT(Real128_Compare(Real128_NegOne, Real128_NaN) < 0);

	ASSERT(Real128_Compare(Real128_Zero, Real128_NegNaN) > 0);
	ASSERT(Real128_Compare(Real128_Zero, Real128_NegInf) > 0);
	ASSERT(Real128_Compare(Real128_Zero, Real128_NegOne) > 0);
	ASSERT(Real128_Compare(Real128_Zero, Real128_Zero) == 0);
	ASSERT(Real128_Compare(Real128_Zero, Real128_One) < 0);
	ASSERT(Real128_Compare(Real128_Zero, Real128_Inf) < 0);
	ASSERT(Real128_Compare(Real128_Zero, Real128_NaN) < 0);

	ASSERT(Real128_Compare(Real128_One, Real128_NegNaN) > 0);
	ASSERT(Real128_Compare(Real128_One, Real128_NegInf) > 0);
	ASSERT(Real128_Compare(Real128_One, Real128_NegOne) > 0);
	ASSERT(Real128_Compare(Real128_One, Real128_Zero) > 0);
	ASSERT(Real128_Compare(Real128_One, Real128_One) == 0);
	ASSERT(Real128_Compare(Real128_One, Real128_Inf) < 0);
	ASSERT(Real128_Compare(Real128_One, Real128_NaN) < 0);

	ASSERT(Real128_Compare(Real128_Inf, Real128_NegNaN) > 0);
	ASSERT(Real128_Compare(Real128_Inf, Real128_NegInf) > 0);
	ASSERT(Real128_Compare(Real128_Inf, Real128_NegOne) > 0);
	ASSERT(Real128_Compare(Real128_Inf, Real128_Zero) > 0);
	ASSERT(Real128_Compare(Real128_Inf, Real128_One) > 0);
	ASSERT(Real128_Compare(Real128_Inf, Real128_Inf) == 0);
	ASSERT(Real128_Compare(Real128_Inf, Real128_NaN) < 0);

	ASSERT(Real128_Compare(Real128_NaN, Real128_NegNaN) > 0);
	ASSERT(Real128_Compare(Real128_NaN, Real128_NegInf) > 0);
	ASSERT(Real128_Compare(Real128_NaN, Real128_NegOne) > 0);
	ASSERT(Real128_Compare(Real128_NaN, Real128_Zero) > 0);
	ASSERT(Real128_Compare(Real128_NaN, Real128_One) > 0);
	ASSERT(Real128_Compare(Real128_NaN, Real128_Inf) > 0);
	ASSERT(Real128_Compare(Real128_NaN, Real128_NaN) == 0);
}
END_TEST

START_TEST(CanDetermineOrderabilityReal128)
{
	ASSERT(!Real128_IsOrderable(Real128_NegNaN, Real128_NegNaN));
	ASSERT(!Real128_IsOrderable(Real128_NaN, Real128_NaN));
	ASSERT(!Real128_IsOrderable(Real128_NegNaN, Real128_NaN));
	ASSERT(!Real128_IsOrderable(Real128_NaN, Real128_NegNaN));
	ASSERT(!Real128_IsOrderable(Real128_One, Real128_NaN));
	ASSERT(!Real128_IsOrderable(Real128_NaN, Real128_One));

	ASSERT(Real128_IsOrderable(Real128_NegZero, Real128_NegZero));
	ASSERT(Real128_IsOrderable(Real128_Zero, Real128_Zero));
	ASSERT(Real128_IsOrderable(Real128_NegZero, Real128_Zero));
	ASSERT(Real128_IsOrderable(Real128_Zero, Real128_NegZero));

	ASSERT(Real128_IsOrderable(Real128_NegInf, Real128_NegInf));
	ASSERT(Real128_IsOrderable(Real128_NegOne, Real128_NegOne));
	ASSERT(Real128_IsOrderable(Real128_One, Real128_One));
	ASSERT(Real128_IsOrderable(Real128_Inf, Real128_Inf));

	ASSERT(Real128_IsOrderable(Real128_NegInf, Real128_Inf));
	ASSERT(Real128_IsOrderable(Real128_Inf, Real128_NegInf));
	ASSERT(Real128_IsOrderable(Real128_NegOne, Real128_One));
	ASSERT(Real128_IsOrderable(Real128_One, Real128_NegOne));
	ASSERT(Real128_IsOrderable(Real128_One, Real128_Two));
	ASSERT(Real128_IsOrderable(Real128_Two, Real128_One));
	ASSERT(Real128_IsOrderable(Real128_Zero, Real128_Inf));
	ASSERT(Real128_IsOrderable(Real128_Inf, Real128_Zero));
	ASSERT(Real128_IsOrderable(Real128_NegInf, Real128_Zero));
	ASSERT(Real128_IsOrderable(Real128_Zero, Real128_NegInf));
}
END_TEST

#include "real128_tests.generated.inc"
