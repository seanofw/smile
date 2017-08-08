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
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static void Parser_TransformListIntoTemplate(SmileList *head, SmileList *tail, LexerPosition lastReadPosition);
static void Parser_TransformListIntoSplicedTemplate(SmileList *head, SmileList *tail, LexerPosition startPosition);
static void Parser_TransformTemplateIntoSplicedTemplate(SmileList *head, SmileList *tail, LexerPosition startPosition);

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
	Int templateKind;

	templateKind = TemplateKind_None;
	parseError = Parser_ParseRawListTerm(parser, result, &templateKind, modeFlags);
	if (parseError != NULL)
		return parseError;

	if (templateKind == TemplateKind_None) {
		*result = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)Smile_KnownObjects._quoteSymbol,
				(SmileObject)SmileList_ConsWithSource(*result, NullObject, position),
			position
		);
	}

	return NULL;
}

static SmileObject WrapForSplicing(SmileObject obj)
{
	return
		(SmileObject)SmileList_Cons((SmileObject)SmilePair_Create((SmileObject)Smile_KnownObjects.ListSymbol, (SmileObject)Smile_KnownObjects.consSymbol),
			(SmileObject)SmileList_Cons(obj,
				(SmileObject)SmileList_Cons(NullObject,
					NullObject)
			)
		);
}

