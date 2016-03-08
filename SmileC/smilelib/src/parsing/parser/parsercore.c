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
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

//-------------------------------------------------------------------------------------------------
// Parser Construction

Parser Parser_Create(void)
{
	Parser parser = GC_MALLOC_STRUCT(struct ParserStruct);
	parser->lexer = NULL;
	parser->currentScope = NULL;
	LIST_INIT(parser->firstMessage, parser->lastMessage);
	return parser;
}

//-------------------------------------------------------------------------------------------------
// Root of the parse, and parsing nested scopes

// program ::= . exprs_opt
SmileList Parser_Parse(Parser parser, Lexer lexer, ParseScope scope)
{
	SmileList head, tail;
	ParseScope parentScope;
	Lexer oldLexer;
	Token token;
	
	LIST_INIT(head, tail);
	parentScope = parser->currentScope;
	oldLexer = parser->lexer;
	
	parser->currentScope = scope;
	parser->lexer = lexer;

	Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED);

	if ((token = Parser_NextToken(parser))->kind != TOKEN_EOI) {
		Parser_AddError(parser, Token_GetPosition(token), "Unexpected content at end of file.");
	}

	parser->currentScope = parentScope;
	parser->lexer = oldLexer;

	return head;
}

//  exprs_opt ::= . exprs | .
//  exprs ::= . expr | . exprs expr
static void Parser_ParseExprsOpt(Parser parser, SmileList *head, SmileList *tail, Int binaryLineBreaks)
{
	Token token;
	LexerPosition lexerPosition;
	SmileObject expr;
	ParseError error;

	// Consume expressions until the lookahead reaches a terminating '}' or ']' or ')'.
	while ((token = Parser_NextToken(parser))->kind != TOKEN_EOI
		&& token->kind != TOKEN_RIGHTBRACE && token->kind != TOKEN_RIGHTBRACKET && token->kind != TOKEN_RIGHTPARENTHESIS) {

		lexerPosition = Token_GetPosition(token);
		Lexer_Unget(parser->lexer);

		// Parse the next expression.
		error = Parser_ParseExpr(parser, &expr, binaryLineBreaks);
		if (error == NULL) {
			if (expr != NullObject) {

				// Add the successfully-parsed expression to the output (if there's something non-null to add).
				LIST_APPEND_WITH_SOURCE(*head, *tail, expr, lexerPosition);
			}
		}
		else {
			// Record the error message.
			Parser_AddMessage(parser, error);

			// If that expression was garbage, perform simple error-recovery by skipping to the
			// next '{' '}' '[' ']' '(' ')' or '|'.
			token = Parser_Recover(parser, Parser_BracesBracketsParenthesesBar_Recovery, Parser_BracesBracketsParenthesesBar_Count);

			// Reached a terminating '}' or ']' or ')', so presume we're done consuming expressions for now.
			if (token->kind == TOKEN_RIGHTBRACE || token->kind == TOKEN_RIGHTBRACKET || token->kind == TOKEN_RIGHTPARENTHESIS)
				return;

			expr = NullObject;
		}
	}

	Lexer_Unget(parser->lexer);
}

//-------------------------------------------------------------------------------------------------
// Includes and sub-parsing

ParseError Parser_ParseOneExpressionFromText(Parser parser, SmileObject *expr, String string, LexerPosition startPosition)
{
	Lexer oldLexer;
	ParseError parseError;
	Token token;

	oldLexer = parser->lexer;
	parser->lexer = Lexer_Create(string, 0, String_Length(string), startPosition->filename, startPosition->line, startPosition->column);

	parseError = Parser_ParseExpr(parser, expr, BINARYLINEBREAKS_DISALLOWED);
	if (parseError != NULL) {
		parser->lexer = oldLexer;
		*expr = NULL;
		return parseError;
	}

	if ((token = Parser_NextToken(parser))->kind != TOKEN_EOI) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			String_Format("Unexpected \"%S\" at end of dynamic string expression.", TokenKind_ToString(token->kind)));
		parser->lexer = oldLexer;
		*expr = NULL;
		return parseError;
	}

	parser->lexer = oldLexer;
	return NULL;
}

//-------------------------------------------------------------------------------------------------
// Base expression parsing

//  nonbreak_expr ::= . expr    // Explicitly in a nonbreak_expr, binary operators cannot be matched
//									if they are the first non-whitespace on a line.  This behavior is
//									disabled at the end of the nonbreak_expr, whenever any [], (), or {}
//									grouping is entered, or whenever we are parsing inside the first
//									expr/arith of an if_then or a do-while.
//
//  expr ::= . base_expr
static ParseError Parser_ParseExpr(Parser parser, SmileObject *expr, Int binaryLineBreaks)
{
	return Parser_ParseTerm(parser, expr, binaryLineBreaks, NULL);
}

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
//         | . REAL
static ParseError Parser_ParseTerm(Parser parser, SmileObject *result, Int binaryLineBreaks, Token firstUnaryTokenForErrorReporting)
{
	Token token = Parser_NextToken(parser);
	LexerPosition startPosition;
	ParseError error;

	UNUSED(binaryLineBreaks);

	switch (token->kind) {

		case TOKEN_LEFTPARENTHESIS:
			startPosition = Token_GetPosition(token);

			// Parse the inside of the '(...)' block as an expression, with binary line-breaks allowed.
			error = Parser_ParseExpr(parser, result, BINARYLINEBREAKS_ALLOWED);

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

		case TOKEN_ALPHANAME:
		case TOKEN_PUNCTNAME:
			*result = (SmileObject)SmileSymbol_Create(token->data.symbol);
			return NULL;

		case TOKEN_RAWSTRING:
			*result = (SmileObject)SmileString_Create(token->text);
			return NULL;

		case TOKEN_DYNSTRING:
			return Parser_ParseDynamicString(parser, result, binaryLineBreaks, token->text, Token_GetPosition(token));

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
