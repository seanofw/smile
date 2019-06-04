//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#define _CRT_SECURE_NO_WARNINGS		// Shut up, Visual Studio. It's okay.
#include <stdio.h>

#include <smile/types.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/parseinclude.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/internal/staticstring.h>
#include <smile/stringbuilder.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilesyntax.h>

static ModuleInfo Parser_LoadInstalledPackage(Parser parser, String filename, LexerPosition position);
static ModuleInfo Parser_LoadUserFile(Parser parser, String filename, LexerPosition position);

static ParseError Parser_ParseIncludeName(Parser parser, Symbol *oldName, Symbol *newName);

static SmileObject Parser_ExposeAll(Parser parser, ParseScope target, ModuleInfo moduleInfo, LexerPosition position);
static SmileObject Parser_ExposeOne(Parser parser, ParseScope target, ModuleInfo moduleInfo, Symbol oldName, Symbol newName);
static Bool Parser_ExposeSyntax(Parser parser, ParseScope target, ModuleInfo moduleInfo, LexerPosition position);

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

// term ::= LOANWORD_INCLUDE include_expr
// include_expr :: = STRING
//     | STRING COLON include_names
// include_names :: = anyname
//     | anyname AS anyname
//     | LOANWORD_SYNTAX
//     | LOANWORD_ALL
//     | include_names COMMA anyname
ParseResult Parser_ParseInclude(Parser parser)
{
	Int tokenKind;
	String filename;
	LexerPosition position;
	ClosureInfo oldGlobalClosure;
	ModuleInfo moduleInfo;
	STATIC_STRING(dot, ".");
	ParseError error;
	Symbol oldName, newName;
	SmileList head, tail;
	SmileList resultHead, resultTail;
	SmileObject setExpr;

	if ((tokenKind = Lexer_Next(parser->lexer)) != TOKEN_DYNSTRING
		&& tokenKind != TOKEN_RAWSTRING) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_FromC("#include must be followed by a string naming a source file or a package")));
	}

	position = Token_GetPosition(parser->lexer->token);

	filename = parser->lexer->token->text;

	// Save the current global closure, just in case the library switches and forgets to switch back.
	oldGlobalClosure = Smile_GetGlobalClosureInfo();

	if (String_StartsWith(filename, dot) || DoesStringEndWithAlphabeticExtension(filename)) {
		// This either starts with something like "./" or ends with something like ".sm", so it's a
		// request to import a file in a path relative to the current source file, and not an
		// installed package name.
		moduleInfo = Parser_LoadUserFile(parser, filename, position);
	}
	else {
		// This is an installed package name, so inhale it.
		moduleInfo = Parser_LoadInstalledPackage(parser, filename, position);
	}

	// Restore the global closure.
	Smile_SetGlobalClosureInfo(oldGlobalClosure);

	// If we failed, let the caller deal with it.
	if (!moduleInfo->loadedSuccessfully) {
		return ERROR_RESULT(
			ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Loading of module \"%S\" failed.", filename)));
	}

	// See if they want only a partial import.
	if (Lexer_Next(parser->lexer) != TOKEN_COLON) {
		Lexer_Unget(parser->lexer);
		return EXPR_RESULT(Parser_ExposeAll(parser, parser->currentScope, moduleInfo, position));
	}

	head = tail = NullList;
	resultHead = resultTail = NullList;

	// If there's a comma, keep doing that.
	do {

		tokenKind = Lexer_Peek(parser->lexer);
		if (tokenKind == TOKEN_ALPHANAME || tokenKind == TOKEN_UNKNOWNALPHANAME
			|| tokenKind == TOKEN_PUNCTNAME || tokenKind == TOKEN_UNKNOWNPUNCTNAME) {

			// Parse the next include name.
			error = Parser_ParseIncludeName(parser, &oldName, &newName);
			if (error != NULL)
				return ERROR_RESULT(error);

			// Expose it in the current scope.
			setExpr = Parser_ExposeOne(parser, parser->currentScope, moduleInfo, oldName, newName);
			LIST_APPEND(head, tail, setExpr);
			LIST_APPEND(resultHead, resultTail, SmileSymbol_Create(newName));
		}
		else if (tokenKind == TOKEN_LOANWORD_SYNTAX) {
			Lexer_Next(parser->lexer);

			// They want to import the #syntax rules exported by this module, so import them all
			// (it's all-or-nothing, since there can be interdependencies between them).
			Parser_ExposeSyntax(parser, parser->currentScope, moduleInfo, Token_GetPosition(parser->lexer->token));
		}
		else {
			Parser_AddError(parser, Token_GetPosition(parser->lexer->token), "Missing variable name after #include directive");

			// If this was a comma, they wrote something like "foo, bar,, baz", so we
			// can recover by just pretending we didn't see two in a row.
			if (tokenKind == TOKEN_COMMA)
				continue;
		}

		// If there's a comma, keep consuming input.
	} while (Lexer_Next(parser->lexer) == TOKEN_COMMA);
	Lexer_Unget(parser->lexer);

	// Turn the list of symbols into a [$quote ...] expression, and attach it to the end of the work.
	LIST_APPEND(head, tail,
		(SmileObject)SmileList_Cons((SmileObject)Smile_KnownObjects._quoteSymbol,
			(SmileObject)SmileList_Cons((SmileObject)resultHead,
				NullObject)));

	// Prepend a [$progn ...] on the front of the list to make it executable.
	head = SmileList_Cons((SmileObject)Smile_KnownObjects._prognSymbol, (SmileObject)head);

	// Return the fully-executable expression.
	return EXPR_RESULT(head);
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

