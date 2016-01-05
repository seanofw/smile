//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2016 Sean Werkema
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

TEST_SUITE(Real32Tests)

//-------------------------------------------------------------------------------------------------
//  Helper functions and macros.

#define RI(__n__) Real32_FromInt32(__n__)
#define RL(__n__) Real32_FromInt64(__n__)
#define RF(__n__) Real32_FromFloat32(__n__)
#define RD(__n__) Real32_FromFloat64(__n__)

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
/// Convert a string of hex characters in high-to-low order into a Real32.
/// </summary>
static Real32 RR(const char *str)
{
	// TODO: This only works properly on little-endian architectures.

	union {
		Byte buffer[4];
		Real32 real;
	} u;
	Byte *bufptr = u.buffer;
	Bool highNybble = False;
	const char *src = str;

	while (*src) src++;

	while (src >= str && bufptr < u.buffer + 4) {
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
	while (bufptr < u.buffer + 4) {
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

Inline Eq(Real32 a, Real32 b)
{
	return a.value == b.value;
}

//-------------------------------------------------------------------------------------------------
//  Static constants.

// Common limits.
static const Real32 _int32Max = { { 0x3420C49CUL } };
static const Real32 _int32Min = { { 0xB420C49CUL } };
static const Real32 _int64Max = { { 0x6E2CBCCCUL } };
static const Real32 _int64Min = { { 0xEE2CBCCCUL } };

// Miscellaneous integers.
static const Real32 _fiveSevenNine = { { 0x32800243UL } };
static const Real32 _fiveSevenNineOhOh = { { 0x3180E22CUL } };
static const Real32 _shortBigPi = { { 0x33AFEFD9UL } };
static const Real32 _longBigPi = { { 0x382FEFD9UL } };
static const Real32 _longMediumPi = { { 0x312FEFD9UL } };
static const Real32 _pi = { { 0x2FAFEFD9UL } };
static const Real32 _longNegMediumPi = { { 0xB12FEFD9UL } };
static const Real32 _negPi = { { 0xAFAFEFD9UL } };

//-------------------------------------------------------------------------------------------------
//  Type-Conversion Tests.
//
//  Some of these are ganked straight from Intel's tests included with their BID library.

START_TEST(CanConvertInt32ToReal32)
{
	ASSERT(Eq(RI(0), RR("32800000")));
	ASSERT(Eq(RI(100000000), RR("338f4240")));
	ASSERT(Eq(RI(11111111), RR("3310f447")));
	ASSERT(Eq(RI(1), RR("32800001")));
	ASSERT(Eq(RI(-1), RR("b2800001")));
	ASSERT(Eq(RI(2147483647), RR("3420c49c")));
	ASSERT(Eq(RI((UInt32)(-2147483648LL)), RR("b420c49c")));
	ASSERT(Eq(RI(32767), RR("32807fff")));
	ASSERT(Eq(RI(-32767), RR("b2807fff")));
	ASSERT(Eq(RI(32768), RR("32808000")));
	ASSERT(Eq(RI(-32768), RR("b2808000")));
	ASSERT(Eq(RI(32769), RR("32808001")));
	ASSERT(Eq(RI(-32769), RR("b2808001")));
	ASSERT(Eq(RI(65534), RR("3280fffe")));
	ASSERT(Eq(RI(-65534), RR("b280fffe")));
	ASSERT(Eq(RI(65535), RR("3280ffff")));
	ASSERT(Eq(RI(-65535), RR("b280ffff")));
	ASSERT(Eq(RI(65536), RR("32810000")));
	ASSERT(Eq(RI(-65536), RR("b2810000")));
	ASSERT(Eq(RI(693127475), RR("33e9c34b")));
	ASSERT(Eq(RI(-937230081), RR("ecef028d")));
	ASSERT(Eq(RI(9999999), RR("6cb8967f")));
}
END_TEST

START_TEST(CanConvertInt64ToReal32)
{
	ASSERT(Eq(RL(0), RR("32800000")));
	ASSERT(Eq(RL(10000000000000000), RR("378f4240")));
	ASSERT(Eq(RL(1000000000000000), RR("370f4240")));
	ASSERT(Eq(RL(100000000000000), RR("368f4240")));
	ASSERT(Eq(RL(10000000000000), RR("360f4240")));
	ASSERT(Eq(RL(1000000000000), RR("358f4240")));
	ASSERT(Eq(RL(100000000000), RR("350f4240")));
	ASSERT(Eq(RL(10000000000), RR("348f4240")));
	ASSERT(Eq(RL(100000000), RR("338f4240")));
	ASSERT(Eq(RL(11111111), RR("3310f447")));
	ASSERT(Eq(RL(1), RR("32800001")));
	ASSERT(Eq(RL(-134217729), RR("b3947ae1")));
	ASSERT(Eq(RL(-17592722915393), RR("b61ad828")));
	ASSERT(Eq(RL(-1), RR("b2800001")));
	ASSERT(Eq(RL(2147483647), RR("3420c49c")));
	ASSERT(Eq(RL(2147483648), RR("3420c49c")));
	ASSERT(Eq(RL(-2147483648LL), RR("b420c49c")));
	ASSERT(Eq(RL(-22523495699198977), RR("b7a25e3e")));
	ASSERT(Eq(RL(32767), RR("32807fff")));
	ASSERT(Eq(RL(-32767), RR("b2807fff")));
	ASSERT(Eq(RL(32768), RR("32808000")));
	ASSERT(Eq(RL(-32768), RR("b2808000")));
	ASSERT(Eq(RL(32769), RR("32808001")));
	ASSERT(Eq(RL(-32769), RR("b2808001")));
	ASSERT(Eq(RL(34368127232), RR("34b4710d")));
	ASSERT(Eq(RL(362540080113918042), RR("383751b9")));
	ASSERT(Eq(RL(4294967295), RR("34418937")));
	ASSERT(Eq(RL(-4294967295LL), RR("b4418937")));
	ASSERT(Eq(RL(4294967297), RR("34418937")));
	ASSERT(Eq(RL(-4294967297), RR("b4418937")));
	ASSERT(Eq(RL(-4398205899137), RR("b5c31c7e")));
	ASSERT(Eq(RL(-4503599628435553), RR("b744b830")));
	ASSERT(Eq(RL(-493149287878913), RR("b6cb3fa5")));
	ASSERT(Eq(RL(-549755814017), RR("b553e2d6")));
	ASSERT(Eq(RL(65534), RR("3280fffe")));
	ASSERT(Eq(RL(-65534), RR("b280fffe")));
	ASSERT(Eq(RL(65535), RR("3280ffff")));
	ASSERT(Eq(RL(-65535), RR("b280ffff")));
	ASSERT(Eq(RL(65536), RR("32810000")));
	ASSERT(Eq(RL(-65536), RR("b2810000")));
	ASSERT(Eq(RL(9223372036854775806), RR("6e2cbccc")));
	ASSERT(Eq(RL(-9223372036854775806), RR("ee2cbccc")));
	ASSERT(Eq(RL(9223372036854775807), RR("6e2cbccc")));
	ASSERT(Eq(RL(-9223372036854775807), RR("ee2cbccc")));
	ASSERT(Eq(RL(9999999), RR("6cb8967f")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Parsing Tests.

START_TEST(CanParseReal32)
{
	Real32 result;

	ASSERT(Real32_TryParse(String_FromC("579"), &result));
	ASSERT(Eq(result, _fiveSevenNine));

	ASSERT(Real32_TryParse(String_FromC("579.00"), &result));
	ASSERT(Eq(result, _fiveSevenNineOhOh));

	ASSERT(Real32_TryParse(String_FromC("314159265358979323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real32_TryParse(String_FromC("314_159_265_358_979_323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real32_TryParse(String_FromC("314'159'265'358'979'323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real32_TryParse(String_FromC("314'159\"265'358\"979'323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real32_TryParse(String_FromC("-3'141.59265"), &result));
	ASSERT(Eq(result, _longNegMediumPi));

	ASSERT(Real32_TryParse(String_FromC("3.14159'265"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real32_TryParse(String_FromC("-3.14159'265"), &result));
	ASSERT(Eq(result, _negPi));

	ASSERT(Real32_TryParse(String_FromC("+3.14159'265"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real32_TryParse(String_FromC("+3.14159'265E+3"), &result));
	ASSERT(Eq(result, _longMediumPi));

	ASSERT(Real32_TryParse(String_FromC("-3.14159'265E+3"), &result));
	ASSERT(Eq(result, _longNegMediumPi));

	ASSERT(Real32_TryParse(String_FromC("+314'159'265.358'979'323E-8"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real32_TryParse(String_FromC("-314'159'265.358'979'323E-8"), &result));
	ASSERT(Eq(result, _negPi));

	ASSERT(Real32_TryParse(String_FromC("+inf"), &result));
	ASSERT(Eq(result, Real32_Inf));

	ASSERT(Real32_TryParse(String_FromC("inf"), &result));
	ASSERT(Eq(result, Real32_Inf));
	ASSERT(Real32_TryParse(String_FromC("INF"), &result));
	ASSERT(Eq(result, Real32_Inf));
	ASSERT(Real32_TryParse(String_FromC("Inf"), &result));
	ASSERT(Eq(result, Real32_Inf));

	ASSERT(Real32_TryParse(String_FromC("-inf"), &result));
	ASSERT(Eq(result, Real32_NegInf));

	ASSERT(Real32_TryParse(String_FromC("+nan"), &result));
	ASSERT(Real32_IsNaN(result));
	ASSERT(!Real32_IsNeg(result));

	ASSERT(Real32_TryParse(String_FromC("nan"), &result));
	ASSERT(Real32_IsNaN(result));
	ASSERT(!Real32_IsNeg(result));
	ASSERT(Real32_TryParse(String_FromC("NAN"), &result));
	ASSERT(Real32_IsNaN(result));
	ASSERT(!Real32_IsNeg(result));
	ASSERT(Real32_TryParse(String_FromC("NaN"), &result));
	ASSERT(Real32_IsNaN(result));
	ASSERT(!Real32_IsNeg(result));

	ASSERT(Real32_TryParse(String_FromC("-nan"), &result));
	ASSERT(Real32_IsNaN(result));
	ASSERT(Real32_IsNeg(result));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Stringification Tests.

START_TEST(CanStringifyReal32NumbersInExponentialForm)
{
	ASSERT_STRING(Real32_ToExpString(_pi, 1, False), "3.141593e+0", 11);
	ASSERT_STRING(Real32_ToExpString(_pi, 1, True), "+3.141593e+0", 12);
	ASSERT_STRING(Real32_ToExpString(_negPi, 1, False), "-3.141593e+0", 12);

	ASSERT_STRING(Real32_ToExpString(_longMediumPi, 1, False), "3.141593e+3", 11);
	ASSERT_STRING(Real32_ToExpString(_longNegMediumPi, 1, False), "-3.141593e+3", 12);

	ASSERT_STRING(Real32_ToExpString(RD(12345000000), 1, False), "1.234500e+10", 12);
	ASSERT_STRING(Real32_ToExpString(RD(-12345000000), 1, False), "-1.234500e+10", 13);

	ASSERT_STRING(Real32_ToExpString(RD(12345000000), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real32_ToExpString(RD(-12345000000), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real32_ToExpString(Real32_ParseC("12345000000"), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real32_ToExpString(Real32_ParseC("-12345000000"), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real32_ToExpString(Real32_ParseC("1.2345E+10"), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real32_ToExpString(Real32_ParseC("-1.2345E+10"), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real32_ToExpString(Real32_ParseC("1.2345E+10"), 1, False), "1.2345e+10", 10);
	ASSERT_STRING(Real32_ToExpString(Real32_ParseC("-1.2345E+10"), 1, False), "-1.2345e+10", 11);

	ASSERT_STRING(Real32_ToExpString(RD(0.125), 1, False), "1.25e-1", 7);
	ASSERT_STRING(Real32_ToExpString(RD(-0.125), 1, False), "-1.25e-1", 8);

	ASSERT_STRING(Real32_ToExpString(Real32_ParseC("0.125000"), 1, False), "1.25000e-1", 10);
	ASSERT_STRING(Real32_ToExpString(Real32_ParseC("-0.125000"), 1, False), "-1.25000e-1", 11);

	ASSERT_STRING(Real32_ToExpString(_fiveSevenNineOhOh, 1, False), "5.7900e+2", 9);
	ASSERT_STRING(Real32_ToExpString(_fiveSevenNineOhOh, 2, False), "5.7900e+2", 9);
	ASSERT_STRING(Real32_ToExpString(_fiveSevenNineOhOh, 3, False), "5.7900e+2", 9);
	ASSERT_STRING(Real32_ToExpString(_fiveSevenNineOhOh, 4, False), "5.7900e+2", 9);
	ASSERT_STRING(Real32_ToExpString(_fiveSevenNineOhOh, 5, False), "5.79000e+2", 10);
	ASSERT_STRING(Real32_ToExpString(_fiveSevenNineOhOh, 6, False), "5.790000e+2", 11);
}
END_TEST

START_TEST(CanStringifySpecialReal32ValuesInExponentialForm)
{
	ASSERT_STRING(Real32_ToExpString(Real32_Zero, 0, False), "0", 1);
	ASSERT_STRING(Real32_ToExpString(Real32_Zero, 0, True), "+0", 2);
	ASSERT_STRING(Real32_ToExpString(Real32_Zero, 5, False), "0.00000", 7);
	ASSERT_STRING(Real32_ToExpString(Real32_Zero, 5, True), "+0.00000", 8);

	ASSERT_STRING(Real32_ToExpString(Real32_One, 0, False), "1e+0", 4);
	ASSERT_STRING(Real32_ToExpString(Real32_One, 0, True), "+1e+0", 5);
	ASSERT_STRING(Real32_ToExpString(Real32_One, 5, False), "1.00000e+0", 10);
	ASSERT_STRING(Real32_ToExpString(Real32_One, 5, True), "+1.00000e+0", 11);

	ASSERT_STRING(Real32_ToExpString(Real32_Inf, 1, False), "inf", 3);
	ASSERT_STRING(Real32_ToExpString(Real32_Inf, 1, True), "+inf", 4);
	ASSERT_STRING(Real32_ToExpString(Real32_NegInf, 1, False), "-inf", 4);

	ASSERT_STRING(Real32_ToExpString(Real32_NaN, 1, False), "NaN", 3);
	ASSERT_STRING(Real32_ToExpString(Real32_NaN, 1, True), "+NaN", 4);
	ASSERT_STRING(Real32_ToExpString(Real32_NegNaN, 1, False), "-NaN", 4);
}
END_TEST

START_TEST(CanStringifyReal32NumbersInFixedForm)
{
	// Things split across the decimal point.
	ASSERT_STRING(Real32_ToFixedString(_pi, 1, 1, False), "3.141593", 8);
	ASSERT_STRING(Real32_ToFixedString(_pi, 1, 1, True), "+3.141593", 9);
	ASSERT_STRING(Real32_ToFixedString(_negPi, 1, 1, False), "-3.141593", 9);

	ASSERT_STRING(Real32_ToFixedString(_longMediumPi, 1, 1, False), "3141.593", 8);
	ASSERT_STRING(Real32_ToFixedString(_longMediumPi, 1, 1, True), "+3141.593", 9);
	ASSERT_STRING(Real32_ToFixedString(_longNegMediumPi, 1, 1, False), "-3141.593", 9);

	// Things that only exist after the decimal point.
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("0.125000"), 1, 1, False), "0.125000", 8);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("-0.125000"), 1, 1, False), "-0.125000", 9);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("125e-5"), 1, 1, False), "0.00125", 7);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("-125e-5"), 1, 1, False), "-0.00125", 8);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("125e-5"), 0, 0, False), ".00125", 6);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("-125e-5"), 0, 0, False), "-.00125", 7);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("125e-5"), 3, 7, False), "000.0012500", 11);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("-125e-5"), 3, 7, False), "-000.0012500", 12);

	// Things that only exist before the decimal point.
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("125000"), 1, 1, False), "125000.0", 8);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("-125000"), 1, 1, False), "-125000.0", 9);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("125e+3"), 1, 1, False), "125000.0", 8);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("-125e+3"), 1, 1, False), "-125000.0", 9);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("125e+3"), 0, 0, False), "125000", 6);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("-125e+3"), 0, 0, False), "-125000", 7);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("125e+3"), 10, 3, False), "0000125000.000", 14);
	ASSERT_STRING(Real32_ToFixedString(Real32_ParseC("-125e+3"), 10, 3, False), "-0000125000.000", 15);
}
END_TEST

