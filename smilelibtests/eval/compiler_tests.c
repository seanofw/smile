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

#include <smile/env/env.h>
#include <smile/eval/bytecode.h>
#include <smile/eval/opcode.h>
#include <smile/eval/compiler.h>
#include <smile/parsing/parser.h>
#include <smile/smiletypes/numeric/smileinteger32.h>

TEST_SUITE(CompilerTests)

static SmileObject Parse(const char *text)
{
	String source;
	Lexer lexer;
	Parser parser;
	ParseScope scope;
	SmileObject expr;

	Smile_ResetEnvironment();

	source = String_FromC(text);

	lexer = Lexer_Create(source, 0, String_Length(source), GetTestScriptName(), 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	parser = Parser_Create();
	scope = ParseScope_CreateRoot();

	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"), PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"), PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gc"), PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gd"), PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "ge"), PARSEDECL_GLOBAL, NULL, NULL);

	expr = Parser_Parse(parser, lexer, scope);

	if (SMILE_KIND(parser->firstMessage) == SMILE_KIND_NULL) {
		String stringified = SmileObject_Stringify((SmileObject)expr);
		return expr;
	}
	else {
		ParseError error = (ParseMessage)(parser->firstMessage->a);
		String errorMessage = String_Format("%S: line %d: %S",
			error->position != NULL ? error->position->filename : String_Empty,
			error->position != NULL ? error->position->line : 0,
			error->message
		);
		FAIL_TEST(String_ToC(errorMessage));
		return NullObject;
	}
}