static ModuleInfo Parser_LoadInstalledPackage(Parser parser, String filename, LexerPosition position)
{
	ModuleInfo moduleInfo;

	UNUSED(parser);

	// Did we load this once already?  If so, return it.
	moduleInfo = ModuleInfo_GetModuleByName(filename);
	if (moduleInfo != NULL)
		return moduleInfo;

	// Empty is just wrong.
	if (String_IsNullOrEmpty(filename)) {
		moduleInfo = ModuleInfo_CreateFromError(filename,
			ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FromC("Unknown #include module \"\".")));
		ModuleInfo_Register(moduleInfo);
		return moduleInfo;
	}

	// Certain packages are built-in, so let's test those first.
	switch (String_GetBytes(filename)[0]) {
		case 's':
			if (String_EqualsC(filename, "stdio")) {
				ModuleInfo_Register(moduleInfo = Stdio_Main());
				return moduleInfo;
			}
			else goto error;

		default:
		error:
			return ModuleInfo_CreateFromError(filename,
				ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Unknown #include module \"%S\".", filename)));
	}
}

static ModuleInfo Parser_LoadUserFile(Parser parser, String filename, LexerPosition position)
{
	String text;
	ParseError error;
	SmileObject expr;
	ParseMessage *parseMessages;
	ParseScope moduleScope;
	Int numParseMessages;
	String relativeDirectory, fullIncludePath;
	ModuleInfo moduleInfo;

	// Includes are always found relative to the file that includes them.
	relativeDirectory = (position->filename != NULL ? Path_GetDirname(position->filename) : String_Empty);
	fullIncludePath = Path_Resolve(relativeDirectory, filename);

	// Did we load this once already?  If so, just return its instance.
	if ((moduleInfo = ModuleInfo_GetModuleByName(fullIncludePath)) != NULL)
		return moduleInfo;

	// Load the source file into memory.
	error = parser->includeLoader(fullIncludePath, position, &text);
	if (error != NULL) {
		moduleInfo = ModuleInfo_CreateFromError(fullIncludePath, error);
		ModuleInfo_Register(moduleInfo);
		return moduleInfo;
	}

	// Parse it!
	expr = Smile_ParseInScope(text, fullIncludePath, NULL, 0, &parseMessages, &numParseMessages, &moduleScope);

	// Record it.
	moduleInfo = ModuleInfo_Create(fullIncludePath, numParseMessages <= 0, expr,
		moduleScope, parseMessages, numParseMessages);
	ModuleInfo_Register(moduleInfo);
	return moduleInfo;
}