START_TEST(CanStringifySpecialReal32ValuesInFixedForm)
{
	ASSERT_STRING(Real32_ToFixedString(Real32_Zero, 0, 0, False), "0", 1);
	ASSERT_STRING(Real32_ToFixedString(Real32_Zero, 0, 0, True), "+0", 2);
	ASSERT_STRING(Real32_ToFixedString(Real32_Zero, 5, 5, False), "00000.00000", 11);
	ASSERT_STRING(Real32_ToFixedString(Real32_Zero, 5, 5, True), "+00000.00000", 12);

	ASSERT_STRING(Real32_ToFixedString(Real32_One, 0, 0, False), "1", 1);
	ASSERT_STRING(Real32_ToFixedString(Real32_One, 0, 0, True), "+1", 2);
	ASSERT_STRING(Real32_ToFixedString(Real32_One, 5, 5, False), "00001.00000", 11);
	ASSERT_STRING(Real32_ToFixedString(Real32_One, 5, 5, True), "+00001.00000", 12);

	ASSERT_STRING(Real32_ToFixedString(Real32_Inf, 0, 0, False), "inf", 3);
	ASSERT_STRING(Real32_ToFixedString(Real32_Inf, 0, 0, True), "+inf", 4);
	ASSERT_STRING(Real32_ToFixedString(Real32_NegInf, 0, 0, False), "-inf", 4);

	ASSERT_STRING(Real32_ToFixedString(Real32_NaN, 0, 0, False), "NaN", 3);
	ASSERT_STRING(Real32_ToFixedString(Real32_NaN, 0, 0, True), "+NaN", 4);
	ASSERT_STRING(Real32_ToFixedString(Real32_NegNaN, 0, 0, False), "-NaN", 4);
}
END_TEST

