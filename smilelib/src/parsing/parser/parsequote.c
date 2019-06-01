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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
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
TemplateResult Parser_ParseQuotedTerm(Parser parser, Int modeFlags, LexerPosition position)
{
	TemplateResult templateResult;

	templateResult = Parser_ParseRawListTerm(parser, modeFlags);
	if (IS_PARSE_ERROR(templateResult.parseResult))
		RETURN_TEMPLATE_PARSE_ERROR(templateResult.parseResult);

	if (templateResult.templateKind == TemplateKind_None) {
		templateResult = TEMPLATE_RESULT(EXPR_RESULT(
				(SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol, templateResult.parseResult.expr, position)
			),
			TemplateKind_None);
	}

	return templateResult;
}

SmileObject Parser_WrapTemplateForSplicing(SmileObject obj)
{
	return (SmileObject)SmileList_CreateTwo(
		SmileList_CreateDot(Smile_KnownObjects.ListSymbol, Smile_KnownObjects.consSymbol),
		obj
	);
}

// raw_list_term :: = . LBRACKET raw_list_items_opt RBRACKET | . any_name
//    | . CHAR | . RAWSTRING
//    | . BYTE | . INT16 | . INT32 | . INT64 | . REAL32 | . REAL64 | . REAL128 | . FLOAT32 | . FLOAT64
//    | . BACKTICK raw_list_term
//    | . LOANWORD_REGEX
//    | . nonraw_term
// nonraw_term :: = . LPAREN expr RPAREN
//    | . scope
//    | . DYNSTRING
TemplateResult Parser_ParseRawListTerm(Parser parser, Int modeFlags)
{
	Token token = Parser_NextToken(parser);
	LexerPosition startPosition;
	ParseError error;
	ParseResult parseResult;
	TemplateResult templateResult;

	switch (token->kind) {

		case TOKEN_LEFTPARENTHESIS:
			Lexer_Unget(parser->lexer);
			return TEMPLATE_RESULT(Parser_ParseParentheses(parser, modeFlags), TemplateKind_Template);

		case TOKEN_LEFTBRACE:
			Lexer_Unget(parser->lexer);
			return TEMPLATE_RESULT(Parser_ParseScope(parser), TemplateKind_Template);

		case TOKEN_DYNSTRING:
			parseResult = Parser_ParseDynamicString(parser, token->text, startPosition = Token_GetPosition(token));
			if (IS_PARSE_ERROR(parseResult))
				RETURN_TEMPLATE_PARSE_ERROR(parseResult);
			return TEMPLATE_RESULT(parseResult, (SMILE_KIND(parseResult.expr) != SMILE_KIND_STRING) ? TemplateKind_Template : TemplateKind_None);

		case TOKEN_LEFTBRACKET:
			{
				SmileList head = NullList, tail = NullList;
				Int childTemplateKind;

				startPosition = Token_GetPosition(token);

				templateResult = Parser_ParseRawListItemsOpt(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS, &head, &tail);
				childTemplateKind = templateResult.templateKind;

				if (!Parser_HasLookahead(parser, TOKEN_RIGHTBRACKET))
					return TEMPLATE_RESULT(ERROR_RESULT(
						ParseMessage_Create(PARSEMESSAGE_ERROR,
							startPosition, String_Format("Missing ']' in raw list starting on line %d.", startPosition->line))),
						TemplateKind_None);
				Parser_NextToken(parser);

				return TEMPLATE_RESULT(EXPR_RESULT(head),
					(childTemplateKind == TemplateKind_None ? TemplateKind_None : TemplateKind_Template));
			}

		case TOKEN_BACKTICK:
			{
				Int tokenKind;
				if ((tokenKind = Lexer_Peek(parser->lexer)) == TOKEN_LEFTPARENTHESIS
					|| tokenKind == TOKEN_LEFTBRACE) {
					templateResult = TEMPLATE_RESULT(Parser_ParseTerm(parser, modeFlags, Token_Clone(token)), TemplateKind_None);
				}
				else {
					templateResult = Parser_ParseRawListTerm(parser, modeFlags);
				}
				if (IS_PARSE_ERROR(templateResult.parseResult))
					RETURN_TEMPLATE_PARSE_ERROR(templateResult.parseResult);
				return TEMPLATE_RESULT(templateResult.parseResult,
					(templateResult.templateKind == TemplateKind_None ? TemplateKind_None : TemplateKind_Template));
			}

		case TOKEN_AT:
			{
				Int nextToken = Lexer_Next(parser->lexer);
				if (nextToken == TOKEN_LEFTPARENTHESIS) {
					// This is the special '@(...)' form, which works like ',@' in Lisp, and
					// captures the inner list, and then splices it into the current list.
					Lexer_Unget(parser->lexer);
					parseResult = Parser_ParseParentheses(parser, modeFlags);
					if (IS_PARSE_ERROR(parseResult))
						RETURN_TEMPLATE_PARSE_ERROR(parseResult);
					return TEMPLATE_RESULT(parseResult, TemplateKind_TemplateWithSplicing);
				}
				else if (nextToken == TOKEN_ALPHANAME || nextToken == TOKEN_PUNCTNAME
					|| nextToken == TOKEN_UNKNOWNALPHANAME || nextToken == TOKEN_UNKNOWNPUNCTNAME) {
					// This is '@symbol', which is a shorthand for writing '(symbol)'.
					//
					// (TODO: This form exists primarily to allow '@symbol' and '@@symbol' to be
					// embedded in the future in quoted syntax-translated template forms,
					// which would be a really useful ability to have.)
					return TEMPLATE_RESULT(EXPR_RESULT(SmileSymbol_Create(parser->lexer->token->data.symbol)), TemplateKind_Template);
				}
				else {
					Lexer_Unget(parser->lexer);
					return TEMPLATE_RESULT(ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
						String_FromC("A symbol or a left parenthesis must follow an '@' in a template."))), TemplateKind_None);
				}
			}

		case TOKEN_ATAT:
			{
				Int nextToken = Lexer_Next(parser->lexer);
				if (nextToken == TOKEN_ALPHANAME || nextToken == TOKEN_PUNCTNAME
					|| nextToken == TOKEN_UNKNOWNALPHANAME || nextToken == TOKEN_UNKNOWNPUNCTNAME) {
					// This is '@@symbol', which is a shorthand for writing '@(symbol)'.
					//
					// (TODO: This form exists primarily to allow '@@symbol' to be
					// embedded in the future in quoted syntax-translated template forms,
					// which would be a really useful ability to have.)
					return TEMPLATE_RESULT(EXPR_RESULT(SmileSymbol_Create(parser->lexer->token->data.symbol)),
						TemplateKind_TemplateWithSplicing);
				}
				else {
					Lexer_Unget(parser->lexer);
					return TEMPLATE_RESULT(ERROR_RESULT(
						ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
							String_FromC("A symbol must follow an '@@' in a template."))),
						TemplateKind_None);
				}
			}

		case TOKEN_ALPHANAME:
		case TOKEN_PUNCTNAME:
		case TOKEN_UNKNOWNALPHANAME:
		case TOKEN_UNKNOWNPUNCTNAME:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileSymbol_Create(token->data.symbol)), TemplateKind_None);

		case TOKEN_RAWSTRING:
			return TEMPLATE_RESULT(EXPR_RESULT(token->text), TemplateKind_None);

		case TOKEN_CHAR:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileChar_Create(token->data.ch)), TemplateKind_None);

		case TOKEN_UNI:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileUni_Create(token->data.uni)), TemplateKind_None);

		case TOKEN_BYTE:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileByte_Create(token->data.byte)), TemplateKind_None);

		case TOKEN_INTEGER16:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileInteger16_Create(token->data.int16)), TemplateKind_None);

		case TOKEN_INTEGER32:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileInteger32_Create(token->data.int32)), TemplateKind_None);

		case TOKEN_INTEGER64:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileInteger64_Create(token->data.int64)), TemplateKind_None);

		case TOKEN_REAL32:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileReal32_Create(token->data.real32)), TemplateKind_None);

		case TOKEN_REAL64:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileReal64_Create(token->data.real64)), TemplateKind_None);

		case TOKEN_FLOAT32:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileFloat32_Create(token->data.float32)), TemplateKind_None);

		case TOKEN_FLOAT64:
			return TEMPLATE_RESULT(EXPR_RESULT(SmileFloat64_Create(token->data.float64)), TemplateKind_None);

		case TOKEN_LOANWORD_REGEX:
			// The Lexer already constructed [Regex.of pattern-string options-string] for us,
			// so there's nothing we need to do except return it.
			return TEMPLATE_RESULT(EXPR_RESULT(token->data.ptr), TemplateKind_None);

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
			return TEMPLATE_RESULT(ERROR_RESULT(error), TemplateKind_None);
	}
}

