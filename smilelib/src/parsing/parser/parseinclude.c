//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#include <smile/types.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/parseinclude.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/internal/staticstring.h>

static ParseError Parser_LoadInstalledPackage(Parser parser, String filename, LibraryInfo *libraryInfo, LexerPosition position);
static ParseError Parser_LoadUserFile(Parser parser, String filename, LibraryInfo *libraryInfo, LexerPosition position);

static ParseError Parser_ParseIncludeName(Parser parser, Symbol *oldName, Symbol *newName);

static Bool DoesStringEndWithAlphabeticExtension(String str)
{
	const Byte *text = String_GetBytes(str);
	Int length = String_Length(str);
	const Byte *end = text + length;

	while (end > text) {
		Byte ch = end[-1];
		if (ch == '.')
			return True;
		if (ch == '/' || !((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')))
			return False;
		end--;
	}

	return False;
}

// term ::= LOANWORD_INCLUDE string include_set_opt
// include_set_opt ::= ':' include_names |
// include_names ::= include_name | include_names ',' include_name
// include_name ::= NAME | NAME AS NAME
ParseError Parser_ParseInclude(Parser parser, SmileObject *expr)
{
	Int tokenKind;
	String filename;
	LexerPosition position;
	ClosureInfo oldGlobalClosure;
	LibraryInfo libraryInfo;
	STATIC_STRING(dot, ".");
	ParseError error;
	Symbol oldName, newName;
	SmileList head, tail;
	SmileList resultHead, resultTail;
	SmileObject setExpr;

	*expr = NullObject;

	if ((tokenKind = Lexer_Next(parser->lexer)) != TOKEN_DYNSTRING
		&& tokenKind != TOKEN_RAWSTRING) {
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_FromC("#include must be followed by a string naming a source file or a package"));
	}

	position = Token_GetPosition(parser->lexer->token);

	filename = parser->lexer->token->text;

	// Save the current global closure, just in case the library switches and forgets to switch back.
	oldGlobalClosure = Smile_GetGlobalClosureInfo();

	if (String_StartsWith(filename, dot) || DoesStringEndWithAlphabeticExtension(filename)) {
		// This either starts with something like "./" or ends with something like ".sm", so it's a
		// request to import a file in a path relative to the current source file, and not an
		// installed package name.
		error = Parser_LoadUserFile(parser, filename, &libraryInfo, position);
	}
	else {
		// This is an installed package name, so inhale it.
		error = Parser_LoadInstalledPackage(parser, filename, &libraryInfo, position);
	}

	// Restore the global closure.
	Smile_SetGlobalClosureInfo(oldGlobalClosure);

	if (error != NULL)
		return error;

	// See if they want only a partial import.
	if (Lexer_Next(parser->lexer) != TOKEN_COLON) {
		Lexer_Unget(parser->lexer);
		*expr = LibraryInfo_ExposeAll(libraryInfo, parser, parser->currentScope);
		return NULL;
	}

	head = tail = NullList;
	resultHead = resultTail = NullList;

	// Parse the first include name.
	error = Parser_ParseIncludeName(parser, &oldName, &newName);
	if (error != NULL)
		return error;

	// Expose it in the current scope.
	setExpr = LibraryInfo_ExposeOne(libraryInfo, parser, parser->currentScope, oldName, newName);
	LIST_APPEND(head, tail, setExpr);
	LIST_APPEND(resultHead, resultTail, SmileSymbol_Create(newName));

	// If there's a comma, keep doing that.
	while (Lexer_Next(parser->lexer) == TOKEN_COMMA) {

		tokenKind = Lexer_Peek(parser->lexer);
		if (tokenKind != TOKEN_ALPHANAME && tokenKind != TOKEN_UNKNOWNALPHANAME
			&& tokenKind != TOKEN_PUNCTNAME && tokenKind != TOKEN_UNKNOWNPUNCTNAME) {

			Parser_AddError(parser, Token_GetPosition(parser->lexer->token), "Missing variable name in #include directive");

			// If this was a comma, they wrote something like "foo, bar,, baz", so we
			// can recover by just pretending we didn't see two in a row.
			if (tokenKind == TOKEN_COMMA)
				continue;
		}

		// Parse the next include name.
		error = Parser_ParseIncludeName(parser, &oldName, &newName);
		if (error != NULL)
			return error;

		// Expose it in the current scope.
		setExpr = LibraryInfo_ExposeOne(libraryInfo, parser, parser->currentScope, oldName, newName);
		LIST_APPEND(head, tail, setExpr);
		LIST_APPEND(resultHead, resultTail, SmileSymbol_Create(newName));
	}
	Lexer_Unget(parser->lexer);

	// Turn the list of symbols into a [$quote ...] expression, and attach it to the end of the work.
	LIST_APPEND(head, tail,
		(SmileObject)SmileList_Cons((SmileObject)Smile_KnownObjects._quoteSymbol,
			(SmileObject)SmileList_Cons((SmileObject)resultHead,
				NullObject)));

	// Prepend a [$progn ...] on the front of the list to make it executable.
	head = SmileList_Cons((SmileObject)Smile_KnownObjects._prognSymbol, (SmileObject)head);

	// Return the fully-executable expression.
	*expr = (SmileObject)head;
	return NULL;
}

static ParseError Parser_ParseIncludeName(Parser parser, Symbol *oldName, Symbol *newName)
{
	Int tokenKind;

	tokenKind = Lexer_Next(parser->lexer);
	if (tokenKind != TOKEN_ALPHANAME && tokenKind != TOKEN_UNKNOWNALPHANAME
		&& tokenKind != TOKEN_PUNCTNAME && tokenKind != TOKEN_UNKNOWNPUNCTNAME) {

		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_FromC("Missing variable name in #include directive"));
	}

	*oldName = parser->lexer->token->data.symbol;

	tokenKind = Lexer_Next(parser->lexer);
	if ((tokenKind == TOKEN_ALPHANAME || tokenKind == TOKEN_UNKNOWNALPHANAME)
		&& parser->lexer->token->data.symbol == Smile_KnownSymbols.as) {

		tokenKind = Lexer_Next(parser->lexer);
		if (tokenKind != TOKEN_ALPHANAME && tokenKind != TOKEN_UNKNOWNALPHANAME
			&& tokenKind != TOKEN_PUNCTNAME && tokenKind != TOKEN_UNKNOWNPUNCTNAME) {

			return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_FromC("Missing variable name after 'as' in #include directive"));
		}

		*newName = parser->lexer->token->data.symbol;
	}
	else {
		Lexer_Unget(parser->lexer);
		*newName = *oldName;
	}

	return NULL;
}

