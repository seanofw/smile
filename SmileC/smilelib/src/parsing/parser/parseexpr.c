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
#include <smile/smiletypes/smilepair.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

//-------------------------------------------------------------------------------------------------
// Base expression parsing

//  nonbreak_expr ::= . expr    // Explicitly in a nonbreak_expr, binary operators cannot be matched
//									if they are the first non-whitespace on a line.  This behavior is
//									disabled at the end of the nonbreak_expr, whenever any [], (), or {}
//									grouping is entered, or whenever we are parsing inside the first
//									expr/arith of an if_then or a do-while.
//
//  expr ::= . base_expr
ParseError Parser_ParseExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	return Parser_ParseStmt(parser, expr, modeFlags);
}

//  base_expr ::= . arith
//         | . var_decl
//         | . scope
//         | . return
//         | . INCLUDE string
//         | . INSERT_BRK base_expr
//         | . INSERT_UNDEFINE any_name
ParseError Parser_ParseStmt(Parser parser, SmileObject *expr, Int modeFlags)
{
	Token token;
	ParseError parseError;
	CustomSyntaxResult customSyntaxResult;

	switch ((token = Parser_NextToken(parser))->kind) {

		case TOKEN_LEFTBRACE:
			Lexer_Unget(parser->lexer);
			return Parser_ParseScope(parser, expr);

		case TOKEN_ALPHANAME:
		case TOKEN_UNKNOWNALPHANAME:
			switch (token->data.symbol) {
				case SMILE_SPECIAL_SYMBOL_VAR:
					return Parser_ParseVarDecls(parser, expr, modeFlags, PARSEDECL_VARIABLE);
				case SMILE_SPECIAL_SYMBOL_CONST:
					return Parser_ParseVarDecls(parser, expr, modeFlags, PARSEDECL_CONST);
				case SMILE_SPECIAL_SYMBOL_AUTO:
					return Parser_ParseVarDecls(parser, expr, modeFlags, PARSEDECL_AUTO);
			}
			// Fall through to default case if not a declaration.

		default:
			Lexer_Unget(parser->lexer);
			customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_STMT, SYNTAXROOT_KEYWORD, 0, &parseError);
			if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
				return parseError;

			return Parser_ParseOpEquals(parser, expr, (modeFlags & ~COMMAMODE_MASK) | COMMAMODE_NORMAL);
	}
}

