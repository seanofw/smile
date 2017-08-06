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

TEST_SUITE(Real64Tests)

//-------------------------------------------------------------------------------------------------
//  Helper functions and macros.

#define RI(__n__) Real64_FromInt32(__n__)
#define RL(__n__) Real64_FromInt64(__n__)
#define RF(__n__) Real64_FromFloat32(__n__)
#define RD(__n__) Real64_FromFloat64(__n__)

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
/// Convert a string of hex characters in high-to-low order into a Real64.
/// </summary>
static Real64 RR(const char *str)
{
	// TODO: This only works properly on little-endian architectures.

	union {
		Byte buffer[8];
		Real64 real;
	} u;
	Byte *bufptr = u.buffer;
	Bool highNybble = False;
	const char *src = str;

	while (*src) src++;

	while (src >= str && bufptr < u.buffer + 8) {
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
	while (bufptr < u.buffer + 8) {
		if (highNybble) {
			bufptr++;
			highNybble = False;
		}
		else {
			*bufptr = 0;
			highNybble = True;
		}
	}

	return u.real;
}

Inline Bool Eq(Real64 a, Real64 b)
{
	return a.value == b.value;
}

//-------------------------------------------------------------------------------------------------
//  Static constants.

// Common limits.
static const Real64 _int32Max = { 0x31C000007FFFFFFFULL };
static const Real64 _int32Min = { 0xB1C0000080000000ULL };
static const Real64 _int64Max = { 0x6C88C49BA5E353F8ULL };
static const Real64 _int64Min = { 0xEC88C49BA5E353F8ULL };

// Miscellaneous integers.
static const Real64 _fiveSevenNine = { 0x31C0000000000243ULL };
static const Real64 _fiveSevenNineOhOh = { 0x318000000000E22CULL };
static const Real64 _shortBigPi = { 0x31C0000012B9B0A1ULL };
static const Real64 _longBigPi = { 0x320B29430A256D21ULL };
static const Real64 _longMediumPi = { 0x30EB29430A256D21ULL };
static const Real64 _pi = { 0x2FEB29430A256D21ULL };
static const Real64 _longNegMediumPi = { 0xB0EB29430A256D21ULL };
static const Real64 _negPi = { 0xAFEB29430A256D21ULL };

//-------------------------------------------------------------------------------------------------
//  Type-Conversion Tests.
//
//  Some of these are ganked straight from Intel's tests included with their BID library.

START_TEST(CanConvertInt32ToReal64)
{
	ASSERT(Eq(RI(1151048260), RR("31c00000449b9a44")));
	ASSERT(Eq(RI(1238620169), RR("31c0000049d3d809")));
	ASSERT(Eq(RI(1300695084), RR("31c000004d87082c")));
	ASSERT(Eq(RI(-1321821308), RR("b1c000004ec9647c")));
	ASSERT(Eq(RI(-1409558545), RR("b1c0000054042811")));
	ASSERT(Eq(RI(-1411393610), RR("b1c000005420284a")));
	ASSERT(Eq(RI(-1558551822), RR("b1c000005ce59d0e")));
	ASSERT(Eq(RI(-1680014258), RR("b1c000006422fbb2")));
	ASSERT(Eq(RI(1850397233), RR("31c000006e4ad231")));
	ASSERT(Eq(RI(1961604583), RR("31c0000074ebb5e7")));
	ASSERT(Eq(RI(1965388277), RR("31c00000752571f5")));
	ASSERT(Eq(RI(1983979873), RR("31c0000076412161")));
	ASSERT(Eq(RI(2042218777), RR("31c0000079b9c919")));
	ASSERT(Eq(RI(-2132132449), RR("b1c000007f15c261")));
	ASSERT(Eq(RI(-647085063), RR("b1c000002691bc07")));
	ASSERT(Eq(RI(665889169), RR("31c0000027b0a991")));
	ASSERT(Eq(RI(753240669), RR("31c000002ce58a5d")));
	ASSERT(Eq(RI(-765857262), RR("b1c000002da60dee")));
	ASSERT(Eq(RI(-768244481), RR("b1c000002dca7b01")));
	ASSERT(Eq(RI(988813101), RR("31c000003af0172d")));
}
END_TEST

START_TEST(CanConvertInt64ToReal64)
{
	ASSERT(Eq(RL(1139264871891773575), RR("32240c27c5fe833e")));
	ASSERT(Eq(RL(-1750062212783916257), RR("b22637ac1c871f2c")));
	ASSERT(Eq(RL(-17891329), RR("b1c0000001110001")));
	ASSERT(Eq(RL(2097835747362605220), RR("322773f87284c72d")));
	ASSERT(Eq(RL(239600704763147600), RR("32088327dcf41274")));
	ASSERT(Eq(RL(-3699397591305768705), RR("b22d24951f500e29")));
	ASSERT(Eq(RL(-4021596677842024000), RR("b22e499ef169a468")));
	ASSERT(Eq(RL(5456652339267465168), RR("323362cbe0584789")));
	ASSERT(Eq(RL(-5681820659447707127), RR("b2342f95f615779b")));
	ASSERT(Eq(RL(589782372181576380), RR("3214f40a17e2fed4")));
	ASSERT(Eq(RL(-6247827353969653838), RR("b236325daa0d23f6")));
	ASSERT(Eq(RL(-6251581962144061921), RR("b23635c7da23493e")));
	ASSERT(Eq(RL(-6277401661671197000), RR("b2364d437828071d")));
	ASSERT(Eq(RL(63716561664156733), RR("31f6a2fcce6e1d39")));
	ASSERT(Eq(RL(6987444851531894805), RR("3238d30b48436077")));
	ASSERT(Eq(RL(-7058028164498851412), RR("b239133d3d8f45a3")));
	ASSERT(Eq(RL(-791485443906179333), RR("b21c1e84a74a0921")));
	ASSERT(Eq(RL(-7999593679867525011), RR("b23c6b968ba2ce85")));
	ASSERT(Eq(RL(875842350099016200), RR("321f1dbd6191a6d2")));
	ASSERT(Eq(RL(8840994672450821792), RR("323f68d67ae99d06")));
	ASSERT(Eq(RL(931846679491469800), RR("6c811b189f29838a")));
	ASSERT(Eq(RL(9781575579355265), RR("6c72c04a89c04881")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Parsing Tests.

START_TEST(CanParseReal64)
{
	Real64 result;

	ASSERT(Real64_TryParse(String_FromC("579"), &result));
	ASSERT(Eq(result, _fiveSevenNine));

	ASSERT(Real64_TryParse(String_FromC("579.00"), &result));
	ASSERT(Eq(result, _fiveSevenNineOhOh));

	ASSERT(Real64_TryParse(String_FromC("314159265358979323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real64_TryParse(String_FromC("314_159_265_358_979_323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real64_TryParse(String_FromC("314'159'265'358'979'323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real64_TryParse(String_FromC("314'159\"265'358\"979'323"), &result));
	ASSERT(Eq(result, _longBigPi));

	ASSERT(Real64_TryParse(String_FromC("-314'159'265.358'979'323"), &result));
	ASSERT(Eq(result, _longNegMediumPi));

	ASSERT(Real64_TryParse(String_FromC("3.14159'26535'89793'23"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real64_TryParse(String_FromC("-3.14159'26535'89793'23"), &result));
	ASSERT(Eq(result, _negPi));

	ASSERT(Real64_TryParse(String_FromC("+3.14159'26535'89793'23"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real64_TryParse(String_FromC("+3.14159'26535'89793'23E+8"), &result));
	ASSERT(Eq(result, _longMediumPi));

	ASSERT(Real64_TryParse(String_FromC("-3.14159'26535'89793'23E+8"), &result));
	ASSERT(Eq(result, _longNegMediumPi));

	ASSERT(Real64_TryParse(String_FromC("+314'159'265.358'979'323E-8"), &result));
	ASSERT(Eq(result, _pi));

	ASSERT(Real64_TryParse(String_FromC("-314'159'265.358'979'323E-8"), &result));
	ASSERT(Eq(result, _negPi));

	ASSERT(Real64_TryParse(String_FromC("+inf"), &result));
	ASSERT(Eq(result, Real64_Inf));

	ASSERT(Real64_TryParse(String_FromC("inf"), &result));
	ASSERT(Eq(result, Real64_Inf));
	ASSERT(Real64_TryParse(String_FromC("INF"), &result));
	ASSERT(Eq(result, Real64_Inf));
	ASSERT(Real64_TryParse(String_FromC("Inf"), &result));
	ASSERT(Eq(result, Real64_Inf));

	ASSERT(Real64_TryParse(String_FromC("-inf"), &result));
	ASSERT(Eq(result, Real64_NegInf));

	ASSERT(Real64_TryParse(String_FromC("+nan"), &result));
	ASSERT(Real64_IsNaN(result));
	ASSERT(!Real64_IsNeg(result));

	ASSERT(Real64_TryParse(String_FromC("nan"), &result));
	ASSERT(Real64_IsNaN(result));
	ASSERT(!Real64_IsNeg(result));
	ASSERT(Real64_TryParse(String_FromC("NAN"), &result));
	ASSERT(Real64_IsNaN(result));
	ASSERT(!Real64_IsNeg(result));
	ASSERT(Real64_TryParse(String_FromC("NaN"), &result));
	ASSERT(Real64_IsNaN(result));
	ASSERT(!Real64_IsNeg(result));

	ASSERT(Real64_TryParse(String_FromC("-nan"), &result));
	ASSERT(Real64_IsNaN(result));
	ASSERT(Real64_IsNeg(result));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Stringification Tests.

START_TEST(CanStringifyReal64NumbersInExponentialForm)
{
	ASSERT_STRING(Real64_ToExpString(_pi, 1, False), "3.141592653589793e+0", 20);
	ASSERT_STRING(Real64_ToExpString(_pi, 1, True), "+3.141592653589793e+0", 21);
	ASSERT_STRING(Real64_ToExpString(_negPi, 1, False), "-3.141592653589793e+0", 21);

	ASSERT_STRING(Real64_ToExpString(_longMediumPi, 1, False), "3.141592653589793e+8", 20);
	ASSERT_STRING(Real64_ToExpString(_longNegMediumPi, 1, False), "-3.141592653589793e+8", 21);

	ASSERT_STRING(Real64_ToExpString(RD(12345000000), 1, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real64_ToExpString(RD(-12345000000), 1, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real64_ToExpString(RD(12345000000), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real64_ToExpString(RD(-12345000000), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real64_ToExpString(Real64_ParseC("12345000000"), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real64_ToExpString(Real64_ParseC("-12345000000"), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real64_ToExpString(Real64_ParseC("1.2345E+10"), 10, False), "1.2345000000e+10", 16);
	ASSERT_STRING(Real64_ToExpString(Real64_ParseC("-1.2345E+10"), 10, False), "-1.2345000000e+10", 17);

	ASSERT_STRING(Real64_ToExpString(Real64_ParseC("1.2345E+10"), 1, False), "1.2345e+10", 10);
	ASSERT_STRING(Real64_ToExpString(Real64_ParseC("-1.2345E+10"), 1, False), "-1.2345e+10", 11);

	ASSERT_STRING(Real64_ToExpString(RD(0.125), 1, False), "1.25e-1", 7);
	ASSERT_STRING(Real64_ToExpString(RD(-0.125), 1, False), "-1.25e-1", 8);

	ASSERT_STRING(Real64_ToExpString(Real64_ParseC("0.125000"), 1, False), "1.25000e-1", 10);
	ASSERT_STRING(Real64_ToExpString(Real64_ParseC("-0.125000"), 1, False), "-1.25000e-1", 11);

	ASSERT_STRING(Real64_ToExpString(_fiveSevenNineOhOh, 1, False), "5.7900e+2", 9);
	ASSERT_STRING(Real64_ToExpString(_fiveSevenNineOhOh, 2, False), "5.7900e+2", 9);
	ASSERT_STRING(Real64_ToExpString(_fiveSevenNineOhOh, 3, False), "5.7900e+2", 9);
	ASSERT_STRING(Real64_ToExpString(_fiveSevenNineOhOh, 4, False), "5.7900e+2", 9);
	ASSERT_STRING(Real64_ToExpString(_fiveSevenNineOhOh, 5, False), "5.79000e+2", 10);
	ASSERT_STRING(Real64_ToExpString(_fiveSevenNineOhOh, 6, False), "5.790000e+2", 11);
}
END_TEST

START_TEST(CanStringifySpecialReal64ValuesInExponentialForm)
{
	ASSERT_STRING(Real64_ToExpString(Real64_Zero, 0, False), "0", 1);
	ASSERT_STRING(Real64_ToExpString(Real64_Zero, 0, True), "+0", 2);
	ASSERT_STRING(Real64_ToExpString(Real64_Zero, 5, False), "0.00000", 7);
	ASSERT_STRING(Real64_ToExpString(Real64_Zero, 5, True), "+0.00000", 8);

	ASSERT_STRING(Real64_ToExpString(Real64_One, 0, False), "1e+0", 4);
	ASSERT_STRING(Real64_ToExpString(Real64_One, 0, True), "+1e+0", 5);
	ASSERT_STRING(Real64_ToExpString(Real64_One, 5, False), "1.00000e+0", 10);
	ASSERT_STRING(Real64_ToExpString(Real64_One, 5, True), "+1.00000e+0", 11);

	ASSERT_STRING(Real64_ToExpString(Real64_Inf, 1, False), "inf", 3);
	ASSERT_STRING(Real64_ToExpString(Real64_Inf, 1, True), "+inf", 4);
	ASSERT_STRING(Real64_ToExpString(Real64_NegInf, 1, False), "-inf", 4);

	ASSERT_STRING(Real64_ToExpString(Real64_NaN, 1, False), "NaN", 3);
	ASSERT_STRING(Real64_ToExpString(Real64_NaN, 1, True), "+NaN", 4);
	ASSERT_STRING(Real64_ToExpString(Real64_NegNaN, 1, False), "-NaN", 4);
}
END_TEST

START_TEST(CanStringifyReal64NumbersInFixedForm)
{
	// Things split across the decimal point.
	ASSERT_STRING(Real64_ToFixedString(_pi, 1, 1, False), "3.141592653589793", 17);
	ASSERT_STRING(Real64_ToFixedString(_pi, 1, 1, True), "+3.141592653589793", 18);
	ASSERT_STRING(Real64_ToFixedString(_negPi, 1, 1, False), "-3.141592653589793", 18);

	ASSERT_STRING(Real64_ToFixedString(_longMediumPi, 1, 1, False), "314159265.3589793", 17);
	ASSERT_STRING(Real64_ToFixedString(_longMediumPi, 1, 1, True), "+314159265.3589793", 18);
	ASSERT_STRING(Real64_ToFixedString(_longNegMediumPi, 1, 1, False), "-314159265.3589793", 18);

	// Things that only exist after the decimal point.
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("0.125000"), 1, 1, False), "0.125000", 8);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("-0.125000"), 1, 1, False), "-0.125000", 9);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("125e-5"), 1, 1, False), "0.00125", 7);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("-125e-5"), 1, 1, False), "-0.00125", 8);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("125e-5"), 0, 0, False), ".00125", 6);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("-125e-5"), 0, 0, False), "-.00125", 7);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("125e-5"), 3, 7, False), "000.0012500", 11);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("-125e-5"), 3, 7, False), "-000.0012500", 12);

	// Things that only exist before the decimal point.
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("125000"), 1, 1, False), "125000.0", 8);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("-125000"), 1, 1, False), "-125000.0", 9);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("125e+3"), 1, 1, False), "125000.0", 8);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("-125e+3"), 1, 1, False), "-125000.0", 9);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("125e+3"), 0, 0, False), "125000", 6);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("-125e+3"), 0, 0, False), "-125000", 7);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("125e+3"), 10, 3, False), "0000125000.000", 14);
	ASSERT_STRING(Real64_ToFixedString(Real64_ParseC("-125e+3"), 10, 3, False), "-0000125000.000", 15);
}
END_TEST