// raw_list_term :: = . LBRACKET raw_list_items_opt RBRACKET | . any_name
//    | . CHAR | . RAWSTRING
//    | . BYTE | . INT16 | . INT32 | . INT64 | . REAL32 | . REAL64 | . REAL128 | . FLOAT32 | . FLOAT64
//    | . BACKTICK raw_list_term
//    | . nonraw_term
// nonraw_term :: = . LPAREN expr RPAREN
//    | . scope
//    | . DYNSTRING
ParseError Parser_ParseRawListTerm(Parser parser, SmileObject *result, Int *templateKind, Int modeFlags)
{
	Token token = Parser_NextToken(parser);
	LexerPosition startPosition;
	ParseError error;

	switch (token->kind) {

	case TOKEN_LEFTPARENTHESIS:
		Lexer_Unget(parser->lexer);
		*templateKind = TemplateKind_Template;
		startPosition = Token_GetPosition(parser->lexer->token);
		error = Parser_ParseParentheses(parser, result, modeFlags);
		if (error != NULL)
			return error;
		return NULL;

	case TOKEN_LEFTBRACE:
		Lexer_Unget(parser->lexer);
		*templateKind = TemplateKind_Template;
		error = Parser_ParseScope(parser, result);
		if (error != NULL)
			return error;
		return NULL;

	case TOKEN_DYNSTRING:
		error = Parser_ParseDynamicString(parser, result, token->text, Token_GetPosition(token));
		if (error != NULL)
			return error;
		*templateKind = (SMILE_KIND(*result) != SMILE_KIND_STRING) ? TemplateKind_Template : TemplateKind_None;
		return error;

	case TOKEN_LEFTBRACKET:
		{
			SmileList head = NullList, tail = NullList;

			startPosition = Token_GetPosition(token);

			Parser_ParseRawListItemsOpt(parser, &head, &tail, templateKind, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

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
			Int childTemplateKind, tokenKind;
			startPosition = Token_GetPosition(token);
			if ((tokenKind = Lexer_Peek(parser->lexer)) == TOKEN_LEFTPARENTHESIS
				|| tokenKind == TOKEN_LEFTBRACE) {
				error = Parser_ParseTerm(parser, result, modeFlags, Token_Clone(token));
			}
			else {
				error = Parser_ParseRawListTerm(parser, result, &childTemplateKind, modeFlags);
			}
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
			*templateKind = TemplateKind_None;
			return NULL;
		}

	case TOKEN_ALPHANAME:
	case TOKEN_PUNCTNAME:
	case TOKEN_UNKNOWNALPHANAME:
	case TOKEN_UNKNOWNPUNCTNAME:
		if (token->data.symbol == SMILE_SPECIAL_SYMBOL_ATSIGN && Lexer_Peek(parser->lexer) == TOKEN_LEFTPARENTHESIS) {
			// This is the special '@(...)' form, which works like ',@' in Lisp, and
			// captures the inner list, and then splices it into the current list.
			*templateKind = TemplateKind_TemplateWithSplicing;
			startPosition = Token_GetPosition(parser->lexer->token);
			error = Parser_ParseParentheses(parser, result, modeFlags);
			if (error != NULL)
				return error;
			*templateKind = TemplateKind_TemplateWithSplicing;
			return NULL;
		}
		else {
			*result = (SmileObject)SmileSymbol_Create(token->data.symbol);
			*templateKind = TemplateKind_None;
			return NULL;
		}

	case TOKEN_RAWSTRING:
		*result = (SmileObject)token->text;
		*templateKind = TemplateKind_None;
		return NULL;

	case TOKEN_CHAR:
		*result = (SmileObject)SmileByte_Create(token->data.byte);
		*templateKind = TemplateKind_None;
		return NULL;

	case TOKEN_BYTE:
		*result = (SmileObject)SmileByte_Create(token->data.byte);
		*templateKind = TemplateKind_None;
		return NULL;

	case TOKEN_INTEGER16:
		*result = (SmileObject)SmileInteger16_Create(token->data.int16);
		*templateKind = TemplateKind_None;
		return NULL;

	case TOKEN_INTEGER32:
		*result = (SmileObject)SmileInteger32_Create(token->data.int32);
		*templateKind = TemplateKind_None;
		return NULL;

	case TOKEN_INTEGER64:
		*result = (SmileObject)SmileInteger64_Create(token->data.int64);
		*templateKind = TemplateKind_None;
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
ParseError Parser_ParseRawListItemsOpt(Parser parser, SmileList *head, SmileList *tail, Int *templateKind, Int modeFlags)
{
	Token token;
	LexerPosition lexerPosition, startPosition;
	SmileObject expr;
	ParseError error;
	Int itemTemplateKind;
	Int listTemplateKind;

	listTemplateKind = TemplateKind_None;
	startPosition = NULL;

	// Consume expressions until the lookahead reaches a terminating '}' or ']' or ')'.
	while ((token = Parser_NextToken(parser))->kind != TOKEN_EOI
		&& token->kind != TOKEN_RIGHTBRACE && token->kind != TOKEN_RIGHTBRACKET && token->kind != TOKEN_RIGHTPARENTHESIS) {

		lexerPosition = Token_GetPosition(token);
		Lexer_Unget(parser->lexer);

		if (startPosition == NULL) startPosition = lexerPosition;

		// Parse the next expression.
		itemTemplateKind = TemplateKind_None;
		error = Parser_ParseRawListDotExpr(parser, &expr, &itemTemplateKind, modeFlags);
		if (error == NULL) {
			if (expr != NullObject) {

				if (itemTemplateKind > listTemplateKind) {
					// Uh oh.  The list item is a template form, but this list (so far) is not
					// yet a template form, or is the wrong kind of template form.  So transform
					// the list into the right kind of template form, because we can't append
					// template items to a non-template list.
					switch (itemTemplateKind) {
						case TemplateKind_Template:
							Parser_TransformListIntoTemplate(head, tail, startPosition);
							break;
						case TemplateKind_TemplateWithSplicing:
							if (listTemplateKind == TemplateKind_None)
								Parser_TransformListIntoSplicedTemplate(head, tail, startPosition);
							else if (listTemplateKind == TemplateKind_Template)
								Parser_TransformTemplateIntoSplicedTemplate(head, tail, startPosition);
							break;
					}
					listTemplateKind = itemTemplateKind;
				}
				else if (itemTemplateKind < listTemplateKind) {
					// This is a templated list, but not a templated item (or not templated enough).
					// So we need to wrap/quote it before adding it to the list.
					if (listTemplateKind == TemplateKind_Template && itemTemplateKind == TemplateKind_None) {
						expr = (SmileObject)SmileList_ConsWithSource(
							(SmileObject)Smile_KnownObjects._quoteSymbol,
							(SmileObject)SmileList_ConsWithSource(
								expr,
								NullObject,
								lexerPosition),
							lexerPosition
						);
						itemTemplateKind = TemplateKind_Template;
					}
					else if (listTemplateKind == TemplateKind_TemplateWithSplicing && itemTemplateKind == TemplateKind_None) {
						expr = (SmileObject)SmileList_ConsWithSource(
							(SmileObject)Smile_KnownObjects._quoteSymbol,
							(SmileObject)SmileList_ConsWithSource(
								(SmileObject)SmileList_ConsWithSource(expr, NullObject, lexerPosition),
								NullObject,
								lexerPosition),
							lexerPosition
						);
						itemTemplateKind = TemplateKind_TemplateWithSplicing;
					}
					else if (listTemplateKind == TemplateKind_TemplateWithSplicing && itemTemplateKind == TemplateKind_Template) {
						expr = WrapForSplicing(expr);
						itemTemplateKind = TemplateKind_TemplateWithSplicing;
					}
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

	*templateKind = listTemplateKind;

	Lexer_Unget(parser->lexer);
	return NULL;
}

// raw_list_dotexpr :: = . raw_list_dotexpr DOT any_name | . raw_list_term
ParseError Parser_ParseRawListDotExpr(Parser parser, SmileObject *result, Int *templateKind, Int modeFlags)
{
	ParseError parseError;
	Int tokenKind;
	LexerPosition lexerPosition;
	Symbol symbol;

	parseError = Parser_ParseRawListTerm(parser, result, templateKind, modeFlags);
	if (parseError != NULL)
		return parseError;

	while (Parser_NextToken(parser)->kind == TOKEN_DOT) {

		if ((tokenKind = Lexer_Next(parser->lexer)) == TOKEN_ALPHANAME
			|| tokenKind == TOKEN_UNKNOWNALPHANAME
			|| tokenKind == TOKEN_PUNCTNAME
			|| tokenKind == TOKEN_UNKNOWNPUNCTNAME) {

			symbol = parser->lexer->token->data.symbol;
			lexerPosition = Token_GetPosition(parser->lexer->token);

			if (*templateKind == TemplateKind_None) {
				// Generate a simple (expr).symbol as output.
				*result = (SmileObject)SmilePair_CreateWithSource(*result, (SmileObject)SmileSymbol_Create(symbol), lexerPosition);
			}
			else {
				// If the left-side expression is a template (spliced or not), then generate
				// [Pair.of (expr) [$quote symbol]] as output.
				*result = (SmileObject)SmileList_ConsWithSource(
					(SmileObject)SmilePair_CreateWithSource(
						(SmileObject)Smile_KnownObjects.PairSymbol,
						(SmileObject)Smile_KnownObjects.ofSymbol,
						lexerPosition
					),
					(SmileObject)SmileList_ConsWithSource(
						*result,
						(SmileObject)SmileList_ConsWithSource(
							(SmileObject)SmileList_ConsWithSource(
								(SmileObject)Smile_KnownObjects._quoteSymbol,
								(SmileObject)SmileList_ConsWithSource(
									(SmileObject)SmileSymbol_Create(symbol),
									NullObject,
									lexerPosition
								),
								lexerPosition
							),
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
/// that same list, like [List.of [$quote x] [$quote y] [$quote z]].  The original list will not
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

		// Take each element x in the old list, and turn it into [$quote x] in the new list.
		newExpr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)Smile_KnownObjects._quoteSymbol,
			(SmileObject)SmileList_ConsWithSource(oldExpr, NullObject, position),
			position
		);

		LIST_APPEND_WITH_SOURCE(newHead, newTail, newExpr, position);
	}

	// Return the new template, which now looks like [List.of [$quote x] [$quote y] [$quote z] ... ]
	*head = newHead;
	*tail = newTail;
}

/// <summary>
/// Transform a simple implicitly-quoted list like [x y z] into an expression that indirectly generates
/// that same list, like [List.combine [$quote [x]] [$quote [y]] [$quote [z]]].
/// The original list will not be modified; a new list will be generated that contains the same data.
/// </summary>
/// <param name="head">The head of the old list, which will be updated to point to the head of the new list.</param>
/// <param name="tail">The tail of the old list, which will be updated to point to the tail of the new list.</param>
/// <param name="startPosition">The lexer position of the start of this list, which will be applied to the new [List.of] cell.</param>
static void Parser_TransformListIntoSplicedTemplate(SmileList *head, SmileList *tail, LexerPosition startPosition)
{
	SmileList oldHead, oldTail, newHead, newTail;
	SmileObject oldExpr, newExpr;
	LexerPosition position;

	oldHead = *head;
	oldTail = *tail;
	newHead = NullList;
	newTail = NullList;

	// Add an initial [List.combine ... ] to the new list to make it into a proper list template.
	newHead = newTail = SmileList_ConsWithSource(
		(SmileObject)SmilePair_CreateWithSource(
			(SmileObject)Smile_KnownObjects.ListSymbol,
			(SmileObject)Smile_KnownObjects.combineSymbol,
			startPosition
		),
		NullObject,
		startPosition
	);

	// Copy each element from the old list, projecting it...
	for (; SMILE_KIND(oldHead) != SMILE_KIND_NULL; oldHead = LIST_REST(oldHead)) {
		oldExpr = oldHead->a;
		position = ((struct SmileListWithSourceInt *)oldHead)->position;

		// Take each element x in the old list, and turn it into [$quote [x]] in the new list.
		newExpr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)Smile_KnownObjects._quoteSymbol,
			(SmileObject)SmileList_ConsWithSource(
				(SmileObject)SmileList_ConsWithSource(oldExpr, NullObject, position),
				NullObject,
				position),
			position
		);

		LIST_APPEND_WITH_SOURCE(newHead, newTail, newExpr, position);
	}

	// Return the new template, which now looks like [List.combine [$quote [x]] [$quote [y]] [$quote [z]] ... ]
	*head = newHead;
	*tail = newTail;
}

/// <summary>
/// Transform a simple template list of the form [List.of x y z] into an expression that generates
/// that same output using sublists, like [List.combine [List.cons x null] [List.cons y null] [List.cons z null]].
/// The original list will not be modified; a new list will be generated that contains the same data.
/// </summary>
/// <param name="head">The head of the old list, which will be updated to point to the head of the new list.</param>
/// <param name="tail">The tail of the old list, which will be updated to point to the tail of the new list.</param>
/// <param name="startPosition">The lexer position of the start of this list, which will be applied to the new [List.of] cell.</param>
static void Parser_TransformTemplateIntoSplicedTemplate(SmileList *head, SmileList *tail, LexerPosition startPosition)
{
	SmileList oldHead, oldTail, newHead, newTail;
	SmileObject oldExpr, newExpr;
	LexerPosition position;

	oldHead = *head;
	oldTail = *tail;
	newHead = NullList;
	newTail = NullList;

	// Add an initial [List.combine ... ] to the new list to make it into a proper list template.
	newHead = newTail = SmileList_ConsWithSource(
		(SmileObject)SmilePair_CreateWithSource(
			(SmileObject)Smile_KnownObjects.ListSymbol,
			(SmileObject)Smile_KnownObjects.combineSymbol,
			startPosition
		),
		NullObject,
		startPosition
	);

	// Copy each element from the old list, projecting it...
	for (oldHead = LIST_REST(oldHead); SMILE_KIND(oldHead) != SMILE_KIND_NULL; oldHead = LIST_REST(oldHead)) {
		oldExpr = oldHead->a;
		position = ((struct SmileListWithSourceInt *)oldHead)->position;

		// See if this element x is actually of the form [$quote x].  If it is, we can use a better
		// replacement for it --- [$quote [x]] --- than the general-purpose cons technique below.
		if (SMILE_KIND(oldExpr) == SMILE_KIND_LIST
			&& SMILE_KIND(((SmileList)oldExpr)->a) == SMILE_KIND_SYMBOL
			&& ((SmileSymbol)((SmileList)oldExpr)->a)->symbol == SMILE_SPECIAL_SYMBOL__QUOTE
			&& SmileList_SafeLength((SmileList)oldExpr) == 2) {

			SmileObject quotedExpr = ((SmileList)((SmileList)oldExpr)->d)->a;
			newExpr = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)Smile_KnownObjects._quoteSymbol,
				(SmileObject)SmileList_ConsWithSource(
					(SmileObject)SmileList_ConsWithSource(quotedExpr, NullObject, position),
					NullObject,
					position),
				position
			);
		}
		else {
			// Take each element x in the old list, and wrap it with a [List.cons x null] in the new list.
			newExpr = WrapForSplicing(oldExpr);
		}

		LIST_APPEND_WITH_SOURCE(newHead, newTail, newExpr, position);
	}

	// Return the new template, which now looks like [List.combine [$quote [x]] [List.cons y null] [$quote [z]] ... ]
	*head = newHead;
	*tail = newTail;
}

// term ::= '[' '$quote' raw-list-term ']'
ParseError Parser_ParseClassicQuote(Parser parser, SmileObject *result, LexerPosition startPosition)
{
	ParseError error;

	error = Parser_ParseQuoteBody(parser, result, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS, startPosition);
	if (error != NULL) {
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		*result = NullObject;
		return error;
	}

	if ((error = Parser_ExpectRightBracket(parser, result, NULL, "[$quote] form", startPosition)) != NULL)
		return error;

	return NULL;
}

ParseError Parser_ParseQuoteBody(Parser parser, SmileObject *result, Int modeFlags, LexerPosition startPosition)
{
	ParseError error;

	if (Lexer_Peek(parser->lexer) == TOKEN_LEFTPARENTHESIS) {
		// This is a quote of a parenthesized expression, so parse the expression normally and then quote it.
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

	// This is a quote of a more generic thing, like a list or a symbol, so recursively parse
	// this "quoted term".  Because the "quoted term" might be a list that somewhere contains
	// a (parenthetical escape), thus turning the "quoted term" from an ordinary quoted list
	// into a template, we do not do the quoting here, but instead do that quoting work inside
	// Parser_ParseQuotedTerm() itself, which is the only code that knows how to do it correctly.
	return Parser_ParseQuotedTerm(parser, result, modeFlags, startPosition);
}