// or :: = . or OR and | . and
ParseError Parser_ParseOr(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileList tail = NullList;
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	Bool isFirst = True;
	CustomSyntaxResult customSyntaxResult;

	customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_EXPR, SYNTAXROOT_KEYWORD, 0, &parseError);
	if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
		return parseError;

	parseError = Parser_ParseAnd(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	while (((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_OR
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseAnd(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		if (isFirst) {
			*expr = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)Smile_KnownObjects.orSymbol,
				(SmileObject)SmileList_ConsWithSource(*expr,
					(SmileObject)(tail = SmileList_ConsWithSource(rvalue,
						NullObject,
						lexerPosition)),
					lexerPosition),
				lexerPosition
			);
			isFirst = False;
		}
		else {
			tail = (SmileList)(tail->d = (SmileObject)SmileList_ConsWithSource(rvalue, NullObject, lexerPosition));
		}
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// and :: = . and AND not | . not
ParseError Parser_ParseAnd(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileList tail = NullList;
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	Bool isFirst = True;

	parseError = Parser_ParseNot(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	while (((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_AND
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseNot(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		if (isFirst) {
			*expr = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)Smile_KnownObjects.andSymbol,
				(SmileObject)SmileList_ConsWithSource(*expr,
					(SmileObject)(tail = SmileList_ConsWithSource(rvalue,
						NullObject,
						lexerPosition)),
					lexerPosition),
				lexerPosition
			);
			isFirst = False;
		}
		else {
			tail = (SmileList)(tail->d = (SmileObject)SmileList_ConsWithSource(rvalue, NullObject, lexerPosition));
		}
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// not :: = . NOT not | . cmpexpr
ParseError Parser_ParseNot(Parser parser, SmileObject *expr, Int modeFlags)
{
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	struct TokenStruct stackOperators[16];					// The first 16 unary operators will go on C's stack.
	struct TokenStruct *unaryOperators = stackOperators;	// Any successive unary operators will be reallocated on the heap.
	struct TokenStruct *tempOperators;
	Int numOperators = 0, maxOperators = 16, newMax, i;

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
	parseError = Parser_ParseCmpExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	// If there were no unary operators, just return the term.
	if (numOperators <= 0)
		return NULL;

	// We have unary operators to apply.  So spin out Smile expressions of the form [(expr.unary)] for
	// each unary operator, going from last (innermost) to first (outermost).
	for (i = numOperators - 1; i >= 0; i--) {
		lexerPosition = Token_GetPosition(&unaryOperators[i]);
		// Not is a special built-in form:  [not x]
		*expr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)Smile_KnownObjects.notSymbol,
			(SmileObject)SmileList_ConsWithSource(
				*expr,
				NullObject,
				lexerPosition
			),
			lexerPosition
		);
	}

	return NULL;
}

Inline SmileSymbol Parser_GetSymbolObjectForCmpOperator(Symbol symbol)
{
	switch (symbol) {
		case SMILE_SPECIAL_SYMBOL_EQ:
			return Smile_KnownObjects.eqSymbol;
			break;
		case SMILE_SPECIAL_SYMBOL_NE:
			return Smile_KnownObjects.neSymbol;
			break;
		case SMILE_SPECIAL_SYMBOL_LT:
			return Smile_KnownObjects.ltSymbol;
			break;
		case SMILE_SPECIAL_SYMBOL_GT:
			return Smile_KnownObjects.gtSymbol;
			break;
		case SMILE_SPECIAL_SYMBOL_LE:
			return Smile_KnownObjects.leSymbol;
			break;
		case SMILE_SPECIAL_SYMBOL_GE:
			return Smile_KnownObjects.geSymbol;
			break;
		case SMILE_SPECIAL_SYMBOL_SUPEREQ:
			return Smile_KnownObjects.supereqSymbol;
			break;
		case SMILE_SPECIAL_SYMBOL_SUPERNE:
			return Smile_KnownObjects.superneSymbol;
			break;
		case SMILE_SPECIAL_SYMBOL_IS:
			return Smile_KnownObjects.isSymbol;
			break;
		default:
			return NULL;
	}
}

// cmpexpr ::= . cmpexpr LT addexpr | . cmpexpr GT addexpr | . cmpexpr LE addexpr | . cmpexpr GE addexpr
//       | . cmpexpr EQ addexpr | . cmpexpr NE addexpr | . cmpexpr SUPEREQ addexpr | . cmpexpr SUPERNE addexpr
//       | . cmpexpr IS addexpr
//       | . addexpr
ParseError Parser_ParseCmpExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileObject rvalue;
	SmileSymbol symbolObject;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	CustomSyntaxResult customSyntaxResult;

	customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_CMPEXPR, SYNTAXROOT_KEYWORD, 0, &parseError);
	if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
		return parseError;

	parseError = Parser_ParseAddExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

parseNextOperator:

	if ((customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_CMPEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_ADDEXPR, &parseError))
		!= CustomSyntaxResult_NotMatchedAndNoTokensConsumed) {
		
		if (customSyntaxResult == CustomSyntaxResult_PartialApplicationWithError)
			return parseError;
	
		goto parseNextOperator;
	}

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNPUNCTNAME || token->kind == TOKEN_UNKNOWNALPHANAME
			|| token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_ALPHANAME)
		&& (symbolObject = Parser_GetSymbolObjectForCmpOperator(symbol = token->data.symbol)) != NULL
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseAddExpr(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		if (symbol == SMILE_SPECIAL_SYMBOL_SUPEREQ || symbol == SMILE_SPECIAL_SYMBOL_SUPERNE
			|| symbol == SMILE_SPECIAL_SYMBOL_IS) {
			*expr = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)symbolObject,
				(SmileObject)SmileList_ConsWithSource(*expr,
					(SmileObject)SmileList_ConsWithSource(rvalue,
						NullObject,
						lexerPosition),
					lexerPosition),
				lexerPosition
			);
		}
		else {
			*expr = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)SmilePair_Create(*expr, (SmileObject)symbolObject),
				(SmileObject)SmileList_ConsWithSource(rvalue,
					NullObject,
					lexerPosition),
				lexerPosition
			);
		}
		
		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// addexpr ::= . addexpr PLUS mulexpr | . addexpr MINUS mulexpr | . mulexpr