START_TEST(CanStringifySpecialReal64ValuesInFixedForm)
{
	ASSERT_STRING(Real64_ToFixedString(Real64_Zero, 0, 0, False), "0", 1);
	ASSERT_STRING(Real64_ToFixedString(Real64_Zero, 0, 0, True), "+0", 2);
	ASSERT_STRING(Real64_ToFixedString(Real64_Zero, 5, 5, False), "00000.00000", 11);
	ASSERT_STRING(Real64_ToFixedString(Real64_Zero, 5, 5, True), "+00000.00000", 12);

	ASSERT_STRING(Real64_ToFixedString(Real64_One, 0, 0, False), "1", 1);
	ASSERT_STRING(Real64_ToFixedString(Real64_One, 0, 0, True), "+1", 2);
	ASSERT_STRING(Real64_ToFixedString(Real64_One, 5, 5, False), "00001.00000", 11);
	ASSERT_STRING(Real64_ToFixedString(Real64_One, 5, 5, True), "+00001.00000", 12);

	ASSERT_STRING(Real64_ToFixedString(Real64_Inf, 0, 0, False), "inf", 3);
	ASSERT_STRING(Real64_ToFixedString(Real64_Inf, 0, 0, True), "+inf", 4);
	ASSERT_STRING(Real64_ToFixedString(Real64_NegInf, 0, 0, False), "-inf", 4);

	ASSERT_STRING(Real64_ToFixedString(Real64_NaN, 0, 0, False), "NaN", 3);
	ASSERT_STRING(Real64_ToFixedString(Real64_NaN, 0, 0, True), "+NaN", 4);
	ASSERT_STRING(Real64_ToFixedString(Real64_NegNaN, 0, 0, False), "-NaN", 4);
}
END_TEST

