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

static void Parser_TransformListIntoTemplate(SmileList *head, SmileList *tail, LexerPosition lastReadPosition);

// raw_list_term :: = . LBRACKET raw_list_items_opt RBRACKET | . any_name
//    | . CHAR | . RAWSTRING
//    | . BYTE | . INT16 | . INT32 | . INT64 | . REAL32 | . REAL64 | . REAL128 | . FLOAT32 | . FLOAT64
//    | . BACKTICK raw_list_term
//    | . nonraw_term
// nonraw_term :: = . LPAREN expr RPAREN
//    | . scope
//    | . DYNSTRING
ParseError Parser_ParseQuotedTerm(Parser parser, SmileObject *result, Int modeFlags, LexerPosition position)
{
	ParseError parseError;
	Bool isTemplate;

	isTemplate = False;
	parseError = Parser_ParseRawListTerm(parser, result, &isTemplate, modeFlags);
	if (parseError != NULL)
		return parseError;

	if (!isTemplate) {
		*result = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)Smile_KnownObjects._quoteSymbol,
			(SmileObject)SmileList_ConsWithSource(*result, NullObject, position),
			position
		);
	}

	return NULL;
}

// raw_list_term :: = . LBRACKET raw_list_items_opt RBRACKET | . any_name
//    | . CHAR | . RAWSTRING
//    | . BYTE | . INT16 | . INT32 | . INT64 | . REAL32 | . REAL64 | . REAL128 | . FLOAT32 | . FLOAT64
//    | . BACKTICK raw_list_term
//    | . nonraw_term
// nonraw_term :: = . LPAREN expr RPAREN
//    | . scope
//    | . DYNSTRING
ParseError Parser_ParseRawListTerm(Parser parser, SmileObject *result, Bool *isTemplate, Int modeFlags)
{
	Token token = Parser_NextToken(parser);
	LexerPosition startPosition;
	ParseError error;

	switch (token->kind) {

	case TOKEN_LEFTPARENTHESIS:
		Lexer_Unget(parser->lexer);
		*isTemplate = True;
		return Parser_ParseParentheses(parser, result, modeFlags);

	case TOKEN_LEFTBRACE:
		Lexer_Unget(parser->lexer);
		*isTemplate = True;
		error = Parser_ParseScope(parser, result);
		return error;

	case TOKEN_DYNSTRING:
		error = Parser_ParseDynamicString(parser, result, token->text, Token_GetPosition(token));
		if (error != NULL)
			return error;
		*isTemplate = (SMILE_KIND(*result) != SMILE_KIND_STRING);
		return error;

	case TOKEN_LEFTBRACKET:
		{
			SmileList head = NullList, tail = NullList;

			startPosition = Token_GetPosition(token);

			Parser_ParseRawListItemsOpt(parser, &head, &tail, isTemplate, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

			if (!Parser_HasLookahead(parser, TOKEN_RIGHTBRACKET)) {
				error = ParseMessage_Create(PARSEMESSAGE_ERROR,
					startPosition, String_Format("Missing ']' in raw list starting on line %d.", startPosition->line));
				*result = NullObject;
				return error;
			}
			Parser_NextToken(parser);

			*result = (SmileObject)head;
			return NULL;
		}

	case TOKEN_BACKTICK:
		{
			Bool temp;
			startPosition = Token_GetPosition(token);
			error = Parser_ParseRawListTerm(parser, result, &temp, modeFlags);
			if (error != NULL)
				return error;
			*result = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)Smile_KnownObjects._quoteSymbol,
				(SmileObject)SmileList_ConsWithSource(
					*result,
					NullObject,
					startPosition
				),
				startPosition
			);
			*isTemplate = False;
			return NULL;
		}

	case TOKEN_ALPHANAME:
	case TOKEN_PUNCTNAME:
	case TOKEN_UNKNOWNALPHANAME:
	case TOKEN_UNKNOWNPUNCTNAME:
		*result = (SmileObject)SmileSymbol_Create(token->data.symbol);
		*isTemplate = False;
		return NULL;

	case TOKEN_RAWSTRING:
		*result = (SmileObject)SmileString_Create(token->text);
		*isTemplate = False;
		return NULL;

	case TOKEN_CHAR:
		*result = (SmileObject)SmileChar_Create(token->data.byte);
		*isTemplate = False;
		return NULL;

	case TOKEN_BYTE:
		*result = (SmileObject)SmileByte_Create(token->data.byte);
		*isTemplate = False;
		return NULL;

	case TOKEN_INTEGER16:
		*result = (SmileObject)SmileInteger16_Create(token->data.int16);
		*isTemplate = False;
		return NULL;

	case TOKEN_INTEGER32:
		*result = (SmileObject)SmileInteger32_Create(token->data.int32);
		*isTemplate = False;
		return NULL;

	case TOKEN_INTEGER64:
		*result = (SmileObject)SmileInteger64_Create(token->data.int64);
		*isTemplate = False;
		return NULL;

	default:
		// We got an unknown token that can't be turned into a term.  So we're going to generate
		// an error message, but we do our best to specialize that message according to the most
		// common mistakes people make.
		if (token->kind == TOKEN_SEMICOLON) {
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

// raw_list_items_opt :: = . raw_list_items | .
// raw_list_items :: = . raw_list_items raw_list_item | . raw_list_item
// raw_list_item :: = raw_list_dotexpr
ParseError Parser_ParseRawListItemsOpt(Parser parser, SmileList *head, SmileList *tail, Bool *isTemplate, Int modeFlags)
{
	Token token;
	LexerPosition lexerPosition, startPosition;
	SmileObject expr;
	ParseError error;
	Bool itemTriggersTemplateMode;
	Bool listIsTemplate;

	listIsTemplate = False;
	startPosition = NULL;

	// Consume expressions until the lookahead reaches a terminating '}' or ']' or ')'.
	while ((token = Parser_NextToken(parser))->kind != TOKEN_EOI
		&& token->kind != TOKEN_RIGHTBRACE && token->kind != TOKEN_RIGHTBRACKET && token->kind != TOKEN_RIGHTPARENTHESIS) {

		lexerPosition = Token_GetPosition(token);
		Lexer_Unget(parser->lexer);

		if (startPosition == NULL) startPosition = lexerPosition;

		// Parse the next expression.
		itemTriggersTemplateMode = False;
		error = Parser_ParseRawListDotExpr(parser, &expr, &itemTriggersTemplateMode, modeFlags);
		if (error == NULL) {
			if (expr != NullObject) {

				if (itemTriggersTemplateMode && !listIsTemplate) {
					// Uh oh.  The list item is a template form, but this list (so far) is not
					// yet a template form.  So transform the list into a template form, because
					// we can't append template items to a non-template list.
					Parser_TransformListIntoTemplate(head, tail, startPosition);
					listIsTemplate = True;
				}

				if (listIsTemplate && !itemTriggersTemplateMode) {
					// This is a templated list, but not a templated item.  So we need to quote it
					// before adding it to the list.
					expr = (SmileObject)SmileList_ConsWithSource(
						(SmileObject)Smile_KnownObjects._quoteSymbol,
						(SmileObject)SmileList_ConsWithSource(expr, NullObject, lexerPosition),
						lexerPosition
					);
				}

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
				return NULL;

			expr = NullObject;
		}
	}

	*isTemplate = listIsTemplate;

	Lexer_Unget(parser->lexer);
	return NULL;
}

// raw_list_dotexpr :: = . raw_list_dotexpr DOT any_name | . raw_list_term
ParseError Parser_ParseRawListDotExpr(Parser parser, SmileObject *result, Bool *isTemplate, Int modeFlags)
{
	ParseError parseError;
	Int tokenKind;
	LexerPosition lexerPosition;
	Symbol symbol;

	parseError = Parser_ParseRawListTerm(parser, result, isTemplate, modeFlags);
	if (parseError != NULL)
		return parseError;

	while (Parser_NextToken(parser)->kind == TOKEN_DOT) {

		if ((tokenKind = Lexer_Next(parser->lexer)) == TOKEN_ALPHANAME
			|| tokenKind == TOKEN_UNKNOWNALPHANAME
			|| tokenKind == TOKEN_PUNCTNAME
			|| tokenKind == TOKEN_UNKNOWNPUNCTNAME) {

			symbol = parser->lexer->token->data.symbol;
			lexerPosition = Token_GetPosition(parser->lexer->token);

			if (!*isTemplate) {
				// Generate a simple (expr).symbol as output.
				*result = (SmileObject)SmilePair_CreateWithSource(*result, (SmileObject)SmileSymbol_Create(symbol), lexerPosition);
			}
			else {
				// If this is a template, then generate [Pair.of (expr) symbol] as output.
				*result = (SmileObject)SmileList_ConsWithSource(
					(SmileObject)SmilePair_CreateWithSource(
						(SmileObject)Smile_KnownObjects.PairSymbol,
						(SmileObject)Smile_KnownObjects.ofSymbol,
						lexerPosition
					),
					(SmileObject)SmileList_ConsWithSource(
						*result,
						(SmileObject)SmileList_ConsWithSource(
							(SmileObject)SmileSymbol_Create(symbol),
							NullObject,
							lexerPosition
						),
						lexerPosition
					),
					lexerPosition
				);
			}
		}
		else {
			return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Expected a property name after '.', not '%S.'", TokenKind_ToString(tokenKind)));
		}
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

/// <summary>
/// Transform a simple implicitly-quoted list like [x y z] into an expression that generates
/// that same list, like [List.of [quote x] [quote y] [quote z]].  The original list will not
/// be modified; a new list will be generated that contains the same data.
/// </summary>
/// <param name="head">The head of the old list, which will be updated to point to the head of the new list.</param>
/// <param name="tail">The tail of the old list, which will be updated to point to the tail of the new list.</param>
/// <param name="startPosition">The lexer position of the start of this list, which will be applied to the new [List.of] cell.</param>
static void Parser_TransformListIntoTemplate(SmileList *head, SmileList *tail, LexerPosition startPosition)
{
	SmileList oldHead, oldTail, newHead, newTail;
	SmileObject oldExpr, newExpr;
	LexerPosition position;

	oldHead = *head;
	oldTail = *tail;
	newHead = NullList;
	newTail = NullList;

	// Add an initial [List.of ... ] to the new list to make it into a proper list template.
	newHead = newTail = SmileList_ConsWithSource(
		(SmileObject)SmilePair_CreateWithSource(
			(SmileObject)Smile_KnownObjects.ListSymbol,
			(SmileObject)Smile_KnownObjects.ofSymbol,
			startPosition
		),
		NullObject,
		startPosition
	);

	// Copy each element from the old list, projecting it...
	for (; SMILE_KIND(oldHead) != SMILE_KIND_NULL; oldHead = LIST_REST(oldHead)) {
		oldExpr = oldHead->a;
		position = ((struct SmileListWithSourceInt *)oldHead)->position;

		// Take each element x in the old list, and turn it into [quote x] in the new list.
		newExpr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)Smile_KnownObjects._quoteSymbol,
			(SmileObject)SmileList_ConsWithSource(oldExpr, NullObject, position),
			position
		);

		LIST_APPEND_WITH_SOURCE(newHead, newTail, newExpr, position);
	}

	// Return the new template, which now looks like [List.of [quote x] [quote y] [quote z] ... ]
	*head = newHead;
	*tail = newTail;
}

