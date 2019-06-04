//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static struct SmileListInt _parser_ignorableObject = { 0 };

// This object is used to identify constructs that may safely be elided from the parser's output.
SmileObject Parser_IgnorableObject = (SmileObject)&_parser_ignorableObject;

#define FORWARD_CUSTOM_SYNTAX_RESULT(result) do { \
		if ((result).status != ParseStatus_NotMatchedAndNoTokensConsumed) { \
			return (result); \
		} \
	} while (0)

#define FORWARD_CHAINED_CUSTOM_SYNTAX_RESULT(result) do { \
		if ((result).status != ParseStatus_NotMatchedAndNoTokensConsumed) { \
			switch ((result).status) { \
				case ParseStatus_SuccessfulWithResult: \
				case ParseStatus_SuccessfulWithNoResult: \
				case ParseStatus_ErroredButRecovered: \
					goto parseNextOperator; \
				case ParseStatus_PartialParseWithError: \
					return (result); \
				default: \
					Smile_Abort_FatalError("Unhandled ParseStatus in forwarding logic."); \
			} \
		} \
	} while (0)

//-------------------------------------------------------------------------------------------------
// Base expression parsing

//  nonbreak_expr ::= . expr    // Explicitly in a nonbreak_expr, binary operators cannot be matched
//									if they are the first non-whitespace on a line.  This behavior is
//									disabled at the end of the nonbreak_expr, whenever any [], (), or {}
//									grouping is entered, or whenever we are parsing inside the first
//									expr/arith of an if_then or a do-while.
//
//  expr ::= . base_expr
ParseResult Parser_ParseExpr(Parser parser, Int modeFlags)
{
	return Parser_ParseStmt(parser, modeFlags);
}

