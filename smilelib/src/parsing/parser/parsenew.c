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
#include <smile/parsing/internal/parsescope.h>

// newexpr ::= . NEW LBRACE members_opt RBRACE
// 	| . NEW dotexpr LBRACE members_opt RBRACE
//	| . LBRACE members_opt RBRACE
// 	| . consexpr
ParseResult Parser_ParseNewExpr(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	Token token, newToken;
	SmileObject base, body, expr;
	ParseResult parseResult;
	LexerPosition newTokenPosition;

	// Peek at the next token.
	token = Parser_NextToken(parser);

	if (token->kind == TOKEN_LEFTBRACE) {
		// If we got here, that means it's a curly brace in an rvalue position, so we treat it
		// as a shorthand object instantiation, same as JavaScript does.
		newTokenPosition = Token_GetPosition(token);
		base = (SmileObject)Smile_KnownObjects.ObjectSymbol;
		goto shorthandForm;
	}
	else if (!((token->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == Smile_KnownSymbols.new_)) {
		// Didn't the keyword 'new', so just recurse deeper.
		Lexer_Unget(parser->lexer);
		return Parser_ParsePostfixExpr(parser, modeFlags, firstUnaryTokenForErrorReporting);
	}

	// Got the keyword 'new', so parse an object construction after it.
	newTokenPosition = Token_GetPosition(token);
	newToken = Token_Clone(token);

	// If we got a '{', then inherit from Object.  Otherwise, inherit from the next expression.
	if (Parser_HasLookahead(parser, TOKEN_LEFTBRACE)) {
		base = (SmileObject)Smile_KnownObjects.ObjectSymbol;
	}
	else {
		parseResult = Parser_ParseDotExpr(parser, modeFlags, newToken);
		if (IS_PARSE_ERROR(parseResult)) {
			HANDLE_PARSE_ERROR(parser, parseResult);
			token = Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
			if (token->kind != TOKEN_LEFTBRACE)
				return RECOVERY_RESULT();
			base = NullObject;
		}
		else base = parseResult.expr;
	}

	// If we didn't get a '{' at this point to start the object's members, that's an error.
	if (Lexer_Next(parser->lexer) != TOKEN_LEFTBRACE) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_FromC("Missing a '{' after 'new'.")));
	}

shorthandForm:
	// Collect the member-declarations in the object.
	parseResult = Parser_ParseMembers(parser);

	// Make sure there's a succeeding closing '}'.
	if (IS_PARSE_ERROR(parseResult) || Lexer_Peek(parser->lexer) != TOKEN_RIGHTBRACE) {
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		if (!IS_PARSE_ERROR(parseResult)) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Missing a '}' to end the members in the 'new' block starting on line %d.", newTokenPosition->line)));
		}
		return RECOVERY_RESULT();
	}
	else body = parseResult.expr;
	Lexer_Next(parser->lexer);

	// Finally, build the resulting object-construction.
	expr = (SmileObject)SmileList_ConsWithSource(
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

	return EXPR_RESULT(expr);
}

static Int Parser_RightBracesColons_Recovery[] = {
	TOKEN_RIGHTBRACE, TOKEN_COLON,
};
static Int Parser_RightBracesColons_Count = sizeof(Parser_RightBracesColons_Recovery) / sizeof(Int);

// members_opt :: = . members | .
// members :: = . members member | . member
// member :: = . name COLON orexpr
ParseResult Parser_ParseMembers(Parser parser)
{
	SmileList head = NullList, tail = NullList;
	Token token;
	ParseError parseError;
	ParseResult parseResult;
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
			if (Parser_Recover(parser, Parser_RightBracesColons_Recovery, Parser_RightBracesColons_Count)->kind != TOKEN_COLON) {
				Lexer_Unget(parser->lexer);
				return ERROR_RESULT(parseError);
			}
			else {
				Parser_AddMessage(parser, parseError);
			}
		}
	
		parseResult = Parser_ParseOrExpr(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERDECL);
		if (IS_PARSE_ERROR(parseResult)) {
			HANDLE_PARSE_ERROR(parser, parseResult);
			hasErrors = True;
		}
		valueExpr = parseResult.expr;

		memberExpr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)SmileSymbol_Create(symbol),
			(SmileObject)SmileList_ConsWithSource(valueExpr, NullObject, lexerPosition),
			lexerPosition
		);

		LIST_APPEND_WITH_SOURCE(head, tail, memberExpr, lexerPosition);
	}

	Lexer_Unget(parser->lexer);

	return hasErrors ? RECOVERY_RESULT() : EXPR_RESULT(head);
}

