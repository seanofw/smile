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
#include <smile/smiletypes/smilepair.h>

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
		"\tLdNull\n"
		"\tRet\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd8 123\n"
		"\tRet\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd16 123\n"
		"\tRet\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd32 123\n"
		"\tRet\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd64 123\n"
		"\tRet\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd64 123\n"
		"\tLd64 456\n"
		"\tBinary %d\t; +\n"
		"\tRet\n",
		Smile_KnownSymbols.plus
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd64 123\n"
		"\tLd64 456\n"
		"\tUnary %d\t; -\n"
		"\tBinary %d\t; +\n"
		"\tLd64 50\n"
		"\tBinary %d\t; *\n"
		"\tRet\n",
		Smile_KnownSymbols.minus,
		Smile_KnownSymbols.plus,
		Smile_KnownSymbols.star
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLdX %d\t; gb\n"
		"\tStX %d\t; ga\n"
		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLdX %d\t; gb\n"
		"\tLdProp %d\t; foo\n"
		"\tStX %d\t; ga\n"
		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "foo"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLdX %d\t; ga\n"
		"\tLdX %d\t; gb\n"
		"\tStProp %d\t; foo\n"
		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "foo")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLdX %d\t; gb\n"
		"\tLd64 10\n"
		"\tLdMember\n"
		"\tStX %d\t; ga\n"
		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLdX %d\t; ga\n"
		"\tLd64 10\n"
		"\tLdX %d\t; gb\n"
		"\tStMember\n"
		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLdX %d\t; gb\n"
		"\tStLoc0 0\t; a\n"
		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "a")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd64 10\n"
		"\tStpLoc0 0\t; b\n"
		"\tLdLoc0 0\t; b\n"
		"\tStpLoc0 1\t; a\n"
		"\tLdLoc0 1\t; a\n"
		"\tLdLoc0 0\t; b\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 2\t; c\n"
		"\tRet\n",
		Smile_KnownSymbols.plus
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd64 10\n"
		"\tStpLoc0 0\t; b\n"
		"\tLdLoc0 0\t; b\n"
		"\tStpLoc0 1\t; a\n"
		"\tLdLoc0 1\t; a\n"
		"\tLdLoc0 0\t; b\n"
		"\tBinary %d\t; +\n"
		"\tStpLoc0 2\t; c\n"
		"\tLdLoc0 0\t; b\n"
		"\tLd64 20\n"
		"\tBinary %d\t; *\n"
		"\tStLoc0 3\t; d\n"
		"\tRet\n",
		Smile_KnownSymbols.plus,
		Smile_KnownSymbols.star
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
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
		"\tLd64 1\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBf >L6\n"
		"\tLdSym %d\t; then-side\n"
		"\tJmp >L7\n"
		"\tLdSym %d\t; else-side\n"
		"\tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "then-side"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "else-side")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsWithNullThenSide)
{
	SmileObject expr = Parse("{ var a, b [$if 10 < 1 [] { a = 20 }] `done }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 10\n"
		"\tLd64 1\n"
		"\tBinary %d\t; <\n"
		"\tBt >L6\n"
		"\tLd64 20\n"
		"\tStpLoc0 0\t; a\n"
		"\tLdSym %d\t; done\n"
		"\tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "done")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 2);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsWithNullElseSide)
{
	SmileObject expr = Parse("{ var a, b [$if 1 < 10 { a = 20 } []] `done }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 1\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBf >L6\n"
		"\tLd64 20\n"
		"\tStpLoc0 0\t; a\n"
		"\tLdSym %d\t; done\n"
		"\tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "done")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 2);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsWithAMeaninglessThenSide)
{
	SmileObject expr = Parse("{ var a, b [$if 10 < 1 `bar { a = 20 }] `done }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 10\n"
		"\tLd64 1\n"
		"\tBinary %d\t; <\n"
		"\tBt >L6\n"
		"\tLd64 20\n"
		"\tStpLoc0 0\t; a\n"
		"\tLdSym %d\t; done\n"
		"\tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "done")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 2);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsWithMeaninglessElseSide)
{
	SmileObject expr = Parse("{ var a, b [$if 1 < 10 { a = 20 } `bar] `done }");

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 1\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBf >L6\n"
		"\tLd64 20\n"
		"\tStpLoc0 0\t; a\n"
		"\tLdSym %d\t; done\n"
		"\tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "done")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->closureInfo.numVariables == 2);
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

START_TEST(CanCompileConditionalsAllTheWay)
{
	SmileObject expr = Parse(
		"#syntax STMT: [if [EXPR x] then [STMT y]] => [$if x y]\n"
		"#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [$if x y z]\n"
		"\n"
		"if 1 < 10 then\n"
		"\t`then-side\n"
		"else\n"
		"\t`else-side\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 1\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBf >L6\n"
		"\tLdSym %d\t; then-side\n"
		"\tJmp >L7\n"
		"\tLdSym %d\t; else-side\n"
		"\tRet\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "then-side"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "else-side")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
	ASSERT(globalFunction->closureInfo.tempSize == 2);
}
END_TEST

//-------------------------------------------------------------------------------------------------

START_TEST(CanCompileAPreCondPostWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0, y = 0\n"
		"[$while x += 1 x < 10 y -= 1]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 0\n"
		"\tStpLoc0 0\t; x\n"
		"\tLd64 0\n"
		"\tStpLoc0 1\t; y\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 1\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 0\t; x\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBt >L18\n"

		"\tPop1\n"

		"\tLdLoc0 1\t; y\n"
		"\tLd64 1\n"
		"\tBinary %d\t; -\n"
		"\tStpLoc0 1\t; y\n"

		"\tJmp L4\n"

		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "-")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileAPreCondWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0\n"
		"[$while x += 1 x < 10 []]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 0\n"
		"\tStpLoc0 0\t; x\n"

		"\tJmp >L4\n"

		"\tPop1\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 1\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 0\t; x\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBt L3\n"

		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileANullCondPostWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0\n"
		"[$while [] x < 10 x += 1]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 0\n"
		"\tStpLoc0 0\t; x\n"

		"\tLdNull\n"
		"\tJmp >L9\n"

		"\tPop1\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 1\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 0\t; x\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBt L4\n"

		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileACondPostWhileLoop)
{
	SmileObject expr = Parse(
		"var x = 0\n"
		"[$while x < 10 x += 1]\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 0\n"
		"\tStpLoc0 0\t; x\n"

		"\tLdNull\n"
		"\tJmp >L9\n"

		"\tPop1\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 1\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 0\t; x\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBt L4\n"

		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);

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
		"\tLd64 0\n"
		"\tStpLoc0 0\t; x\n"

		"\tLdLoc0 0\t; x\n"
		"\tLd64 1\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 0\t; x\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBt L2\n"

		"\tLdNull\n"

		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "<")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileAWhileLoopThatComputesLogarithms)
{
	SmileObject expr = Parse(
		"#syntax STMT: [while [EXPR x] do [STMT y]] => [$while [] x y]\n"
		"\n"
		"n = 12345678\n"
		"log = 0\n"
		"while n do {\n"
		"\tn >>>= 1\n"
		"\tlog += 1\n"
		"}\n"
	);

	Compiler compiler = Compiler_Create();
	UserFunctionInfo globalFunction = Compiler_CompileGlobal(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 12345678\n"
		"\tStpLoc0 0\t; n\n"
		"\tLd64 0\n"
		"\tStpLoc0 1\t; log\n"
		"\tLdNull\n"
		"\tJmp >L15\n"
		"\tPop1\n"
		"\tLdLoc0 0\t; n\n"
		"\tLd64 1\n"
		"\tBinary %d\t; >>>\n"
		"\tStpLoc0 0\t; n\n"
		"\tLdLoc0 1\t; log\n"
		"\tLd64 1\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 1\t; log\n"
		"\tLdLoc0 0\t; n\n"
		"\tBt L6\n"
		"\tRet\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">>>"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+")
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------

START_TEST(CanCompileASimpleTillLoop)
{
/*
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
		"\tNop\n"
		"\tNop\n"
		"L0:\n"
		"\tLd64 1\n"
		"\tBf >L4\n"
		"\tJmp >L10\n"
		"L4:\n"
		"\tLd64 2\n"
		"\tBf >L8\n"
		"\tJmp >L10\n"
		"L8:\n"
		"\tJmp L0\n"
		"L10:\n"
		"\tLdNull\n"
		"\tRet\n"
	);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
*/
}
END_TEST

#include "compiler_tests.generated.inc"