ParseError Parser_ParseAddExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	CustomSyntaxResult customSyntaxResult;

	customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_ADDEXPR, SYNTAXROOT_KEYWORD, 0, &parseError);
	if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
		return parseError;

	parseError = Parser_ParseMulExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

parseNextOperator:

	if ((customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_ADDEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_MULEXPR, &parseError))
		!= CustomSyntaxResult_NotMatchedAndNoTokensConsumed) {

		if (customSyntaxResult == CustomSyntaxResult_PartialApplicationWithError)
			return parseError;

		goto parseNextOperator;
	}

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNPUNCTNAME || token->kind == TOKEN_PUNCTNAME)
		&& ((symbol = token->data.symbol) == SMILE_SPECIAL_SYMBOL_PLUS || symbol == SMILE_SPECIAL_SYMBOL_MINUS)
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseMulExpr(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		*expr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)SmilePair_Create(*expr, symbol == SMILE_SPECIAL_SYMBOL_PLUS
				? (SmileObject)Smile_KnownObjects.plusSymbol
				: (SmileObject)Smile_KnownObjects.minusSymbol),
			(SmileObject)SmileList_ConsWithSource(rvalue,
				NullObject,
				lexerPosition),
			lexerPosition
		);

		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// mulexpr ::= . mulexpr STAR binary | . mulexpr SLASH binary | . binary
ParseError Parser_ParseMulExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	CustomSyntaxResult customSyntaxResult;

	customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_MULEXPR, SYNTAXROOT_KEYWORD, 0, &parseError);
	if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
		return parseError;

	parseError = Parser_ParseBinary(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

parseNextOperator:

	if ((customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_MULEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_BINARY, &parseError))
		!= CustomSyntaxResult_NotMatchedAndNoTokensConsumed) {

		if (customSyntaxResult == CustomSyntaxResult_PartialApplicationWithError)
			return parseError;

		goto parseNextOperator;
	}

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNPUNCTNAME || token->kind == TOKEN_PUNCTNAME)
		&& ((symbol = token->data.symbol) == SMILE_SPECIAL_SYMBOL_STAR || symbol == SMILE_SPECIAL_SYMBOL_SLASH)
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseBinary(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		*expr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)SmilePair_Create(*expr, symbol == SMILE_SPECIAL_SYMBOL_STAR
				? (SmileObject)Smile_KnownObjects.starSymbol
				: (SmileObject)Smile_KnownObjects.slashSymbol),
			(SmileObject)SmileList_ConsWithSource(rvalue,
				NullObject,
				lexerPosition),
			lexerPosition
		);

		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return NULL;
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