START_TEST(CanStringifyReal32NumbersGenerically)
{
	// Things split across the decimal point.
	ASSERT_STRING(Real32_ToString(_pi), "3.141593", 8);
	ASSERT_STRING(Real32_ToString(_negPi), "-3.141593", 9);

	ASSERT_STRING(Real32_ToString(_longMediumPi), "3141.593", 8);
	ASSERT_STRING(Real32_ToString(_longNegMediumPi), "-3141.593", 9);

	// Things that only exist after the decimal point.
	ASSERT_STRING(Real32_ToString(Real32_ParseC("0.125000")), ".125000", 7);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("-0.125000")), "-.125000", 8);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("125e-5")), ".00125", 6);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("-125e-5")), "-.00125", 7);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("125e-10")), "1.25e-8", 7);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("-125e-10")), "-1.25e-8", 8);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("1250e-11")), "1.250e-8", 8);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("-1250e-11")), "-1.250e-8", 9);

	// Things that only exist before the decimal point.
	ASSERT_STRING(Real32_ToString(Real32_ParseC("125000")), "125000", 6);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("-125000")), "-125000", 7);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("125e+3")), "125000", 6);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("-125e+3")), "-125000", 7);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("125e+10")), "1.25e+12", 8);
	ASSERT_STRING(Real32_ToString(Real32_ParseC("-125e+10")), "-1.25e+12", 9);
}
END_TEST