START_TEST(CanStringifyReal64NumbersGenerically)
{
	// Things split across the decimal point.
	ASSERT_STRING(Real64_ToString(_pi), "3.141592653589793", 17);
	ASSERT_STRING(Real64_ToString(_negPi), "-3.141592653589793", 18);

	ASSERT_STRING(Real64_ToString(_longMediumPi), "314159265.3589793", 17);
	ASSERT_STRING(Real64_ToString(_longNegMediumPi), "-314159265.3589793", 18);

	// Things that only exist after the decimal point.
	ASSERT_STRING(Real64_ToString(Real64_ParseC("0.125000")), ".125000", 7);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("-0.125000")), "-.125000", 8);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("125e-5")), ".00125", 6);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("-125e-5")), "-.00125", 7);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("125e-10")), "1.25e-8", 7);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("-125e-10")), "-1.25e-8", 8);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("1250e-11")), "1.250e-8", 8);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("-1250e-11")), "-1.250e-8", 9);

	// Things that only exist before the decimal point.
	ASSERT_STRING(Real64_ToString(Real64_ParseC("125000")), "125000", 6);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("-125000")), "-125000", 7);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("125e+3")), "125000", 6);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("-125e+3")), "-125000", 7);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("125e+10")), "1.25e+12", 8);
	ASSERT_STRING(Real64_ToString(Real64_ParseC("-125e+10")), "-1.25e+12", 9);
}
END_TEST