// binary ::= . binary UNKNOWN_PUNCT_NAME binary_args
// 		| . binary UNKNOWN_ALPHA_NAME binary_args
// 		| . range
// binary_args ::= binary_args COMMA range | range
ParseError Parser_ParseBinary(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileList binaryExpr, tail;
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	CustomSyntaxResult customSyntaxResult;

	customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_BINARY, SYNTAXROOT_KEYWORD, 0, &parseError);
	if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
		return parseError;

	parseError = Parser_ParseColon(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

parseNextOperator:

	if ((customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_BINARY, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_COLON_NAME, &parseError))
		!= CustomSyntaxResult_NotMatchedAndNoTokensConsumed) {

		if (customSyntaxResult == CustomSyntaxResult_PartialApplicationWithError)
			return parseError;

		goto parseNextOperator;
	}

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)
		&& Parser_IsAcceptableArbitraryBinaryOperator(parser, symbol = token->data.symbol)) {

		lexerPosition = Token_GetPosition(token);

		if ((modeFlags & COLONMODE_MASK) == COLONMODE_MEMBERDECL && Parser_HasLookahead(parser, TOKEN_COLON)) {
			Lexer_Unget(parser->lexer);
			return NULL;
		}

		parseError = Parser_ParseColon(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		binaryExpr = tail = SmileList_ConsWithSource(
			(SmileObject)SmilePair_CreateWithSource(*expr, (SmileObject)SmileSymbol_Create(symbol), lexerPosition),
			NullObject,
			lexerPosition
		);

		*expr = (SmileObject)binaryExpr;

		tail = (SmileList)(tail->d = (SmileObject)SmileList_ConsWithSource(rvalue, NullObject, lexerPosition));

		if ((modeFlags & COMMAMODE_MASK) == COMMAMODE_NORMAL) {
			while (Lexer_Peek(parser->lexer) == TOKEN_COMMA) {
				Lexer_Next(parser->lexer);

				lexerPosition = Token_GetPosition(parser->lexer->token);

				parseError = Parser_ParseColon(parser, &rvalue, modeFlags);
				if (parseError != NULL)
					return parseError;

				tail = (SmileList)(tail->d = (SmileObject)SmileList_ConsWithSource(rvalue, NullObject, lexerPosition));
			}
		}

		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// colon :: = . colon COLON range | . range
ParseError Parser_ParseColon(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;

	parseError = Parser_ParseRange(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	if ((modeFlags & COLONMODE_MASK) == COLONMODE_MEMBERDECL)
		return NULL;

	while ((token = Parser_NextToken(parser))->kind == TOKEN_COLON
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseRange(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		*expr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)SmilePair_Create(*expr, (SmileObject)Smile_KnownObjects.getMemberSymbol),
			(SmileObject)SmileList_ConsWithSource(rvalue,
				NullObject,
				lexerPosition),
			lexerPosition
		);
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// range ::= . unary RANGE unary | . unary
ParseError Parser_ParseRange(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;

	parseError = Parser_ParseUnary(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	if ((token = Parser_NextToken(parser))->kind == TOKEN_RANGE
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseUnary(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		*expr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)SmilePair_Create((SmileObject)Smile_KnownObjects.RangeSymbol, (SmileObject)Smile_KnownObjects.ofSymbol),
			(SmileObject)SmileList_ConsWithSource(*expr,
				(SmileObject)SmileList_ConsWithSource(rvalue,
					NullObject,
					lexerPosition),
				lexerPosition),
			lexerPosition
		);
	}
	else {
		Lexer_Unget(parser->lexer);
	}

	return NULL;
}

Inline Bool Parser_IsAcceptableArbitraryUnaryOperator(Symbol symbol)
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

		case SMILE_SPECIAL_SYMBOL_BRK:
			return False;
	}
}

// unary ::= . UNKNOWN_PUNCT_NAME unary | . UNKNOWN_ALPHA_NAME unary
// 		| . AND unary | . OR unary
// 		| . EQ unary | . NE unary
// 		| . SUPER_EQ unary | . SUPER_NE unary
// 		| . LE unary | . GE unary
// 		| . LT unary | . GT unary
// 		| . PLUS unary | . MINUS unary
// 		| . STAR unary | . SLASH unary
//      | . TYPEOF unary
// 		| . new
ParseError Parser_ParseUnary(Parser parser, SmileObject *expr, Int modeFlags)
{
	LexerPosition position;
	ParseError parseError;
	Token token, firstUnaryTokenForErrorReporting = NULL;
	struct TokenStruct stackOperators[16];					// The first 16 unary operators will go on C's stack.
	struct TokenStruct *unaryOperators = stackOperators;	// Any successive unary operators will be reallocated on the heap.
	struct TokenStruct *tempOperators;
	Int numOperators = 0, maxOperators = 16, newMax, i;
	Symbol symbol;

	// Because this rule is right-recursive, and we don't want to recurse wherever we can
	// avoid it, we loop to collect unary operators, and then parse the 'new' expression,
	// and then build up the same tree of unary invocations we would have built recursively.

	// Collect the first unary prefix operator.
	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
		&& Parser_IsAcceptableArbitraryUnaryOperator(token->data.symbol)) {

		MemCpy(unaryOperators + numOperators++, token, sizeof(struct TokenStruct));
		firstUnaryTokenForErrorReporting = &unaryOperators[0];

		// Collect up any successive unary prefix operators in the unaryOperators array.
		while ((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME) {
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
	parseError = Parser_ParseNew(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
	if (parseError != NULL)
		return parseError;

	// If there were no unary operators, just return the term.
	if (numOperators <= 0)
		return NULL;

	// We have unary operators to apply.  So spin out Smile expressions of the form [(expr.unary)] for
	// each unary operator, going from last (innermost) to first (outermost).
	for (i = numOperators - 1; i >= 0; i--) {
		position = Token_GetPosition(&unaryOperators[i]);
		symbol = unaryOperators[i].data.symbol;
		if (symbol == SMILE_SPECIAL_SYMBOL_TYPEOF) {
			// Typeof is a special built-in form:  [typeof x]
			*expr = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)Smile_KnownObjects.typeofSymbol,
				(SmileObject)SmileList_ConsWithSource(
					*expr,
					NullObject,
					position
				),
				position
			);
		}
		else {
			// Construct an expression of the unary method-call form:  [(expr.unary)]
			*expr = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)SmilePair_CreateWithSource(
					*expr,
					(SmileObject)SmileSymbol_Create(symbol),
					position
				),
				NullObject,
				position
			);
		}
	}
	return NULL;
}

// new ::= . NEW LBRACE members_opt RBRACE
// 		| . NEW dot LBRACE members_opt RBRACE
// 		| . doublehash
ParseError Parser_ParseNew(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	Token token, newToken;
	SmileObject base, body;
	ParseError parseError;
	LexerPosition newTokenPosition;

	token = Parser_NextToken(parser);
	if (!((token->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == Smile_KnownSymbols.new_)) {
		Lexer_Unget(parser->lexer);
		return Parser_ParsePostfix(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
	}

	newTokenPosition = Token_GetPosition(token);

	newToken = Token_Clone(token);

	if (Parser_HasLookahead(parser, TOKEN_LEFTBRACE)) {
		base = (SmileObject)Smile_KnownObjects.ObjectSymbol;
	}
	else {
		parseError = Parser_ParseDot(parser, &base, modeFlags, newToken);
		if (parseError != NULL) {
			token = Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
			if (token->kind != TOKEN_LEFTBRACE) {
				*expr = NullObject;
				return parseError;
			}
			Parser_AddMessage(parser, parseError);
		}
	}

	if (Lexer_Next(parser->lexer) != TOKEN_LEFTBRACE) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_FromC("Missing a '{' after 'new'."));
		*expr = NullObject;
		return parseError;
	}

	if (!Parser_ParseMembers(parser, &body)) {
		token = Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		if (token->kind != TOKEN_RIGHTBRACE) {
			parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Missing a '}' to end the members in the 'new' block starting on line %d.", newTokenPosition->line));
			*expr = NullObject;
			return parseError;
		}
		Lexer_Unget(parser->lexer);
	}

	if (Lexer_Next(parser->lexer) != TOKEN_RIGHTBRACE) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_Format("Missing a '}' to end the members in the 'new' block starting on line %d.", newTokenPosition->line));
		*expr = NullObject;
		return parseError;
	}

	*expr = (SmileObject)SmileList_ConsWithSource(
		(SmileObject)Smile_KnownObjects.newSymbol,
		(SmileObject)SmileList_ConsWithSource(
			base,
			(SmileObject)SmileList_ConsWithSource(
				body,
				NullObject,
				newTokenPosition
			),
			newTokenPosition
		),
		newTokenPosition
	);

	return NULL;
}