//  base_expr ::= . arith
//         | . var_decl
//         | . scope
//         | . return
//         | . LOANWORD_INCLUDE string
//         | . LOANWORD_BRK base_expr
ParseResult Parser_ParseStmt(Parser parser, Int modeFlags)
{
	Token token;
	ParseResult parseResult;

	switch ((token = Parser_NextToken(parser))->kind) {

		case TOKEN_LEFTBRACE:
			Lexer_Unget(parser->lexer);
			return Parser_ParseScope(parser);

		case TOKEN_LOANWORD_INCLUDE:
			return Parser_ParseInclude(parser);

		case TOKEN_LOANWORD_BRK:
			return EXPR_RESULT((SmileObject)SmileList_ConsWithSource(
				(SmileObject)SmileSymbol_Create(Smile_KnownSymbols._brk),
				NullObject,
				Token_GetPosition(token)
			));

		case TOKEN_ALPHANAME:
		case TOKEN_UNKNOWNALPHANAME:
			switch (token->data.symbol) {
				case SMILE_SPECIAL_SYMBOL_VAR:
					return Parser_ParseVarDecls(parser, modeFlags, PARSEDECL_VARIABLE);
				case SMILE_SPECIAL_SYMBOL_CONST:
					return Parser_ParseVarDecls(parser, modeFlags, PARSEDECL_CONST);
				case SMILE_SPECIAL_SYMBOL_AUTO:
					return Parser_ParseVarDecls(parser, modeFlags, PARSEDECL_AUTO);
				case SMILE_SPECIAL_SYMBOL_KEYWORD:
					return Parser_ParseKeywordList(parser);
				case SMILE_SPECIAL_SYMBOL_IF:
					return Parser_ParseIfUnless(parser, modeFlags, False, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_UNLESS:
					return Parser_ParseIfUnless(parser, modeFlags, True, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_DO:
					return Parser_ParseDo(parser, modeFlags, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_WHILE:
					return Parser_ParseWhileUntil(parser, modeFlags, False, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_UNTIL:
					return Parser_ParseWhileUntil(parser, modeFlags, True, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_RETURN:
					return Parser_ParseReturn(parser, modeFlags, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_TILL:
					return Parser_ParseTill(parser, modeFlags, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_TRY:
					return Parser_ParseTry(parser, modeFlags, Token_GetPosition(token));
			}
			// Fall through to default case if not a special form.

		default:
			Lexer_Unget(parser->lexer);
			parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_STMT, SYNTAXROOT_KEYWORD, 0);
			if (parseResult.status != ParseStatus_NotMatchedAndNoTokensConsumed)
				return parseResult;

			return Parser_ParseOpEquals(parser, (modeFlags & ~COMMAMODE_MASK) | COMMAMODE_NORMAL);
	}
}

// if_then ::= IF . arith THEN expr
//           | IF . arith THEN expr ELSE expr
//           | UNLESS . arith THEN expr
//           | UNLESS . arith THEN expr ELSE expr
ParseResult Parser_ParseIfUnless(Parser parser, Int modeFlags, Bool invert, LexerPosition lexerPosition)
{
	ParseResult conditionResult, thenResult, elseResult;
	SmileObject condition, thenBody, elseBody;
	Token token;

	// Parse the condition.
	conditionResult = Parser_ParseOpEquals(parser, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (IS_PARSE_ERROR(conditionResult)) {
		// Bad condition.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_THEN) {

			// Bad condition, but we recovered to a 'then', so try to keep going.
			Parser_NextToken(parser);
			HANDLE_PARSE_ERROR(parser, conditionResult);
			condition = NullObject;
		}
		else {
			// Didn't recover to anything meaningful, so give up.
			return conditionResult;
		}
	}
	else condition = conditionResult.expr;

	// Consume the 'then' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_THEN)) {
		// Missing 'then' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'then' keyword after '%s'.", invert ? "unless" : "if");
		Lexer_Unget(parser->lexer);
	}

	// Parse the then-body.
	thenResult = Parser_ParseExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(thenResult)) {
		// Bad then-body.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_ELSE) {

			// Bad condition, but we recovered to an 'else', so try to keep going.
			Parser_NextToken(parser);
			HANDLE_PARSE_ERROR(parser, thenResult);
			thenBody = NullObject;
		}
		else {
			// Didn't recover to anything meaningful, so give up.
			return thenResult;
		}
	}
	else thenBody = thenResult.expr;

	// Consume an optional 'else' keyword.
	if (((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_ELSE) {

		// Parse the else-body.
		elseResult = Parser_ParseExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(elseResult)) {
			// No recovery past this point; there are no more tokens we can consume in the grammar.
			return elseResult;
		}
		elseBody = elseResult.expr;
	}
	else {
		Lexer_Unget(parser->lexer);
		elseBody = NullObject;
	}

	// If we're an 'unless' form, add a [$not] around the condition.
	if (invert) {
		condition = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._notSymbol, condition, lexerPosition);
	}

	// Now make the resulting form.
	if (SMILE_KIND(elseBody) != SMILE_KIND_NULL) {
		// Make an [$if cond then else] construct.
		return EXPR_RESULT(SmileList_CreateFourWithSource(Smile_KnownObjects._ifSymbol, condition, thenBody, elseBody, lexerPosition));
	}
	else {
		// Make an [$if cond then] construct.
		return EXPR_RESULT(SmileList_CreateThreeWithSource(Smile_KnownObjects._ifSymbol, condition, thenBody, lexerPosition));
	}
}

// do_while ::= DO . expr WHILE arith
//            | DO . expr UNTIL arith
ParseResult Parser_ParseDo(Parser parser, Int modeFlags, LexerPosition lexerPosition)
{
	ParseResult conditionResult, bodyResult;
	SmileObject condition, body;
	Token token;
	Bool invert;

	// Parse the body.
	bodyResult = Parser_ParseExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(bodyResult)) {
		// Bad body.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& (recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_WHILE || recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_UNTIL)) {

			// Bad condition, but we recovered to a 'while' or an 'until', so try to keep going.
			Parser_NextToken(parser);
			HANDLE_PARSE_ERROR(parser, bodyResult);
			body = NullObject;
		}
		else {
			// Didn't recover to anything meaningful, so give up.
			return bodyResult;
		}
	}
	else body = bodyResult.expr;

	// Consume the 'while' or 'until' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& (token->data.symbol == SMILE_SPECIAL_SYMBOL_WHILE || token->data.symbol == SMILE_SPECIAL_SYMBOL_UNTIL))) {
		// Missing 'do' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'while' or 'until' keyword after 'do'.");
		Lexer_Unget(parser->lexer);
	}
	invert = token->data.symbol == SMILE_SPECIAL_SYMBOL_UNTIL;

	// Parse the condition.
	conditionResult = Parser_ParseOpEquals(parser, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (IS_PARSE_ERROR(conditionResult)) {
		// No recovery past this point; there are no more tokens we can consume in the grammar.
		return conditionResult;
	}
	else condition = conditionResult.expr;

	// If we're an 'until' form, add a [$not] around the condition.
	if (invert) {
		condition = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._notSymbol, condition, lexerPosition);
	}

	// Now make the resulting form: [$while body condition null]
	return EXPR_RESULT(SmileList_CreateFourWithSource(Smile_KnownObjects._whileSymbol, body, condition, NullObject, lexerPosition));
}