ParseError Parser_DefaultIncludeLoader(const String fullPath, const LexerPosition position, String *result)
{
	FILE *fp;
	StringBuilder stringBuilder;
	Byte *buffer;
	size_t readLength;
	ParseError parseError;

	const int ReadLength = 0x10000;	// Read 64K at a time.

	// Open it for reading, raw and fast.
	if ((fp = fopen(String_ToC(fullPath), "rb")) == NULL) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position,
			String_Format("Cannot open \"%s\" for reading.", String_ToC(fullPath)));
		*result = NULL;
		return parseError;
	}

	// Make a StringBuilder to hold the file we're about to load.  We start it at 64K because
	// that's enough to hold most (even fairly large!) source files without reallocation.
	stringBuilder = StringBuilder_CreateWithSize(ReadLength);

	// Allocate a read buffer for fread() to push data into.
	buffer = GC_MALLOC_RAW_ARRAY(Byte, ReadLength);
	if (buffer == NULL)
		Smile_Abort_OutOfMemory();

	// Inhale the entire file into the StringBuilder.
	while ((readLength = fread(buffer, 1, ReadLength, fp)) > 0) {
		StringBuilder_Append(stringBuilder, buffer, 0, readLength);
	}

	// Cleanup.
	fclose(fp);

	// Return the file we read.
	*result = StringBuilder_ToString(stringBuilder);
	return NULL;
}

/// <summary>
/// Expose all of the module's global objects into the given scope with their original
/// names, and return an expression that will properly assign all of those names
/// at execution time.  This also exposes all of the module's syntax rules.
/// </summary>
/// <param name="parser">The parser, used here for collecting error messages.</param>
/// <param name="target">The target scope in which the objects are to be exposed.</param>
/// <param name="moduleInfo">The module that contains the symbol to expose.</param>
/// <param name="position">The position at which the #include was found (for error-reporting.</param>
SmileObject Parser_ExposeAll(Parser parser, ParseScope target, ModuleInfo moduleInfo, LexerPosition position)
{
	SmileList head, tail;
	SmileList resultHead, resultTail;
	SmileObject expr;
	Symbol symbol;
	Int32Int32Dict sourceSymbolDict;
	Int32Int32DictKeyValuePair *pairs;
	ParseDecl *sourceDecls;
	ParseDecl sourceDecl;
	Int i, numPairs;

	// Get all the names that need to be exposed.
	sourceSymbolDict = moduleInfo->parseScope->symbolDict;
	sourceDecls = moduleInfo->parseScope->decls;
	numPairs = Int32Int32Dict_Count(sourceSymbolDict);
	pairs = Int32Int32Dict_GetAll(sourceSymbolDict);

	// Create an [$include] expression that exposes each one, and join them all into a big list of work.
	LIST_INIT(head, tail);
	LIST_INIT(resultHead, resultTail);
	for (i = 0; i < numPairs; i++) {
		symbol = pairs[i].key;
		sourceDecl = sourceDecls[pairs[i].value];
		// Only re-expose const or var names explicitly declared in the source scope;
		// we don't re-expose names that were imported by it.
		if (sourceDecl->declKind == PARSEDECL_CONST
			|| sourceDecl->declKind == PARSEDECL_VARIABLE
			|| sourceDecl->declKind == PARSEDECL_GLOBAL) {
			expr = Parser_ExposeOne(parser, target, moduleInfo, symbol, symbol);
			LIST_APPEND(head, tail, expr);
			LIST_APPEND(resultHead, resultTail, SmileSymbol_Create(symbol));
		}
	}

	// Turn the list of symbols into a [$quote ...] expression, and attach it to the end of the work.
	LIST_APPEND(head, tail,
		(SmileObject)SmileList_Cons((SmileObject)Smile_KnownObjects._quoteSymbol,
			(SmileObject)SmileList_Cons((SmileObject)resultHead,
				NullObject)));

	// Prepend a [$progn ...] on the front of the list to make it executable.
	head = SmileList_Cons((SmileObject)Smile_KnownObjects._prognSymbol, (SmileObject)head);

	// Redeclare all of the module's syntax rules (if any) in the current scope.
	Parser_ExposeSyntax(parser, target, moduleInfo, position);

	// Finally, return the compound expression for the imported symbols.
	return (SmileObject)head;
}

