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

	Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

	if ((token = Parser_NextToken(parser))->kind != TOKEN_EOI) {
		Parser_AddError(parser, Token_GetPosition(token), "Unexpected content at end of file.");
	}

	parser->currentScope = parentScope;
	parser->lexer = oldLexer;

	return head;
}

STATIC_STRING(ExpectedOpenBraceError, "Expected { ... to begin a scope");
STATIC_STRING(ExpectedCloseBraceError, "Expected ... } to end the scope");

//  scope ::= . LBRACE exprs_opt RBRACE
ParseError Parser_ParseScope(Parser parser, SmileObject *expr)
{
	ParseError parseError;
	Token token;
	LexerPosition startPosition;
	int i, numVariables;
	Symbol *symbolNames;
	SmileList head, tail;
	SmileList declHead, declTail;
	ClosureInfo closureInfo;

	if ((token = Parser_NextToken(parser))->kind != TOKEN_LEFTBRACE) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), ExpectedOpenBraceError);
		return parseError;
	}
	startPosition = Token_GetPosition(token);

	Parser_BeginScope(parser, PARSESCOPE_SCOPEDECL);

	LIST_INIT(head, tail);
	Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

	closureInfo = parser->currentScope->closure->closureInfo;
	Parser_EndScope(parser);

	if ((token = Parser_NextToken(parser))->kind != TOKEN_RIGHTBRACE) {
		*expr = NULL;
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), ExpectedCloseBraceError);
		return parseError;
	}

	if (closureInfo->numVariables == 0) {
		*expr = (SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects.prognSymbol, (SmileObject)head, startPosition);
		return NULL;
	}
	else {
		numVariables = closureInfo->numVariables;
		symbolNames = (Symbol *)Int32Int32Dict_GetKeys(closureInfo->symbolDictionary);
		LIST_INIT(declHead, declTail);
		for (i = 0; i < numVariables; i++) {
			LIST_APPEND(declHead, declTail, SmileSymbol_Create(symbolNames[i]));
		}
		*expr = (SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects.scopeSymbol,
			(SmileObject)SmileList_ConsWithSource((SmileObject)declHead, (SmileObject)head, startPosition), startPosition);
		return NULL;
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