static ParseError Parser_LoadInstalledPackage(Parser parser, String filename, LibraryInfo *libraryInfo, LexerPosition position)
{
	UNUSED(parser);

	// Empty is just wrong.
	if (String_IsNullOrEmpty(filename))
		return ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FromC("Unknown #include package \"\"."));

	// Certain packages are built-in, so let's test those first.
	switch (String_At(filename, 0)) {
		case 's':
			if (String_EqualsC(filename, "stdio")) {
				*libraryInfo = Stdio_Main();
				break;
			}
			else goto error;

		default:
		error:
			*libraryInfo = NULL;
			return ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Unknown #include package \"%S\".", filename));
	}

	return NULL;
}

static ParseError Parser_LoadUserFile(Parser parser, String filename, LibraryInfo *libraryInfo, LexerPosition position)
{
	ParseError error;

	*libraryInfo = NULL;
	UNUSED(parser);

	error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Cannot load #include file \"%S\".", filename));
	return error;
}

LibraryInfo LibraryInfo_Create(String name, Bool loadedSuccessfully, ClosureInfo globalClosureInfo,
	ParseMessage *parseMessages, Int numParseMessages)
{
	LibraryInfo libraryInfo = GC_MALLOC_STRUCT(struct LibraryInfoStruct);

	libraryInfo->name = name;
	libraryInfo->loadedSuccessfully = loadedSuccessfully;
	libraryInfo->globalClosureInfo = globalClosureInfo;
	libraryInfo->parseMessages = parseMessages;
	libraryInfo->numParseMessages = numParseMessages;

	return libraryInfo;
}