/// <summary>
/// Expose the syntax from the given module as a set of syntax rules in the current
/// scope, but import them in the "included" list so that they don't automatically re-export.
/// </summary>
Bool Parser_ExposeSyntax(Parser parser, ParseScope target, ModuleInfo moduleInfo, LexerPosition position)
{
	SmileList list;
	SmileSyntax rule;

	// Add the #syntax rules declared in the module's scope, exactly if we'd just written
	// those same #syntax rules right here, but put them into the 'include' list rather
	// than the 'declared' list when we add them.
	for (list = moduleInfo->parseScope->syntaxListHead;
		SMILE_KIND(list) == SMILE_KIND_LIST; list = (SmileList)list->d) {

		rule = (SmileSyntax)list->a;
		if (SMILE_KIND(rule) != SMILE_KIND_SYNTAX) {
			Parser_AddFatalError(parser, position, "Illegal non-Syntax object in module's scope's syntax list; this is probably a bug.");
			return False;
		}

		// This may generate an error, if the imported rule clashes with existing rules.
		if (ParserSyntaxTable_AddRule(parser, &target->syntaxTable, rule)) {
			ParseScope_AddIncludeSyntax(target, (SmileSyntax)rule);
		}
	}

	// If this module is re-exporting its own imported #syntax rules, bring those in too.
	if (moduleInfo->parseScope->reexport) {

		// Add the #syntax rules included in the module's scope, exactly if we'd included
		// those same #syntax rules from wherever they came from.
		for (list = moduleInfo->parseScope->syntaxIncludeListHead;
			SMILE_KIND(list) == SMILE_KIND_LIST; list = (SmileList)list->d) {

			rule = (SmileSyntax)list->a;
			if (SMILE_KIND(rule) != SMILE_KIND_SYNTAX) {
				Parser_AddFatalError(parser, position, "Illegal non-Syntax object in module's scope's syntax list; this is probably a bug.");
				return False;
			}

			// This may generate an error, if the imported rule clashes with existing rules.
			if (ParserSyntaxTable_AddRule(parser, &target->syntaxTable, rule)) {
				ParseScope_AddIncludeSyntax(target, (SmileSyntax)rule);
			}
		}
	}

	return True;
}

/// <summary>
/// Expose the given module symbol in the given scope with the given new name, and
/// return an expression that will properly assign the new name at execution time.
/// </summary>
/// <param name="moduleInfo">The module that contains the symbol to expose.</param>
/// <param name="parser">The parser, used here for collecting error messages.</param>
/// <param name="target">The target scope in which the new name is to be exposed.</param>
/// <param name="oldName">The original name of the module object to be exposed (i.e., the
/// name that the object uses in the module's global closure).</param>
/// <param name="newName">The new name of the symbol that the module object will have
/// in this scope.</param>
SmileObject Parser_ExposeOne(Parser parser, ParseScope target, ModuleInfo moduleInfo, Symbol oldName, Symbol newName)
{
	ParseError parseError;
	SmileObject result;

	// Declare it in the current scope, by name.
	parseError = ParseScope_Declare(target, newName, PARSEDECL_INCLUDE, NULL, NULL);

	// We have nowhere else to report errors, so if we got a conflict during declaration, add it
	// to the provided parser's error list.
	if (parseError != NULL)
		Parser_AddMessage(parser, parseError);

	// We can now construct a [$include ...] form that copies the preexisting member of the module
	// object into a local variable.
	if (newName != oldName) {
		result =
			(SmileObject)SmileList_Cons((SmileObject)Smile_KnownObjects._includeSymbol,
				(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(moduleInfo->id),
					(SmileObject)SmileList_Cons((SmileObject)SmileSymbol_Create(oldName),
						(SmileObject)SmileList_Cons((SmileObject)SmileSymbol_Create(newName),
							NullObject))));
	}
	else {
		result =
			(SmileObject)SmileList_Cons((SmileObject)Smile_KnownObjects._includeSymbol,
				(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(moduleInfo->id),
					(SmileObject)SmileList_Cons((SmileObject)SmileSymbol_Create(oldName),
						NullObject)));
	}

	return result;
}