static Int Parser_RightBracesColons_Recovery[] = {
	TOKEN_RIGHTBRACE, TOKEN_COLON,
};
static Int Parser_RightBracesColons_Count = sizeof(Parser_RightBracesColons_Recovery) / sizeof(Int);

// members_opt :: = . members | .
// members :: = . members member | . member
// member :: = . name COLON expr
Bool Parser_ParseMembers(Parser parser, SmileObject *expr)
{
	SmileList head = NullList, tail = NullList;
	Token token;
	ParseError parseError;
	Symbol symbol;
	SmileObject valueExpr;
	SmileObject memberExpr;
	LexerPosition lexerPosition;
	Bool hasErrors = False;

	while ((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME
		|| token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_UNKNOWNPUNCTNAME) {
		symbol = token->data.symbol;
		lexerPosition = Token_GetPosition(token);

		if (Lexer_Next(parser->lexer) != TOKEN_COLON) {
			parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Missing ':' after '%S' member.", SymbolTable_GetName(Smile_SymbolTable, symbol)));
			Parser_AddMessage(parser, parseError);
			if (Parser_Recover(parser, Parser_RightBracesColons_Recovery, Parser_RightBracesColons_Count)->kind != TOKEN_COLON) {
				Lexer_Unget(parser->lexer);
				*expr = NullObject;
				return False;
			}
		}

		parseError = Parser_ParseExpr(parser, &valueExpr, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERDECL);
		if (parseError != NULL) {
			Parser_AddMessage(parser, parseError);
			hasErrors = True;
		}

		memberExpr = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)SmileSymbol_Create(symbol),
			(SmileObject)SmileList_ConsWithSource(valueExpr, NullObject, lexerPosition),
			lexerPosition
		);

		LIST_APPEND_WITH_SOURCE(head, tail, memberExpr, lexerPosition);
	}

	Lexer_Unget(parser->lexer);

	*expr = (SmileObject)head;
	return True;
}

