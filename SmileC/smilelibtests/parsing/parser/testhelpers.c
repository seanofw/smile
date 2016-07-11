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

#include "../../stdafx.h"

#include <smile/parsing/parser.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/env/env.h>

#include "testhelpers.h"

STATIC_STRING(TestFilename, "test.sm");

static SmileObject RecursiveSimpleParse(Lexer lexer)
{
	switch (Lexer_Next(lexer)) {

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
	break;

	case TOKEN_LEFTPARENTHESIS:
	{
		SmileObject left, right;
		left = RecursiveSimpleParse(lexer);
		if (Lexer_Next(lexer) != TOKEN_DOT)
			ASSERT(False);
		right = RecursiveSimpleParse(lexer);
		if (Lexer_Next(lexer) != TOKEN_RIGHTPARENTHESIS)
			ASSERT(False);
		return (SmileObject)SmilePair_Create(left, right);
	}
	break;

	case TOKEN_ALPHANAME:
	case TOKEN_UNKNOWNALPHANAME:
	case TOKEN_PUNCTNAME:
	case TOKEN_UNKNOWNPUNCTNAME:
		if (lexer->token->data.symbol == Smile_KnownSymbols.null_)
			return NullObject;
		else
			return (SmileObject)SmileSymbol_Create(lexer->token->data.symbol);

	case TOKEN_INTEGER32:
		return (SmileObject)SmileInteger32_Create(lexer->token->data.i);

	case TOKEN_DYNSTRING:
	case TOKEN_RAWSTRING:
		return (SmileObject)SmileString_Create(lexer->token->text);

	default:
		return NullObject;
	}
}

SmileObject SimpleParse(const char *input)
{
	Lexer lexer;
	String source = String_FromC(input);

	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	return RecursiveSimpleParse(lexer);
}

SmileList FullParse(const char *input)
{
	String source;
	Lexer lexer;
	Parser parser;
	ParseScope parseScope;
	SmileList result;

	source = String_FromC(input);
	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	return result;
}

Bool RecursiveEquals(SmileObject a, SmileObject b)
{
	if (a == NULL || b == NULL) return False;		// Should never have C NULL.

	if (SMILE_KIND(a) != SMILE_KIND(b)) return False;

next:
	switch (SMILE_KIND(a)) {

	case SMILE_KIND_LIST:
		if (!RecursiveEquals(((SmileList)a)->a, ((SmileList)b)->a))
			return False;
		a = ((SmileList)a)->d;
		b = ((SmileList)b)->d;
		goto next;

	case SMILE_KIND_OBJECT:
		return True;

	case SMILE_KIND_NULL:
		return True;

	case SMILE_KIND_PAIR:
		if (!RecursiveEquals(((SmilePair)a)->left, ((SmilePair)b)->left))
			return False;
		if (!RecursiveEquals(((SmilePair)a)->right, ((SmilePair)b)->right))
			return False;
		return True;

	case SMILE_KIND_SYMBOL:
		if (((SmileSymbol)a)->symbol != ((SmileSymbol)b)->symbol)
			return False;
		return True;

	case SMILE_KIND_INTEGER32:
		if (((SmileInteger32)a)->value != ((SmileInteger32)b)->value)
			return False;
		return True;

	case SMILE_KIND_STRING:
		if (!String_Equals((String)&((SmileString)a)->string, (String)&((SmileString)b)->string))
			return False;
		return True;

	default:
		return False;
	}
}

Bool ContainsNestedList(SmileObject obj)
{
	SmileObject item;
	SmileList list;
	SmilePair pair;

	switch (SMILE_KIND(obj)) {

	case SMILE_KIND_PAIR:
		pair = (SmilePair)obj;
		if (ContainsNestedList(pair->left)) return True;
		if (ContainsNestedList(pair->right)) return True;
		break;

	case SMILE_KIND_LIST:
		for (list = (SmileList)obj; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			item = list->a;
			if (SMILE_KIND(item) == SMILE_KIND_LIST) return True;
		}
		break;
	}

	return False;
}

Bool IsRegularList(SmileObject list)
{
	while (SMILE_KIND(list) == SMILE_KIND_LIST) {
		list = (SmileObject)LIST_REST((SmileList)list);
	}
	return SMILE_KIND(list) == SMILE_KIND_NULL;
}

static void StringifyRecursive(SmileObject obj, StringBuilder stringBuilder, Int indent);

String Stringify(SmileObject obj)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 1024);
	INIT_INLINE_STRINGBUILDER(stringBuilder);
	StringifyRecursive(obj, stringBuilder, 0);
	return StringBuilder_ToString(stringBuilder);
}

const char *StringifyToC(SmileObject obj)
{
	return String_ToC(Stringify(obj));
}

