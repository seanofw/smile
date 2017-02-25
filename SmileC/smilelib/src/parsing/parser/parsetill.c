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
#include <smile/smiletypes/smilepair.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

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

	// The name should also not have been used already.
	if (flag->scopeIndex < 0) {
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Flag name for \"{0}\" cannot be used for multiple when-clauses in the same [$till] form.",
			SymbolTable_GetName(Smile_SymbolTable, name)));
		Parser_AddMessage(parser, error);
	}
	flag->scopeIndex = -1;

	// Now consume the expression that forms the body.
	error = Parser_ParseExpr(parser, &body, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (error != NULL) {
		*result = NullObject;
		return error;
	}

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
		Parser_EndScope(parser);
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
			ParseScope_DeclareHere(parser->currentScope, smileSymbol->symbol, PARSEDECL_TILL, SmileList_GetSourceLocation(temp), &decl);
		}
	}

	// Parse the body expression, whatever it may be.
	if ((error = Parser_ParseExpr(parser, &body, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS)) != NULL) {
		Parser_AddMessage(parser, error);
		body = NullObject;
	}

	// The scope for the flags is no longer valid after the body expression.
	Parser_EndScope(parser);

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
