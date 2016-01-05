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

TEST_SUITE(StringHtmlTests)

//-------------------------------------------------------------------------------------------------
//  Basic HTML-Encoding Tests.

START_TEST(EmptyStringsShouldHtmlEncodeToEmptyStrings)
{
	ASSERT_STRING(String_HtmlEncode(String_Empty), NULL, 0);
}
END_TEST

START_TEST(HtmlEncodeShouldEncodeTheFourDangerousCharactersToNamedEntities)
{
	String str = String_Create("<This 'is & a \"test.\">", 22);
	ASSERT_STRING(String_HtmlEncode(str), "&lt;This 'is &amp; a &quot;test.&quot;&gt;", 42);
}
END_TEST

START_TEST(HtmlEncodeShouldOnlyChangeTheFourDangerousCharacters)
{
	String str = String_Create("<\xA0\x00\x1A\xFF'\t\n\r&#\x7F?\"test.\">", 21);
	ASSERT_STRING(String_HtmlEncode(str), "&lt;\xA0\x00\x1A\xFF'\t\n\r&amp;#\x7F?&quot;test.&quot;&gt;", 41);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  HTML-Encoding-to-ASCII Tests.

START_TEST(EmptyStringsShouldHtmlEncodeToAsciiToEmptyStrings)
{
	ASSERT_STRING(String_HtmlEncodeToAscii(String_Empty), NULL, 0);
}
END_TEST

START_TEST(HtmlEncodeToAsciiShouldEncodeTheFourDangerousCharactersToNamedEntities)
{
	String str = String_Create("<This 'is & a \"test.\">", 22);
	ASSERT_STRING(String_HtmlEncodeToAscii(str), "&lt;This 'is &amp; a &quot;test.&quot;&gt;", 42);
}
END_TEST

START_TEST(HtmlEncodeToAsciiShouldEncodeAllNonAsciiCharacters)
{
	String str = String_Create("\xEF\xBB\xBF<\xC2\xA0\x00\x1A'\n\r&#\x7F?\"test.\">", 23);
	ASSERT_STRING(String_HtmlEncodeToAscii(str), "&#65279;&lt;&nbsp;\x00\x1A'\n\r&amp;#\x7F?&quot;test.&quot;&gt;", 52);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  HTML-Decoding Tests.

START_TEST(EmptyStringsShouldHtmlDecodeToEmptyStrings)
{
	ASSERT_STRING(String_HtmlDecode(String_Empty), NULL, 0);
}
END_TEST

START_TEST(UnencodedStringsShouldHtmlDecodeToUnencodedStrings)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("This is a test.")), "This is a test.", 15);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeTheAsciiNamedEntities)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&lt;This &apos;is &amp; a &quot;test.&quot;&gt;")), "<This 'is & a \"test.\">", 22);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeNamedLatin1EntitiesToUtf8)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&nbsp;This &Aacute;is &frac12; a &aacute;test.&yuml;&THORN;")), "\xC2\xA0This \xC3\x81is \xC2\xBD a \xC3\xA1test.\xC3\xBF\xC3\x9E", 28);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeNamedNonLatin1EntitiesToUtf8)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&Sigma;This &alpha;is &Yuml; a &mdash;test.&forall;&hearts;")), "\xCE\xA3This \xCE\xB1is \xC5\xB8 a \xE2\x80\x94test.\xE2\x88\x80\xE2\x99\xA5", 31);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeNumericLatin1EntitiesToUtf8)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#160;This &#193;is &#189; a &#225;test.&#255;&#222;")), "\xC2\xA0This \xC3\x81is \xC2\xBD a \xC3\xA1test.\xC3\xBF\xC3\x9E", 28);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeDecimalNonLatin1EntitiesToUtf8)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#931;This &#945;is &#376; a &#8212;test.&#8704;&#9829;")), "\xCE\xA3This \xCE\xB1is \xC5\xB8 a \xE2\x80\x94test.\xE2\x88\x80\xE2\x99\xA5", 31);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeUppercaseHexadecimalNonLatin1EntitiesToUtf8)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#x3A3;This &#x3B1;is &#x178; a &#x2014;test.&#x2200;&#x2665;")), "\xCE\xA3This \xCE\xB1is \xC5\xB8 a \xE2\x80\x94test.\xE2\x88\x80\xE2\x99\xA5", 31);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeLowercaseHexadecimalNonLatin1EntitiesToUtf8)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#x3a3;This &#x3b1;is &#x178; a &#x2014;test.&#x2200;&#x2665;")), "\xCE\xA3This \xCE\xB1is \xC5\xB8 a \xE2\x80\x94test.\xE2\x88\x80\xE2\x99\xA5", 31);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeDecimalCornerCases)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#0;This &#1;is &#10; a &#65;test.&#27;&#32;")), "\0This \1is \n a Atest.\x1B ", 22);
}
END_TEST

START_TEST(HtmlDecodeShouldDecodeHexadecimalCornerCases)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#x0;This &#x1;is &#xA; a &#x41;test.&#x1B;&#x20;")), "\0This \1is \n a Atest.\x1B ", 22);
}
END_TEST

