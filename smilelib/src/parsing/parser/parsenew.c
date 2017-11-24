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
#include <smile/parsing/internal/parsescope.h>

// newexpr ::=	  . NEW LBRACE members_opt RBRACE
// 	| . NEW dotexpr LBRACE members_opt RBRACE
//	| . LBRACE members_opt RBRACE
// 	| . consexpr
ParseError Parser_ParseNewExpr(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	Token token, newToken;
	SmileObject base, body;
	ParseError parseError;
	LexerPosition newTokenPosition;
	Bool membersHaveErrors;

	token = Parser_NextToken(parser);
	if (token->kind == TOKEN_LEFTBRACE) {
		// If we got here, that means it's a curly brace in an rvalue position, so we treat it
		// as a shorthand object instantiation, same as JavaScript does.
		newTokenPosition = Token_GetPosition(token);
		base = (SmileObject)Smile_KnownObjects.ObjectSymbol;
		goto shorthandForm;
	}

	if (!((token->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == Smile_KnownSymbols.new_)) {
		Lexer_Unget(parser->lexer);
		return Parser_ParsePostfixExpr(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
	}

	newTokenPosition = Token_GetPosition(token);

	newToken = Token_Clone(token);

	if (Parser_HasLookahead(parser, TOKEN_LEFTBRACE)) {
		base = (SmileObject)Smile_KnownObjects.ObjectSymbol;
	}
	else {
		parseError = Parser_ParseDotExpr(parser, &base, modeFlags, newToken);
		if (parseError != NULL) {
			token = Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
			if (token->kind != TOKEN_LEFTBRACE) {
				*expr = NullObject;
				return parseError;
			}
			Parser_AddMessage(parser, parseError);
		}
	}

	if (Lexer_Next(parser->lexer) != TOKEN_LEFTBRACE) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_FromC("Missing a '{' after 'new'."));
		*expr = NullObject;
		return parseError;
	}

shorthandForm:
	membersHaveErrors = Parser_ParseMembers(parser, &body);
	if (membersHaveErrors || Lexer_Peek(parser->lexer) != TOKEN_RIGHTBRACE) {
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		if (!membersHaveErrors) {
			parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Missing a '}' to end the members in the 'new' block starting on line %d.", newTokenPosition->line));
			*expr = NullObject;
			return parseError;
		}
		return NULL;
	}

	// Reached a curly brace, so consume it.
	Lexer_Next(parser->lexer);

	*expr = (SmileObject)SmileList_ConsWithSource(
		(SmileObject)Smile_KnownObjects._newSymbol,
		(SmileObject)SmileList_ConsWithSource(
			base,
			(SmileObject)SmileList_ConsWithSource(
				body,
				NullObject,
				newTokenPosition
			),
			newTokenPosition
		),
		newTokenPosition
	);

	return NULL;
}

static Int Parser_RightBracesColons_Recovery[] = {
	TOKEN_RIGHTBRACE, TOKEN_COLON,
};
static Int Parser_RightBracesColons_Count = sizeof(Parser_RightBracesColons_Recovery) / sizeof(Int);

// members_opt :: = . members | .
// members :: = . members member | . member
// member :: = . name COLON orexpr
Bool Parser_ParseMembers(Parser parser, SmileObject *expr)
{
	SmileList head = NullList, tail = NullList;
	Token token;
	ParseError parseError;
	Symbol symbol;
	SmileObject valueExpr;
	SmileObject memberExpr;
	LexerPosition lexerPosition;
	Bool hasErrors = False;

	while ((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME
		|| token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_UNKNOWNPUNCTNAME) {
		symbol = token->data.symbol;
		lexerPosition = Token_GetPosition(token);

		if (Lexer_Next(parser->lexer) != TOKEN_COLON) {
			parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Missing ':' after '%S' member.", SymbolTable_GetName(Smile_SymbolTable, symbol)));
			Parser_AddMessage(parser, parseError);
			if (Parser_Recover(parser, Parser_RightBracesColons_Recovery, Parser_RightBracesColons_Count)->kind != TOKEN_COLON) {
				Lexer_Unget(parser->lexer);
				*expr = NullObject;
				return False;
			}
		}
	
		parseError = Parser_ParseOrExpr(parser, &valueExpr, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERDECL);
		if (parseError != NULL) {
			Parser_AddMessage(parser, parseError);
			hasErrors = True;
		}

		memberExpr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)SmileSymbol_Create(symbol),
			(SmileObject)SmileList_ConsWithSource(valueExpr, NullObject, lexerPosition),
			lexerPosition
		);

		LIST_APPEND_WITH_SOURCE(head, tail, memberExpr, lexerPosition);
	}

	Lexer_Unget(parser->lexer);

	*expr = (SmileObject)head;
	return hasErrors;
}