// while_do ::= WHILE . arith DO expr
//            | UNTIL . arith DO expr
ParseResult Parser_ParseWhileUntil(Parser parser, Int modeFlags, Bool invert, LexerPosition lexerPosition)
{
	ParseResult conditionResult, bodyResult;
	SmileObject condition, body;
	Token token;

	// Parse the condition.
	conditionResult = Parser_ParseOpEquals(parser, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (IS_PARSE_ERROR(conditionResult)) {
		// Bad condition.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_DO) {

			// Bad condition, but we recovered to a 'do', so try to keep going.
			Parser_NextToken(parser);
			HANDLE_PARSE_ERROR(parser, conditionResult);
			condition = NullObject;
		}
		else {
			// Can't recover to a known state, so bail.
			return conditionResult;
		}
	}
	else condition = conditionResult.expr;

	// Consume the 'do' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_DO)) {
		// Missing 'do' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'do' keyword after '%s'.", invert ? "until" : "while");
		Lexer_Unget(parser->lexer);
	}

	// Parse the then-body.
	bodyResult = Parser_ParseExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(bodyResult)) {
		// No recovery past this point; there are no more tokens we can consume in the grammar.
		return bodyResult;
	}
	else body = bodyResult.expr;

	// If we're an 'until' form, add a [$not] around the condition.
	if (invert) {
		condition = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._notSymbol, condition, lexerPosition);
	}

	// Now make the resulting form: [$while condition body]
	return EXPR_RESULT(SmileList_CreateThreeWithSource(Smile_KnownObjects._whileSymbol, condition, body, lexerPosition));
}

// return ::= RETURN . arith
ParseResult Parser_ParseReturn(Parser parser, Int modeFlags, LexerPosition lexerPosition)
{
	ParseResult parseResult;
	SmileObject result;

	// Parse the result.
	parseResult = Parser_ParseOpEquals(parser, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	result = parseResult.expr;

	// Make a [$return result] construct.
	return EXPR_RESULT(SmileList_CreateTwoWithSource(Smile_KnownObjects._returnSymbol, result, lexerPosition));
}

// try_catch ::= TRY . expr CATCH func
ParseResult Parser_ParseTry(Parser parser, Int modeFlags, LexerPosition lexerPosition)
{
	ParseResult handlerResult, bodyResult;
	SmileObject handler, body;
	Token token;
	Int tokenKind;

	// Parse the body.
	bodyResult = Parser_ParseExpr(parser, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (IS_PARSE_ERROR(bodyResult)) {
		// Bad condition.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_CATCH) {

			// Bad body, but we recovered to a 'catch', so try to keep going.
			Parser_NextToken(parser);
			HANDLE_PARSE_ERROR(parser, bodyResult);
			body = NullObject;
		}
		else {
			// Can't recover to a known state, so bail.
			return bodyResult;
		}
	}
	else body = bodyResult.expr;

	// Consume the 'catch' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_CATCH)) {
		// Missing 'catch' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'catch' keyword after 'try'.");
		Lexer_Unget(parser->lexer);
	}

	// Parse the handler function.
	if ((tokenKind = Lexer_Next(parser->lexer)) == TOKEN_BAR) {
		handlerResult = Parser_ParseFunc(parser, modeFlags);
		if (IS_PARSE_ERROR(handlerResult)) {
			// No recovery past this point; there are no more tokens we can consume in the grammar.
			return handlerResult;
		}
		handler = handlerResult.expr;
	}
	else if (tokenKind == TOKEN_LEFTBRACKET) {
		// Might be [$fn ...], so try to parse that.
		Lexer_Unget(parser->lexer);
		handlerResult = Parser_ParseTerm(parser, modeFlags, NULL);
		if (IS_PARSE_ERROR(handlerResult)) {
			// No recovery past this point; there are no more tokens we can consume in the grammar.
			return handlerResult;
		}
		handler = handlerResult.expr;
		if (SMILE_KIND(handler) != SMILE_KIND_LIST
			|| SMILE_KIND(LIST_FIRST((SmileList)handler)) != SMILE_KIND_SYMBOL
			|| ((SmileSymbol)LIST_FIRST((SmileList)handler))->symbol != SMILE_SPECIAL_SYMBOL__FN) {
			// No recovery past this point; there are no more tokens we can consume in the grammar.
			Lexer_Unget(parser->lexer);
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("'catch' handler must be a function.")));
		}
	}
	else {
		// No recovery past this point; there are no more tokens we can consume in the grammar.
		Lexer_Unget(parser->lexer);
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("'catch' handler must be a function.")));
	}

	// Now make the resulting form: [$catch body handler]
	return EXPR_RESULT(SmileList_CreateThreeWithSource(Smile_KnownObjects._catchSymbol, body, handler, lexerPosition));
}

// orexpr ::= . orexpr OR andexpr | . andexpr
ParseResult Parser_ParseOrExpr(Parser parser, Int modeFlags)
{
	SmileList tail = NullList;
	SmileObject expr, rvalue;
	Token token;
	LexerPosition lexerPosition;
	Bool isFirst = True;
	ParseResult parseResult;

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_EXPR, SYNTAXROOT_KEYWORD, 0);
	FORWARD_CUSTOM_SYNTAX_RESULT(parseResult);

	parseResult = Parser_ParseAndExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

	while (((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_OR
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseResult = Parser_ParseAndExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		if (isFirst) {
			expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._orSymbol, expr, rvalue, lexerPosition);
			isFirst = False;
		}
		else {
			tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));
		}
	}

	Lexer_Unget(parser->lexer);

	return EXPR_RESULT(expr);
}