START_TEST(CanStringifySpecialReal64ValuesGenerically)
{
	ASSERT_STRING(Real64_ToString(Real64_Zero), "0", 1);
	ASSERT_STRING(Real64_ToString(Real64_One), "1", 1);

	ASSERT_STRING(Real64_ToString(Real64_Inf), "inf", 3);
	ASSERT_STRING(Real64_ToString(Real64_NegInf), "-inf", 4);

	ASSERT_STRING(Real64_ToString(Real64_NaN), "NaN", 3);
	ASSERT_STRING(Real64_ToString(Real64_NegNaN), "-NaN", 4);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Arithmetic Tests.

START_TEST(CanAddReal64)
{
	ASSERT(Eq(Real64_Add(RI(123), RI(456)), RI(579)));
	ASSERT(Eq(Real64_Add(RD(1.5), RD(0.125)), RD(1.625)));
	ASSERT(Eq(Real64_Add(RD(1.5), RD(-3.0)), RD(-1.5)));
}
END_TEST

START_TEST(CanMultiplyReal64)
{
	ASSERT(Eq(Real64_Mul(RI(123), RI(456)), RI(56088)));
	ASSERT(Eq(Real64_Mul(RD(1.5), RD(0.125)), RD(0.1875)));
	ASSERT(Eq(Real64_Mul(RD(1.5), RD(-3.0)), RD(-4.5)));
}
END_TEST

START_TEST(CanSubtractReal64)
{
	ASSERT(Eq(Real64_Sub(RI(123), RI(456)), RI(-333)));
	ASSERT(Eq(Real64_Sub(RD(1.5), RD(0.125)), RD(1.375)));
	ASSERT(Eq(Real64_Sub(RD(1.5), RD(-3.0)), RD(4.5)));
}
END_TEST

START_TEST(CanDivideReal64)
{
	ASSERT(Eq(Real64_Div(RI(123), RI(456)), Real64_ParseC(".2697368421052631578947368421052632")));
	ASSERT(Eq(Real64_Div(RD(1.5), RD(0.125)), RI(12)));
	ASSERT(Eq(Real64_Div(RD(1.5), RD(-3.0)), RD(-0.5)));
}
END_TEST

START_TEST(CanModulusReal64)
{
	ASSERT(Eq(Real64_Mod(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real64_Mod(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real64_Mod(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real64_Mod(RI(-456), RI(123)), RI(87)));
	ASSERT(Eq(Real64_Mod(RI(456), RI(-123)), RI(-87)));
	ASSERT(Eq(Real64_Mod(RI(-456), RI(-123)), RI(-87)));
}
END_TEST

START_TEST(CanRemainderReal64)
{
	ASSERT(Eq(Real64_Rem(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real64_Rem(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real64_Rem(RI(456), RI(123)), RI(87)));
	ASSERT(Eq(Real64_Rem(RI(-456), RI(123)), RI(-87)));
	ASSERT(Eq(Real64_Rem(RI(456), RI(-123)), RI(87)));
	ASSERT(Eq(Real64_Rem(RI(-456), RI(-123)), RI(-87)));
}
END_TEST

START_TEST(CanIeeeRemainderReal64)
{
	ASSERT(Eq(Real64_IeeeRem(RI(456), RI(123)), RI(-36)));
	ASSERT(Eq(Real64_IeeeRem(RD(5), RD(1.5)), RD(0.5)));

	ASSERT(Eq(Real64_IeeeRem(RI(456), RI(123)), RI(-36)));
	ASSERT(Eq(Real64_IeeeRem(RI(-456), RI(123)), RI(36)));
	ASSERT(Eq(Real64_IeeeRem(RI(456), RI(-123)), RI(-36)));
	ASSERT(Eq(Real64_IeeeRem(RI(-456), RI(-123)), RI(36)));
}
END_TEST

START_TEST(CanNegateReal64)
{
	ASSERT(Eq(Real64_Neg(RI(123)), RI(-123)));
	ASSERT(Eq(Real64_Neg(RI(-123)), RI(123)));

	ASSERT(Eq(Real64_Neg(Real64_Zero), Real64_NegZero));
	ASSERT(Eq(Real64_Neg(Real64_NegZero), Real64_Zero));

	ASSERT(Eq(Real64_Neg(Real64_Inf), Real64_NegInf));
	ASSERT(Eq(Real64_Neg(Real64_NegInf), Real64_Inf));
}
END_TEST

START_TEST(CanAbsoluteReal64)
{
	ASSERT(Eq(Real64_Abs(RI(123)), RI(123)));
	ASSERT(Eq(Real64_Abs(RI(-123)), RI(123)));

	ASSERT(Eq(Real64_Abs(Real64_Zero), Real64_Zero));
	ASSERT(Eq(Real64_Abs(Real64_NegZero), Real64_Zero));

	ASSERT(Eq(Real64_Abs(Real64_Inf), Real64_Inf));
	ASSERT(Eq(Real64_Abs(Real64_NegInf), Real64_Inf));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Powers and logarithms.

START_TEST(CanSqrtReal64)
{
	ASSERT(Eq(Real64_Sqrt(Real64_Zero), Real64_Zero));
	ASSERT(Eq(Real64_Sqrt(Real64_NegZero), Real64_NegZero));		// Weird, but correct.
	ASSERT(Eq(Real64_Sqrt(Real64_One), Real64_One));
	ASSERT(Eq(Real64_Sqrt(Real64_Inf), Real64_Inf));

	ASSERT(Eq(Real64_Sqrt(RI(256)), RI(16)));
	ASSERT(Eq(Real64_Sqrt(RI(25)), RI(5)));
	ASSERT(Eq(Real64_Sqrt(Real64_ParseC("18'446'744'065'119'617'025")), Real64_ParseC("4'294'967'295.000000")));
	ASSERT(Eq(Real64_Sqrt(RI(123)), Real64_ParseC("11.09053'65064'09417'16205'16001'02609'93")));

	ASSERT(Eq(Real64_Sqrt(Real64_NegOne), Real64_NaN));
	ASSERT(Eq(Real64_Sqrt(RI(-123)), Real64_NaN));
	ASSERT(Eq(Real64_Sqrt(Real64_NegInf), Real64_NaN));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Rounding Tests.

START_TEST(CanFloorReal64)
{
	ASSERT(Eq(Real64_Floor(RI(123)), RI(123)));
	ASSERT(Eq(Real64_Floor(RI(-123)), RI(-123)));

	ASSERT(Eq(Real64_Floor(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real64_Floor(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real64_Floor(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real64_Floor(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real64_Floor(RD(-1.2)), RD(-2.0)));
	ASSERT(Eq(Real64_Floor(RD(1.8)), RD(1.0)));
	ASSERT(Eq(Real64_Floor(RD(-1.8)), RD(-2.0)));
	ASSERT(Eq(Real64_Floor(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real64_Floor(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanCeilReal64)
{
	ASSERT(Eq(Real64_Ceil(RI(123)), RI(123)));
	ASSERT(Eq(Real64_Ceil(RI(-123)), RI(-123)));

	ASSERT(Eq(Real64_Ceil(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real64_Ceil(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real64_Ceil(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real64_Ceil(RD(1.2)), RD(2.0)));
	ASSERT(Eq(Real64_Ceil(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real64_Ceil(RD(1.8)), RD(2.0)));
	ASSERT(Eq(Real64_Ceil(RD(-1.8)), RD(-1.0)));
	ASSERT(Eq(Real64_Ceil(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real64_Ceil(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanTruncReal64)
{
	ASSERT(Eq(Real64_Trunc(RI(123)), RI(123)));
	ASSERT(Eq(Real64_Trunc(RI(-123)), RI(-123)));

	ASSERT(Eq(Real64_Trunc(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real64_Trunc(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real64_Trunc(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real64_Trunc(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real64_Trunc(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real64_Trunc(RD(1.8)), RD(1.0)));
	ASSERT(Eq(Real64_Trunc(RD(-1.8)), RD(-1.0)));
	ASSERT(Eq(Real64_Trunc(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real64_Trunc(RD(-2.0)), RD(-2.0)));
}
END_TEST

START_TEST(CanSplitReal64WithModf)
{
	Real64 intPart;

	ASSERT(Eq(Real64_Modf(RI(123), &intPart), Real64_Zero));
	ASSERT(Eq(intPart, RI(123)));
	ASSERT(Eq(Real64_Modf(RI(-123), &intPart), Real64_NegZero));
	ASSERT(Eq(intPart, RI(-123)));

	ASSERT(Eq(Real64_Modf(RD(0.0), &intPart), Real64_Zero));
	ASSERT(Eq(intPart, RI(0)));
	ASSERT(Eq(Real64_Modf(RD(1.0), &intPart), Real64_Zero));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real64_Modf(RD(-1.0), &intPart), Real64_NegZero));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real64_Modf(Real64_ParseC("1.2"), &intPart), Real64_ParseC(".2")));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real64_Modf(Real64_ParseC("-1.2"), &intPart), Real64_ParseC("-.2")));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real64_Modf(Real64_ParseC("1.8"), &intPart), Real64_ParseC(".8")));
	ASSERT(Eq(intPart, RI(1)));
	ASSERT(Eq(Real64_Modf(Real64_ParseC("-1.8"), &intPart), Real64_ParseC("-.8")));
	ASSERT(Eq(intPart, RI(-1)));
	ASSERT(Eq(Real64_Modf(RD(2.0), &intPart), Real64_Zero));
	ASSERT(Eq(intPart, RI(2)));
	ASSERT(Eq(Real64_Modf(RD(-2.0), &intPart), Real64_NegZero));
	ASSERT(Eq(intPart, RI(-2)));
}
END_TEST

START_TEST(CanRoundReal64)
{
	ASSERT(Eq(Real64_Round(RI(123)), RI(123)));
	ASSERT(Eq(Real64_Round(RI(-123)), RI(-123)));

	ASSERT(Eq(Real64_Round(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real64_Round(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real64_Round(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real64_Round(RD(1.2)), RD(1.0)));
	ASSERT(Eq(Real64_Round(RD(-1.2)), RD(-1.0)));
	ASSERT(Eq(Real64_Round(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real64_Round(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real64_Round(RD(1.8)), RD(2.0)));
	ASSERT(Eq(Real64_Round(RD(-1.8)), RD(-2.0)));
	ASSERT(Eq(Real64_Round(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real64_Round(RD(-2.0)), RD(-2.0)));

	ASSERT(Eq(Real64_Round(RD(4.5)), RD(5.0)));
	ASSERT(Eq(Real64_Round(RD(4.0)), RD(4.0)));
	ASSERT(Eq(Real64_Round(RD(3.5)), RD(4.0)));
	ASSERT(Eq(Real64_Round(RD(3.0)), RD(3.0)));
	ASSERT(Eq(Real64_Round(RD(2.5)), RD(3.0)));
	ASSERT(Eq(Real64_Round(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real64_Round(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real64_Round(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real64_Round(RD(0.5)), RD(1.0)));
	ASSERT(Eq(Real64_Round(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real64_Round(RD(-0.5)), RD(-1.0)));
	ASSERT(Eq(Real64_Round(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real64_Round(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real64_Round(RD(-2.0)), RD(-2.0)));
	ASSERT(Eq(Real64_Round(RD(-2.5)), RD(-3.0)));
	ASSERT(Eq(Real64_Round(RD(-3.0)), RD(-3.0)));
	ASSERT(Eq(Real64_Round(RD(-3.5)), RD(-4.0)));
	ASSERT(Eq(Real64_Round(RD(-4.0)), RD(-4.0)));
	ASSERT(Eq(Real64_Round(RD(-4.5)), RD(-5.0)));
}
END_TEST

START_TEST(CanBankRoundReal64)
{
	ASSERT(Eq(Real64_BankRound(RD(4.5)), RD(4.0)));
	ASSERT(Eq(Real64_BankRound(RD(4.0)), RD(4.0)));
	ASSERT(Eq(Real64_BankRound(RD(3.5)), RD(4.0)));
	ASSERT(Eq(Real64_BankRound(RD(3.0)), RD(3.0)));
	ASSERT(Eq(Real64_BankRound(RD(2.5)), RD(2.0)));
	ASSERT(Eq(Real64_BankRound(RD(2.0)), RD(2.0)));
	ASSERT(Eq(Real64_BankRound(RD(1.5)), RD(2.0)));
	ASSERT(Eq(Real64_BankRound(RD(1.0)), RD(1.0)));
	ASSERT(Eq(Real64_BankRound(RD(0.5)), RD(0.0)));
	ASSERT(Eq(Real64_BankRound(RD(0.0)), RD(0.0)));
	ASSERT(Eq(Real64_BankRound(RD(-0.5)), Real64_NegZero));
	ASSERT(Eq(Real64_BankRound(RD(-1.0)), RD(-1.0)));
	ASSERT(Eq(Real64_BankRound(RD(-1.5)), RD(-2.0)));
	ASSERT(Eq(Real64_BankRound(RD(-2.0)), RD(-2.0)));
	ASSERT(Eq(Real64_BankRound(RD(-2.5)), RD(-2.0)));
	ASSERT(Eq(Real64_BankRound(RD(-3.0)), RD(-3.0)));
	ASSERT(Eq(Real64_BankRound(RD(-3.5)), RD(-4.0)));
	ASSERT(Eq(Real64_BankRound(RD(-4.0)), RD(-4.0)));
	ASSERT(Eq(Real64_BankRound(RD(-4.5)), RD(-4.0)));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Test-Function Tests.

START_TEST(CanTestForInfinityReal64)
{
	ASSERT(!Real64_IsInf(Real64_NegNaN));
	ASSERT( Real64_IsInf(Real64_NegInf));
	ASSERT(!Real64_IsInf(Real64_NegSixteen));
	ASSERT(!Real64_IsInf(Real64_NegTen));
	ASSERT(!Real64_IsInf(Real64_NegTwo));
	ASSERT(!Real64_IsInf(Real64_NegOne));
	ASSERT(!Real64_IsInf(Real64_NegZero));
	ASSERT(!Real64_IsInf(Real64_Zero));
	ASSERT(!Real64_IsInf(Real64_One));
	ASSERT(!Real64_IsInf(Real64_Two));
	ASSERT(!Real64_IsInf(Real64_Ten));
	ASSERT(!Real64_IsInf(Real64_Sixteen));
	ASSERT( Real64_IsInf(Real64_Inf));
	ASSERT(!Real64_IsInf(Real64_NaN));
}
END_TEST

START_TEST(CanTestForNaNReal64)
{
	ASSERT( Real64_IsNaN(Real64_NegNaN));
	ASSERT(!Real64_IsNaN(Real64_NegInf));
	ASSERT(!Real64_IsNaN(Real64_NegSixteen));
	ASSERT(!Real64_IsNaN(Real64_NegTen));
	ASSERT(!Real64_IsNaN(Real64_NegTwo));
	ASSERT(!Real64_IsNaN(Real64_NegOne));
	ASSERT(!Real64_IsNaN(Real64_NegZero));
	ASSERT(!Real64_IsNaN(Real64_Zero));
	ASSERT(!Real64_IsNaN(Real64_One));
	ASSERT(!Real64_IsNaN(Real64_Two));
	ASSERT(!Real64_IsNaN(Real64_Ten));
	ASSERT(!Real64_IsNaN(Real64_Sixteen));
	ASSERT(!Real64_IsNaN(Real64_Inf));
	ASSERT( Real64_IsNaN(Real64_NaN));
}
END_TEST

START_TEST(CanTestForNegativeReal64)
{
	ASSERT( Real64_IsNeg(Real64_NegNaN));
	ASSERT( Real64_IsNeg(Real64_NegInf));
	ASSERT( Real64_IsNeg(Real64_NegSixteen));
	ASSERT( Real64_IsNeg(Real64_NegTen));
	ASSERT( Real64_IsNeg(Real64_NegTwo));
	ASSERT( Real64_IsNeg(Real64_NegOne));
	ASSERT( Real64_IsNeg(Real64_NegZero));
	ASSERT(!Real64_IsNeg(Real64_Zero));
	ASSERT(!Real64_IsNeg(Real64_One));
	ASSERT(!Real64_IsNeg(Real64_Two));
	ASSERT(!Real64_IsNeg(Real64_Ten));
	ASSERT(!Real64_IsNeg(Real64_Sixteen));
	ASSERT(!Real64_IsNeg(Real64_Inf));
	ASSERT(!Real64_IsNeg(Real64_NaN));
}
END_TEST

START_TEST(CanTestForZeroReal64)
{
	ASSERT(!Real64_IsZero(Real64_NegNaN));
	ASSERT(!Real64_IsZero(Real64_NegInf));
	ASSERT(!Real64_IsZero(Real64_NegSixteen));
	ASSERT(!Real64_IsZero(Real64_NegTen));
	ASSERT(!Real64_IsZero(Real64_NegTwo));
	ASSERT(!Real64_IsZero(Real64_NegOne));
	ASSERT( Real64_IsZero(Real64_NegZero));
	ASSERT( Real64_IsZero(Real64_Zero));
	ASSERT(!Real64_IsZero(Real64_One));
	ASSERT(!Real64_IsZero(Real64_Two));
	ASSERT(!Real64_IsZero(Real64_Ten));
	ASSERT(!Real64_IsZero(Real64_Sixteen));
	ASSERT(!Real64_IsZero(Real64_Inf));
	ASSERT(!Real64_IsZero(Real64_NaN));
}
END_TEST

START_TEST(CanTestForFiniteReal64)
{
	ASSERT(!Real64_IsFinite(Real64_NegNaN));
	ASSERT(!Real64_IsFinite(Real64_NegInf));
	ASSERT( Real64_IsFinite(Real64_NegSixteen));
	ASSERT( Real64_IsFinite(Real64_NegTen));
	ASSERT( Real64_IsFinite(Real64_NegTwo));
	ASSERT( Real64_IsFinite(Real64_NegOne));
	ASSERT( Real64_IsFinite(Real64_NegZero));
	ASSERT( Real64_IsFinite(Real64_Zero));
	ASSERT( Real64_IsFinite(Real64_One));
	ASSERT( Real64_IsFinite(Real64_Two));
	ASSERT( Real64_IsFinite(Real64_Ten));
	ASSERT( Real64_IsFinite(Real64_Sixteen));
	ASSERT(!Real64_IsFinite(Real64_Inf));
	ASSERT(!Real64_IsFinite(Real64_NaN));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Comparison Tests.

START_TEST(CanCompareEqualReal64)
{
	ASSERT(!Real64_Eq(Real64_NegNaN, Real64_NegNaN));
	ASSERT(!Real64_Eq(Real64_NaN, Real64_NaN));
	ASSERT(!Real64_Eq(Real64_NegNaN, Real64_NaN));
	ASSERT(!Real64_Eq(Real64_NaN, Real64_NegNaN));

	ASSERT(Real64_Eq(Real64_NegZero, Real64_NegZero));
	ASSERT(Real64_Eq(Real64_Zero, Real64_Zero));
	ASSERT(Real64_Eq(Real64_NegZero, Real64_Zero));
	ASSERT(Real64_Eq(Real64_Zero, Real64_NegZero));

	ASSERT(Real64_Eq(Real64_NegInf, Real64_NegInf));
	ASSERT(Real64_Eq(Real64_NegOne, Real64_NegOne));
	ASSERT(Real64_Eq(Real64_One, Real64_One));
	ASSERT(Real64_Eq(Real64_Inf, Real64_Inf));

	ASSERT(!Real64_Eq(Real64_NegInf, Real64_Inf));
	ASSERT(!Real64_Eq(Real64_NegOne, Real64_One));
	ASSERT(!Real64_Eq(Real64_One, Real64_Two));
	ASSERT(!Real64_Eq(Real64_Inf, Real64_NegInf));
}
END_TEST

START_TEST(CanCompareNotEqualReal64)
{
	ASSERT(Real64_Ne(Real64_NegNaN, Real64_NegNaN));
	ASSERT(Real64_Ne(Real64_NaN, Real64_NaN));
	ASSERT(Real64_Ne(Real64_NegNaN, Real64_NaN));
	ASSERT(Real64_Ne(Real64_NaN, Real64_NegNaN));

	ASSERT(!Real64_Ne(Real64_NegZero, Real64_NegZero));
	ASSERT(!Real64_Ne(Real64_Zero, Real64_Zero));
	ASSERT(!Real64_Ne(Real64_NegZero, Real64_Zero));
	ASSERT(!Real64_Ne(Real64_Zero, Real64_NegZero));

	ASSERT(!Real64_Ne(Real64_NegInf, Real64_NegInf));
	ASSERT(!Real64_Ne(Real64_NegOne, Real64_NegOne));
	ASSERT(!Real64_Ne(Real64_One, Real64_One));
	ASSERT(!Real64_Ne(Real64_Inf, Real64_Inf));

	ASSERT(Real64_Ne(Real64_NegInf, Real64_Inf));
	ASSERT(Real64_Ne(Real64_NegOne, Real64_One));
	ASSERT(Real64_Ne(Real64_One, Real64_Two));
	ASSERT(Real64_Ne(Real64_Inf, Real64_NegInf));
}
END_TEST

START_TEST(CanCompareLessThanReal64)
{
	ASSERT(!Real64_Lt(Real64_NegNaN, Real64_NegNaN));
	ASSERT(!Real64_Lt(Real64_NaN, Real64_NaN));
	ASSERT(!Real64_Lt(Real64_NegNaN, Real64_NaN));
	ASSERT(!Real64_Lt(Real64_NaN, Real64_NegNaN));
	ASSERT(!Real64_Lt(Real64_One, Real64_NaN));
	ASSERT(!Real64_Lt(Real64_NaN, Real64_One));

	ASSERT(!Real64_Lt(Real64_NegZero, Real64_NegZero));
	ASSERT(!Real64_Lt(Real64_Zero, Real64_Zero));
	ASSERT(!Real64_Lt(Real64_NegZero, Real64_Zero));
	ASSERT(!Real64_Lt(Real64_Zero, Real64_NegZero));

	ASSERT(!Real64_Lt(Real64_NegInf, Real64_NegInf));
	ASSERT(!Real64_Lt(Real64_NegOne, Real64_NegOne));
	ASSERT(!Real64_Lt(Real64_One, Real64_One));
	ASSERT(!Real64_Lt(Real64_Inf, Real64_Inf));

	ASSERT(Real64_Lt(Real64_NegInf, Real64_Inf));
	ASSERT(!Real64_Lt(Real64_Inf, Real64_NegInf));
	ASSERT(Real64_Lt(Real64_NegOne, Real64_One));
	ASSERT(!Real64_Lt(Real64_One, Real64_NegOne));
	ASSERT(Real64_Lt(Real64_One, Real64_Two));
	ASSERT(!Real64_Lt(Real64_Two, Real64_One));
	ASSERT(Real64_Lt(Real64_Zero, Real64_Inf));
	ASSERT(!Real64_Lt(Real64_Inf, Real64_Zero));
	ASSERT(Real64_Lt(Real64_NegInf, Real64_Zero));
	ASSERT(!Real64_Lt(Real64_Zero, Real64_NegInf));
}
END_TEST

START_TEST(CanCompareGreaterThanReal64)
{
	ASSERT(!Real64_Gt(Real64_NegNaN, Real64_NegNaN));
	ASSERT(!Real64_Gt(Real64_NaN, Real64_NaN));
	ASSERT(!Real64_Gt(Real64_NegNaN, Real64_NaN));
	ASSERT(!Real64_Gt(Real64_NaN, Real64_NegNaN));
	ASSERT(!Real64_Gt(Real64_One, Real64_NaN));
	ASSERT(!Real64_Gt(Real64_NaN, Real64_One));

	ASSERT(!Real64_Gt(Real64_NegZero, Real64_NegZero));
	ASSERT(!Real64_Gt(Real64_Zero, Real64_Zero));
	ASSERT(!Real64_Gt(Real64_NegZero, Real64_Zero));
	ASSERT(!Real64_Gt(Real64_Zero, Real64_NegZero));

	ASSERT(!Real64_Gt(Real64_NegInf, Real64_NegInf));
	ASSERT(!Real64_Gt(Real64_NegOne, Real64_NegOne));
	ASSERT(!Real64_Gt(Real64_One, Real64_One));
	ASSERT(!Real64_Gt(Real64_Inf, Real64_Inf));

	ASSERT(!Real64_Gt(Real64_NegInf, Real64_Inf));
	ASSERT(Real64_Gt(Real64_Inf, Real64_NegInf));
	ASSERT(!Real64_Gt(Real64_NegOne, Real64_One));
	ASSERT(Real64_Gt(Real64_One, Real64_NegOne));
	ASSERT(!Real64_Gt(Real64_One, Real64_Two));
	ASSERT(Real64_Gt(Real64_Two, Real64_One));
	ASSERT(!Real64_Gt(Real64_Zero, Real64_Inf));
	ASSERT(Real64_Gt(Real64_Inf, Real64_Zero));
	ASSERT(!Real64_Gt(Real64_NegInf, Real64_Zero));
	ASSERT(Real64_Gt(Real64_Zero, Real64_NegInf));
}
END_TEST

START_TEST(CanCompareLessThanOrEqualReal64)
{
	ASSERT(!Real64_Le(Real64_NegNaN, Real64_NegNaN));
	ASSERT(!Real64_Le(Real64_NaN, Real64_NaN));
	ASSERT(!Real64_Le(Real64_NegNaN, Real64_NaN));
	ASSERT(!Real64_Le(Real64_NaN, Real64_NegNaN));
	ASSERT(!Real64_Le(Real64_One, Real64_NaN));
	ASSERT(!Real64_Le(Real64_NaN, Real64_One));

	ASSERT(Real64_Le(Real64_NegZero, Real64_NegZero));
	ASSERT(Real64_Le(Real64_Zero, Real64_Zero));
	ASSERT(Real64_Le(Real64_NegZero, Real64_Zero));
	ASSERT(Real64_Le(Real64_Zero, Real64_NegZero));

	ASSERT(Real64_Le(Real64_NegInf, Real64_NegInf));
	ASSERT(Real64_Le(Real64_NegOne, Real64_NegOne));
	ASSERT(Real64_Le(Real64_One, Real64_One));
	ASSERT(Real64_Le(Real64_Inf, Real64_Inf));

	ASSERT(Real64_Le(Real64_NegInf, Real64_Inf));
	ASSERT(!Real64_Le(Real64_Inf, Real64_NegInf));
	ASSERT(Real64_Le(Real64_NegOne, Real64_One));
	ASSERT(!Real64_Le(Real64_One, Real64_NegOne));
	ASSERT(Real64_Le(Real64_One, Real64_Two));
	ASSERT(!Real64_Le(Real64_Two, Real64_One));
	ASSERT(Real64_Le(Real64_Zero, Real64_Inf));
	ASSERT(!Real64_Le(Real64_Inf, Real64_Zero));
	ASSERT(Real64_Le(Real64_NegInf, Real64_Zero));
	ASSERT(!Real64_Le(Real64_Zero, Real64_NegInf));
}
END_TEST

START_TEST(CanCompareGreaterThanOrEqualReal64)
{
	ASSERT(!Real64_Ge(Real64_NegNaN, Real64_NegNaN));
	ASSERT(!Real64_Ge(Real64_NaN, Real64_NaN));
	ASSERT(!Real64_Ge(Real64_NegNaN, Real64_NaN));
	ASSERT(!Real64_Ge(Real64_NaN, Real64_NegNaN));
	ASSERT(!Real64_Ge(Real64_One, Real64_NaN));
	ASSERT(!Real64_Ge(Real64_NaN, Real64_One));

	ASSERT(Real64_Ge(Real64_NegZero, Real64_NegZero));
	ASSERT(Real64_Ge(Real64_Zero, Real64_Zero));
	ASSERT(Real64_Ge(Real64_NegZero, Real64_Zero));
	ASSERT(Real64_Ge(Real64_Zero, Real64_NegZero));

	ASSERT(Real64_Ge(Real64_NegInf, Real64_NegInf));
	ASSERT(Real64_Ge(Real64_NegOne, Real64_NegOne));
	ASSERT(Real64_Ge(Real64_One, Real64_One));
	ASSERT(Real64_Ge(Real64_Inf, Real64_Inf));

	ASSERT(!Real64_Ge(Real64_NegInf, Real64_Inf));
	ASSERT(Real64_Ge(Real64_Inf, Real64_NegInf));
	ASSERT(!Real64_Ge(Real64_NegOne, Real64_One));
	ASSERT(Real64_Ge(Real64_One, Real64_NegOne));
	ASSERT(!Real64_Ge(Real64_One, Real64_Two));
	ASSERT(Real64_Ge(Real64_Two, Real64_One));
	ASSERT(!Real64_Ge(Real64_Zero, Real64_Inf));
	ASSERT(Real64_Ge(Real64_Inf, Real64_Zero));
	ASSERT(!Real64_Ge(Real64_NegInf, Real64_Zero));
	ASSERT(Real64_Ge(Real64_Zero, Real64_NegInf));
}
END_TEST

START_TEST(CanCompareOrderedReal64)
{
	ASSERT(Real64_Compare(Real64_NegNaN, Real64_NegNaN) == 0);
	ASSERT(Real64_Compare(Real64_NegNaN, Real64_NegInf) < 0);
	ASSERT(Real64_Compare(Real64_NegNaN, Real64_NegOne) < 0);
	ASSERT(Real64_Compare(Real64_NegNaN, Real64_Zero) < 0);
	ASSERT(Real64_Compare(Real64_NegNaN, Real64_One) < 0);
	ASSERT(Real64_Compare(Real64_NegNaN, Real64_Inf) < 0);
	ASSERT(Real64_Compare(Real64_NegNaN, Real64_NaN) < 0);

	ASSERT(Real64_Compare(Real64_NegInf, Real64_NegNaN) > 0);
	ASSERT(Real64_Compare(Real64_NegInf, Real64_NegInf) == 0);
	ASSERT(Real64_Compare(Real64_NegInf, Real64_NegOne) < 0);
	ASSERT(Real64_Compare(Real64_NegInf, Real64_Zero) < 0);
	ASSERT(Real64_Compare(Real64_NegInf, Real64_One) < 0);
	ASSERT(Real64_Compare(Real64_NegInf, Real64_Inf) < 0);
	ASSERT(Real64_Compare(Real64_NegInf, Real64_NaN) < 0);

	ASSERT(Real64_Compare(Real64_NegOne, Real64_NegNaN) > 0);
	ASSERT(Real64_Compare(Real64_NegOne, Real64_NegInf) > 0);
	ASSERT(Real64_Compare(Real64_NegOne, Real64_NegOne) == 0);
	ASSERT(Real64_Compare(Real64_NegOne, Real64_Zero) < 0);
	ASSERT(Real64_Compare(Real64_NegOne, Real64_One) < 0);
	ASSERT(Real64_Compare(Real64_NegOne, Real64_Inf) < 0);
	ASSERT(Real64_Compare(Real64_NegOne, Real64_NaN) < 0);

	ASSERT(Real64_Compare(Real64_Zero, Real64_NegNaN) > 0);
	ASSERT(Real64_Compare(Real64_Zero, Real64_NegInf) > 0);
	ASSERT(Real64_Compare(Real64_Zero, Real64_NegOne) > 0);
	ASSERT(Real64_Compare(Real64_Zero, Real64_Zero) == 0);
	ASSERT(Real64_Compare(Real64_Zero, Real64_One) < 0);
	ASSERT(Real64_Compare(Real64_Zero, Real64_Inf) < 0);
	ASSERT(Real64_Compare(Real64_Zero, Real64_NaN) < 0);

	ASSERT(Real64_Compare(Real64_One, Real64_NegNaN) > 0);
	ASSERT(Real64_Compare(Real64_One, Real64_NegInf) > 0);
	ASSERT(Real64_Compare(Real64_One, Real64_NegOne) > 0);
	ASSERT(Real64_Compare(Real64_One, Real64_Zero) > 0);
	ASSERT(Real64_Compare(Real64_One, Real64_One) == 0);
	ASSERT(Real64_Compare(Real64_One, Real64_Inf) < 0);
	ASSERT(Real64_Compare(Real64_One, Real64_NaN) < 0);

	ASSERT(Real64_Compare(Real64_Inf, Real64_NegNaN) > 0);
	ASSERT(Real64_Compare(Real64_Inf, Real64_NegInf) > 0);
	ASSERT(Real64_Compare(Real64_Inf, Real64_NegOne) > 0);
	ASSERT(Real64_Compare(Real64_Inf, Real64_Zero) > 0);
	ASSERT(Real64_Compare(Real64_Inf, Real64_One) > 0);
	ASSERT(Real64_Compare(Real64_Inf, Real64_Inf) == 0);
	ASSERT(Real64_Compare(Real64_Inf, Real64_NaN) < 0);

	ASSERT(Real64_Compare(Real64_NaN, Real64_NegNaN) > 0);
	ASSERT(Real64_Compare(Real64_NaN, Real64_NegInf) > 0);
	ASSERT(Real64_Compare(Real64_NaN, Real64_NegOne) > 0);
	ASSERT(Real64_Compare(Real64_NaN, Real64_Zero) > 0);
	ASSERT(Real64_Compare(Real64_NaN, Real64_One) > 0);
	ASSERT(Real64_Compare(Real64_NaN, Real64_Inf) > 0);
	ASSERT(Real64_Compare(Real64_NaN, Real64_NaN) == 0);
}
END_TEST

START_TEST(CanDetermineOrderabilityReal64)
{
	ASSERT(!Real64_IsOrderable(Real64_NegNaN, Real64_NegNaN));
	ASSERT(!Real64_IsOrderable(Real64_NaN, Real64_NaN));
	ASSERT(!Real64_IsOrderable(Real64_NegNaN, Real64_NaN));
	ASSERT(!Real64_IsOrderable(Real64_NaN, Real64_NegNaN));
	ASSERT(!Real64_IsOrderable(Real64_One, Real64_NaN));
	ASSERT(!Real64_IsOrderable(Real64_NaN, Real64_One));

	ASSERT(Real64_IsOrderable(Real64_NegZero, Real64_NegZero));
	ASSERT(Real64_IsOrderable(Real64_Zero, Real64_Zero));
	ASSERT(Real64_IsOrderable(Real64_NegZero, Real64_Zero));
	ASSERT(Real64_IsOrderable(Real64_Zero, Real64_NegZero));

	ASSERT(Real64_IsOrderable(Real64_NegInf, Real64_NegInf));
	ASSERT(Real64_IsOrderable(Real64_NegOne, Real64_NegOne));
	ASSERT(Real64_IsOrderable(Real64_One, Real64_One));
	ASSERT(Real64_IsOrderable(Real64_Inf, Real64_Inf));

	ASSERT(Real64_IsOrderable(Real64_NegInf, Real64_Inf));
	ASSERT(Real64_IsOrderable(Real64_Inf, Real64_NegInf));
	ASSERT(Real64_IsOrderable(Real64_NegOne, Real64_One));
	ASSERT(Real64_IsOrderable(Real64_One, Real64_NegOne));
	ASSERT(Real64_IsOrderable(Real64_One, Real64_Two));
	ASSERT(Real64_IsOrderable(Real64_Two, Real64_One));
	ASSERT(Real64_IsOrderable(Real64_Zero, Real64_Inf));
	ASSERT(Real64_IsOrderable(Real64_Inf, Real64_Zero));
	ASSERT(Real64_IsOrderable(Real64_NegInf, Real64_Zero));
	ASSERT(Real64_IsOrderable(Real64_Zero, Real64_NegInf));
}
END_TEST

#include "real64_tests.generated.inc"