// raw_list_items_opt :: = . raw_list_items | .
// raw_list_items :: = . raw_list_items raw_list_item | . raw_list_item
// raw_list_item :: = raw_list_dotexpr
TemplateResult Parser_ParseRawListItemsOpt(Parser parser, Int modeFlags, SmileList *head, SmileList *tail)
{
	Token token;
	LexerPosition lexerPosition, startPosition;
	SmileObject expr;
	Int itemTemplateKind;
	Int listTemplateKind;
	TemplateResult templateResult;

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
		templateResult = Parser_ParseRawListDotExpr(parser, modeFlags);
		if (!IS_PARSE_ERROR(templateResult.parseResult)) {

			expr = templateResult.parseResult.expr;
			itemTemplateKind = templateResult.templateKind;

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
						expr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol, expr, lexerPosition);
						itemTemplateKind = TemplateKind_Template;
					}
					else if (listTemplateKind == TemplateKind_TemplateWithSplicing && itemTemplateKind == TemplateKind_None) {
						expr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol,
							SmileList_CreateOneWithSource(expr, lexerPosition),
							lexerPosition
						);
						itemTemplateKind = TemplateKind_TemplateWithSplicing;
					}
					else if (listTemplateKind == TemplateKind_TemplateWithSplicing && itemTemplateKind == TemplateKind_Template) {
						expr = Parser_WrapTemplateForSplicing(expr);
						itemTemplateKind = TemplateKind_TemplateWithSplicing;
					}
				}

				// Add the successfully-parsed expression to the output (if there's something non-null to add).
				LIST_APPEND_WITH_SOURCE(*head, *tail, expr, lexerPosition);
			}
		}
		else {
			// Record the error message.
			HANDLE_PARSE_ERROR(parser, templateResult.parseResult);

			// If that expression was garbage, perform simple error-recovery by skipping to the
			// next '{' '}' '[' ']' '(' ')' or '|'.
			token = Parser_Recover(parser, Parser_BracesBracketsParenthesesBar_Recovery, Parser_BracesBracketsParenthesesBar_Count);

			// Reached a terminating '}' or ']' or ')', so presume we're done consuming expressions for now.
			if (token->kind == TOKEN_RIGHTBRACE || token->kind == TOKEN_RIGHTBRACKET || token->kind == TOKEN_RIGHTPARENTHESIS)
				return TEMPLATE_RESULT(RECOVERY_RESULT(), TemplateKind_None);
		}
	}

	Lexer_Unget(parser->lexer);
	return TEMPLATE_RESULT(EXPR_RESULT(*head), listTemplateKind);
}