// andexpr :: = . andexpr AND notexpr | . notexpr
ParseResult Parser_ParseAndExpr(Parser parser, Int modeFlags)
{
	SmileList tail = NullList;
	SmileObject expr, rvalue;
	Token token;
	LexerPosition lexerPosition;
	Bool isFirst = True;
	ParseResult parseResult;

	parseResult = Parser_ParseNotExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

	while (((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_AND
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseResult = Parser_ParseNotExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		if (isFirst) {
			expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._andSymbol, expr, rvalue, lexerPosition);
			isFirst = False;
		}
		else {
			tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));
		}
	}

	Lexer_Unget(parser->lexer);

	return EXPR_RESULT(expr);
}

// notexpr :: = . NOT notexpr | . cmpexpr
ParseResult Parser_ParseNotExpr(Parser parser, Int modeFlags)
{
	SmileObject expr;
	Token token;
	LexerPosition lexerPosition;
	struct TokenStruct stackOperators[16];					// The first 16 unary operators will go on C's stack.
	struct TokenStruct *unaryOperators = stackOperators;	// Any successive unary operators will be reallocated on the heap.
	struct TokenStruct *tempOperators;
	Int numOperators = 0, maxOperators = 16, newMax, i;
	ParseResult parseResult;

	// Because this rule is right-recursive, and we don't want to recurse wherever we can
	// avoid it, we loop to collect NOTs, and then parse the 'cmpexpr' expression, and then build
	// up the same tree of NOTs we would have built recursively.

	// Collect the first unary prefix operator.
	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_ALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_NOT) {

		MemCpy(unaryOperators + numOperators++, token, sizeof(struct TokenStruct));

		// Collect up any successive unary prefix operators in the unaryOperators array.
		while (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_ALPHANAME)
			&& token->data.symbol == SMILE_SPECIAL_SYMBOL_NOT) {

			if (numOperators >= maxOperators) {
				newMax = maxOperators * 2;
				tempOperators = GC_MALLOC_STRUCT_ARRAY(struct TokenStruct, newMax);
				MemCpy(tempOperators, unaryOperators, maxOperators * sizeof(struct TokenStruct));
				maxOperators = newMax;
				unaryOperators = tempOperators;
			}

			MemCpy(unaryOperators + numOperators++, token, sizeof(struct TokenStruct));
		}
	}

	Lexer_Unget(parser->lexer);

	// We now have all of the unary prefix operators.  Now go parse the term itself.
	parseResult = Parser_ParseCmpExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

	// Apply any unary operators by spinning out Smile expressions of the form [(expr.unary)] for
	// each unary operator, going from last (innermost) to first (outermost).
	for (i = numOperators - 1; i >= 0; i--) {
		lexerPosition = Token_GetPosition(&unaryOperators[i]);
		// Not is a special built-in form:  [$not x]
		expr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._notSymbol, expr, lexerPosition);
	}

	return EXPR_RESULT(expr);
}

Inline SmileSymbol Parser_GetSymbolObjectForCmpOperator(Symbol symbol)
{
	switch (symbol) {
		case SMILE_SPECIAL_SYMBOL_EQ:
			return Smile_KnownObjects.eqSymbol;
		case SMILE_SPECIAL_SYMBOL_NE:
			return Smile_KnownObjects.neSymbol;
		case SMILE_SPECIAL_SYMBOL_LT:
			return Smile_KnownObjects.ltSymbol;
		case SMILE_SPECIAL_SYMBOL_GT:
			return Smile_KnownObjects.gtSymbol;
		case SMILE_SPECIAL_SYMBOL_LE:
			return Smile_KnownObjects.leSymbol;
		case SMILE_SPECIAL_SYMBOL_GE:
			return Smile_KnownObjects.geSymbol;
		case SMILE_SPECIAL_SYMBOL_SUPEREQ:
			return Smile_KnownObjects._eqSymbol;
		case SMILE_SPECIAL_SYMBOL_SUPERNE:
			return Smile_KnownObjects._neSymbol;
		case SMILE_SPECIAL_SYMBOL_IS:
			return Smile_KnownObjects._isSymbol;
		default:
			return NULL;
	}
}

// cmpexpr ::= . cmpexpr LT addexpr | . cmpexpr GT addexpr | . cmpexpr LE addexpr | . cmpexpr GE addexpr
//       | . cmpexpr EQ addexpr | . cmpexpr NE addexpr | . cmpexpr SUPEREQ addexpr | . cmpexpr SUPERNE addexpr
//       | . cmpexpr IS addexpr
//       | . addexpr
ParseResult Parser_ParseCmpExpr(Parser parser, Int modeFlags)
{
	SmileObject expr, rvalue;
	SmileSymbol symbolObject;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	ParseResult parseResult;

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_CMPEXPR, SYNTAXROOT_KEYWORD, 0);
	FORWARD_CUSTOM_SYNTAX_RESULT(parseResult);

	parseResult = Parser_ParseAddExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