static void StringifyRecursive(SmileObject obj, StringBuilder stringBuilder, Int indent)
{
	SmileList list;
	SmilePair pair;
	Bool isFirst;

	if (obj == NULL) {
		StringBuilder_AppendC(stringBuilder, "<NULL>", 0, 6);
		return;
	}

	switch (SMILE_KIND(obj)) {

	case SMILE_KIND_LIST:
		list = (SmileList)obj;
		if (!IsRegularList(obj)) {
			StringBuilder_AppendByte(stringBuilder, '(');
			while (SMILE_KIND(list) == SMILE_KIND_LIST) {
				StringifyRecursive((SmileObject)list->a, stringBuilder, indent + 1);
				StringBuilder_AppendC(stringBuilder, " ## ", 0, 4);
				list = LIST_REST(list);
			}
			StringifyRecursive((SmileObject)list, stringBuilder, indent + 1);
			StringBuilder_AppendByte(stringBuilder, ')');
		}
		else if (ContainsNestedList(obj)) {
			Bool isFirst = True;
			StringBuilder_AppendByte(stringBuilder, '[');
			while (SMILE_KIND(list) == SMILE_KIND_LIST && SMILE_KIND(list->a) == SMILE_KIND_SYMBOL) {
				if (!isFirst) {
					StringBuilder_AppendByte(stringBuilder, ' ');
				}
				StringifyRecursive((SmileObject)list->a, stringBuilder, indent + 1);
				list = LIST_REST(list);
				isFirst = False;
			}
			StringBuilder_AppendByte(stringBuilder, '\n');
			while (SMILE_KIND(list) == SMILE_KIND_LIST) {
				StringBuilder_AppendRepeat(stringBuilder, ' ', (indent + 1) * 4);
				StringifyRecursive((SmileObject)list->a, stringBuilder, indent + 1);
				StringBuilder_AppendByte(stringBuilder, '\n');
				list = LIST_REST(list);
			}
			StringBuilder_AppendRepeat(stringBuilder, ' ', indent * 4);
			StringBuilder_AppendByte(stringBuilder, ']');
		}
		else {
			isFirst = True;
			StringBuilder_AppendByte(stringBuilder, '[');
			while (SMILE_KIND(list) == SMILE_KIND_LIST) {
				if (!isFirst) {
					StringBuilder_AppendByte(stringBuilder, ' ');
				}
				StringifyRecursive((SmileObject)list->a, stringBuilder, indent + 1);
				list = LIST_REST(list);
				isFirst = False;
			}
			StringBuilder_AppendByte(stringBuilder, ']');
		}
		return;

	case SMILE_KIND_OBJECT:
		StringBuilder_AppendC(stringBuilder, "Object", 0, 8);
		return;

	case SMILE_KIND_NULL:
		StringBuilder_AppendC(stringBuilder, "null", 0, 4);
		return;

	case SMILE_KIND_PAIR:
		pair = (SmilePair)obj;
		if (SMILE_KIND(pair->left) == SMILE_KIND_PAIR || ((SMILE_KIND(pair->left) & 0xF0) == 0x10)) {	// Pairs and numbers
			StringBuilder_AppendByte(stringBuilder, '(');
			StringifyRecursive(pair->left, stringBuilder, indent + 1);
			StringBuilder_AppendByte(stringBuilder, ')');
		}
		else {
			StringifyRecursive(pair->left, stringBuilder, indent);
		}
		StringBuilder_AppendByte(stringBuilder, '.');
		if (SMILE_KIND(pair->right) == SMILE_KIND_PAIR || ((SMILE_KIND(pair->right) & 0xF0) == 0x10)) {	// Pairs and numbers
			StringBuilder_AppendByte(stringBuilder, '(');
			StringifyRecursive(pair->right, stringBuilder, indent + 1);
			StringBuilder_AppendByte(stringBuilder, ')');
		}
		else {
			StringifyRecursive(pair->right, stringBuilder, indent);
		}
		return;

	case SMILE_KIND_SYMBOL:
		StringBuilder_AppendString(stringBuilder, SymbolTable_GetName(Smile_SymbolTable, ((SmileSymbol)obj)->symbol));
		return;

	case SMILE_KIND_INTEGER32:
		StringBuilder_AppendFormat(stringBuilder, "%d", ((SmileInteger32)obj)->value);
		return;

	case SMILE_KIND_STRING:
		StringBuilder_AppendFormat(stringBuilder, "\"%S\"", String_AddCSlashes((String)&((SmileString)obj)->string));
		return;

	case SMILE_KIND_NONTERMINAL:
		StringBuilder_AppendFormat(stringBuilder, "[%S %S %S %S]",
			((SmileNonterminal)obj)->nonterminal != 0 ? SymbolTable_GetName(Smile_SymbolTable, ((SmileNonterminal)obj)->nonterminal) : String_Empty,
			((SmileNonterminal)obj)->name != 0 ? SymbolTable_GetName(Smile_SymbolTable, ((SmileNonterminal)obj)->name) : String_Empty,
			((SmileNonterminal)obj)->repeat != 0 ? SymbolTable_GetName(Smile_SymbolTable, ((SmileNonterminal)obj)->repeat) : String_Empty,
			((SmileNonterminal)obj)->separator != 0 ? SymbolTable_GetName(Smile_SymbolTable, ((SmileNonterminal)obj)->separator) : String_Empty
		);
		return;

	case SMILE_KIND_SYNTAX:
		StringBuilder_AppendFormat(stringBuilder, "#syntax %S ",
			((SmileSyntax)obj)->nonterminal != 0 ? SymbolTable_GetName(Smile_SymbolTable, ((SmileSyntax)obj)->nonterminal) : String_Empty);
		StringifyRecursive((SmileObject)((SmileSyntax)obj)->pattern, stringBuilder, indent + 1);
		StringBuilder_AppendC(stringBuilder, " => ", 0, 4);
		StringifyRecursive(((SmileSyntax)obj)->replacement, stringBuilder, indent + 1);
		return;

	default:
		StringBuilder_AppendFormat(stringBuilder, "<%d>", SMILE_KIND(obj));
		return;
	}
}

Lexer SetupLexerFromString(String source)
{
	Lexer lexer;

	Smile_ResetEnvironment();

	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	return lexer;
}

Lexer SetupLexer(const char *string)
{
	return SetupLexerFromString(String_FromC(string));
}