// raw_list_dotexpr :: = . raw_list_dotexpr DOT any_name | . raw_list_term
TemplateResult Parser_ParseRawListDotExpr(Parser parser, Int modeFlags)
{
	Int tokenKind;
	LexerPosition lexerPosition;
	Symbol symbol;
	TemplateResult templateResult;

	templateResult = Parser_ParseRawListTerm(parser, modeFlags);
	if (IS_PARSE_ERROR(templateResult.parseResult))
		RETURN_TEMPLATE_PARSE_ERROR(templateResult.parseResult);

	while (Parser_NextToken(parser)->kind == TOKEN_DOT) {

		if ((tokenKind = Lexer_Next(parser->lexer)) == TOKEN_ALPHANAME
			|| tokenKind == TOKEN_UNKNOWNALPHANAME
			|| tokenKind == TOKEN_PUNCTNAME
			|| tokenKind == TOKEN_UNKNOWNPUNCTNAME) {

			symbol = parser->lexer->token->data.symbol;
			lexerPosition = Token_GetPosition(parser->lexer->token);

			if (templateResult.templateKind == TemplateKind_None) {
				// Generate a simple (expr).symbol as output.
				templateResult = TEMPLATE_RESULT(EXPR_RESULT(
					SmileList_CreateDotWithSource(templateResult.parseResult.expr, SmileSymbol_Create(symbol), lexerPosition)),
					TemplateKind_None);
			}
			else {
				// If the left-side expression is a template (spliced or not), then generate
				// [List.of [$quote $dot] (expr) [$quote symbol]] as output.
				templateResult = TEMPLATE_RESULT(EXPR_RESULT(
					SmileList_CreateFourWithSource(
						SmileList_CreateThreeWithSource(Smile_KnownObjects._dotSymbol, Smile_KnownObjects.ListSymbol, Smile_KnownObjects.ofSymbol, lexerPosition),
						SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol, Smile_KnownObjects._dotSymbol, lexerPosition),
						templateResult.parseResult.expr,
						SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol, SmileSymbol_Create(symbol), lexerPosition),
						lexerPosition
					)),
					TemplateKind_None);
			}
		}
		else {
			return TEMPLATE_RESULT(ERROR_RESULT(
				ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
					String_Format("Expected a property name after '.', not '%S.'", TokenKind_ToString(tokenKind)))),
				TemplateKind_None);
		}
	}

	Lexer_Unget(parser->lexer);

	return templateResult;
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
	SmileList oldHead, newHead, newTail;
	SmileObject oldExpr, newExpr;
	LexerPosition position;

	oldHead = *head;

	// Add an initial [List.of ... ] to the new list to make it into a proper list template.
	newHead = newTail = SmileList_CreateOneWithSource(
		(SmileObject)SmileList_CreateDotWithSource(Smile_KnownObjects.ListSymbol, Smile_KnownObjects.ofSymbol, startPosition),
		startPosition
	);

	// Copy each element from the old list, projecting it...
	for (; SMILE_KIND(oldHead) != SMILE_KIND_NULL; oldHead = LIST_REST(oldHead)) {
		oldExpr = oldHead->a;
		position = ((struct SmileListWithSourceInt *)oldHead)->position;

		// Take each element x in the old list, and turn it into [$quote x] in the new list.
		newExpr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol, oldExpr, position);

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
	SmileList oldHead, newHead, newTail;
	SmileObject oldExpr, newExpr;
	LexerPosition position;

	oldHead = *head;

	// Add an initial [List.combine ... ] to the new list to make it into a proper list template.
	newHead = newTail = SmileList_CreateOneWithSource(
		(SmileObject)SmileList_CreateDotWithSource(Smile_KnownObjects.ListSymbol, Smile_KnownObjects.combineSymbol, startPosition),
		startPosition
	);

	// Copy each element from the old list, projecting it...
	for (; SMILE_KIND(oldHead) != SMILE_KIND_NULL; oldHead = LIST_REST(oldHead)) {
		oldExpr = oldHead->a;
		position = ((struct SmileListWithSourceInt *)oldHead)->position;

		// Take each element x in the old list, and turn it into [$quote [x]] in the new list.
		newExpr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol,
			SmileList_CreateOneWithSource(oldExpr, position),
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
	SmileList oldHead, newHead, newTail;
	SmileObject oldExpr, newExpr;
	LexerPosition position;

	oldHead = *head;

	// Add an initial [List.combine ... ] to the new list to make it into a proper list template.
	newHead = newTail = SmileList_CreateOneWithSource(
		(SmileObject)SmileList_CreateDotWithSource(Smile_KnownObjects.ListSymbol, Smile_KnownObjects.combineSymbol, startPosition),
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
			newExpr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol,
				SmileList_CreateOneWithSource(quotedExpr, position),
				position
			);
		}
		else {
			// Take each element x in the old list, and wrap it with a [List.cons x null] in the new list.
			newExpr = Parser_WrapTemplateForSplicing(oldExpr);
		}

		LIST_APPEND_WITH_SOURCE(newHead, newTail, newExpr, position);
	}

	// Return the new template, which now looks like [List.combine [$quote [x]] [List.cons y null] [$quote [z]] ... ]
	*head = newHead;
	*tail = newTail;
}