/// <summary>
/// Expose all of the library's global objects into the given scope with their original
/// names, and return an expression that will properly assign all of those names
/// at execution time.
/// </summary>
/// <param name="libraryInfo">The library that contains the symbol to expose.</param>
/// <param name="parser">The parser, used here for collecting error messages.</param>
/// <param name="target">The target scope in which the objects are to be exposed.</param>
SmileObject LibraryInfo_ExposeAll(LibraryInfo libraryInfo, Parser parser, ParseScope target)
{
	VarDict varDict;
	Symbol *symbols;
	Int i, numSymbols;
	SmileList head, tail;
	SmileList resultHead, resultTail;
	SmileObject expr;

	// Get all the names that need to be exposed.
	varDict = libraryInfo->globalClosureInfo->variableDictionary;
	symbols = VarDict_GetKeys(varDict);
	numSymbols = VarDict_Count(varDict);

	// Create an expression that exposes each one, and join them all into a big list of work.
	LIST_INIT(head, tail);
	LIST_INIT(resultHead, resultTail);
	for (i = 0; i < numSymbols; i++) {
		expr = LibraryInfo_ExposeOne(libraryInfo, parser, target, symbols[i], symbols[i]);
		LIST_APPEND(head, tail, expr);
		LIST_APPEND(resultHead, resultTail, SmileSymbol_Create(symbols[i]));
	}

	// Turn the list of symbols into a [$quote ...] expression, and attach it to the end of the work.
	LIST_APPEND(head, tail,
		(SmileObject)SmileList_Cons((SmileObject)Smile_KnownObjects._quoteSymbol,
			(SmileObject)SmileList_Cons((SmileObject)resultHead,
				NullObject)));

	// Prepend a [$progn ...] on the front of the list to make it executable.
	head = SmileList_Cons((SmileObject)Smile_KnownObjects._prognSymbol, (SmileObject)head);

	// And return the compound expression.
	return (SmileObject)head;
}

/// <summary>
/// Expose the given library symbol in the given scope with the given new name, and
/// return an expression that will properlly assign the new name at execution time.
/// </summary>
/// <param name="libraryInfo">The library that contains the symbol to expose.</param>
/// <param name="parser">The parser, used here for collecting error messages.</param>
/// <param name="target">The target scope in which the new name is to be exposed.</param>
/// <param name="oldName">The original name of the library object to be exposed (i.e., the
/// name that the object uses in the library's global closure).</param>
/// <param name="newName">The new name of the symbol that the library object will have
/// in this scope.</param>
SmileObject LibraryInfo_ExposeOne(LibraryInfo libraryInfo, Parser parser, ParseScope target, Symbol oldName, Symbol newName)
{
	VarDict varDict;
	VarInfo varInfo;
	ParseError parseError;
	SmileObject result;

	// Get the value to be exposed out of the library's dictionary of global variables.
	varDict = libraryInfo->globalClosureInfo->variableDictionary;
	if (!VarDict_TryGetValue(varDict, oldName, &varInfo))
		return False;

	// Declare it in the current scope, by name.
	parseError = ParseScope_Declare(target, newName, PARSEDECL_CONST, NULL, NULL);

	// We have nowhere else to report errors, so if we got a conflict during declaration, add it
	// to the provided parser's error list.
	if (parseError != NULL)
		Parser_AddMessage(parser, parseError);

	// We can now construct a [$set ...] form that assigns the new name to the preexisting object.
	result =
		(SmileObject)SmileList_Cons((SmileObject)Smile_KnownObjects._setSymbol,
			(SmileObject)SmileList_Cons((SmileObject)SmileSymbol_Create(newName),
				(SmileObject)SmileList_Cons(varInfo->value,
					NullObject)));

	return result;
}
