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
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

//-----------------------------------------------------------------------------
//  Syntax-based till:  till x, y, z do { ... } when x { ... } when y { ... }

// till_do ::= TILL . till_names DO expr whens_opt
ParseError Parser_ParseTill(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition)
{
	ParseError parseError;
	Int32Int32Dict tillFlags;
	SmileList names;
	SmileObject body;
	Token token;
	SmileObject whens;

	Parser_BeginScope(parser, PARSESCOPE_TILLDO);
	tillFlags = parser->currentScope->symbolDict;

	// Parse the till-names first.
	if ((parseError = Parser_ParseTillNames(parser, &names)) != NULL) {
		// Bad flags.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_DO) {

			// Bad names, but we recovered to a 'do', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			names = NullList;
		}
		else {
			Parser_EndScope(parser, False);
			*expr = NullObject;
			return parseError;
		}
	}

	// Consume the 'do' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_DO)) {
		// Missing 'do' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'do' keyword after 'till'.");
		Lexer_Unget(parser->lexer);
	}

	// Parse the loop body.
	parseError = Parser_ParseExpr(parser, &body, modeFlags);
	if (parseError != NULL) {
		// Bad flags.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_WHEN) {

			// Bad body, but we recovered to a 'when', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			names = NullList;
		}
		else {
			Parser_EndScope(parser, False);
			*expr = NullObject;
			return parseError;
		}
	}

	Parser_EndScope(parser, False);

	// Parse any 'when' clauses hanging off the bottom of the loop.
	parseError = Parser_ParseWhens(parser, &whens, tillFlags, modeFlags);
	if (parseError != NULL) {
		Parser_AddMessage(parser, parseError);
		whens = NullObject;
	}

	// Now make the resulting form: [$till flags body whens]
	if (SMILE_KIND(whens) != SMILE_KIND_NULL) {
		*expr = (SmileObject)SmileList_CreateFourWithSource(Smile_KnownObjects._tillSymbol, names, body, whens, lexerPosition);
	}
	else {
		*expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._tillSymbol, names, body, lexerPosition);
	}
	return NULL;
}

// till_names ::= . anyname | . till_names COMMA anyname
ParseError Parser_ParseTillNames(Parser parser, SmileList *names)
{
	SmileObject decl;
	ParseError error;
	SmileList head, tail;

	// Parse the first name, which results in a symbol like 'x'.
	error = Parser_ParseTillName(parser, &decl);
	if (error != NULL) return error;

	// Wrap it in a list, so it becomes [x].
	LIST_INIT(head, tail);
	if (decl->kind != SMILE_KIND_NULL) {
		LIST_APPEND_WITH_SOURCE(head, tail, decl, ((struct SmileListWithSourceInt *)decl)->position);
	}

	// Every time we see a comma, parse the next name, and add it to the list.
	while (Parser_NextToken(parser)->kind == TOKEN_COMMA) {

		error = Parser_ParseTillName(parser, &decl);
		if (error != NULL) return error;

		if (decl->kind != SMILE_KIND_NULL) {
			LIST_APPEND_WITH_SOURCE(head, tail, decl, ((struct SmileListWithSourceInt *)decl)->position);
		}
	}

	// Don't overconsume at the end.
	Lexer_Unget(parser->lexer);

	// Return the list of names.
	*names = head;
	return NULL;
}

// anyname ::= NAME
ParseError Parser_ParseTillName(Parser parser, SmileObject *expr)
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
		*expr = NullObject;
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("Missing flag name after 'till'."));
		return error;
	}

	// Declare it in the current scope.
	symbol = token->data.symbol;
	error = ParseScope_DeclareHere(parser->currentScope, symbol, PARSEDECL_TILL, Token_GetPosition(token), NULL);
	if (error != NULL)
		return error;

	*expr = (SmileObject)SmileSymbol_Create(symbol);
	return NULL;
}

// whens_opt ::= whens |
// whens ::= when whens | when
// when ::= WHEN name expr
ParseError Parser_ParseWhens(Parser parser, SmileObject *expr, Int32Int32Dict tillFlags, Int modeFlags)
{
	ParseError parseError;
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
		parseError = Parser_ParseExpr(parser, &whenBody, modeFlags);
		if (parseError != NULL) {
			Parser_AddMessage(parser, parseError);
		}

		// Construct the when clause itself:  [flag body]
		whenClause = (SmileObject)SmileList_CreateTwoWithSource(flagName ? (SmileObject)SmileSymbol_Create(flagName) : NullObject, whenBody, position);

		// Add it to the result list.
		LIST_APPEND_WITH_SOURCE(whenHead, whenTail, whenClause, position);
	}

	*expr = (SmileObject)whenHead;
	return NULL;
}

//-----------------------------------------------------------------------------
//  Lisp-style till:  [$till [x y z] body [[x when] [y when] [z when]]]

