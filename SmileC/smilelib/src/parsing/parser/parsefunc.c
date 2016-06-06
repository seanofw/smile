//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#include <smile/types.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static Int _barRecoveryTokens[] = {
	TOKEN_LEFTBRACE, TOKEN_LEFTBRACKET, TOKEN_LEFTPARENTHESIS, TOKEN_RIGHTBRACE, TOKEN_RIGHTBRACKET, TOKEN_RIGHTPARENTHESIS,
	TOKEN_BAR, TOKEN_SEMICOLON,
};

// func ::= BAR . params_opt BAR expr semi_opt
ParseError Parser_ParseFunc(Parser parser, SmileObject *expr, Int modeFlags)
{
	LexerPosition funcPosition;
	ParseScope parentScope, newScope;
	SmileList paramList;
	SmileObject body;
	ParseError parseError;
	Token token;

	funcPosition = Token_GetPosition(parser->lexer->token);

	parentScope = parser->currentScope;
	newScope = ParseScope_CreateChild(parentScope, PARSESCOPE_FUNCTION);
	parser->currentScope = newScope;

	parseError = Parser_ParseParamsOpt(parser, &paramList);
	if (parseError != NULL) {
		Parser_AddMessage(parser, parseError);
		token = Parser_Recover(parser, _barRecoveryTokens, sizeof(_barRecoveryTokens));
		if (token->kind != TOKEN_BAR) {
			*expr = NullObject;
			parser->currentScope = parentScope;
			return NULL;
		}
	}

	if (Lexer_Next(parser->lexer) != TOKEN_BAR) {
		Lexer_Unget(parser->lexer);
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_Format("Expected |...| to end function parameters starting on line %d", funcPosition->lineStart));
		*expr = NullObject;
		parser->currentScope = parentScope;
		return parseError;
	}

	parseError = Parser_ParseExpr(parser, &body, modeFlags);
	if (parseError != NULL) {
		*expr = NullObject;
		parser->currentScope = parentScope;
		return parseError;
	}

	// Allow an optional semicolon to terminate the function, which is very useful for short inline functions in long compound expressions.
	if (Lexer_Next(parser->lexer) != TOKEN_SEMICOLON) {
		Lexer_Unget(parser->lexer);
	}

	parser->currentScope = parentScope;

	*expr =
		(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects.fnSymbol,
			(SmileObject)SmileList_ConsWithSource((SmileObject)paramList,
				(SmileObject)SmileList_ConsWithSource(body, NullObject, funcPosition),
			funcPosition),
		funcPosition);

	return NULL;
}

// params_opt :: = . params | .
// params :: = . params param | . params ',' param | . param
ParseError Parser_ParseParamsOpt(Parser parser, SmileList *params)
{
	SmileList head = NullList, tail = NullList;
	SmileObject param;
	ParseError parseError = NULL, paramError;
	Int tokenKind;
	Bool isFirst;
	LexerPosition paramPosition;

	isFirst = True;
	for (;;) {

		tokenKind = Lexer_Next(parser->lexer);
		if (tokenKind == TOKEN_BAR) {
			// End of arguments.
			Lexer_Unget(parser->lexer);
			*params = head;
			return parseError;
		}
		if (tokenKind == TOKEN_COMMA) {
			if (isFirst) {
				if (parseError != NULL)
					Parser_AddMessage(parser, parseError);
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
					String_Format("Illegal comma found in function parameter list"));
				// Recover by simply absorbing the comma.
			}
		}
		else {
			Lexer_Unget(parser->lexer);
		}

		// Parse the next argument.
		paramError = Parser_ParseParam(parser, &param, &paramPosition);
		if (paramError != NULL) {
			if (parseError != NULL)
				Parser_AddMessage(parser, parseError);
			parseError = paramError;
			*params = head;
			return parseError;
		}

		LIST_APPEND_WITH_SOURCE(head, tail, param, paramPosition);

		isFirst = False;
	}
}