START_TEST(CanStringifySpecialReal32ValuesGenerically)
{
	ASSERT_STRING(Real32_ToString(Real32_Zero), "0", 1);
	ASSERT_STRING(Real32_ToString(Real32_One), "1", 1);

	ASSERT_STRING(Real32_ToString(Real32_Inf), "inf", 3);
	ASSERT_STRING(Real32_ToString(Real32_NegInf), "-inf", 4);

	ASSERT_STRING(Real32_ToString(Real32_NaN), "NaN", 3);
	ASSERT_STRING(Real32_ToString(Real32_NegNaN), "-NaN", 4);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Arithmetic Tests.

START_TEST(CanAddReal32)
{
	ASSERT(Eq(Real32_Add(RI(123), RI(456)), RI(579)));
	ASSERT(Eq(Real32_Add(RD(1.5), RD(0.125)), RD(1.625)));
	ASSERT(Eq(Real32_Add(RD(1.5), RD(-3.0)), RD(-1.5)));
}
END_TEST

START_TEST(CanMultiplyReal32)
{
	ASSERT(Eq(Real32_Mul(RI(123), RI(456)), RI(56088)));
	ASSERT(Eq(Real32_Mul(RD(1.5), RD(0.125)), RD(0.1875)));
	ASSERT(Eq(Real32_Mul(RD(1.5), RD(-3.0)), RD(-4.5)));
}
END_TEST

START_TEST(CanSubtractReal32)
{
	ASSERT(Eq(Real32_Sub(RI(123), RI(456)), RI(-333)));
	ASSERT(Eq(Real32_Sub(RD(1.5), RD(0.125)), RD(1.375)));
	ASSERT(Eq(Real32_Sub(RD(1.5), RD(-3.0)), RD(4.5)));
}
END_TEST

START_TEST(CanDivideReal32)
{
	ASSERT(Eq(Real32_Div(RI(123), RI(456)), Real32_ParseC(".2697368421052631578947368421052632")));
	ASSERT(Eq(Real32_Div(RD(1.5), RD(0.125)), RI(12)));
	ASSERT(Eq(Real32_Div(RD(1.5), RD(-3.0)), RD(-0.5)));
}
END_TEST

START_TEST(CanModulusReal32)
{
	ASSERT(Eq(Real32_Mod(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real32_Mod(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real32_Mod(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real32_Mod(RI(-456), RI(123)), RI(87)));
	ASSERT(Eq(Real32_Mod(RI(456), RI(-123)), RI(-87)));
	ASSERT(Eq(Real32_Mod(RI(-456), RI(-123)), RI(-87)));
}
END_TEST

START_TEST(CanRemainderReal32)
{
	ASSERT(Eq(Real32_Rem(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real32_Rem(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real32_Rem(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real32_Rem(RI(-456), RI(123)), RI(-87)));
	ASSERT(Eq(Real32_Rem(RI(456), RI(-123)), RI(87)));
	ASSERT(Eq(Real32_Rem(RI(-456), RI(-123)), RI(-87)));
}
END_TEST

START_TEST(CanIeeeRemainderReal32)
{
	ASSERT(Eq(Real32_IeeeRem(RI(456), RI(123)), RI(-36)));
	ASSERT(Eq(Real32_IeeeRem(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real32_IeeeRem(RI(456), RI(123)), RI(-36)));
	ASSERT(Eq(Real32_IeeeRem(RI(-456), RI(123)), RI(36)));
	ASSERT(Eq(Real32_IeeeRem(RI(456), RI(-123)), RI(-36)));
	ASSERT(Eq(Real32_IeeeRem(RI(-456), RI(-123)), RI(36)));
}
END_TEST

START_TEST(CanNegateReal32)
{
	ASSERT(Eq(Real32_Neg(RI(123)), RI(-123)));
	ASSERT(Eq(Real32_Neg(RI(-123)), RI(123)));

	ASSERT(Eq(Real32_Neg(Real32_Zero), Real32_NegZero));
	ASSERT(Eq(Real32_Neg(Real32_NegZero), Real32_Zero));

	ASSERT(Eq(Real32_Neg(Real32_Inf), Real32_NegInf));
	ASSERT(Eq(Real32_Neg(Real32_NegInf), Real32_Inf));
}
END_TEST

START_TEST(CanAbsoluteReal32)
{
	ASSERT(Eq(Real32_Abs(RI(123)), RI(123)));
	ASSERT(Eq(Real32_Abs(RI(-123)), RI(123)));

	ASSERT(Eq(Real32_Abs(Real32_Zero), Real32_Zero));
	ASSERT(Eq(Real32_Abs(Real32_NegZero), Real32_Zero));

	ASSERT(Eq(Real32_Abs(Real32_Inf), Real32_Inf));
	ASSERT(Eq(Real32_Abs(Real32_NegInf), Real32_Inf));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Powers and logarithms.

START_TEST(CanSqrtReal32)
{
	ASSERT(Eq(Real32_Sqrt(Real32_Zero), Real32_Zero));
	ASSERT(Eq(Real32_Sqrt(Real32_NegZero), Real32_NegZero));		// Weird, but correct.
	ASSERT(Eq(Real32_Sqrt(Real32_One), Real32_One));
	ASSERT(Eq(Real32_Sqrt(Real32_Inf), Real32_Inf));

	ASSERT(Eq(Real32_Sqrt(RI(256)), RI(16)));
	ASSERT(Eq(Real32_Sqrt(RI(25)), RI(5)));
	ASSERT(Eq(Real32_Sqrt(Real32_ParseC("18'446'744'065'119'617'025")), Real32_ParseC("4'294'967'295")));
	ASSERT(Eq(Real32_Sqrt(RI(123)), Real32_ParseC("11.09053'65064'09417'16205'16001'02609'93")));

	ASSERT(Eq(Real32_Sqrt(Real32_NegOne), Real32_NaN));
	ASSERT(Eq(Real32_Sqrt(RI(-123)), Real32_NaN));
	ASSERT(Eq(Real32_Sqrt(Real32_NegInf), Real32_NaN));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Rounding Tests.

START_TEST(CanFloorReal32)
{
	ASSERT(Eq(Real32_Floor(RI(123)), RI(123)));
	ASSERT(Eq(Real32_Floor(RI(-123)), RI(-123)));

	ASSERT(Eq(Real32_Floor(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real32_Floor(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real32_Floor(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real32_Floor(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real32_Floor(RD(-1.2)), RD(-2.0)));
	ASSERT(Eq(Real32_Floor(RD(1.8)), RD(1.0)));
	ASSERT(Eq(Real32_Floor(RD(-1.8)), RD(-2.0)));
	ASSERT(Eq(Real32_Floor(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real32_Floor(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanCeilReal32)
{
	ASSERT(Eq(Real32_Ceil(RI(123)), RI(123)));
	ASSERT(Eq(Real32_Ceil(RI(-123)), RI(-123)));

	ASSERT(Eq(Real32_Ceil(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real32_Ceil(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real32_Ceil(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real32_Ceil(RD(1.2)), RD(2.0)));
	ASSERT(Eq(Real32_Ceil(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real32_Ceil(RD(1.8)), RD(2.0)));
	ASSERT(Eq(Real32_Ceil(RD(-1.8)), RD(-1.0)));
	ASSERT(Eq(Real32_Ceil(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real32_Ceil(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanTruncReal32)
{
	ASSERT(Eq(Real32_Trunc(RI(123)), RI(123)));
	ASSERT(Eq(Real32_Trunc(RI(-123)), RI(-123)));

	ASSERT(Eq(Real32_Trunc(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real32_Trunc(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real32_Trunc(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real32_Trunc(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real32_Trunc(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real32_Trunc(RD(1.8)), RD(1.0)));
	ASSERT(Eq(Real32_Trunc(RD(-1.8)), RD(-1.0)));
	ASSERT(Eq(Real32_Trunc(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real32_Trunc(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanSplitReal32WithModf)
{
	Real32 intPart;

	ASSERT(Eq(Real32_Modf(RI(123), &intPart), Real32_Zero));
	ASSERT(Eq(intPart, RI(123)));
	ASSERT(Eq(Real32_Modf(RI(-123), &intPart), Real32_NegZero));
	ASSERT(Eq(intPart, RI(-123)));

	ASSERT(Eq(Real32_Modf(RD(0.0), &intPart), Real32_Zero));
	ASSERT(Eq(intPart, RI(0)));
	ASSERT(Eq(Real32_Modf(RD(1.0), &intPart), Real32_Zero));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real32_Modf(RD(-1.0), &intPart), Real32_NegZero));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real32_Modf(Real32_ParseC("1.2"), &intPart), Real32_ParseC(".2")));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real32_Modf(Real32_ParseC("-1.2"), &intPart), Real32_ParseC("-.2")));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real32_Modf(Real32_ParseC("1.8"), &intPart), Real32_ParseC(".8")));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real32_Modf(Real32_ParseC("-1.8"), &intPart), Real32_ParseC("-.8")));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real32_Modf(RD(2.0), &intPart), Real32_Zero));
	ASSERT(Eq(intPart, RI(2)));
	ASSERT(Eq(Real32_Modf(RD(-2.0), &intPart), Real32_NegZero));
	ASSERT(Eq(intPart, RI(-2)));
}
END_TEST

START_TEST(CanRoundReal32)
{
	ASSERT(Eq(Real32_Round(RI(123)), RI(123)));
	ASSERT(Eq(Real32_Round(RI(-123)), RI(-123)));

	ASSERT(Eq(Real32_Round(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real32_Round(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real32_Round(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real32_Round(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real32_Round(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real32_Round(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real32_Round(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real32_Round(RD(1.8)), RD(2.0)));
	ASSERT(Eq(Real32_Round(RD(-1.8)), RD(-2.0)));
	ASSERT(Eq(Real32_Round(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real32_Round(RD(-2.0)), RD(-2.0)));

	ASSERT(Eq(Real32_Round(RD(4.5)), RD(5.0)));
	ASSERT(Eq(Real32_Round(RD(4.0)), RD(4.0)));
	ASSERT(Eq(Real32_Round(RD(3.5)), RD(4.0)));
	ASSERT(Eq(Real32_Round(RD(3.0)), RD(3.0)));
	ASSERT(Eq(Real32_Round(RD(2.5)), RD(3.0)));
	ASSERT(Eq(Real32_Round(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real32_Round(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real32_Round(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real32_Round(RD(0.5)), RD(1.0)));
	ASSERT(Eq(Real32_Round(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real32_Round(RD(-0.5)), RD(-1.0)));
	ASSERT(Eq(Real32_Round(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real32_Round(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real32_Round(RD(-2.0)), RD(-2.0)));
	ASSERT(Eq(Real32_Round(RD(-2.5)), RD(-3.0)));
	ASSERT(Eq(Real32_Round(RD(-3.0)), RD(-3.0)));
	ASSERT(Eq(Real32_Round(RD(-3.5)), RD(-4.0)));
	ASSERT(Eq(Real32_Round(RD(-4.0)), RD(-4.0)));
	ASSERT(Eq(Real32_Round(RD(-4.5)), RD(-5.0)));
}
END_TEST

START_TEST(CanBankRoundReal32)
{
	ASSERT(Eq(Real32_BankRound(RD(4.5)), RD(4.0)));
	ASSERT(Eq(Real32_BankRound(RD(4.0)), RD(4.0)));
	ASSERT(Eq(Real32_BankRound(RD(3.5)), RD(4.0)));
	ASSERT(Eq(Real32_BankRound(RD(3.0)), RD(3.0)));
	ASSERT(Eq(Real32_BankRound(RD(2.5)), RD(2.0)));
	ASSERT(Eq(Real32_BankRound(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real32_BankRound(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real32_BankRound(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real32_BankRound(RD(0.5)), RD(0.0)));
	ASSERT(Eq(Real32_BankRound(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real32_BankRound(RD(-0.5)), Real32_NegZero));
	ASSERT(Eq(Real32_BankRound(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real32_BankRound(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real32_BankRound(RD(-2.0)), RD(-2.0)));
	ASSERT(Eq(Real32_BankRound(RD(-2.5)), RD(-2.0)));
	ASSERT(Eq(Real32_BankRound(RD(-3.0)), RD(-3.0)));
	ASSERT(Eq(Real32_BankRound(RD(-3.5)), RD(-4.0)));
	ASSERT(Eq(Real32_BankRound(RD(-4.0)), RD(-4.0)));
	ASSERT(Eq(Real32_BankRound(RD(-4.5)), RD(-4.0)));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Test-Function Tests.

START_TEST(CanTestForInfinityReal32)
{
	ASSERT(!Real32_IsInf(Real32_NegNaN));
	ASSERT( Real32_IsInf(Real32_NegInf));
	ASSERT(!Real32_IsInf(Real32_NegSixteen));
	ASSERT(!Real32_IsInf(Real32_NegTen));
	ASSERT(!Real32_IsInf(Real32_NegTwo));
	ASSERT(!Real32_IsInf(Real32_NegOne));
	ASSERT(!Real32_IsInf(Real32_NegZero));
	ASSERT(!Real32_IsInf(Real32_Zero));
	ASSERT(!Real32_IsInf(Real32_One));
	ASSERT(!Real32_IsInf(Real32_Two));
	ASSERT(!Real32_IsInf(Real32_Ten));
	ASSERT(!Real32_IsInf(Real32_Sixteen));
	ASSERT( Real32_IsInf(Real32_Inf));
	ASSERT(!Real32_IsInf(Real32_NaN));
}
END_TEST

START_TEST(CanTestForNaNReal32)
{
	ASSERT( Real32_IsNaN(Real32_NegNaN));
	ASSERT(!Real32_IsNaN(Real32_NegInf));
	ASSERT(!Real32_IsNaN(Real32_NegSixteen));
	ASSERT(!Real32_IsNaN(Real32_NegTen));
	ASSERT(!Real32_IsNaN(Real32_NegTwo));
	ASSERT(!Real32_IsNaN(Real32_NegOne));
	ASSERT(!Real32_IsNaN(Real32_NegZero));
	ASSERT(!Real32_IsNaN(Real32_Zero));
	ASSERT(!Real32_IsNaN(Real32_One));
	ASSERT(!Real32_IsNaN(Real32_Two));
	ASSERT(!Real32_IsNaN(Real32_Ten));
	ASSERT(!Real32_IsNaN(Real32_Sixteen));
	ASSERT(!Real32_IsNaN(Real32_Inf));
	ASSERT( Real32_IsNaN(Real32_NaN));
}
END_TEST

START_TEST(CanTestForNegativeReal32)
{
	ASSERT( Real32_IsNeg(Real32_NegNaN));
	ASSERT( Real32_IsNeg(Real32_NegInf));
	ASSERT( Real32_IsNeg(Real32_NegSixteen));
	ASSERT( Real32_IsNeg(Real32_NegTen));
	ASSERT( Real32_IsNeg(Real32_NegTwo));
	ASSERT( Real32_IsNeg(Real32_NegOne));
	ASSERT( Real32_IsNeg(Real32_NegZero));
	ASSERT(!Real32_IsNeg(Real32_Zero));
	ASSERT(!Real32_IsNeg(Real32_One));
	ASSERT(!Real32_IsNeg(Real32_Two));
	ASSERT(!Real32_IsNeg(Real32_Ten));
	ASSERT(!Real32_IsNeg(Real32_Sixteen));
	ASSERT(!Real32_IsNeg(Real32_Inf));
	ASSERT(!Real32_IsNeg(Real32_NaN));
}
END_TEST

START_TEST(CanTestForZeroReal32)
{
	ASSERT(!Real32_IsZero(Real32_NegNaN));
	ASSERT(!Real32_IsZero(Real32_NegInf));
	ASSERT(!Real32_IsZero(Real32_NegSixteen));
	ASSERT(!Real32_IsZero(Real32_NegTen));
	ASSERT(!Real32_IsZero(Real32_NegTwo));
	ASSERT(!Real32_IsZero(Real32_NegOne));
	ASSERT( Real32_IsZero(Real32_NegZero));
	ASSERT( Real32_IsZero(Real32_Zero));
	ASSERT(!Real32_IsZero(Real32_One));
	ASSERT(!Real32_IsZero(Real32_Two));
	ASSERT(!Real32_IsZero(Real32_Ten));
	ASSERT(!Real32_IsZero(Real32_Sixteen));
	ASSERT(!Real32_IsZero(Real32_Inf));
	ASSERT(!Real32_IsZero(Real32_NaN));
}
END_TEST

START_TEST(CanTestForFiniteReal32)
{
	ASSERT(!Real32_IsFinite(Real32_NegNaN));
	ASSERT(!Real32_IsFinite(Real32_NegInf));
	ASSERT( Real32_IsFinite(Real32_NegSixteen));
	ASSERT( Real32_IsFinite(Real32_NegTen));
	ASSERT( Real32_IsFinite(Real32_NegTwo));
	ASSERT( Real32_IsFinite(Real32_NegOne));
	ASSERT( Real32_IsFinite(Real32_NegZero));
	ASSERT( Real32_IsFinite(Real32_Zero));
	ASSERT( Real32_IsFinite(Real32_One));
	ASSERT( Real32_IsFinite(Real32_Two));
	ASSERT( Real32_IsFinite(Real32_Ten));
	ASSERT( Real32_IsFinite(Real32_Sixteen));
	ASSERT(!Real32_IsFinite(Real32_Inf));
	ASSERT(!Real32_IsFinite(Real32_NaN));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Comparison Tests.

START_TEST(CanCompareEqualReal32)
{
	ASSERT(!Real32_Eq(Real32_NegNaN, Real32_NegNaN));
	ASSERT(!Real32_Eq(Real32_NaN, Real32_NaN));
	ASSERT(!Real32_Eq(Real32_NegNaN, Real32_NaN));
	ASSERT(!Real32_Eq(Real32_NaN, Real32_NegNaN));

	ASSERT(Real32_Eq(Real32_NegZero, Real32_NegZero));
	ASSERT(Real32_Eq(Real32_Zero, Real32_Zero));
	ASSERT(Real32_Eq(Real32_NegZero, Real32_Zero));
	ASSERT(Real32_Eq(Real32_Zero, Real32_NegZero));

	ASSERT(Real32_Eq(Real32_NegInf, Real32_NegInf));
	ASSERT(Real32_Eq(Real32_NegOne, Real32_NegOne));
	ASSERT(Real32_Eq(Real32_One, Real32_One));
	ASSERT(Real32_Eq(Real32_Inf, Real32_Inf));

	ASSERT(!Real32_Eq(Real32_NegInf, Real32_Inf));
	ASSERT(!Real32_Eq(Real32_NegOne, Real32_One));
	ASSERT(!Real32_Eq(Real32_One, Real32_Two));
	ASSERT(!Real32_Eq(Real32_Inf, Real32_NegInf));
}
END_TEST

START_TEST(CanCompareNotEqualReal32)
{
	ASSERT(Real32_Ne(Real32_NegNaN, Real32_NegNaN));
	ASSERT(Real32_Ne(Real32_NaN, Real32_NaN));
	ASSERT(Real32_Ne(Real32_NegNaN, Real32_NaN));
	ASSERT(Real32_Ne(Real32_NaN, Real32_NegNaN));

	ASSERT(!Real32_Ne(Real32_NegZero, Real32_NegZero));
	ASSERT(!Real32_Ne(Real32_Zero, Real32_Zero));
	ASSERT(!Real32_Ne(Real32_NegZero, Real32_Zero));
	ASSERT(!Real32_Ne(Real32_Zero, Real32_NegZero));

	ASSERT(!Real32_Ne(Real32_NegInf, Real32_NegInf));
	ASSERT(!Real32_Ne(Real32_NegOne, Real32_NegOne));
	ASSERT(!Real32_Ne(Real32_One, Real32_One));
	ASSERT(!Real32_Ne(Real32_Inf, Real32_Inf));

	ASSERT(Real32_Ne(Real32_NegInf, Real32_Inf));
	ASSERT(Real32_Ne(Real32_NegOne, Real32_One));
	ASSERT(Real32_Ne(Real32_One, Real32_Two));
	ASSERT(Real32_Ne(Real32_Inf, Real32_NegInf));
}
END_TEST

START_TEST(CanCompareLessThanReal32)
{
	ASSERT(!Real32_Lt(Real32_NegNaN, Real32_NegNaN));
	ASSERT(!Real32_Lt(Real32_NaN, Real32_NaN));
	ASSERT(!Real32_Lt(Real32_NegNaN, Real32_NaN));
	ASSERT(!Real32_Lt(Real32_NaN, Real32_NegNaN));
	ASSERT(!Real32_Lt(Real32_One, Real32_NaN));
	ASSERT(!Real32_Lt(Real32_NaN, Real32_One));

	ASSERT(!Real32_Lt(Real32_NegZero, Real32_NegZero));
	ASSERT(!Real32_Lt(Real32_Zero, Real32_Zero));
	ASSERT(!Real32_Lt(Real32_NegZero, Real32_Zero));
	ASSERT(!Real32_Lt(Real32_Zero, Real32_NegZero));

	ASSERT(!Real32_Lt(Real32_NegInf, Real32_NegInf));
	ASSERT(!Real32_Lt(Real32_NegOne, Real32_NegOne));
	ASSERT(!Real32_Lt(Real32_One, Real32_One));
	ASSERT(!Real32_Lt(Real32_Inf, Real32_Inf));

	ASSERT(Real32_Lt(Real32_NegInf, Real32_Inf));
	ASSERT(!Real32_Lt(Real32_Inf, Real32_NegInf));
	ASSERT(Real32_Lt(Real32_NegOne, Real32_One));
	ASSERT(!Real32_Lt(Real32_One, Real32_NegOne));
	ASSERT(Real32_Lt(Real32_One, Real32_Two));
	ASSERT(!Real32_Lt(Real32_Two, Real32_One));
	ASSERT(Real32_Lt(Real32_Zero, Real32_Inf));
	ASSERT(!Real32_Lt(Real32_Inf, Real32_Zero));
	ASSERT(Real32_Lt(Real32_NegInf, Real32_Zero));
	ASSERT(!Real32_Lt(Real32_Zero, Real32_NegInf));
}
END_TEST

START_TEST(CanCompareGreaterThanReal32)
{
	ASSERT(!Real32_Gt(Real32_NegNaN, Real32_NegNaN));
	ASSERT(!Real32_Gt(Real32_NaN, Real32_NaN));
	ASSERT(!Real32_Gt(Real32_NegNaN, Real32_NaN));
	ASSERT(!Real32_Gt(Real32_NaN, Real32_NegNaN));
	ASSERT(!Real32_Gt(Real32_One, Real32_NaN));
	ASSERT(!Real32_Gt(Real32_NaN, Real32_One));

	ASSERT(!Real32_Gt(Real32_NegZero, Real32_NegZero));
	ASSERT(!Real32_Gt(Real32_Zero, Real32_Zero));
	ASSERT(!Real32_Gt(Real32_NegZero, Real32_Zero));
	ASSERT(!Real32_Gt(Real32_Zero, Real32_NegZero));

	ASSERT(!Real32_Gt(Real32_NegInf, Real32_NegInf));
	ASSERT(!Real32_Gt(Real32_NegOne, Real32_NegOne));
	ASSERT(!Real32_Gt(Real32_One, Real32_One));
	ASSERT(!Real32_Gt(Real32_Inf, Real32_Inf));

	ASSERT(!Real32_Gt(Real32_NegInf, Real32_Inf));
	ASSERT(Real32_Gt(Real32_Inf, Real32_NegInf));
	ASSERT(!Real32_Gt(Real32_NegOne, Real32_One));
	ASSERT(Real32_Gt(Real32_One, Real32_NegOne));
	ASSERT(!Real32_Gt(Real32_One, Real32_Two));
	ASSERT(Real32_Gt(Real32_Two, Real32_One));
	ASSERT(!Real32_Gt(Real32_Zero, Real32_Inf));
	ASSERT(Real32_Gt(Real32_Inf, Real32_Zero));
	ASSERT(!Real32_Gt(Real32_NegInf, Real32_Zero));
	ASSERT(Real32_Gt(Real32_Zero, Real32_NegInf));
}
END_TEST

START_TEST(CanCompareLessThanOrEqualReal32)
{
	ASSERT(!Real32_Le(Real32_NegNaN, Real32_NegNaN));
	ASSERT(!Real32_Le(Real32_NaN, Real32_NaN));
	ASSERT(!Real32_Le(Real32_NegNaN, Real32_NaN));
	ASSERT(!Real32_Le(Real32_NaN, Real32_NegNaN));
	ASSERT(!Real32_Le(Real32_One, Real32_NaN));
	ASSERT(!Real32_Le(Real32_NaN, Real32_One));

	ASSERT(Real32_Le(Real32_NegZero, Real32_NegZero));
	ASSERT(Real32_Le(Real32_Zero, Real32_Zero));
	ASSERT(Real32_Le(Real32_NegZero, Real32_Zero));
	ASSERT(Real32_Le(Real32_Zero, Real32_NegZero));

	ASSERT(Real32_Le(Real32_NegInf, Real32_NegInf));
	ASSERT(Real32_Le(Real32_NegOne, Real32_NegOne));
	ASSERT(Real32_Le(Real32_One, Real32_One));
	ASSERT(Real32_Le(Real32_Inf, Real32_Inf));

	ASSERT(Real32_Le(Real32_NegInf, Real32_Inf));
	ASSERT(!Real32_Le(Real32_Inf, Real32_NegInf));
	ASSERT(Real32_Le(Real32_NegOne, Real32_One));
	ASSERT(!Real32_Le(Real32_One, Real32_NegOne));
	ASSERT(Real32_Le(Real32_One, Real32_Two));
	ASSERT(!Real32_Le(Real32_Two, Real32_One));
	ASSERT(Real32_Le(Real32_Zero, Real32_Inf));
	ASSERT(!Real32_Le(Real32_Inf, Real32_Zero));
	ASSERT(Real32_Le(Real32_NegInf, Real32_Zero));
	ASSERT(!Real32_Le(Real32_Zero, Real32_NegInf));
}
END_TEST

START_TEST(CanCompareGreaterThanOrEqualReal32)
{
	ASSERT(!Real32_Ge(Real32_NegNaN, Real32_NegNaN));
	ASSERT(!Real32_Ge(Real32_NaN, Real32_NaN));
	ASSERT(!Real32_Ge(Real32_NegNaN, Real32_NaN));
	ASSERT(!Real32_Ge(Real32_NaN, Real32_NegNaN));
	ASSERT(!Real32_Ge(Real32_One, Real32_NaN));
	ASSERT(!Real32_Ge(Real32_NaN, Real32_One));

	ASSERT(Real32_Ge(Real32_NegZero, Real32_NegZero));
	ASSERT(Real32_Ge(Real32_Zero, Real32_Zero));
	ASSERT(Real32_Ge(Real32_NegZero, Real32_Zero));
	ASSERT(Real32_Ge(Real32_Zero, Real32_NegZero));

	ASSERT(Real32_Ge(Real32_NegInf, Real32_NegInf));
	ASSERT(Real32_Ge(Real32_NegOne, Real32_NegOne));
	ASSERT(Real32_Ge(Real32_One, Real32_One));
	ASSERT(Real32_Ge(Real32_Inf, Real32_Inf));

	ASSERT(!Real32_Ge(Real32_NegInf, Real32_Inf));
	ASSERT(Real32_Ge(Real32_Inf, Real32_NegInf));
	ASSERT(!Real32_Ge(Real32_NegOne, Real32_One));
	ASSERT(Real32_Ge(Real32_One, Real32_NegOne));
	ASSERT(!Real32_Ge(Real32_One, Real32_Two));
	ASSERT(Real32_Ge(Real32_Two, Real32_One));
	ASSERT(!Real32_Ge(Real32_Zero, Real32_Inf));
	ASSERT(Real32_Ge(Real32_Inf, Real32_Zero));
	ASSERT(!Real32_Ge(Real32_NegInf, Real32_Zero));
	ASSERT(Real32_Ge(Real32_Zero, Real32_NegInf));
}
END_TEST

START_TEST(CanCompareOrderedReal32)
{
	ASSERT(Real32_Compare(Real32_NegNaN, Real32_NegNaN) == 0);
	ASSERT(Real32_Compare(Real32_NegNaN, Real32_NegInf) < 0);
	ASSERT(Real32_Compare(Real32_NegNaN, Real32_NegOne) < 0);
	ASSERT(Real32_Compare(Real32_NegNaN, Real32_Zero) < 0);
	ASSERT(Real32_Compare(Real32_NegNaN, Real32_One) < 0);
	ASSERT(Real32_Compare(Real32_NegNaN, Real32_Inf) < 0);
	ASSERT(Real32_Compare(Real32_NegNaN, Real32_NaN) < 0);

	ASSERT(Real32_Compare(Real32_NegInf, Real32_NegNaN) > 0);
	ASSERT(Real32_Compare(Real32_NegInf, Real32_NegInf) == 0);
	ASSERT(Real32_Compare(Real32_NegInf, Real32_NegOne) < 0);
	ASSERT(Real32_Compare(Real32_NegInf, Real32_Zero) < 0);
	ASSERT(Real32_Compare(Real32_NegInf, Real32_One) < 0);
	ASSERT(Real32_Compare(Real32_NegInf, Real32_Inf) < 0);
	ASSERT(Real32_Compare(Real32_NegInf, Real32_NaN) < 0);

	ASSERT(Real32_Compare(Real32_NegOne, Real32_NegNaN) > 0);
	ASSERT(Real32_Compare(Real32_NegOne, Real32_NegInf) > 0);
	ASSERT(Real32_Compare(Real32_NegOne, Real32_NegOne) == 0);
	ASSERT(Real32_Compare(Real32_NegOne, Real32_Zero) < 0);
	ASSERT(Real32_Compare(Real32_NegOne, Real32_One) < 0);
	ASSERT(Real32_Compare(Real32_NegOne, Real32_Inf) < 0);
	ASSERT(Real32_Compare(Real32_NegOne, Real32_NaN) < 0);

	ASSERT(Real32_Compare(Real32_Zero, Real32_NegNaN) > 0);
	ASSERT(Real32_Compare(Real32_Zero, Real32_NegInf) > 0);
	ASSERT(Real32_Compare(Real32_Zero, Real32_NegOne) > 0);
	ASSERT(Real32_Compare(Real32_Zero, Real32_Zero) == 0);
	ASSERT(Real32_Compare(Real32_Zero, Real32_One) < 0);
	ASSERT(Real32_Compare(Real32_Zero, Real32_Inf) < 0);
	ASSERT(Real32_Compare(Real32_Zero, Real32_NaN) < 0);

	ASSERT(Real32_Compare(Real32_One, Real32_NegNaN) > 0);
	ASSERT(Real32_Compare(Real32_One, Real32_NegInf) > 0);
	ASSERT(Real32_Compare(Real32_One, Real32_NegOne) > 0);
	ASSERT(Real32_Compare(Real32_One, Real32_Zero) > 0);
	ASSERT(Real32_Compare(Real32_One, Real32_One) == 0);
	ASSERT(Real32_Compare(Real32_One, Real32_Inf) < 0);
	ASSERT(Real32_Compare(Real32_One, Real32_NaN) < 0);

	ASSERT(Real32_Compare(Real32_Inf, Real32_NegNaN) > 0);
	ASSERT(Real32_Compare(Real32_Inf, Real32_NegInf) > 0);
	ASSERT(Real32_Compare(Real32_Inf, Real32_NegOne) > 0);
	ASSERT(Real32_Compare(Real32_Inf, Real32_Zero) > 0);
	ASSERT(Real32_Compare(Real32_Inf, Real32_One) > 0);
	ASSERT(Real32_Compare(Real32_Inf, Real32_Inf) == 0);
	ASSERT(Real32_Compare(Real32_Inf, Real32_NaN) < 0);

	ASSERT(Real32_Compare(Real32_NaN, Real32_NegNaN) > 0);
	ASSERT(Real32_Compare(Real32_NaN, Real32_NegInf) > 0);
	ASSERT(Real32_Compare(Real32_NaN, Real32_NegOne) > 0);
	ASSERT(Real32_Compare(Real32_NaN, Real32_Zero) > 0);
	ASSERT(Real32_Compare(Real32_NaN, Real32_One) > 0);
	ASSERT(Real32_Compare(Real32_NaN, Real32_Inf) > 0);
	ASSERT(Real32_Compare(Real32_NaN, Real32_NaN) == 0);
}
END_TEST

START_TEST(CanDetermineOrderabilityReal32)
{
	ASSERT(!Real32_IsOrderable(Real32_NegNaN, Real32_NegNaN));
	ASSERT(!Real32_IsOrderable(Real32_NaN, Real32_NaN));
	ASSERT(!Real32_IsOrderable(Real32_NegNaN, Real32_NaN));
	ASSERT(!Real32_IsOrderable(Real32_NaN, Real32_NegNaN));
	ASSERT(!Real32_IsOrderable(Real32_One, Real32_NaN));
	ASSERT(!Real32_IsOrderable(Real32_NaN, Real32_One));

	ASSERT(Real32_IsOrderable(Real32_NegZero, Real32_NegZero));
	ASSERT(Real32_IsOrderable(Real32_Zero, Real32_Zero));
	ASSERT(Real32_IsOrderable(Real32_NegZero, Real32_Zero));
	ASSERT(Real32_IsOrderable(Real32_Zero, Real32_NegZero));

	ASSERT(Real32_IsOrderable(Real32_NegInf, Real32_NegInf));
	ASSERT(Real32_IsOrderable(Real32_NegOne, Real32_NegOne));
	ASSERT(Real32_IsOrderable(Real32_One, Real32_One));
	ASSERT(Real32_IsOrderable(Real32_Inf, Real32_Inf));

	ASSERT(Real32_IsOrderable(Real32_NegInf, Real32_Inf));
	ASSERT(Real32_IsOrderable(Real32_Inf, Real32_NegInf));
	ASSERT(Real32_IsOrderable(Real32_NegOne, Real32_One));
	ASSERT(Real32_IsOrderable(Real32_One, Real32_NegOne));
	ASSERT(Real32_IsOrderable(Real32_One, Real32_Two));
	ASSERT(Real32_IsOrderable(Real32_Two, Real32_One));
	ASSERT(Real32_IsOrderable(Real32_Zero, Real32_Inf));
	ASSERT(Real32_IsOrderable(Real32_Inf, Real32_Zero));
	ASSERT(Real32_IsOrderable(Real32_NegInf, Real32_Zero));
	ASSERT(Real32_IsOrderable(Real32_Zero, Real32_NegInf));
}
END_TEST

#include "real32_tests.generated.inc"