parseNextOperator:

	if (Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE)) {
		// This might not be possible, but if this is something like >==, then don't allow it to be consumed as a binary operator.
		return EXPR_RESULT(expr);
	}

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_CMPEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_ADDEXPR);
	FORWARD_CHAINED_CUSTOM_SYNTAX_RESULT(parseResult);

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNPUNCTNAME || token->kind == TOKEN_UNKNOWNALPHANAME
			|| token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_ALPHANAME)
		&& (symbolObject = Parser_GetSymbolObjectForCmpOperator(symbol = token->data.symbol)) != NULL
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseResult = Parser_ParseAddExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		if (symbol == SMILE_SPECIAL_SYMBOL_SUPEREQ || symbol == SMILE_SPECIAL_SYMBOL_SUPERNE
			|| symbol == SMILE_SPECIAL_SYMBOL_IS) {
			expr = (SmileObject)SmileList_CreateThreeWithSource(symbolObject, expr, rvalue, lexerPosition);
		}
		else {
			expr = (SmileObject)SmileList_CreateTwoWithSource(
				SmileList_CreateDotWithSource(expr, symbolObject, lexerPosition), rvalue, lexerPosition);
		}
		
		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return EXPR_RESULT(expr);
}

// addexpr ::= . addexpr PLUS mulexpr | . addexpr MINUS mulexpr | . mulexpr
ParseResult Parser_ParseAddExpr(Parser parser, Int modeFlags)
{
	SmileObject expr, rvalue;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	ParseResult parseResult;

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_ADDEXPR, SYNTAXROOT_KEYWORD, 0);
	FORWARD_CUSTOM_SYNTAX_RESULT(parseResult);

	parseResult = Parser_ParseMulExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

parseNextOperator:

	if (Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE)) {
		// If this is something like +=, then don't allow it to be consumed as a binary operator.
		return EXPR_RESULT(expr);
	}

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_ADDEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_MULEXPR);
	FORWARD_CHAINED_CUSTOM_SYNTAX_RESULT(parseResult);

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNPUNCTNAME || token->kind == TOKEN_PUNCTNAME)
		&& ((symbol = token->data.symbol) == SMILE_SPECIAL_SYMBOL_PLUS || symbol == SMILE_SPECIAL_SYMBOL_MINUS)
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseResult = Parser_ParseMulExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		expr = (SmileObject)SmileList_CreateTwoWithSource(
			SmileList_CreateDotWithSource(expr,
				symbol == SMILE_SPECIAL_SYMBOL_PLUS ? Smile_KnownObjects.plusSymbol : Smile_KnownObjects.minusSymbol,
				lexerPosition),
			rvalue,
			lexerPosition
		);

		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return EXPR_RESULT(expr);
}

// mulexpr ::= . mulexpr STAR binaryexpr | . mulexpr SLASH binaryexpr | . binaryexpr
ParseResult Parser_ParseMulExpr(Parser parser, Int modeFlags)
{
	SmileObject expr, rvalue;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	ParseResult parseResult;

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_MULEXPR, SYNTAXROOT_KEYWORD, 0);
	FORWARD_CUSTOM_SYNTAX_RESULT(parseResult);

	parseResult = Parser_ParseBinaryExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

parseNextOperator:

	if (Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE)) {
		// If this is something like *=, then don't allow it to be consumed as a binary operator.
		return EXPR_RESULT(expr);
	}

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_MULEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_BINARYEXPR);
	FORWARD_CHAINED_CUSTOM_SYNTAX_RESULT(parseResult);

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNPUNCTNAME || token->kind == TOKEN_PUNCTNAME)
		&& ((symbol = token->data.symbol) == SMILE_SPECIAL_SYMBOL_STAR || symbol == SMILE_SPECIAL_SYMBOL_SLASH)
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseResult = Parser_ParseBinaryExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		expr = (SmileObject)SmileList_CreateTwoWithSource(
			SmileList_CreateDotWithSource(expr,
				symbol == SMILE_SPECIAL_SYMBOL_STAR ? Smile_KnownObjects.starSymbol : Smile_KnownObjects.slashSymbol,
				lexerPosition),
			rvalue,
			lexerPosition
		);

		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return EXPR_RESULT(expr);
}