START_TEST(HtmlDecodeShouldSkipBadInput)
{
	ASSERT_STRING(String_HtmlDecode(String_FromC("&frog;This is a test.")), "&frog;This is a test.", 15 + 6);
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#;This is a test.")), "&#;This is a test.", 15 + 3);
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#15f;This is a test.")), "&#15f;This is a test.", 15 + 6);
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#frog;This is a test.")), "&#frog;This is a test.", 15 + 7);
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#x;This is a test.")), "&#x;This is a test.", 15 + 4);
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#xz;This is a test.")), "&#xz;This is a test.", 15 + 5);
	ASSERT_STRING(String_HtmlDecode(String_FromC("&#x15z;This is a test.")), "&#x15z;This is a test.", 15 + 7);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Basic URL-Encoding Tests.
//
//  URL-encoding means encoding non-ASCII characters, space, and these punctuation marks:
//     !  *  '  ;  :  @  &  =  +  $  ,  /  ?  #  %  (  )  [  ]

START_TEST(EmptyStringsShouldUrlEncodeToEmptyStrings)
{
	ASSERT_STRING(String_UrlEncode(String_Empty), NULL, 0);
}
END_TEST

START_TEST(UrlEncodeShouldEncodeDangerousCharactersToEscapes)
{
	String str = String_Create("[(!*This ';is :@&= a +%$test,/?#)]", 34);
	const char *expectedResult = "%5B%28%21%2AThis%20%27%3Bis%20%3A%40%26%3D%20a%20%2B%25%24test%2C%2F%3F%23%29%5D";
	ASSERT_STRING(String_UrlEncode(str), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(UrlEncodeShouldIgnoreSafePunctuation)
{
	String str = String_FromC("\"-.<>\\^_`{|}~Pack my box with five dozen liquor jugs.");
	const char *expectedResult = "\"-.<>\\^_`{|}~Pack%20my%20box%20with%20five%20dozen%20liquor%20jugs.";
	ASSERT_STRING(String_UrlEncode(str), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(UrlEncodeShouldEncodeNonAsciiCharacters)
{
	String str = String_Create("This\0is\r\na\x7F\xC2\xA0test.", 18);
	const char *expectedResult = "This%00is%0D%0Aa%7F%C2%A0test.";
	ASSERT_STRING(String_UrlEncode(str), expectedResult, StrLen(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  URL-Query-Encoding Tests.
//
//  URL-encoding means encoding non-ASCII characters, space, and these punctuation marks:
//     &  =  ?  #  %

START_TEST(EmptyStringsShouldUrlQueryEncodeToEmptyStrings)
{
	ASSERT_STRING(String_UrlQueryEncode(String_Empty), NULL, 0);
}
END_TEST

START_TEST(UrlQueryEncodeShouldEncodeOnlyAFewDangerousCharactersToEscapes)
{
	String str = String_Create("[(!*This ';is :@&= a +%$test,/?#)]", 34);
	const char *expectedResult = "[(!*This%20';is%20:@%26%3D%20a%20+%25$test,/%3F%23)]";
	ASSERT_STRING(String_UrlQueryEncode(str), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(UrlQueryEncodeShouldIgnoreSafePunctuation)
{
	String str = String_FromC("\"-.<>\\^_`{|}~Pack my box with five dozen liquor jugs.");
	const char *expectedResult = "\"-.<>\\^_`{|}~Pack%20my%20box%20with%20five%20dozen%20liquor%20jugs.";
	ASSERT_STRING(String_UrlQueryEncode(str), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(UrlQueryEncodeShouldEncodeNonAsciiCharacters)
{
	String str = String_Create("This\0is\r\na\x7F\xC2\xA0test.", 18);
	const char *expectedResult = "This%00is%0D%0Aa%7F%C2%A0test.";
	ASSERT_STRING(String_UrlQueryEncode(str), expectedResult, StrLen(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  URL-Decoding Tests.
//
//  URL-decoding means decoding anything starting with a '%' followed by two hex characters
//  into its equivalent byte.  Malformed codes are skipped.

START_TEST(EmptyStringsShouldUrlDecodeToEmptyStrings)
{
	ASSERT_STRING(String_UrlDecode(String_Empty), NULL, 0);
}
END_TEST

START_TEST(UrlDecodeShouldDecodeDangerousCharactersFromEscapes)
{
	String str = String_FromC("%5B%28%21%2AThis%20%27%3Bis%20%3A%40%26%3D%20a%20%2B%25%24test%2C%2F%3F%23%29%5D");
	const char *expectedResult = "[(!*This ';is :@&= a +%$test,/?#)]";
	ASSERT_STRING(String_UrlDecode(str), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(UrlDecodeShouldDecodeUppercaseNonAsciiCharacters)
{
	String str = String_FromC("This%00is%0D%0Aa%7F%C2%A0test.");
	ASSERT_STRING(String_UrlDecode(str), "This\0is\r\na\x7F\xC2\xA0test.", 18);
}
END_TEST

START_TEST(UrlDecodeShouldDecodeLowercaseNonAsciiCharacters)
{
	String str = String_FromC("This%00is%0d%0aa%7f%c2%a0test.");
	ASSERT_STRING(String_UrlDecode(str), "This\0is\r\na\x7F\xC2\xA0test.", 18);
}
END_TEST

START_TEST(UrlDecodeShouldSkipBadEscapes)
{
	String str = String_FromC("This%is a%%aa%test.%f");
	const char *expectedResult = "This%is a%\xAA%test.%f";
	ASSERT_STRING(String_UrlDecode(str), expectedResult, StrLen(expectedResult));

	str = String_FromC("This%is a%%aa%test.%");
	expectedResult = "This%is a%\xAA%test.%";
	ASSERT_STRING(String_UrlDecode(str), expectedResult, StrLen(expectedResult));
}
END_TEST

#include "stringhtml_tests.generated.inc"
