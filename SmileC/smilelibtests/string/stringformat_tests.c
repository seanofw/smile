#include "../stdafx.h"

TEST_SUITE(StringFormatTests)

//-------------------------------------------------------------------------------------------------
//  Formatting Tests.

START_TEST(EmptyFormatStringsShouldProduceEmptyOutput)
{
	AssertString(String_FormatString(String_Empty), NULL, 0);
	AssertString(String_FormatString(NULL), NULL, 0);
}
END_TEST

#include "stringformat_tests.generated.inc"
