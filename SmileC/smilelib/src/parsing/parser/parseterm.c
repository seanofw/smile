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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

//-------------------------------------------------------------------------------------------------
// Terms

//  term ::= . LPAREN expr RPAREN
//         | . scope
//         | . func
//         | . LBRACKET exprs_opt RBRACKET
//         | . BACKTICK raw_list_term
//         | . BACKTICK LPAREN expr RPAREN
//         | . VAR_NAME
//         | . RAWSTRING
//         | . DYNSTRING
//         | . CHAR
//         | . INTEGER
//         | . FLOAT
//         | . REAL
ParseError Parser_ParseTerm(Parser parser, SmileObject *result, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	Token token = Parser_NextToken(parser);
	LexerPosition startPosition;
	ParseError error;

	switch (token->kind) {

	case TOKEN_LEFTPARENTHESIS:
		startPosition = Token_GetPosition(token);

		// Parse the inside of the '(...)' block as an expression, with binary line-breaks allowed.
		error = Parser_ParseExpr(parser, result, BINARYLINEBREAKS_ALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

		if (error != NULL) {
			// Handle any errors generated inside the expression parse by recovering here, and then
			// telling the caller everything was successful so that it continues trying the parse.
			Parser_AddMessage(parser, error);
			Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
			*result = NullObject;
			return NULL;
		}

		// Make sure there's a matching ')' following the opening '('.
		if (!Parser_HasLookahead(parser, TOKEN_RIGHTPARENTHESIS)) {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR,
				Token_GetPosition(firstUnaryTokenForErrorReporting != NULL ? firstUnaryTokenForErrorReporting : token),
				String_Format("Missing ')' after expression starting on line %d.", startPosition->line));
			*result = NullObject;
			return error;
		}
		Parser_NextToken(parser);

		// No errors, yay!
		return NULL;

	case TOKEN_LEFTBRACKET:
		{
			SmileList head = NullList, tail = NullList;

			startPosition = Token_GetPosition(token);

			Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

			if (!Parser_HasLookahead(parser, TOKEN_RIGHTBRACKET)) {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR,
					Token_GetPosition(firstUnaryTokenForErrorReporting != NULL ? firstUnaryTokenForErrorReporting : token),
					String_Format("Missing ']' in list starting on line %d.", startPosition->line));
				*result = NullObject;
				return error;
			}
			Parser_NextToken(parser);

			*result = (SmileObject)head;
			return NULL;
		}

	case TOKEN_BAR:
		error = Parser_ParseFunc(parser, result, modeFlags);
		return error;

	case TOKEN_BACKTICK:
		error = ParseMessage_Create(PARSEMESSAGE_ERROR,
			Token_GetPosition(firstUnaryTokenForErrorReporting != NULL ? firstUnaryTokenForErrorReporting : token),
			String_Format("Backtick is not yet supported.  Sorry!"));
		return error;

	case TOKEN_LEFTBRACE:
		Lexer_Unget(parser->lexer);
		error = Parser_ParseScope(parser, result);
		return error;

	case TOKEN_ALPHANAME:
	case TOKEN_PUNCTNAME:
		*result = (SmileObject)SmileSymbol_Create(token->data.symbol);
		return NULL;

	case TOKEN_RAWSTRING:
		*result = (SmileObject)SmileString_Create(token->text);
		return NULL;

	case TOKEN_DYNSTRING:
		return Parser_ParseDynamicString(parser, result, token->text, Token_GetPosition(token));

	case TOKEN_CHAR:
		*result = (SmileObject)SmileChar_Create((Byte)token->data.i);
		return NULL;

	case TOKEN_BYTE:
		*result = (SmileObject)SmileByte_Create((Byte)token->data.i);
		return NULL;

	case TOKEN_INTEGER16:
		*result = (SmileObject)SmileInteger16_Create((Int16)token->data.i);
		return NULL;

	case TOKEN_INTEGER32:
		*result = (SmileObject)SmileInteger32_Create(token->data.i);
		return NULL;

	case TOKEN_INTEGER64:
		*result = (SmileObject)SmileInteger64_Create(token->data.int64);
		return NULL;

	case TOKEN_UNKNOWNALPHANAME:
	case TOKEN_UNKNOWNPUNCTNAME:
		// If we get an operator name instead of a variable name, we can't use it as a term.
		error = ParseMessage_Create(PARSEMESSAGE_ERROR,
			Token_GetPosition(firstUnaryTokenForErrorReporting != NULL ? firstUnaryTokenForErrorReporting : token),
			String_Format("\"%S\" is not a known variable name", token->text));
		return error;

	case TOKEN_LOANWORD_SYNTAX:
		return Parser_ParseSyntax(parser, result, modeFlags);

	default:
		// We got an unknown token that can't be turned into a term.  So we're going to generate
		// an error message, but we do our best to specialize that message according to the most
		// common mistakes people make.
		if (firstUnaryTokenForErrorReporting != NULL) {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(firstUnaryTokenForErrorReporting),
				String_Format("\"%S\" is not a known variable name", firstUnaryTokenForErrorReporting->text));
		}
		else if (token->kind == TOKEN_SEMICOLON) {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_FromC("Expected a variable or number or other legal expression term, not a semicolon (remember, semicolons don't terminate statements in Smile!)"));
		}
		else if (token->kind == TOKEN_COMMA) {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_FromC("Expected a variable or number or other legal expression term, not a comma (did you mistakenly put commas in a list?)"));
		}
		else if (token->kind == TOKEN_ERROR) {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), token->text);
		}
		else {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_Format("Expected a variable or number or other legal expression term, not \"%S\".", TokenKind_ToString(token->kind)));
		}
		return error;
	}
}