Inline Bool Parser_IsAcceptableArbitraryBinaryOperator(Parser parser, Symbol symbol)
{
	switch (symbol) {
		default:
			if (parser->customFollowSet != NULL) {
				return !Int32Int32Dict_ContainsKey(parser->customFollowSet, symbol);
			}
			return True;

		case SMILE_SPECIAL_SYMBOL_VAR:
		case SMILE_SPECIAL_SYMBOL_CONST:
		case SMILE_SPECIAL_SYMBOL_AUTO:

		case SMILE_SPECIAL_SYMBOL_NOT:
		case SMILE_SPECIAL_SYMBOL_OR:
		case SMILE_SPECIAL_SYMBOL_AND:

		case SMILE_SPECIAL_SYMBOL_NEW:
		case SMILE_SPECIAL_SYMBOL_IS:
		case SMILE_SPECIAL_SYMBOL_TYPEOF:
		case SMILE_SPECIAL_SYMBOL_SUPEREQ:
		case SMILE_SPECIAL_SYMBOL_SUPERNE:

		case SMILE_SPECIAL_SYMBOL_EQ:
		case SMILE_SPECIAL_SYMBOL_NE:
		case SMILE_SPECIAL_SYMBOL_LT:
		case SMILE_SPECIAL_SYMBOL_GT:
		case SMILE_SPECIAL_SYMBOL_LE:
		case SMILE_SPECIAL_SYMBOL_GE:

		case SMILE_SPECIAL_SYMBOL_PLUS:
		case SMILE_SPECIAL_SYMBOL_MINUS:
		case SMILE_SPECIAL_SYMBOL_STAR:
		case SMILE_SPECIAL_SYMBOL_SLASH:
			return False;
	}
}

// binaryexpr ::= . binaryexpr UNKNOWN_PUNCT_NAME binary_args
// 		| . binaryexpr UNKNOWN_ALPHA_NAME binary_args
// 		| . colonexpr
// binary_args ::= binary_args COMMA colonexpr | colonexpr
ParseResult Parser_ParseBinaryExpr(Parser parser, Int modeFlags)
{
	SmileList binaryExpr, tail;
	SmileObject expr, rvalue;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	ParseResult parseResult;

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_BINARYEXPR, SYNTAXROOT_KEYWORD, 0);
	FORWARD_CUSTOM_SYNTAX_RESULT(parseResult);

	parseResult = Parser_ParseColonExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

parseNextOperator:

	if (Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE)) {
		// If this is something like +=, then don't allow it to be consumed as a binary operator.
		return EXPR_RESULT(expr);
	}

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_BINARYEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_COLONEXPR);
	FORWARD_CHAINED_CUSTOM_SYNTAX_RESULT(parseResult);

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)
		&& Parser_IsAcceptableArbitraryBinaryOperator(parser, symbol = token->data.symbol)) {

		lexerPosition = Token_GetPosition(token);

		if ((modeFlags & COLONMODE_MASK) == COLONMODE_MEMBERDECL && Parser_HasLookahead(parser, TOKEN_COLON)) {
			Lexer_Unget(parser->lexer);
			return EXPR_RESULT(expr);
		}

		parseResult = Parser_ParseColonExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		binaryExpr = tail = SmileList_CreateOneWithSource(
			SmileList_CreateDotWithSource(expr, SmileSymbol_Create(symbol), lexerPosition), lexerPosition);

		expr = (SmileObject)binaryExpr;

		tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));

		if ((modeFlags & COMMAMODE_MASK) == COMMAMODE_NORMAL) {
			while (Lexer_Peek(parser->lexer) == TOKEN_COMMA) {
				Lexer_Next(parser->lexer);

				lexerPosition = Token_GetPosition(parser->lexer->token);

				parseResult = Parser_ParseColonExpr(parser, modeFlags);
				if (IS_PARSE_ERROR(parseResult))
					RETURN_PARSE_ERROR(parseResult);
				rvalue = parseResult.expr;

				tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));
			}
		}

		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return EXPR_RESULT(expr);
}

// colonexpr :: = . colonexpr COLON rangeexpr | . rangeexpr
ParseResult Parser_ParseColonExpr(Parser parser, Int modeFlags)
{
	SmileObject expr, rvalue;
	ParseResult parseResult;
	Token token;
	LexerPosition lexerPosition;

	parseResult = Parser_ParseRangeExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

	if ((modeFlags & COLONMODE_MASK) == COLONMODE_MEMBERDECL)
		return EXPR_RESULT(expr);

	while ((token = Parser_NextToken(parser))->kind == TOKEN_COLON
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseResult = Parser_ParseRangeExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		expr = (SmileObject)SmileList_CreateIndexWithSource(expr, rvalue, lexerPosition);
	}

	Lexer_Unget(parser->lexer);

	return EXPR_RESULT(expr);
}

// rangeexpr ::= . prefixexpr DOTDOT prefixexpr | . prefixexpr
ParseResult Parser_ParseRangeExpr(Parser parser, Int modeFlags)
{
	SmileObject expr, rvalue;
	ParseResult parseResult;
	Token token;
	LexerPosition lexerPosition;

	parseResult = Parser_ParsePrefixExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

	if ((token = Parser_NextToken(parser))->kind == TOKEN_DOTDOT
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseResult = Parser_ParsePrefixExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		expr = (SmileObject)SmileList_CreateTwoWithSource(
			SmileList_CreateDotWithSource(expr, Smile_KnownObjects.rangeToSymbol, lexerPosition),
			rvalue,
			lexerPosition
		);
	}
	else {
		Lexer_Unget(parser->lexer);
	}

	return EXPR_RESULT(expr);
}