// till-when ::= . '[' name expr ']'
static ParseError Parser_ParseClassicTillWhen(Parser parser, SmileObject *result, ParseScope flagScope)
{
	LexerPosition position;
	Token token;
	SmileObject body;
	ParseError error;
	Symbol name;
	ParseDecl flag;

	// Consume the '['.
	token = Parser_NextToken(parser);
	position = Token_GetPosition(token);

	// It should be followed by a name.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME
		&& token->kind != TOKEN_PUNCTNAME && token->kind != TOKEN_UNKNOWNPUNCTNAME) {
		*result = NullObject;
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FromC("Missing flag name for when-clause in [$till] form."));
		return error;
	}
	name = token->data.symbol;

	// The name should be one of the declared flags.
	flag = ParseScope_FindDeclarationHere(flagScope, name);
	if (flag == NULL) {
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Unknown flag name \"{0}\" for when-clause in [$till] form.",
			SymbolTable_GetName(Smile_SymbolTable, name)));
		Parser_AddMessage(parser, error);
	}
	else {
		// The name should also not have been used already.
		if (flag->scopeIndex < 0) {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Flag name for \"{0}\" cannot be used for multiple when-clauses in the same [$till] form.",
				SymbolTable_GetName(Smile_SymbolTable, name)));
			Parser_AddMessage(parser, error);
		}
		flag->scopeIndex = -1;
	}

	// Now consume the expression that forms the body.
	error = Parser_ParseExpr(parser, &body, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (error != NULL) {
		*result = NullObject;
		return error;
	}
	if (body == Parser_IgnorableObject) body = NullObject;

	// Make sure there's a trailing ']' to end the when-clause.
	if ((error = Parser_ExpectRightBracket(parser, result, NULL, "$till when-clause", position)) != NULL)
		return error;

	*result =
		(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(name),
			(SmileObject)SmileList_ConsWithSource(body, NullObject, position),
		position);
	return NULL;
}

// till-whens-opt ::= . till_whens | .
// till-whens ::= . till_whens till-when | . till-when
static ParseError Parser_ParseClassicTillWhens(Parser parser, SmileList *result, ParseScope flagScope)
{
	SmileList head = NullList, tail = NullList;
	SmileObject when;
	ParseError error;
	LexerPosition position;

	while (Parser_HasLookahead(parser, TOKEN_LEFTBRACKET)) {
		position = Lexer_GetPosition(parser->lexer);

		if ((error = Parser_ParseClassicTillWhen(parser, &when, flagScope)) != NULL) {
			Parser_AddMessage(parser, error);
			Parser_Recover(parser, Parser_BracesBracketsParenthesesBar_Recovery, Parser_BracesBracketsParenthesesBar_Count);
			continue;
		}

		LIST_APPEND_WITH_SOURCE(head, tail, when, position);
	}

	*result = head;
	return NULL;
}

// till-flags ::= . names
static ParseError Parser_ParseClassicTillFlagNames(Parser parser, SmileList *result)
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
				*result = head;
				return NULL;
			
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
ParseError Parser_ParseClassicTill(Parser parser, SmileObject *result, LexerPosition startPosition)
{
	SmileObject body;
	SmileList flags;
	SmileList whens;
	SmileList temp;
	ParseError error;
	ParseDecl decl;
	SmileSymbol smileSymbol;
	Token token;

	// Make sure there is a '[' to start the name list.
	if ((error = Parser_ExpectLeftBracket(parser, result, NULL, "$till", startPosition)) != NULL)
		return error;

	Parser_BeginScope(parser, PARSESCOPE_TILLDO);

	// Parse the names.
	if ((error = Parser_ParseClassicTillFlagNames(parser, &flags)) != NULL) {
		Parser_AddMessage(parser, error);
		flags = NullList;
	}

	// Make sure there is a ']' to end the flags list.
	if ((error = Parser_ExpectRightBracket(parser, result, NULL, "$till flags", startPosition)) != NULL) {
		Parser_EndScope(parser, False);
		return error;
	}

	// The flags list cannot be empty.
	if (SMILE_KIND(flags) != SMILE_KIND_LIST) {
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition, String_FromC("A [$till] form must start with a list of flags."));
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
	if ((error = Parser_ParseExpr(parser, &body, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS)) != NULL) {
		Parser_AddMessage(parser, error);
		body = NullObject;
	}
	if (body == Parser_IgnorableObject) body = NullObject;

	// The scope for the flags is no longer valid after the body expression.
	Parser_EndScope(parser, False);

	// If there's a bracket, parse any 'when' declarations.
	if (!Parser_HasLookahead(parser, TOKEN_LEFTBRACKET))
		whens = NullList;
	else {
		token = Parser_NextToken(parser);

		// Parse any 'when' declarations.
		if ((error = Parser_ParseClassicTillWhens(parser, &whens, parser->currentScope)) != NULL) {
			Parser_AddMessage(parser, error);
			whens = NullList;
		}

		// Make sure there is a ']' to end the whens-list.
		if ((error = Parser_ExpectRightBracket(parser, result, NULL, "$till whens", startPosition)) != NULL)
			return error;
	}

	// Make sure there is a ']' to end the till.
	if ((error = Parser_ExpectRightBracket(parser, result, NULL, "$till", startPosition)) != NULL)
		return error;

	// Construct the resulting [$till flags body whens] form.
	if (SMILE_KIND(whens) != SMILE_KIND_NULL) {
		*result =
			(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__TILL),
				(SmileObject)SmileList_ConsWithSource((SmileObject)flags,
					(SmileObject)SmileList_ConsWithSource(body,
						(SmileObject)SmileList_ConsWithSource((SmileObject)whens, NullObject, startPosition),
					startPosition),
				startPosition),
			startPosition);
	}
	else {
		*result =
			(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__TILL),
				(SmileObject)SmileList_ConsWithSource((SmileObject)flags,
					(SmileObject)SmileList_ConsWithSource(body, NullObject, startPosition),
				startPosition),
			startPosition);
	}

	return NULL;
}
