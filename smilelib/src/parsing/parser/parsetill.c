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

#include <smile/types.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

//-----------------------------------------------------------------------------
//  Syntax-based till:  till x, y, z do { ... } when x { ... } when y { ... }

// till_do ::= TILL . till_names DO expr whens_opt
ParseResult Parser_ParseTill(Parser parser, Int modeFlags, LexerPosition lexerPosition)
{
	ParseResult parseResult;
	Int32Int32Dict tillFlags;
	SmileList names;
	SmileObject body;
	Token token;
	SmileObject whens;

	Parser_BeginScope(parser, PARSESCOPE_TILLDO);
	tillFlags = parser->currentScope->symbolDict;

	// Parse the till-names first.
	parseResult = Parser_ParseTillNames(parser);
	if (IS_PARSE_ERROR(parseResult)) {
		// Bad flags.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_DO) {

			// Bad names, but we recovered to a 'do', so try to keep going.
			Parser_NextToken(parser);
			HANDLE_PARSE_ERROR(parser, parseResult);
			names = NullList;
		}
		else {
			Parser_EndScope(parser, False);
			return parseResult;
		}
	}
	else names = (SmileList)parseResult.expr;

	// Consume the 'do' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_DO)) {
		// Missing 'do' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'do' keyword after 'till'.");
		Lexer_Unget(parser->lexer);
	}

	// Parse the loop body.
	parseResult = Parser_ParseExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult)) {
		// Bad flags.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_WHEN) {

			// Bad body, but we recovered to a 'when', so try to keep going.
			Parser_NextToken(parser);
			HANDLE_PARSE_ERROR(parser, parseResult);
			body = NullObject;
		}
		else {
			Parser_EndScope(parser, False);
			return parseResult;
		}
	}
	else body = parseResult.expr;

	Parser_EndScope(parser, False);

	// Parse any 'when' clauses hanging off the bottom of the loop.
	parseResult = Parser_ParseWhens(parser, tillFlags, modeFlags);
	if (IS_PARSE_ERROR(parseResult)) {
		HANDLE_PARSE_ERROR(parser, parseResult);
		whens = NullObject;
	}
	else whens = parseResult.expr;

	// Now make the resulting form: [$till flags body whens]
	if (SMILE_KIND(whens) != SMILE_KIND_NULL)
		return EXPR_RESULT(SmileList_CreateFourWithSource(Smile_KnownObjects._tillSymbol, names, body, whens, lexerPosition));
	else
		return EXPR_RESULT(SmileList_CreateThreeWithSource(Smile_KnownObjects._tillSymbol, names, body, lexerPosition));
}

// till_names ::= . anyname | . till_names COMMA anyname
ParseResult Parser_ParseTillNames(Parser parser)
{
	ParseResult parseResult;
	SmileObject decl;
	SmileList head, tail;
	LexerPosition position;

	// Parse the first name, which results in a symbol like 'x'.
	position = Lexer_GetPosition(parser->lexer);
	parseResult = Parser_ParseTillName(parser);
	if (IS_PARSE_ERROR(parseResult)) return parseResult;
	decl = parseResult.expr;

	// Wrap it in a list, so it becomes [x].
	LIST_INIT(head, tail);
	if (decl->kind != SMILE_KIND_NULL) {
		LIST_APPEND_WITH_SOURCE(head, tail, decl, position);
	}

	// Every time we see a comma, parse the next name, and add it to the list.
	while (Parser_NextToken(parser)->kind == TOKEN_COMMA) {

		position = Lexer_GetPosition(parser->lexer);
		parseResult = Parser_ParseTillName(parser);
		if (IS_PARSE_ERROR(parseResult)) return parseResult;
		decl = parseResult.expr;

		if (decl->kind != SMILE_KIND_NULL) {
			LIST_APPEND_WITH_SOURCE(head, tail, decl, position);
		}
	}

	// Don't overconsume at the end.
	Lexer_Unget(parser->lexer);

	// Return the list of names.
	return EXPR_RESULT(head);
}

