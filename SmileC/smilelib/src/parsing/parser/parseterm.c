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
		Lexer_Unget(parser->lexer);
		return Parser_ParseParentheses(parser, result, modeFlags);

	case TOKEN_LEFTBRACKET:
		{
			SmileList head = NullList, tail = NullList;

			startPosition = Token_GetPosition(token);

			Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

			if (!Parser_HasLookahead(parser, TOKEN_RIGHTBRACKET)) {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR,
					firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : startPosition,
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
		{
			startPosition = Token_GetPosition(token);

			if (Lexer_Peek(parser->lexer) == TOKEN_LEFTPARENTHESIS) {
				// This is a quote of a parenthesized expression, so parse the expression and then quote it.
				error = Parser_ParseParentheses(parser, result, modeFlags);
				if (error != NULL)
					return error;
				*result = (SmileObject)SmileList_ConsWithSource(
					(SmileObject)Smile_KnownObjects._quoteSymbol,
					(SmileObject)SmileList_ConsWithSource(*result, NullObject, startPosition),
					startPosition
				);
				return NULL;
			}
			else {
				// This is a quote of a more generic thing, like a list or a symbol, so recursively parse
				// this "quoted term".  Because the "quoted term" might be a list that somewhere contains
				// a (parenthetical escape), thus turning the "quoted term" from an ordinary quoted list
				// into a template, we do not do the quoting here, but instead do that quoting work inside
				// Parser_ParseQuotedTerm() itself, which is the only code that knows how to do it correctly.
				return Parser_ParseQuotedTerm(parser, result, modeFlags, startPosition);
			}
		}

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
			firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : Token_GetPosition(token),
			String_Format("\"%S\" is not a known variable name", token->text));
		return error;

	case TOKEN_LOANWORD_SYNTAX:
		// Parse the new syntax rule.
		error = Parser_ParseSyntax(parser, result, modeFlags);
		if (error != NULL)
			return error;
	
		// Add the syntax rule to the table of syntax rules for the current scope.
		if (!ParserSyntaxTable_AddRule(parser, &parser->currentScope->syntaxTable, (SmileSyntax)*result)) {
			*result = NullObject;
		}
		return NULL;

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