START_TEST(CanCompileNull)
{
	SmileObject expr = Parse("[]");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	const char *expectedResult =
		"0: \tLdNull\n"
		"1: \tRet\n";

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileByte)
{
	SmileObject expr = Parse("123x");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	const char *expectedResult =
		"0: \tLd8     123\n"
		"1: \tRet\n";

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt16)
{
	SmileObject expr = Parse("123s");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	const char *expectedResult =
		"0: \tLd16    123\n"
		"1: \tRet\n";

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt32)
{
	SmileObject expr = Parse("123t");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	const char *expectedResult =
		"0: \tLd32    123\n"
		"1: \tRet\n";

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt64)
{
	SmileObject expr = Parse("123");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	const char *expectedResult =
		"0: \tLd64    123\n"
		"1: \tRet\n";

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileBasicArithmetic)
{
	SmileObject expr = Parse("123 + 456");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLd64    123\t; test.sm:1\n"
		"1: \tLd64    456\t; test.sm:1\n"
		"2: \tBinary  `+ (%hd)\t; test.sm:1\n"
		"3: \tRet\n",
		Smile_KnownSymbols.plus
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileMildlyInterestingArithmetic)
{
	SmileObject expr = Parse("(123 + -456) * 50");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLd64    123\t; test.sm:1\n"
		"1: \tLd64    456\t; test.sm:1\n"
		"2: \tUnary   `- (%hd)\t; test.sm:1\n"
		"3: \tBinary  `+ (%hd)\t; test.sm:1\n"
		"4: \tLd64    50\t; test.sm:1\n"
		"5: \tBinary  `* (%hd)\t; test.sm:1\n"
		"6: \tRet\n",
		Smile_KnownSymbols.minus,
		Smile_KnownSymbols.plus,
		Smile_KnownSymbols.star
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
// Read/write of global variables, properties, and members.

START_TEST(CanCompileGlobalReadsAndWrites)
{
	SmileObject expr = Parse("ga = gb");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"1: \tStX     `ga (%hd)\t; test.sm:1\n"
		"2: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileReadsFromProperties)
{
	SmileObject expr = Parse("ga = gb.foo");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"1: \tLdProp  `foo (%hd)\t; test.sm:1\n"
		"2: \tStX     `ga (%hd)\t; test.sm:1\n"
		"3: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "foo"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileWritesToProperties)
{
	SmileObject expr = Parse("ga.foo = gb");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLdX     `ga (%hd)\t; test.sm:1\n"
		"1: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"2: \tStProp  `foo (%hd)\t; test.sm:1\n"
		"3: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "foo")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileReadsFromMembers)
{
	SmileObject expr = Parse("ga = gb:10");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"1: \tLd64    10\t; test.sm:1\n"
		"2: \tLdMember\t; test.sm:1\n"
		"3: \tStX     `ga (%hd)\t; test.sm:1\n"
		"4: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileWritesToMembers)
{
	SmileObject expr = Parse("ga:10 = gb");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLdX     `ga (%hd)\t; test.sm:1\n"
		"1: \tLd64    10\t; test.sm:1\n"
		"2: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"3: \tLdNull\t; test.sm:1\n"
		"4: \tStMember\t; test.sm:1\n"
		"5: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
// In-place update of global variables, properties, and members.

START_TEST(CanCompileGlobalMutations)
{
	SmileObject expr = Parse("ga += gb");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLdX     `ga (%hd)\t; test.sm:1\n"
		"1: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"2: \tBinary  `+ (%hd)\t; test.sm:1\n"
		"3: \tStX     `ga (%hd)\t; test.sm:1\n"
		"4: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileMutationsOfProperties)
{
	SmileObject expr = Parse("ga.foo += gb");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLdX     `ga (%hd)\t; test.sm:1\n"
		"1: \tDup1\t; test.sm:1\n"
		"2: \tLdProp  `foo (%hd)\t; test.sm:1\n"
		"3: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"4: \tBinary  `+ (%hd)\t; test.sm:1\n"
		"5: \tStProp  `foo (%hd)\t; test.sm:1\n"
		"6: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "foo"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "foo")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileMutationsOfMembers)
{
	SmileObject expr = Parse("ga:10 += gb");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLdX     `ga (%hd)\t; test.sm:1\n"
		"1: \tLd64    10\t; test.sm:1\n"
		"2: \tDup2\t; test.sm:1\n"
		"3: \tDup2\t; test.sm:1\n"
		"4: \tLdMember\t; test.sm:1\n"
		"5: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"6: \tBinary  `+ (%hd)\t; test.sm:1\n"
		"7: \tLdNull\t; test.sm:1\n"
		"8: \tStMember\t; test.sm:1\n"
		"9: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
// Read/write of global variables, properties, and members.

START_TEST(CanCompileScopeVariableReads)
{
	SmileObject expr = Parse("{ a = gb }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `a (0)\t; test.sm:1\n"
		"1: \tLdX     `gb (%hd)\t; test.sm:1\n"
		"2: \tStLoc0  `a (0)\t; test.sm:1\n"
		"3: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 1);
	ASSERT(globalFunction->closureInfo.tempSize == 1);
}
END_TEST

START_TEST(CanCompileNestedScopeVariableReads)
{
	SmileObject expr = Parse("{ var b = 10 { var a = b, c = a + b } }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `b (0)\t; test.sm:1\n"
		"1: \tLd64    10\t; test.sm:1\n"
		"2: \tStpLoc0 `b (0)\t; test.sm:1\n"
		"3: \tNullLoc0 `a (1)\t; test.sm:1\n"
		"4: \tNullLoc0 `c (2)\t; test.sm:1\n"
		"5: \tLdLoc0  `b (0)\t; test.sm:1\n"
		"6: \tStpLoc0 `a (1)\t; test.sm:1\n"
		"7: \tLdLoc0  `a (1)\t; test.sm:1\n"
		"8: \tLdLoc0  `b (0)\t; test.sm:1\n"
		"9: \tBinary  `+ (%hd)\t; test.sm:1\n"
		"10: \tStLoc0  `c (2)\t; test.sm:1\n"
		"11: \tRet\n",
		Smile_KnownSymbols.plus
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 3);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(NestedScopesVariablesDontOverlap)
{
	SmileObject expr = Parse("{ var b = 10 { var a = b, c = a + b } { var d = b * 20 } }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `b (0)\t; test.sm:1\n"
		"1: \tLd64    10\t; test.sm:1\n"
		"2: \tStpLoc0 `b (0)\t; test.sm:1\n"
		"3: \tNullLoc0 `a (1)\t; test.sm:1\n"
		"4: \tNullLoc0 `c (2)\t; test.sm:1\n"
		"5: \tLdLoc0  `b (0)\t; test.sm:1\n"
		"6: \tStpLoc0 `a (1)\t; test.sm:1\n"
		"7: \tLdLoc0  `a (1)\t; test.sm:1\n"
		"8: \tLdLoc0  `b (0)\t; test.sm:1\n"
		"9: \tBinary  `+ (%hd)\t; test.sm:1\n"
		"10: \tStpLoc0 `c (2)\t; test.sm:1\n"
		"11: \tNullLoc0 `d (3)\t; test.sm:1\n"
		"12: \tLdLoc0  `b (0)\t; test.sm:1\n"
		"13: \tLd64    20\t; test.sm:1\n"
		"14: \tBinary  `* (%hd)\t; test.sm:1\n"
		"15: \tStLoc0  `d (3)\t; test.sm:1\n"
		"16: \tRet\n",
		Smile_KnownSymbols.plus,
		Smile_KnownSymbols.star
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 4);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

//-------------------------------------------------------------------------------------------------

START_TEST(CanCompileSimpleConditionals)
{
	SmileObject expr = Parse("[$if 1 < 10 `then-side `else-side]");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLd64    1\t; test.sm:1\n"
		"1: \tLd64    10\t; test.sm:1\n"
		"2: \tBinary  `< (%hd)\t; test.sm:1\n"
		"3: \tBf      >L6\t; test.sm:1\n"
		"4: \tLdSym   `then-side (%hd)\t; test.sm:1\n"
		"5: \tJmp     >L7\t; test.sm:1\n"
		"6: \tLdSym   `else-side (%hd)\t; test.sm:1\n"
		"7: \tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "then-side"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "else-side")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsWithNullThenSide)
{
	SmileObject expr = Parse("{ var a, b [$if 10 < 1 [] ({ a = 20 })] `done }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `a (0)\t; test.sm:1\n"
		"1: \tNullLoc0 `b (1)\t; test.sm:1\n"
		"2: \tLd64    10\t; test.sm:1\n"
		"3: \tLd64    1\t; test.sm:1\n"
		"4: \tBinary  `< (%hd)\t; test.sm:1\n"
		"5: \tBt      >L8\t; test.sm:1\n"
		"6: \tLd64    20\t; test.sm:1\n"
		"7: \tStpLoc0 `a (0)\t; test.sm:1\n"
		"8: \tLdSym   `done (%hd)\t; test.sm:1\n"
		"9: \tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "done")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 2);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsWithNullElseSide)
{
	SmileObject expr = Parse("{ var a, b [$if 1 < 10 ({ a = 20 }) []] `done }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `a (0)\t; test.sm:1\n"
		"1: \tNullLoc0 `b (1)\t; test.sm:1\n"
		"2: \tLd64    1\t; test.sm:1\n"
		"3: \tLd64    10\t; test.sm:1\n"
		"4: \tBinary  `< (%hd)\t; test.sm:1\n"
		"5: \tBf      >L8\t; test.sm:1\n"
		"6: \tLd64    20\t; test.sm:1\n"
		"7: \tStpLoc0 `a (0)\t; test.sm:1\n"
		"8: \tLdSym   `done (%hd)\t; test.sm:1\n"
		"9: \tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "done")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 2);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsWithAMeaninglessThenSide)
{
	SmileObject expr = Parse("{ var a, b [$if 10 < 1 `bar ({ a = 20 })] `done }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `a (0)\t; test.sm:1\n"
		"1: \tNullLoc0 `b (1)\t; test.sm:1\n"
		"2: \tLd64    10\t; test.sm:1\n"
		"3: \tLd64    1\t; test.sm:1\n"
		"4: \tBinary  `< (%hd)\t; test.sm:1\n"
		"5: \tBt      >L8\t; test.sm:1\n"
		"6: \tLd64    20\t; test.sm:1\n"
		"7: \tStpLoc0 `a (0)\t; test.sm:1\n"
		"8: \tLdSym   `done (%hd)\t; test.sm:1\n"
		"9: \tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "done")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 2);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsWithMeaninglessElseSide)
{
	SmileObject expr = Parse("{ var a, b [$if 1 < 10 ({ a = 20 }) `bar] `done }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `a (0)\t; test.sm:1\n"
		"1: \tNullLoc0 `b (1)\t; test.sm:1\n"
		"2: \tLd64    1\t; test.sm:1\n"
		"3: \tLd64    10\t; test.sm:1\n"
		"4: \tBinary  `< (%hd)\t; test.sm:1\n"
		"5: \tBf      >L8\t; test.sm:1\n"
		"6: \tLd64    20\t; test.sm:1\n"
		"7: \tStpLoc0 `a (0)\t; test.sm:1\n"
		"8: \tLdSym   `done (%hd)\t; test.sm:1\n"
		"9: \tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "done")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 2);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsAllTheWay)
{
	SmileObject expr = Parse(
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => [$if (x) (y)]\n"
		"#syntax STMT: [my-if [EXPR x] then [STMT y] else [STMT z]] => [$if (x) (y) (z)]\n"
		"\n"
		"my-if 1 < 10 then\n"
		"\t`then-side\n"
		"else\n"
		"\t`else-side\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tLd64    1\t; test.sm:4\n"
		"1: \tLd64    10\t; test.sm:4\n"
		"2: \tBinary  `< (%hd)\t; test.sm:4\n"
		"3: \tBf      >L6\t; test.sm:2\n"
		"4: \tLdSym   `then-side (%hd)\t; test.sm:5\n"
		"5: \tJmp     >L7\t; test.sm:2\n"
		"6: \tLdSym   `else-side (%hd)\t; test.sm:7\n"
		"7: \tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "then-side"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "else-side")
	);

	result = UserFunctionInfo_ToString(globalFunction);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

//-------------------------------------------------------------------------------------------------

START_TEST(CanCompileAPreCondPostWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0, y = 0\n"
		"[$while (x += 1) x < 10 (y -= 1)]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"
		"1: \tNullLoc0 `y (1)\t; test.sm:1\n"

		"2: \tLd64    0\t; test.sm:1\n"
		"3: \tStpLoc0 `x (0)\t; test.sm:1\n"
		"4: \tLd64    0\t; test.sm:1\n"
		"5: \tStpLoc0 `y (1)\t; test.sm:1\n"

		"6: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"7: \tLd64    1\t; test.sm:2\n"
		"8: \tBinary  `+ (%hd)\t; test.sm:2\n"
		"9: \tStLoc0  `x (0)\t; test.sm:2\n"

		"10: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"11: \tLd64    10\t; test.sm:2\n"
		"12: \tBinary  `< (%d)\t; test.sm:2\n"
		"13: \tBt      >L20\t; test.sm:2\n"

		"14: \tPop1\t; test.sm:2\n"

		"15: \tLdLoc0  `y (1)\t; test.sm:2\n"
		"16: \tLd64    1\t; test.sm:2\n"
		"17: \tBinary  `- (%hd)\t; test.sm:2\n"
		"18: \tStpLoc0 `y (1)\t; test.sm:2\n"

		"19: \tJmp     L6\t; test.sm:2\n"

		"20: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "-")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileAPreCondWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0\n"
		"[$while (x += 1) x < 10 []]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"

		"1: \tLd64    0\t; test.sm:1\n"
		"2: \tStpLoc0 `x (0)\t; test.sm:1\n"

		"3: \tJmp     >L5\t; test.sm:2\n"

		"4: \tPop1\n"

		"5: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"6: \tLd64    1\t; test.sm:2\n"
		"7: \tBinary  `+ (%hd)\t; test.sm:2\n"
		"8: \tStLoc0  `x (0)\t; test.sm:2\n"

		"9: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"10: \tLd64    10\t; test.sm:2\n"
		"11: \tBinary  `< (%hd)\t; test.sm:2\n"
		"12: \tBt      L4\t; test.sm:2\n"

		"13: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileANullCondPostWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0\n"
		"[$while [] x < 10 (x += 1)]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"

		"1: \tLd64    0\t; test.sm:1\n"
		"2: \tStpLoc0 `x (0)\t; test.sm:1\n"

		"3: \tLdNull\t; test.sm:2\n"
		"4: \tJmp     >L10\t; test.sm:2\n"

		"5: \tPop1\n"

		"6: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"7: \tLd64    1\t; test.sm:2\n"
		"8: \tBinary  `+ (%hd)\t; test.sm:2\n"
		"9: \tStLoc0  `x (0)\t; test.sm:2\n"

		"10: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"11: \tLd64    10\t; test.sm:2\n"
		"12: \tBinary  `< (%hd)\t; test.sm:2\n"
		"13: \tBt      L5\t; test.sm:2\n"

		"14: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileACondPostWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0\n"
		"[$while x < 10 (x += 1)]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"

		"1: \tLd64    0\t; test.sm:1\n"
		"2: \tStpLoc0 `x (0)\t; test.sm:1\n"

		"3: \tLdNull\t; test.sm:2\n"
		"4: \tJmp     >L10\t; test.sm:2\n"

		"5: \tPop1\n"

		"6: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"7: \tLd64    1\t; test.sm:2\n"
		"8: \tBinary  `+ (%hd)\t; test.sm:2\n"
		"9: \tStLoc0  `x (0)\t; test.sm:2\n"

		"10: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"11: \tLd64    10\t; test.sm:2\n"
		"12: \tBinary  `< (%hd)\t; test.sm:2\n"
		"13: \tBt      L5\t; test.sm:2\n"

		"14: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileACondOnlyWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0\n"
		"[$while (x += 1) < 10 []]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"

		"1: \tLd64    0\t; test.sm:1\n"
		"2: \tStpLoc0 `x (0)\t; test.sm:1\n"

		"3: \tLdLoc0  `x (0)\t; test.sm:2\n"
		"4: \tLd64    1\t; test.sm:2\n"
		"5: \tBinary  `+ (%hd)\t; test.sm:2\n"
		"6: \tStLoc0  `x (0)\t; test.sm:2\n"
		"7: \tLd64    10\t; test.sm:2\n"
		"8: \tBinary  `< (%hd)\t; test.sm:2\n"
		"9: \tBt      L3\t; test.sm:2\n"

		"10: \tLdNull\t; test.sm:2\n"

		"11: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileAWhileLoopThatComputesLogarithms)
{
	SmileObject expr = Parse(
		"#syntax STMT: [my-while [EXPR x] do [STMT y]] => [$while [] (x) (y)]\n"
		"\n"
		"n = 12345678\n"
		"log = 0\n"
		"my-while n do {\n"
		"\tn >>>= 1\n"
		"\tlog += 1\n"
		"}\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `n (0)\t; test.sm:1\n"
		"1: \tNullLoc0 `log (1)\t; test.sm:1\n"

		"2: \tLd64    12345678\t; test.sm:3\n"
		"3: \tStpLoc0 `n (0)\t; test.sm:3\n"
		"4: \tLd64    0\t; test.sm:4\n"
		"5: \tStpLoc0 `log (1)\t; test.sm:4\n"

		"6: \tLdNull\t; test.sm:1\n"
		"7: \tJmp     >L17\t; test.sm:1\n"
		"8: \tPop1\n"
		"9: \tLdLoc0  `n (0)\t; test.sm:6\n"
		"10: \tLd64    1\t; test.sm:6\n"
		"11: \tBinary  `>>> (%hd)\t; test.sm:6\n"
		"12: \tStpLoc0 `n (0)\t; test.sm:6\n"
		"13: \tLdLoc0  `log (1)\t; test.sm:7\n"
		"14: \tLd64    1\t; test.sm:7\n"
		"15: \tBinary  `+ (%hd)\t; test.sm:7\n"
		"16: \tStLoc0  `log (1)\t; test.sm:7\n"
		"17: \tLdLoc0  `n (0)\t; test.sm:1\n"
		"18: \tBt      L8\t; test.sm:1\n"

		"19: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">>>"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------

START_TEST(CanCompileASimpleTillLoop)
{
	SmileObject expr = Parse(
		"[$till [found not-found]\n"
		"\t[$progn\n"
		"\t\t[$if 1 found]\n"
		"\t\t[$if 2 not-found]\n"
		"\t]\n"
		"]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_FromC(
		"0: \tLd64    1\t; test.sm:3\n"
		"1: \tBf      >L3\t; test.sm:3\n"
		"2: \tJmp     >L7\t; test.sm:3\n"
		"3: \tLd64    2\t; test.sm:4\n"
		"4: \tBf      >L6\t; test.sm:4\n"
		"5: \tJmp     >L7\t; test.sm:4\n"
		"6: \tJmp     L0\t; test.sm:1\n"
		"7: \tLdNull\t; test.sm:1\n"
		"8: \tRet\n"
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileATillLoopThatActuallyDoesSomething)
{
	SmileObject expr = Parse(
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => [$if (x) (y)]\n"
		"\n"
		"var x = 1\n"
		"[$till [reached-eight-bits] {\n"
		"\tmy-if x > 0xFF then reached-eight-bits\n"
		"\tx <<= 1\n"
		"}]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"
		"1: \tLd64    1\t; test.sm:3\n"
		"2: \tStpLoc0 `x (0)\t; test.sm:3\n"
		"3: \tLdLoc0  `x (0)\t; test.sm:5\n"
		"4: \tLd64    255\t; test.sm:5\n"
		"5: \tBinary  `> (%hd)\t; test.sm:5\n"
		"6: \tBf      >L8\t; test.sm:1\n"
		"7: \tJmp     >L13\t; test.sm:1\n"
		"8: \tLdLoc0  `x (0)\t; test.sm:6\n"
		"9: \tLd64    1\t; test.sm:6\n"
		"10: \tBinary  `<< (%hd)\t; test.sm:6\n"
		"11: \tStpLoc0 `x (0)\t; test.sm:6\n"
		"12: \tJmp     L3\t; test.sm:4\n"
		"13: \tLdNull\t; test.sm:4\n"
		"14: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileASimpleTillLoopUsingBuiltInSyntax)
{
	SmileObject expr = Parse(
		"var x = 1\n"
		"till reached-eight-bits do {\n"
		"\tif x > 0xFF then reached-eight-bits\n"
		"\tx <<= 1\n"
		"}\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"
		"1: \tLd64    1\t; test.sm:1\n"
		"2: \tStpLoc0 `x (0)\t; test.sm:1\n"
		"3: \tLdLoc0  `x (0)\t; test.sm:3\n"
		"4: \tLd64    255\t; test.sm:3\n"
		"5: \tBinary  `> (%hd)\t; test.sm:3\n"
		"6: \tBf      >L8\t; test.sm:3\n"
		"7: \tJmp     >L13\t; test.sm:3\n"
		"8: \tLdLoc0  `x (0)\t; test.sm:4\n"
		"9: \tLd64    1\t; test.sm:4\n"
		"10: \tBinary  `<< (%hd)\t; test.sm:4\n"
		"11: \tStpLoc0 `x (0)\t; test.sm:4\n"
		"12: \tJmp     L3\t; test.sm:2\n"
		"13: \tLdNull\t; test.sm:2\n"
		"14: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileASimpleTillLoopUsingBuiltInSyntaxAndNoResultingValue)
{
	SmileObject expr = Parse(
		"var x = 1\n"
		"till reached-eight-bits do {\n"
		"\tif x > 0xFF then reached-eight-bits\n"
		"\tx <<= 1\n"
		"}\n"
		"x = 0\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"
		"1: \tLd64    1\t; test.sm:1\n"
		"2: \tStpLoc0 `x (0)\t; test.sm:1\n"
		"3: \tLdLoc0  `x (0)\t; test.sm:3\n"
		"4: \tLd64    255\t; test.sm:3\n"
		"5: \tBinary  `> (%hd)\t; test.sm:3\n"
		"6: \tBf      >L8\t; test.sm:3\n"
		"7: \tJmp     >L13\t; test.sm:3\n"
		"8: \tLdLoc0  `x (0)\t; test.sm:4\n"
		"9: \tLd64    1\t; test.sm:4\n"
		"10: \tBinary  `<< (%hd)\t; test.sm:4\n"
		"11: \tStpLoc0 `x (0)\t; test.sm:4\n"
		"12: \tJmp     L3\t; test.sm:2\n"
		"13: \tLd64    0\t; test.sm:6\n"
		"14: \tStLoc0  `x (0)\t; test.sm:6\n"
		"15: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileATillLoopWithWhenClauses)
{
	SmileObject expr = Parse(
		"var x = 1, y\n"
		"till reached-eight-bits, failed do {\n"
		"\tif x > 0xFF then reached-eight-bits\n"
		"\telse if x > 0x1FF then failed\n"
		"\tx <<= 1\n"
		"}\n"
		"when reached-eight-bits {\n"
		"\ty = 1\n"
		"}\n"
		"when failed {\n"
		"\ty = 2\n"
		"}\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"
		"1: \tNullLoc0 `y (1)\t; test.sm:1\n"

		"2: \tLd64    1\t; test.sm:1\n"
		"3: \tStpLoc0 `x (0)\t; test.sm:1\n"

		"4: \tLdLoc0  `x (0)\t; test.sm:3\n"
		"5: \tLd64    255\t; test.sm:3\n"
		"6: \tBinary  `> (%hd)\t; test.sm:3\n"
		"7: \tBf      >L9\t; test.sm:3\n"
		"8: \tJmp     >L19\t; test.sm:3\n"

		"9: \tLdLoc0  `x (0)\t; test.sm:4\n"
		"10: \tLd64    511\t; test.sm:4\n"
		"11: \tBinary  `> (%hd)\t; test.sm:4\n"
		"12: \tBf      >L14\t; test.sm:4\n"
		"13: \tJmp     >L22\t; test.sm:4\n"

		"14: \tLdLoc0  `x (0)\t; test.sm:5\n"
		"15: \tLd64    1\t; test.sm:5\n"
		"16: \tBinary  `<< (%hd)\t; test.sm:5\n"
		"17: \tStpLoc0 `x (0)\t; test.sm:5\n"

		"18: \tJmp     L4\t; test.sm:2\n"

		"19: \tLd64    1\t; test.sm:8\n"
		"20: \tStLoc0  `y (1)\t; test.sm:8\n"
		"21: \tJmp     >L24\t; test.sm:7\n"

		"22: \tLd64    2\t; test.sm:11\n"
		"23: \tStLoc0  `y (1)\t; test.sm:11\n"

		"24: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileATillLoopWithWhenClausesAndNoResultingValue)
{
	SmileObject expr = Parse(
		"var x = 1, y\n"
		"till reached-eight-bits, failed do {\n"
		"\tif x > 0xFF then reached-eight-bits\n"
		"\telse if x > 0x1FF then failed\n"
		"\tx <<= 1\n"
		"}\n"
		"when reached-eight-bits {\n"
		"\ty = 1\n"
		"}\n"
		"when failed {\n"
		"\ty = 2\n"
		"}\n"
		"x = 0\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"0: \tNullLoc0 `x (0)\t; test.sm:1\n"
		"1: \tNullLoc0 `y (1)\t; test.sm:1\n"

		"2: \tLd64    1\t; test.sm:1\n"
		"3: \tStpLoc0 `x (0)\t; test.sm:1\n"

		"4: \tLdLoc0  `x (0)\t; test.sm:3\n"
		"5: \tLd64    255\t; test.sm:3\n"
		"6: \tBinary  `> (%hd)\t; test.sm:3\n"
		"7: \tBf      >L9\t; test.sm:3\n"
		"8: \tJmp     >L19\t; test.sm:3\n"

		"9: \tLdLoc0  `x (0)\t; test.sm:4\n"
		"10: \tLd64    511\t; test.sm:4\n"
		"11: \tBinary  `> (%hd)\t; test.sm:4\n"
		"12: \tBf      >L14\t; test.sm:4\n"
		"13: \tJmp     >L22\t; test.sm:4\n"

		"14: \tLdLoc0  `x (0)\t; test.sm:5\n"
		"15: \tLd64    1\t; test.sm:5\n"
		"16: \tBinary  `<< (%hd)\t; test.sm:5\n"
		"17: \tStpLoc0 `x (0)\t; test.sm:5\n"

		"18: \tJmp     L4\t; test.sm:2\n"

		"19: \tLd64    1\t; test.sm:8\n"
		"20: \tStpLoc0 `y (1)\t; test.sm:8\n"
		"21: \tJmp     >L24\t; test.sm:7\n"

		"22: \tLd64    2\t; test.sm:11\n"
		"23: \tStpLoc0 `y (1)\t; test.sm:11\n"

		"24: \tLd64    0\t; test.sm:13\n"
		"25: \tStLoc0  `x (0)\t; test.sm:13\n"

		"26: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<<")
	);

	result = UserFunctionInfo_ToString(globalFunction);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanUseATillLoopToEscapeNestedFunctions)
{
	SmileObject expr = Parse(
		"var list = `[1 2 3 4 5]\n"
		"till found-odd, found-even, found-nothing do\n"
		"\tlist each |x|\n"
		"\t\tif even? x then found-even\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);

	String outerExpectedResult = String_Format(
		"0: \tNullLoc0 `list (0)\t; test.sm:1\n"

		"1: \tLdObj   @0\t; test.sm:1\n"
		"2: \tStpLoc0 `list (0)\t; test.sm:1\n"

		"3: \tNewTill 0\t; test.sm:2\n"
		"4: \tStpLoc0 ` (1)\t; test.sm:2\n"

		"5: \tLdLoc0  `list (0)\t; test.sm:3\n"
		"6: \tNewFn   @0\t; test.sm:3\n"
		"7: \tBinary  `each (%hd)\t; test.sm:3\n"
		"8: \tPop1\t; test.sm:3\n"
		"9: \tJmp     L5\t; test.sm:2\n"

		"10: \tLdLoc0  ` (1)\t; test.sm:2\n"
		"11: \tEndTill\t; test.sm:2\n"
		"12: \tLdNull\t; test.sm:2\n"

		"13: \tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "each")
	);

	String innerExpectedResult = String_Format(
		"0: \tLdArg0  `x (0)\t; test.sm:4\n"
		"1: \tUnary   `even? (%hd)\t; test.sm:4\n"

		"2: \tBf      >L5\t; test.sm:4\n"

		"3: \tLdLoc1  ` (1)\t; test.sm:4\n"
		"4: \tTillEsc 1\t; test.sm:4\n"

		"5: \tLdNull\t; test.sm:4\n"

		"6: \tRet\t; test.sm:3\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "even?")
	);

	String outerResult = UserFunctionInfo_ToString(globalFunction);
	String innerResult = UserFunctionInfo_ToString(compiler->compiledTables->userFunctions[0]);

	ASSERT_STRING(outerResult, String_ToC(outerExpectedResult), String_Length(outerExpectedResult));
	ASSERT_STRING(innerResult, String_ToC(innerExpectedResult), String_Length(innerExpectedResult));
}
END_TEST

#include "compiler_tests.generated.inc"