Inline Bool Parser_IsAcceptableArbitraryPrefixOperator(Symbol symbol)
{
	switch (symbol) {
		default:
			return True;

		case SMILE_SPECIAL_SYMBOL_VAR:
		case SMILE_SPECIAL_SYMBOL_CONST:
		case SMILE_SPECIAL_SYMBOL_AUTO:

		case SMILE_SPECIAL_SYMBOL_NOT:

		case SMILE_SPECIAL_SYMBOL_NEW:
		case SMILE_SPECIAL_SYMBOL_TYPEOF:
		case SMILE_SPECIAL_SYMBOL_SUPEREQ:
		case SMILE_SPECIAL_SYMBOL_SUPERNE:
			return False;
	}
}

// prefixexpr ::= . UNKNOWN_PUNCT_NAME prefixexpr | . UNKNOWN_ALPHA_NAME prefixexpr
// 		| . AND prefixexpr | . OR prefixexpr
// 		| . EQ prefixexpr | . NE prefixexpr
// 		| . SUPER_EQ prefixexpr | . SUPER_NE prefixexpr
// 		| . LE prefixexpr | . GE prefixexpr
// 		| . LT prefixexpr | . GT prefixexpr
// 		| . PLUS prefixexpr | . MINUS prefixexpr
// 		| . STAR prefixexpr | . SLASH prefixexpr
//      | . TYPEOF prefixexpr
// 		| . new
ParseResult Parser_ParsePrefixExpr(Parser parser, Int modeFlags)
{
	LexerPosition position;
	ParseResult parseResult;
	SmileObject expr;
	Token token, firstUnaryTokenForErrorReporting = NULL;
	struct TokenStruct stackOperators[16];					// The first 16 unary operators will go on C's stack.
	struct TokenStruct *unaryOperators = stackOperators;	// Any successive unary operators will be reallocated on the heap.
	struct TokenStruct *tempOperators;
	Int numOperators = 0, maxOperators = 16, newMax, i;
	Symbol symbol;
	Symbol lastUnaryTokenSymbol;

	// Because this rule is right-recursive, and we don't want to recurse wherever we can
	// avoid it, we loop to collect unary operators, and then parse the 'new' expression,
	// and then build up the same tree of unary invocations we would have built recursively.

	// Collect the first unary prefix operator.
	if ((((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
			&& Parser_IsAcceptableArbitraryPrefixOperator(token->data.symbol))
		|| (token->kind == TOKEN_ALPHANAME && token->data.symbol == SMILE_SPECIAL_SYMBOL_TYPEOF)) {

		// Record which symbol came last, for error-reporting.
		lastUnaryTokenSymbol = token->data.symbol;

		MemCpy(unaryOperators + numOperators++, token, sizeof(struct TokenStruct));
		firstUnaryTokenForErrorReporting = &unaryOperators[0];

		// Collect up any successive unary prefix operators in the unaryOperators array.
		while (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME
				|| (token->kind == TOKEN_ALPHANAME && token->data.symbol == SMILE_SPECIAL_SYMBOL_TYPEOF))
			&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

			lastUnaryTokenSymbol = token->data.symbol;

			if (numOperators >= maxOperators) {
				newMax = maxOperators * 2;
				tempOperators = GC_MALLOC_STRUCT_ARRAY(struct TokenStruct, newMax);
				MemCpy(tempOperators, unaryOperators, maxOperators * sizeof(struct TokenStruct));
				maxOperators = newMax;
				unaryOperators = tempOperators;
			}

			MemCpy(unaryOperators + numOperators++, token, sizeof(struct TokenStruct));
		}

		// The construct after the unary operator cannot be moved to a new line if we're not
		// wrapped in a safe construct like parentheses; this causes trouble for the expected
		// behavior of lower-precedence constructs like statement keywords.  This requirement
		// for unary terms matches the binary-line-break rule; just as "x - \n y" is illegal,
		// "- \n y" is also illegal.
		if (token->isFirstContentOnLine && (modeFlags & BINARYLINEBREAKS_MASK) != BINARYLINEBREAKS_ALLOWED) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Expected an expression term on the same line after unary operator '%S'",
					SymbolTable_GetName(Smile_SymbolTable, lastUnaryTokenSymbol))));
		}
	}

	Lexer_Unget(parser->lexer);

	// We now have all of the unary prefix operators.  Now go parse the term itself.
	parseResult = Parser_ParseNewExpr(parser, modeFlags, firstUnaryTokenForErrorReporting);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

	// If there were no unary operators, just return the term.
	if (numOperators <= 0)
		return EXPR_RESULT(expr);

	// We have unary operators to apply.  So spin out Smile expressions of the form [(expr.unary)] for
	// each unary operator, going from last (innermost) to first (outermost).
	for (i = numOperators - 1; i >= 0; i--) {
		position = Token_GetPosition(&unaryOperators[i]);
		symbol = unaryOperators[i].data.symbol;
		if (symbol == SMILE_SPECIAL_SYMBOL_TYPEOF) {
			// Typeof is a special built-in form:  [$typeof x]
			expr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._typeofSymbol, expr, position);
		}
		else {
			// Construct an expression of the unary method-call form:  [(expr.unary)]
			expr = (SmileObject)SmileList_CreateOneWithSource(
				SmileList_CreateDotWithSource(expr, (SmileObject)SmileSymbol_Create(symbol), position),
				position
			);
		}
	}

	return EXPR_RESULT(expr);
}