//-------------------------------------------------------------------------------------------------

// member ::= . '[' name expr ']'
static ParseResult Parser_ParseClassicNewMember(Parser parser)
{
	LexerPosition position;
	Token token;
	SmileObject body, expr;
	ParseResult parseResult;
	Symbol name;

	// Consume the '['.
	token = Parser_NextToken(parser);
	position = Token_GetPosition(token);

	// It should be followed by a name.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME
		&& token->kind != TOKEN_PUNCTNAME && token->kind != TOKEN_UNKNOWNPUNCTNAME) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, position,
			String_FromC("Missing name for member of [$new] form.")));
	}
	name = token->data.symbol;

	// Now consume the expression that forms the body.
	parseResult = Parser_ParseExpr(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;
	body = parseResult.expr;

	// Make sure there's a trailing ']' to end the member.
	parseResult = Parser_ExpectRightBracket(parser, NULL, "$new member", position);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;

	expr =
		(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(name),
			(SmileObject)SmileList_ConsWithSource(body, NullObject, position),
		position);
	return EXPR_RESULT(expr);
}

// member-list-opt ::= . member-list | .
// member-list ::= . member-list member | . member
static ParseResult Parser_ParseClassicNewMembers(Parser parser)
{
	SmileList head = NullList, tail = NullList;
	SmileObject member;
	ParseResult parseResult;
	LexerPosition position;

	while (Parser_HasLookahead(parser, TOKEN_LEFTBRACKET)) {
		position = Lexer_GetPosition(parser->lexer);

		parseResult = Parser_ParseClassicNewMember(parser);
		if (IS_PARSE_ERROR(parseResult)) {
			HANDLE_PARSE_ERROR(parser, parseResult);
			Parser_Recover(parser, Parser_BracesBracketsParenthesesBar_Recovery, Parser_BracesBracketsParenthesesBar_Count);
			continue;
		}
		else member = parseResult.expr;

		LIST_APPEND_WITH_SOURCE(head, tail, member, position);
	}

	return EXPR_RESULT(head);
}

// term ::= '[' '$new' . expr '[' member-list-opt ']' ']'
ParseResult Parser_ParseClassicNew(Parser parser, LexerPosition startPosition)
{
	SmileObject base, expr;
	SmileList members;
	ParseResult parseResult;

	// Parse the base expression, whatever it may be.
	parseResult = Parser_ParseExpr(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (IS_PARSE_ERROR(parseResult)) {
		HANDLE_PARSE_ERROR(parser, parseResult);
		base = NullObject;
	}
	else base = parseResult.expr;

	// Make sure there is a '[' to start the member list.
	parseResult = Parser_ExpectLeftBracket(parser, NULL, "$new member list", startPosition);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;

	// Parse the members, however many there may be.
	parseResult = Parser_ParseClassicNewMembers(parser);
	if (IS_PARSE_ERROR(parseResult)) {
		HANDLE_PARSE_ERROR(parser, parseResult);
		members = NullList;
	}
	else members = (SmileList)parseResult.expr;

	// Make sure there is a ']' to end the member list.
	parseResult = Parser_ExpectRightBracket(parser, NULL, "$new member list", startPosition);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;

	// Construct the resulting [$new base members] form.
	expr =
		(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__NEW),
			(SmileObject)SmileList_ConsWithSource(base,
				(SmileObject)SmileList_ConsWithSource((SmileObject)members, NullObject, startPosition),
			startPosition),
		startPosition);
	return EXPR_RESULT(expr);
}