ParseError Parser_ParsePostfix(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	ParseError parseError;
	CustomSyntaxResult customSyntaxResult;

	customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_POSTFIX, SYNTAXROOT_KEYWORD, 0, &parseError);
	if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
		return parseError;

	return Parser_ParseDoubleHash(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
}

// doublehash ::= . dot DOUBLEHASH doublehash | . dot
ParseError Parser_ParseDoubleHash(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	SmileList tail;
	SmileObject rvalue, nextRValue;
	ParseError parseError;
	LexerPosition lexerPosition;
	Bool isFirst;

	parseError = Parser_ParseDot(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
	if (parseError != NULL)
		return parseError;

	if (Lexer_Next(parser->lexer) == TOKEN_DOUBLEHASH) {

		lexerPosition = Token_GetPosition(parser->lexer->token);
		tail = SmileList_ConsWithSource(*expr, NullObject, lexerPosition);
		*expr = (SmileObject)tail;
		isFirst = True;
		rvalue = NullObject;

		do {
			parseError = Parser_ParseDot(parser, &nextRValue, modeFlags, firstUnaryTokenForErrorReporting);
			if (parseError != NULL)
				return parseError;

			if (!isFirst) {
				tail = (SmileList)(tail->d = (SmileObject)SmileList_ConsWithSource(rvalue, NullObject, lexerPosition));
			}

			rvalue = nextRValue;
			isFirst = False;

		} while (Lexer_Next(parser->lexer) == TOKEN_DOUBLEHASH);

		tail->d = rvalue;
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// dot ::= . dot DOT any_name | . term
ParseError Parser_ParseDot(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	ParseError parseError;
	Int tokenKind;
	LexerPosition lexerPosition;
	Symbol symbol;

	parseError = Parser_ParseTerm(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
	if (parseError != NULL)
		return parseError;

	while (Parser_NextToken(parser)->kind == TOKEN_DOT) {

		if ((tokenKind = Lexer_Next(parser->lexer)) == TOKEN_ALPHANAME
			|| tokenKind == TOKEN_UNKNOWNALPHANAME
			|| tokenKind == TOKEN_PUNCTNAME
			|| tokenKind == TOKEN_UNKNOWNPUNCTNAME) {

			symbol = parser->lexer->token->data.symbol;
			lexerPosition = Token_GetPosition(parser->lexer->token);

			*expr = (SmileObject)SmilePair_CreateWithSource(*expr, (SmileObject)SmileSymbol_Create(symbol), lexerPosition);
		}
		else {
			return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Expected a property name after '.', not '%S.'", TokenKind_ToString(tokenKind)));
		}
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

//  term ::= . LPAREN expr RPAREN
ParseError Parser_ParseParentheses(Parser parser, SmileObject *result, Int modeFlags)
{
	LexerPosition startPosition;
	ParseError error;

	UNUSED(modeFlags);

	// Expect an initial '('; if it's not there, this is a programming error.
	if (Lexer_Next(parser->lexer) != TOKEN_LEFTPARENTHESIS) {
		Parser_AddFatalError(parser, Token_GetPosition(parser->lexer->token), "Expected '(' as first token in Parser_ParseParentheses().");
		*result = NullObject;
		return NULL;
	}

	startPosition = Token_GetPosition(parser->lexer->token);

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
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition,
			String_Format("Missing ')' after expression starting on line %d.", startPosition->line));
		*result = NullObject;
		return error;
	}
	Parser_NextToken(parser);

	// No errors, yay!
	return NULL;
}