// term ::= '[' '$quote' raw-list-term ']'
ParseResult Parser_ParseClassicQuote(Parser parser, LexerPosition startPosition)
{
	ParseResult parseResult, bracketResult;

	parseResult = Parser_ParseQuoteBody(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS, startPosition);
	if (IS_PARSE_ERROR(parseResult)) {
		HANDLE_PARSE_ERROR(parser, parseResult);
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		return RECOVERY_RESULT();
	}

	bracketResult = Parser_ExpectRightBracket(parser, NULL, "[$quote] form", startPosition);
	if (IS_PARSE_ERROR(bracketResult))
		RETURN_PARSE_ERROR(bracketResult);

	return parseResult;
}

ParseResult Parser_ParseQuoteBody(Parser parser, Int modeFlags, LexerPosition startPosition)
{
	ParseResult parseResult;

	if (Lexer_Peek(parser->lexer) == TOKEN_LEFTPARENTHESIS) {
		// This is a quote of a parenthesized expression, so parse the expression normally and then quote it.
		parseResult = Parser_ParseParentheses(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		return EXPR_RESULT(SmileList_CreateTwoWithSource(Smile_KnownObjects._quoteSymbol, parseResult.expr, startPosition));
	}

	// This is a quote of a more generic thing, like a list or a symbol, so recursively parse
	// this "quoted term".  Because the "quoted term" might be a list that somewhere contains
	// a (parenthetical escape), thus turning the "quoted term" from an ordinary quoted list
	// into a template, we do not do the quoting here, but instead do that quoting work inside
	// Parser_ParseQuotedTerm() itself, which is the only code that knows how to do it correctly.
	return Parser_ParseQuotedTerm(parser, modeFlags, startPosition).parseResult;
}