//-------------------------------------------------------------------------------------------------

// member ::= . '[' name expr ']'
static ParseError Parser_ParseClassicNewMember(Parser parser, SmileObject *result)
{
	LexerPosition position;
	Token token;
	SmileObject body;
	ParseError error;
	Symbol name;

	// Consume the '['.
	token = Parser_NextToken(parser);
	position = Token_GetPosition(token);

	// It should be followed by a name.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME
		&& token->kind != TOKEN_PUNCTNAME && token->kind != TOKEN_UNKNOWNPUNCTNAME) {
		*result = NullObject;
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FromC("Missing name for member of [$new] form."));
		return error;
	}
	name = token->data.symbol;

	// Now consume the expression that forms the body.
	error = Parser_ParseExpr(parser, &body, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (error != NULL) {
		*result = NullObject;
		return error;
	}
	if (body == Parser_IgnorableObject) body = NullObject;

	// Make sure there's a trailing ']' to end the member.
	if ((error = Parser_ExpectRightBracket(parser, result, NULL, "$new member", position)) != NULL)
		return error;

	*result =
		(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(name),
			(SmileObject)SmileList_ConsWithSource(body, NullObject, position),
		position);
	return NULL;
}

// member-list-opt ::= . member-list | .
// member-list ::= . member-list member | . member
static ParseError Parser_ParseClassicNewMembers(Parser parser, SmileList *result)
{
	SmileList head = NullList, tail = NullList;
	SmileObject member;
	ParseError error;
	LexerPosition position;

	while (Parser_HasLookahead(parser, TOKEN_LEFTBRACKET)) {
		position = Lexer_GetPosition(parser->lexer);

		if ((error = Parser_ParseClassicNewMember(parser, &member)) != NULL) {
			Parser_AddMessage(parser, error);
			Parser_Recover(parser, Parser_BracesBracketsParenthesesBar_Recovery, Parser_BracesBracketsParenthesesBar_Count);
			continue;
		}

		LIST_APPEND_WITH_SOURCE(head, tail, member, position);
	}

	*result = head;
	return NULL;
}

// term ::= '[' '$new' . expr '[' member-list-opt ']' ']'
ParseError Parser_ParseClassicNew(Parser parser, SmileObject *result, LexerPosition startPosition)
{
	SmileObject base;
	SmileList members;
	ParseError error;

	// Parse the base expression, whatever it may be.
	if ((error = Parser_ParseExpr(parser, &base, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS)) != NULL) {
		Parser_AddMessage(parser, error);
		base = NullObject;
	}
	if (base == Parser_IgnorableObject) base = NullObject;

	// Make sure there is a '[' to start the member list.
	if ((error = Parser_ExpectLeftBracket(parser, result, NULL, "$new member list", startPosition)) != NULL)
		return error;

	// Parse the members, however many there may be.
	if ((error = Parser_ParseClassicNewMembers(parser, &members)) != NULL) {
		Parser_AddMessage(parser, error);
		members = NullList;
	}

	// Make sure there is a ']' to end the member list.
	if ((error = Parser_ExpectRightBracket(parser, result, NULL, "$new member list", startPosition)) != NULL)
		return error;

	// Construct the resulting [$new base members] form.
	*result =
		(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__NEW),
			(SmileObject)SmileList_ConsWithSource(base,
				(SmileObject)SmileList_ConsWithSource((SmileObject)members, NullObject, startPosition),
			startPosition),
		startPosition);
	return NULL;
}
