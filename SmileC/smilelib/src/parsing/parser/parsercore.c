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
	parser->customFollowSet = NULL;
	LIST_INIT(parser->firstMessage, parser->lastMessage);
	return parser;
}

//-------------------------------------------------------------------------------------------------
// Root of the parse, and parsing nested scopes

/// <summary>
/// Helper function:  Parse the given C string as a source file.
/// </summary>
/// <param name="parser">The parser that is doing the parsing of the source text.</param>
/// <param name="scope">The scope in which the parsing is to take place.</param>
/// <param name="text">The nul-terminated string to parse, which is assumed to come from a file named "".</param>
/// <returns>The results of the parse (if errors are generated, they will be added to the parser's message collection).</returns>
SmileObject Parser_ParseFromC(Parser parser, ParseScope scope, const char *text)
{
	return Parser_ParseString(parser, scope, String_FromC(text));
}

/// <summary>
/// Helper function:  Parse the given string as a source file.
/// </summary>
/// <param name="parser">The parser that is doing the parsing of the source text.</param>
/// <param name="scope">The scope in which the parsing is to take place.</param>
/// <param name="text">The string to parse, which is assumed to come from a file named "".</param>
/// <returns>The results of the parse (if errors are generated, they will be added to the parser's message collection).</returns>
SmileObject Parser_ParseString(Parser parser, ParseScope scope, String text)
{
	Lexer lexer;
	SmileObject result;

	lexer = Lexer_Create(text, 0, String_Length(text), String_Empty, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;
	result = Parser_Parse(parser, lexer, scope);

	return result;
}

/// <summary>
/// Parse the input from the given lexical analyzer, consuming all tokens until it reaches EOI.
/// This is the root, core parsing function.
/// </summary>
/// <param name="parser">The parser that is doing the parsing of the source text.</param>
/// <param name="lexer">The lexical analyzer that will provide both the source tokens and their file locations.</param>
/// <param name="scope">The scope in which the parsing is to take place.</param>
/// <returns>A List that contains the expressions generated by the parse.  If errors are generated, those errors will be
/// added to the parser's message collection, and the return object will always be the Null list.</returns>
/// <remarks>
/// program ::= . exprs_opt
/// </remarks>
SmileObject Parser_Parse(Parser parser, Lexer lexer, ParseScope scope)
{
	ParseScope parentScope;
	Lexer oldLexer;
	SmileObject result;

	parentScope = parser->currentScope;
	oldLexer = parser->lexer;
	
	parser->currentScope = scope;
	parser->lexer = lexer;

	result = Parser_ParseScopeBody(parser);

	parser->currentScope = parentScope;
	parser->lexer = oldLexer;

	return result;
}

/// <summary>
/// Parse the input from the given lexical analyzer as a single constant raw term.  This leaves the lexer
/// sitting at the next content after the raw term, if any.
/// </summary>
/// <param name="parser">The parser that is doing the parsing of the source text.</param>
/// <param name="lexer">The lexical analyzer that will provide both the source tokens and their file locations.</param>
/// <param name="scope">The scope in which the parsing is to take place.</param>
/// <returns>The single item resulting from the parse.  If errors are generated, those errors will be
/// added to the parser's message collection, and the return object will always be the Null list.</returns>
/// <remarks>
/// program ::= . raw_list_term
/// </remarks>
SmileObject Parser_ParseConstant(Parser parser, Lexer lexer, ParseScope scope)
{
	ParseScope parentScope;
	Lexer oldLexer;
	SmileObject result;
	Bool isTemplate;
	ParseError parseError;

	parentScope = parser->currentScope;
	oldLexer = parser->lexer;

	parser->currentScope = scope;
	parser->lexer = lexer;

	parseError = Parser_ParseRawListTerm(parser, &result, &isTemplate, 0);

	if (parseError != NULL) {
		Parser_AddMessage(parser, parseError);
		result = NullObject;
	}
	else if (isTemplate) {
		Parser_AddError(parser, Token_GetPosition(parser->lexer->token), "Template and variable data is not allowed in constant values.");
		result = NullObject;
	}

	parser->currentScope = parentScope;
	parser->lexer = oldLexer;

	return result;
}

STATIC_STRING(ExpectedOpenBraceError, "Expected { ... to begin a scope");
STATIC_STRING(ExpectedCloseBraceError, "Expected ... } to end the scope");

//  scope ::= . LBRACE exprs_opt RBRACE
ParseError Parser_ParseScope(Parser parser, SmileObject *expr)
{
	ParseError parseError;
	Token token;

	if ((token = Parser_NextToken(parser))->kind != TOKEN_LEFTBRACE) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), ExpectedOpenBraceError);
		return parseError;
	}

	*expr = Parser_ParseScopeBody(parser);

	if ((token = Parser_NextToken(parser))->kind != TOKEN_RIGHTBRACE) {
		*expr = NULL;
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), ExpectedCloseBraceError);
		return parseError;
	}

	return NULL;
}

//  scope ::= LBRACE . exprs_opt RBRACE
SmileObject Parser_ParseScopeBody(Parser parser)
{
	LexerPosition startPosition;
	Int i;
	SmileList head, tail;
	SmileList declHead, declTail;
	ParseDecl *decls;
	Int numDecls;

	startPosition = Lexer_GetPosition(parser->lexer);

	Parser_BeginScope(parser, PARSESCOPE_SCOPEDECL);

	LIST_INIT(head, tail);
	Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

	decls = parser->currentScope->decls;
	numDecls = parser->currentScope->numDecls;

	Parser_EndScope(parser);

	if (numDecls == 0) {
		if (SMILE_KIND(head) == SMILE_KIND_NULL) {
			// Empty body, so we got nothin'.
			return NullObject;
		}
		else if (head == tail) {
			// This parsed to a single expression, so no need for a [$progn] to wrap it.
			return head->a;
		}
		else {
			// Multiple expressions, so wrap a [$progn] around it.
			return (SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._prognSymbol, (SmileObject)head, startPosition);
		}
	}
	else {
		// We have declarations, so we need to wrap whatever we got in a [$scope].
		LIST_INIT(declHead, declTail);
		for (i = 0; i < numDecls; i++) {
			LIST_APPEND(declHead, declTail, SmileSymbol_Create(decls[i]->symbol));
		}
		return (SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._scopeSymbol,
			(SmileObject)SmileList_ConsWithSource((SmileObject)declHead, (SmileObject)head, startPosition), startPosition);
	}
}

//  exprs_opt ::= . exprs | .
//  exprs ::= . expr | . exprs expr
void Parser_ParseExprsOpt(Parser parser, SmileList *head, SmileList *tail, Int modeFlags)
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
		error = Parser_ParseExpr(parser, &expr, modeFlags);
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

	parseError = Parser_ParseExpr(parser, expr, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
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