// param :: = . name | . param_type COLON name
ParseError Parser_ParseParam(Parser parser, SmileObject *param, LexerPosition *position)
{
	Int tokenKind, nextTokenKind;
	SmileObject type;
	ParseError parseError;
	LexerPosition paramPosition;
	Token token;
	Symbol symbol;
	ParseDecl decl;

	tokenKind = Lexer_Next(parser->lexer);
	token = parser->lexer->token;
	*position = paramPosition = Token_GetPosition(token);

	if (tokenKind == TOKEN_ALPHANAME || tokenKind == TOKEN_UNKNOWNALPHANAME
		|| tokenKind == TOKEN_PUNCTNAME || tokenKind == TOKEN_UNKNOWNPUNCTNAME) {

		nextTokenKind = Lexer_Next(parser->lexer);
		if (nextTokenKind != TOKEN_COLON && nextTokenKind != TOKEN_DOT) {
			Lexer_Unget(parser->lexer);

			// Just a plain function argument.
			symbol = token->data.symbol;
			ParseScope_DeclareHere(parser->currentScope, symbol, PARSEDECL_ARGUMENT, paramPosition, &decl);
			*param = (SmileObject)SmileSymbol_Create(symbol);
			return NULL;
		}
		else {

			// This looks like a type assertion, so parse it as a type form, and then get the name that follows it.
			Lexer_Unget(parser->lexer);
			Lexer_Unget(parser->lexer);

			parseError = Parser_ParseParamType(parser, &type);
			if (parseError != NULL) {
				*param = NullObject;
				return parseError;
			}

			tokenKind = Lexer_Next(parser->lexer);
			if (tokenKind != TOKEN_COLON) {
				*param = NullObject;
				return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
					String_Format("Expected a ':' after function argument type"));
			}

			tokenKind = Lexer_Next(parser->lexer);
			token = parser->lexer->token;
			paramPosition = Token_GetPosition(token);
			if (!(tokenKind == TOKEN_ALPHANAME || tokenKind == TOKEN_UNKNOWNALPHANAME
				|| tokenKind == TOKEN_PUNCTNAME || tokenKind == TOKEN_UNKNOWNPUNCTNAME)) {
				*param = NullObject;
				return ParseMessage_Create(PARSEMESSAGE_ERROR, paramPosition,
					String_Format("Invalid function argument name after function argument type"));
			}

			symbol = token->data.symbol;
			ParseScope_DeclareHere(parser->currentScope, symbol, PARSEDECL_ARGUMENT, paramPosition, &decl);

			*param =
				(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects.typeSymbol,
					(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(symbol),
						type, paramPosition
					), *position
				);
			return NULL;
		}
	}
	else {
		*param = NullObject;
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_Format("Invalid function argument name"));
	}
}

// param_type ::= . param_type_dot
// param_type_dot ::= . param_type_dot DOT param_type_term | . param_type_term
// param_type_term ::= . ALPHA_NAME | . PUNCT_NAME
ParseError Parser_ParseParamType(Parser parser, SmileObject *type)
{
	Token token;
	SmileSymbol propertySymbol;

	token = Parser_NextToken(parser);

	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_PUNCTNAME) {
		*type = NullObject;
		if (token->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME) {
			return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_Format("The function argument type name \"{0}\" does not exist.",
					SymbolTable_GetName(Smile_SymbolTable, token->data.symbol)));
		}
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			String_Format("Illegal function argument type name"));
	}

	*type = (SmileObject)SmileSymbol_Create(parser->lexer->token->data.symbol);

	while (Lexer_Next(parser->lexer) == TOKEN_DOT) {
		token = Parser_NextToken(parser);
		if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_PUNCTNAME
			&& token->kind != TOKEN_UNKNOWNALPHANAME && token->kind != TOKEN_UNKNOWNPUNCTNAME) {
			return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_Format("Illegal function argument type name"));
		}

		propertySymbol = SmileSymbol_Create(token->data.symbol);
		*type = (SmileObject)SmilePair_CreateWithSource(*type, (SmileObject)propertySymbol, Token_GetPosition(token));
	}

	Lexer_Unget(parser->lexer);
	return NULL;
}