// anyname ::= NAME
ParseResult Parser_ParseTillName(Parser parser)
{
	Token token;
	ParseError error;
	Symbol symbol;

	// Get the name.
	if ((token = Parser_NextToken(parser))->kind != TOKEN_UNKNOWNALPHANAME
		&& token->kind != TOKEN_ALPHANAME
		&& token->kind != TOKEN_UNKNOWNPUNCTNAME
		&& token->kind != TOKEN_PUNCTNAME) {

		// No variable name?  That's an error.
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("Missing flag name after 'till'.")));
	}

	// Declare it in the current scope.
	symbol = token->data.symbol;
	error = ParseScope_DeclareHere(parser->currentScope, symbol, PARSEDECL_TILL, Token_GetPosition(token), NULL);
	if (error != NULL)
		return ERROR_RESULT(error);

	return EXPR_RESULT(SmileSymbol_Create(symbol));
}

// whens_opt ::= whens |
// whens ::= when whens | when
// when ::= WHEN name expr
ParseResult Parser_ParseWhens(Parser parser, Int32Int32Dict tillFlags, Int modeFlags)
{
	ParseResult parseResult;
	Token token;
	Int32Dict usedSymbolDict;
	SmileList whenHead, whenTail;
	Symbol flagName;
	SmileObject whenBody;
	LexerPosition position;
	SmileObject whenClause;

	usedSymbolDict = Int32Dict_Create();

	whenHead = whenTail = NullList;

	// Parse any 'when' constructs that follow.
	for (;;) {

		// Consume a 'when' keyword.
		if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
			&& token->data.symbol == SMILE_SPECIAL_SYMBOL_WHEN)) {
			// No 'when' keyword, so we're done.
			Lexer_Unget(parser->lexer);
			break;
		}
		position = Token_GetPosition(token);

		// There should be a symbol that follows it.
		if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME
			|| token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_UNKNOWNPUNCTNAME))) {
			// Not a symbol, so complain, but rewind a token and keep going if we can.
			Parser_AddError(parser, Token_GetPosition(token), "Missing till-flag name after 'when'.");
			Lexer_Unget(parser->lexer);
			flagName = 0;
		}
		else {
			flagName = token->data.symbol;

			// That symbol should be one of the flag names declared in the 'till'.
			if (!Int32Int32Dict_ContainsKey(tillFlags, flagName)) {
				Parser_AddError(parser, Token_GetPosition(token), "'%S' is not a name declared in this 'till' loop.",
					SymbolTable_GetName(Smile_SymbolTable, flagName));
				// Try to keep going anyway.
			}
			else {
				// That symbol shouldn't have been used already for this 'till'.
				if (!Int32Dict_Add(usedSymbolDict, flagName, NULL)) {
					Parser_AddError(parser, Token_GetPosition(token), "'when %S' was used multiple times for the same 'till' loop.",
						SymbolTable_GetName(Smile_SymbolTable, flagName));
					// Keep going with the parse.
				}
			}
		}

		// There should be a body after the 'when name' clause.
		parseResult = Parser_ParseExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult)) {
			HANDLE_PARSE_ERROR(parser, parseResult);
			whenBody = NullObject;
		}
		else whenBody = parseResult.expr;

		// Construct the when clause itself:  [flag body]
		whenClause = (SmileObject)SmileList_CreateTwoWithSource(flagName ? (SmileObject)SmileSymbol_Create(flagName) : NullObject, whenBody, position);

		// Add it to the result list.
		LIST_APPEND_WITH_SOURCE(whenHead, whenTail, whenClause, position);
	}

	return EXPR_RESULT(whenHead);
}

//-----------------------------------------------------------------------------
//  Lisp-style till:  [$till [x y z] body [[x when] [y when] [z when]]]