ParseResult Parser_ParsePostfixExpr(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	ParseResult parseResult;

	parseResult = Parser_ApplyCustomSyntax(parser, modeFlags, SMILE_SPECIAL_SYMBOL_POSTFIXEXPR, SYNTAXROOT_KEYWORD, 0);
	FORWARD_CUSTOM_SYNTAX_RESULT(parseResult);

	return Parser_ParseConsExpr(parser, modeFlags, firstUnaryTokenForErrorReporting);
}

// consexpr ::= . dotexpr DOUBLEHASH consexpr | . dotexpr
ParseResult Parser_ParseConsExpr(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	SmileList tail;
	SmileObject expr, rvalue, nextRValue;
	ParseResult parseResult;
	LexerPosition lexerPosition;
	Bool isFirst;

	parseResult = Parser_ParseDotExpr(parser, modeFlags, firstUnaryTokenForErrorReporting);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

	if (Lexer_Next(parser->lexer) == TOKEN_DOUBLEHASH) {

		lexerPosition = Token_GetPosition(parser->lexer->token);
		tail = SmileList_CreateOneWithSource(expr, lexerPosition);
		expr = (SmileObject)tail;
		isFirst = True;
		rvalue = NullObject;

		do {
			parseResult = Parser_ParseDotExpr(parser, modeFlags, firstUnaryTokenForErrorReporting);
			if (IS_PARSE_ERROR(parseResult))
				RETURN_PARSE_ERROR(parseResult);
			nextRValue = parseResult.expr;

			if (!isFirst) {
				tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));
			}

			rvalue = nextRValue;
			isFirst = False;

		} while (Lexer_Next(parser->lexer) == TOKEN_DOUBLEHASH);

		tail->d = rvalue;
	}

	Lexer_Unget(parser->lexer);

	return EXPR_RESULT(expr);
}

// dotexpr ::= . dotexpr DOT any_name | . term
ParseResult Parser_ParseDotExpr(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	ParseResult parseResult;
	SmileObject expr;
	LexerPosition lexerPosition;
	Symbol symbol;
	Token token;

	parseResult = Parser_ParseTerm(parser, modeFlags, firstUnaryTokenForErrorReporting);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	expr = parseResult.expr;

	while (Parser_NextToken(parser)->kind == TOKEN_DOT) {

		if ((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME
			|| token->kind == TOKEN_UNKNOWNALPHANAME
			|| token->kind == TOKEN_PUNCTNAME
			|| token->kind == TOKEN_UNKNOWNPUNCTNAME) {

			symbol = token->data.symbol;
			lexerPosition = Token_GetPosition(token);

			expr = (SmileObject)SmileList_CreateDotWithSource(expr, (SmileObject)SmileSymbol_Create(symbol), lexerPosition);
		}
		else {
			Lexer_Unget(parser->lexer);
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_Format("Expected a property name after '.', not '%S.'", TokenKind_ToString(token->kind))));
		}
	}

	Lexer_Unget(parser->lexer);
	return EXPR_RESULT(expr);
}

//  term ::= . LPAREN expr RPAREN
ParseResult Parser_ParseParentheses(Parser parser, Int modeFlags)
{
	LexerPosition startPosition;
	ParseResult parseResult;
	SmileObject expr;

	UNUSED(modeFlags);

	// Expect an initial '('; if it's not there, this is a programming error.
	if (Lexer_Next(parser->lexer) != TOKEN_LEFTPARENTHESIS)
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_FATAL, Token_GetPosition(parser->lexer->token),
			String_FromC("Expected '(' as first token in Parser_ParseParentheses().")));

	startPosition = Token_GetPosition(parser->lexer->token);

	// Parse the inside of the '(...)' block as an expression, with binary line-breaks allowed.
	parseResult = Parser_ParseExpr(parser, BINARYLINEBREAKS_ALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (IS_PARSE_ERROR(parseResult)) {
		// Handle any errors generated inside the expression parse by recovering here, and then
		// telling the caller everything was successful so that it continues trying the parse.
		HANDLE_PARSE_ERROR(parser, parseResult);
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		return RECOVERY_RESULT();
	}
	expr = parseResult.expr;

	// Make sure there's a matching ')' following the opening '('.
	if (!Parser_HasLookahead(parser, TOKEN_RIGHTPARENTHESIS)) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition,
			String_Format("Missing ')' after expression starting on line %d.", startPosition->line)));
	}
	Parser_NextToken(parser);

	// No errors, yay!
	return EXPR_RESULT(expr);
}
