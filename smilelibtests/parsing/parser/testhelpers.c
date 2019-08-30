//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2019 Sean Werkema
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

#include "../../stdafx.h"

#include <smile/parsing/parser.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/env/env.h>

#include "testhelpers.h"

static String _testScriptName;

String GetTestScriptName(void)
{
	if (_testScriptName == NULL)
		_testScriptName = String_FromC("test.sm");
	return _testScriptName;
}

String Stringify(SmileObject obj)
{
	return SmileObject_Stringify(obj);
}

static SmileObject RecursiveSimpleParse(Lexer lexer)
{
	switch (Lexer_Next(lexer)) {

	case TOKEN_EOI:
		ASSERT(False);
		return NullObject;

	case TOKEN_LEFTBRACKET:
	{
		SmileList head = NullList, tail = NullList;
		SmileObject item;
		while (Lexer_Next(lexer) != TOKEN_RIGHTBRACKET)
		{
			Lexer_Unget(lexer);
			item = RecursiveSimpleParse(lexer);
			LIST_APPEND(head, tail, item);
		}
		return (SmileObject)head;
	}

	case TOKEN_LEFTPARENTHESIS:
	{
		SmileObject left, right;
		left = RecursiveSimpleParse(lexer);
		if (Lexer_Next(lexer) != TOKEN_DOT)
			ASSERT(False);
		right = RecursiveSimpleParse(lexer);
		if (Lexer_Next(lexer) != TOKEN_RIGHTPARENTHESIS)
			ASSERT(False);
		return (SmileObject)SmileList_CreateDot(left, right);
	}

	case TOKEN_ALPHANAME:
	case TOKEN_UNKNOWNALPHANAME:
	case TOKEN_PUNCTNAME:
	case TOKEN_UNKNOWNPUNCTNAME:
		if (lexer->token->data.symbol == Smile_KnownSymbols.null_)
			return NullObject;
		else
			return (SmileObject)SmileSymbol_Create(lexer->token->data.symbol);

	case TOKEN_BYTE:
		return (SmileObject)SmileByte_Create(lexer->token->data.byte);

	case TOKEN_INTEGER16:
		return (SmileObject)SmileInteger16_Create(lexer->token->data.int16);

	case TOKEN_INTEGER32:
		return (SmileObject)SmileInteger32_Create(lexer->token->data.int32);

	case TOKEN_INTEGER64:
		return (SmileObject)SmileInteger64_Create(lexer->token->data.int64);

	case TOKEN_DYNSTRING:
	case TOKEN_RAWSTRING:
		return (SmileObject)lexer->token->text;

	default:
		return NullObject;
	}
}

SmileObject SimpleParse(const char *input)
{
	Lexer lexer;
	String source = String_FromC(input);

	lexer = Lexer_Create(source, 0, String_Length(source), GetTestScriptName(), 1, 1, False);
	lexer->symbolTable = Smile_SymbolTable;

	return RecursiveSimpleParse(lexer);
}

SmileObject FullParse(const char *input)
{
	String source;
	Lexer lexer;
	Parser parser;
	ParseScope parseScope;
	SmileObject result;

	source = String_FromC(input);
	lexer = Lexer_Create(source, 0, String_Length(source), GetTestScriptName(), 1, 1, False);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	return result;
}

Lexer SetupLexerFromString(String source)
{
	Lexer lexer;

	Smile_ResetEnvironment();

	lexer = Lexer_Create(source, 0, String_Length(source), GetTestScriptName(), 1, 1, False);
	lexer->symbolTable = Smile_SymbolTable;

	return lexer;
}

Lexer SetupLexer(const char *string)
{
	return SetupLexerFromString(String_FromC(string));
}