// till-when ::= . '[' name expr ']'
static ParseResult Parser_ParseClassicTillWhen(Parser parser, ParseScope flagScope)
{
	LexerPosition position;
	Token token;
	SmileObject body, expr;
	ParseResult parseResult;
	Symbol name;
	ParseDecl flag;

	// Consume the '['.
	token = Parser_NextToken(parser);
	position = Token_GetPosition(token);

	// It should be followed by a name.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME
		&& token->kind != TOKEN_PUNCTNAME && token->kind != TOKEN_UNKNOWNPUNCTNAME) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FromC("Missing flag name for when-clause in [$till] form.")));
	}
	name = token->data.symbol;

	// The name should be one of the declared flags.
	flag = ParseScope_FindDeclarationHere(flagScope, name);
	if (flag == NULL) {
		ParseError error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Unknown flag name \"{0}\" for when-clause in [$till] form.",
			SymbolTable_GetName(Smile_SymbolTable, name)));
		Parser_AddMessage(parser, error);
	}
	else {
		// The name should also not have been used already.
		if (flag->scopeIndex < 0) {
			ParseError error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Flag name for \"{0}\" cannot be used for multiple when-clauses in the same [$till] form.",
				SymbolTable_GetName(Smile_SymbolTable, name)));
			Parser_AddMessage(parser, error);
		}
		flag->scopeIndex = -1;
	}

	// Now consume the expression that forms the body.
	parseResult = Parser_ParseExpr(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;
	else body = parseResult.expr;

	// Make sure there's a trailing ']' to end the when-clause.
	parseResult = Parser_ExpectRightBracket(parser, NULL, "$till when-clause", position);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;

	expr =
		(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(name),
			(SmileObject)SmileList_ConsWithSource(body, NullObject, position),
		position);
	return EXPR_RESULT(expr);
}

// till-whens-opt ::= . till_whens | .
// till-whens ::= . till_whens till-when | . till-when
static ParseResult Parser_ParseClassicTillWhens(Parser parser, ParseScope flagScope)
{
	SmileList head = NullList, tail = NullList;
	SmileObject when;
	ParseResult parseResult;
	LexerPosition position;

	while (Parser_HasLookahead(parser, TOKEN_LEFTBRACKET)) {
		position = Lexer_GetPosition(parser->lexer);

		parseResult = Parser_ParseClassicTillWhen(parser, flagScope);
		if (IS_PARSE_ERROR(parseResult)) {
			HANDLE_PARSE_ERROR(parser, parseResult);
			Parser_Recover(parser, Parser_BracesBracketsParenthesesBar_Recovery, Parser_BracesBracketsParenthesesBar_Count);
			continue;
		}
		when = parseResult.expr;

		LIST_APPEND_WITH_SOURCE(head, tail, when, position);
	}

	return EXPR_RESULT(head);
}

// till-flags ::= . names
static ParseResult Parser_ParseClassicTillFlagNames(Parser parser)
{
	SmileList head = NullList, tail = NullList;
	Token token;
	Symbol name;
	ParseError error;

	for (;;) {
		token = Parser_NextToken(parser);
		switch (token->kind) {
		
			case TOKEN_BAR:
			case TOKEN_LEFTBRACE:
			case TOKEN_LEFTBRACKET:
			case TOKEN_LEFTPARENTHESIS:
			case TOKEN_RIGHTBRACE:
			case TOKEN_RIGHTBRACKET:
			case TOKEN_RIGHTPARENTHESIS:
				Lexer_Unget(parser->lexer);
				return EXPR_RESULT(head);
			
			case TOKEN_ALPHANAME:
			case TOKEN_UNKNOWNALPHANAME:
			case TOKEN_PUNCTNAME:
			case TOKEN_UNKNOWNPUNCTNAME:
				name = token->data.symbol;
				LIST_APPEND_WITH_SOURCE(head, tail, SmileSymbol_Create(name), Token_GetPosition(token));
				break;

			default:
				error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("Missing name for [$till] flag."));
				Parser_AddMessage(parser, error);
				break;
		}
	}
}

