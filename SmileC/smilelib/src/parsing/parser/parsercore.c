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

static void Parser_ParseExprsOpt(Parser parser, SmileList *head, SmileList *tail, Int binaryLineBreaks);
static ParseError Parser_ParseExpr(Parser parser, SmileObject *expr, Int binaryLineBreaks);
static ParseError Parser_ParseBaseExpr(Parser parser, SmileObject *expr, Int binaryLineBreaks);
static ParseError Parser_ParseTerm(Parser parser, SmileObject *expr, Int binaryLineBreaks, Token firstUnaryTokenForErrorReporting);

static Token Parser_Recover(Parser parser, Int *tokenKinds, Int numTokenKinds);
static Bool Parser_HasEqualLookahead(Parser parser);
static Bool Parser_HasEqualOrColonLookahead(Parser parser);
static Bool Parser_HasLookahead(Parser parser, Int tokenKind);
static Bool Parser_Has2Lookahead(Parser parser, Int tokenKind1, Int tokenKind2);

static Int Parser_BracesBracketsParenthesesBar_Recovery[] = {
	TOKEN_LEFTBRACE, TOKEN_LEFTBRACKET, TOKEN_LEFTPARENTHESIS,
	TOKEN_RIGHTBRACE, TOKEN_RIGHTBRACKET, TOKEN_RIGHTPARENTHESIS,
	TOKEN_BAR
};

static Int Parser_RightBracesBracketsParentheses_Recovery[] = {
	TOKEN_RIGHTBRACE, TOKEN_RIGHTBRACKET, TOKEN_RIGHTPARENTHESIS
};

//-------------------------------------------------------------------------------------------------
// Inline helper methods

/// <summary>
/// Determine if the given haystack of integers contains the given needle.
/// </summary>
/// <param name="needle">The integer to search for.</param>
/// <param name="haystack">The base pointer of the haystack that may or may not contain the
/// needle.</param>
/// <param name="count">The number of items in the haystack to test.</param>
/// <returns>True if the given needle can be found in the haystack; False if the haystack does
/// not contain the needle.</returns>
Inline Bool IntArrayContains(Int needle, Int *haystack, Int count)
{
	while (count--) {
		if (*haystack++ == needle)
			return True;
	}
	return False;
}

/// <summary>
/// Read the next token from the input stream.  If the token is an identifier, correctly map
/// it to its declaration (or lack thereof) in the current scope.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>The next token in the input stream.</returns>
Inline Token Parser_NextToken(Parser parser)
{
	Token token;
	
	Lexer_Next(parser->lexer);
	token = parser->lexer->token;

	if (token->kind == TOKEN_ALPHANAME) {
		token->kind = ParseScope_IsDeclared(parser->currentScope, token->data.symbol) ? TOKEN_ALPHANAME : TOKEN_UNKNOWNALPHANAME;
	}
	else if (token->kind == TOKEN_PUNCTNAME) {
		token->kind = ParseScope_IsDeclared(parser->currentScope, token->data.symbol) ? TOKEN_PUNCTNAME : TOKEN_UNKNOWNPUNCTNAME;
	}

	return token;
}

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
		Parser_AddError(parser, &token->position, "Unexpected content at end of file.");
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

		lexerPosition = &token->position;
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
			token = Parser_Recover(parser, Parser_BracesBracketsParenthesesBar_Recovery,
				sizeof(Parser_BracesBracketsParenthesesBar_Recovery) / sizeof(Int));

			// Reached a terminating '}' or ']' or ')', so presume we're done consuming expressions for now.
			if (token->kind == TOKEN_RIGHTBRACE || token->kind == TOKEN_RIGHTBRACKET || token->kind == TOKEN_RIGHTPARENTHESIS)
				return;

			expr = NullObject;
		}
	}

	Lexer_Unget(parser->lexer);
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
	Token startToken;
	ParseError error;

	UNUSED(binaryLineBreaks);

	switch (token->kind) {

		case TOKEN_LEFTPARENTHESIS:
			startToken = token;

			// Parse the inside of the '(...)' block as an expression, with binary line-breaks allowed.
			error = Parser_ParseExpr(parser, result, BINARYLINEBREAKS_ALLOWED);

			if (error != NULL) {
				// Handle any errors generated inside the expression parse by recovering here, and then
				// telling the caller everything was successful so that it continues trying the parse.
				Parser_AddMessage(parser, error);
				Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery,
					sizeof(Parser_RightBracesBracketsParentheses_Recovery) / sizeof(Int));
				*result = NullObject;
				return NULL;
			}

			// Make sure there's a matching ')' following the opening '('.
			if (!Parser_HasLookahead(parser, TOKEN_RIGHTPARENTHESIS)) {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR,
					&(firstUnaryTokenForErrorReporting != NULL ? firstUnaryTokenForErrorReporting : token)->position,
					String_Format("Missing ')' after expression starting on line %d.", startToken->position.line));
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
				&(firstUnaryTokenForErrorReporting != NULL ? firstUnaryTokenForErrorReporting : token)->position,
				String_Format("\"%S\" is not a known variable name", token->text));
			return error;

		default:
			// We got an unknown token that can't be turned into a term.  So we're going to generate
			// an error message, but we do our best to specialize that message according to the most
			// common mistakes people make.
			if (firstUnaryTokenForErrorReporting != NULL) {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR, &firstUnaryTokenForErrorReporting->position,
					String_Format("\"%S\" is not a known variable name", firstUnaryTokenForErrorReporting->text));
			}
			else if (token->kind == TOKEN_SEMICOLON) {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR, &token->position,
					String_FromC("Expected a variable or number or other legal expression term, not a semicolon (remember, semicolons don't terminate statements in Smile!)"));
			}
			else if (token->kind == TOKEN_COMMA) {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR, &token->position,
					String_FromC("Expected a variable or number or other legal expression term, not a comma (did you mistakenly put commas in a list?)"));
			}
			else if (token->kind == TOKEN_ERROR) {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR, &token->position, token->text);
			}
			else {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR, &token->position,
					String_Format("Expected a variable or number or other legal expression term, not \"%S\".", TokenKind_ToString(token->kind)));
			}
			return error;
	}
}

//-------------------------------------------------------------------------------------------------
// Helper methods

/// <summary>
/// When an error occurs, skip through the input until one of the given targets is found.
/// Does not consume the target token, and returns it.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <param name="tokenKinds">The tokens to search for.</param>
/// <param name="numTokenKinds">The number of tokens in the set of tokens to search for.</param>
static Token Parser_Recover(Parser parser, Int *tokenKinds, Int numTokenKinds)
{
	Token token;

	while ((token = Parser_NextToken(parser))->kind != TOKEN_EOI
		&& !IntArrayContains(token->kind, tokenKinds, numTokenKinds));

	Lexer_Unget(parser->lexer);

	return token;
}

/// <summary>
/// Determine if the next token in the input is one of the two forms of '=', without consuming the input.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>True if the next token is one of the two forms of '=', False if it's
// anything else or nonexistent.</returns>
static Bool Parser_HasEqualLookahead(Parser parser)
{
	Token token = Parser_NextToken(parser);
	Lexer_Unget(parser->lexer);
	return (token->kind == TOKEN_EQUAL || token->kind == TOKEN_EQUALWITHOUTWHITESPACE);
}

/// <summary>
/// Determine if the next token in the input is one of the two forms of '=' or a ':', without consuming the input.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>True if the next token is one of the two forms of '=' or a ':', False if it's
// anything else or nonexistent.</returns>
static Bool Parser_HasEqualOrColonLookahead(Parser parser)
{
	Token token = Parser_NextToken(parser);
	Lexer_Unget(parser->lexer);
	return (token->kind == TOKEN_EQUAL || token->kind == TOKEN_EQUALWITHOUTWHITESPACE || token->kind == TOKEN_COLON);
}

/// <summary>
/// Determine if the next token in the input is the given token kind, without consuming the input.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>True if the next token is the named token kind, False if it's anything else or nonexistent.</returns>
static Bool Parser_HasLookahead(Parser parser, Int tokenKind)
{
	Token token = Parser_NextToken(parser);
	Lexer_Unget(parser->lexer);
	return (token->kind == tokenKind);
}

/// <summary>
/// Determine if the next two token in the input are the given token kinds, without consuming the input.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>True if the next two tokens are the named token kinds, False if they're anything else or nonexistent.</returns>
static Bool Parser_Has2Lookahead(Parser parser, Int tokenKind1, Int tokenKind2)
{
	Token token1 = Parser_NextToken(parser);
	Token token2 = Parser_NextToken(parser);
	Lexer_Unget(parser->lexer);
	Lexer_Unget(parser->lexer);
	return (token1->kind == tokenKind1 && token2->kind == tokenKind2);
}