// term ::= '[' '$till' . till-flags expr till-whens-opt ']'
ParseResult Parser_ParseClassicTill(Parser parser, LexerPosition startPosition)
{
	SmileObject body, expr;
	SmileList flags, whens, temp;
	ParseResult parseResult;
	ParseDecl decl;
	SmileSymbol smileSymbol;
	Token token;

	// Make sure there is a '[' to start the name list.
	parseResult = Parser_ExpectLeftBracket(parser, NULL, "$till", startPosition);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;

	Parser_BeginScope(parser, PARSESCOPE_TILLDO);

	// Parse the names.
	parseResult = Parser_ParseClassicTillFlagNames(parser);
	if (IS_PARSE_ERROR(parseResult)) {
		HANDLE_PARSE_ERROR(parser, parseResult);
		flags = NullList;
	}
	else flags = (SmileList)parseResult.expr;

	// Make sure there is a ']' to end the flags list.
	parseResult = Parser_ExpectRightBracket(parser, NULL, "$till flags", startPosition);
	if (IS_PARSE_ERROR(parseResult)) {
		Parser_EndScope(parser, False);
		return parseResult;
	}

	// The flags list cannot be empty.
	if (SMILE_KIND(flags) != SMILE_KIND_LIST) {
		ParseError error = ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition, String_FromC("A [$till] form must start with a list of flags."));
		Parser_AddMessage(parser, error);
	}
	else {
		// Spin over the flags list and declare each one in the new parsing scope.
		for (temp = flags; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
			smileSymbol = (SmileSymbol)temp->a;
			ParseScope_DeclareHere(parser->currentScope, smileSymbol->symbol, PARSEDECL_TILL, SMILE_VCALL(temp, getSourceLocation), &decl);
		}
	}

	// Parse the body expression, whatever it may be.
	parseResult = Parser_ParseExpr(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (IS_PARSE_ERROR(parseResult)) {
		HANDLE_PARSE_ERROR(parser, parseResult);
		body = NullObject;
	}
	else body = parseResult.expr;

	// The scope for the flags is no longer valid after the body expression.
	Parser_EndScope(parser, False);

	// If there's a bracket, parse any 'when' declarations.
	if (!Parser_HasLookahead(parser, TOKEN_LEFTBRACKET))
		whens = NullList;
	else {
		token = Parser_NextToken(parser);

		// Parse any 'when' declarations.
		parseResult = Parser_ParseClassicTillWhens(parser, parser->currentScope);
		if (IS_PARSE_ERROR(parseResult)) {
			HANDLE_PARSE_ERROR(parser, parseResult);
			whens = NullList;
		}
		else whens = (SmileList)parseResult.expr;

		// Make sure there is a ']' to end the whens-list.
		parseResult = Parser_ExpectRightBracket(parser, NULL, "$till whens", startPosition);
		if (IS_PARSE_ERROR(parseResult))
			return parseResult;
	}

	// Make sure there is a ']' to end the till.
	parseResult = Parser_ExpectRightBracket(parser, NULL, "$till", startPosition);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;

	// Construct the resulting [$till flags body whens] form.
	if (SMILE_KIND(whens) != SMILE_KIND_NULL) {
		expr =
			(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__TILL),
				(SmileObject)SmileList_ConsWithSource((SmileObject)flags,
					(SmileObject)SmileList_ConsWithSource(body,
						(SmileObject)SmileList_ConsWithSource((SmileObject)whens, NullObject, startPosition),
					startPosition),
				startPosition),
			startPosition);
	}
	else {
		expr =
			(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__TILL),
				(SmileObject)SmileList_ConsWithSource((SmileObject)flags,
					(SmileObject)SmileList_ConsWithSource(body, NullObject, startPosition),
				startPosition),
			startPosition);
	}

	return EXPR_RESULT(expr);
}
