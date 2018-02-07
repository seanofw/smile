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
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static struct SmileListInt _parser_ignorableObject = { 0 };

// This object is used to identify constructs that may safely be elided from the parser's output.
SmileObject Parser_IgnorableObject = (SmileObject)&_parser_ignorableObject;

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

		case TOKEN_LOANWORD_INCLUDE:
			return Parser_ParseInclude(parser, expr);

		case TOKEN_ALPHANAME:
		case TOKEN_UNKNOWNALPHANAME:
			switch (token->data.symbol) {
				case SMILE_SPECIAL_SYMBOL_VAR:
					return Parser_ParseVarDecls(parser, expr, modeFlags, PARSEDECL_VARIABLE);
				case SMILE_SPECIAL_SYMBOL_CONST:
					return Parser_ParseVarDecls(parser, expr, modeFlags, PARSEDECL_CONST);
				case SMILE_SPECIAL_SYMBOL_AUTO:
					return Parser_ParseVarDecls(parser, expr, modeFlags, PARSEDECL_AUTO);
				case SMILE_SPECIAL_SYMBOL_KEYWORD:
					return Parser_ParseKeywordList(parser, expr);
				case SMILE_SPECIAL_SYMBOL_IF:
					return Parser_ParseIfUnless(parser, expr, modeFlags, False, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_UNLESS:
					return Parser_ParseIfUnless(parser, expr, modeFlags, True, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_DO:
					return Parser_ParseDo(parser, expr, modeFlags, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_WHILE:
					return Parser_ParseWhileUntil(parser, expr, modeFlags, False, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_UNTIL:
					return Parser_ParseWhileUntil(parser, expr, modeFlags, True, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_RETURN:
					return Parser_ParseReturn(parser, expr, modeFlags, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_TILL:
					return Parser_ParseTill(parser, expr, modeFlags, Token_GetPosition(token));
				case SMILE_SPECIAL_SYMBOL_TRY:
					return Parser_ParseTry(parser, expr, modeFlags, Token_GetPosition(token));
			}
			// Fall through to default case if not a special form.

		default:
			Lexer_Unget(parser->lexer);
			customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_STMT, SYNTAXROOT_KEYWORD, 0, &parseError);
			if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
				return parseError;

			return Parser_ParseOpEquals(parser, expr, (modeFlags & ~COMMAMODE_MASK) | COMMAMODE_NORMAL);
	}
}

// if_then ::= IF . arith THEN expr
//           | IF . arith THEN expr ELSE expr
//           | UNLESS . arith THEN expr
//           | UNLESS . arith THEN expr ELSE expr
ParseError Parser_ParseIfUnless(Parser parser, SmileObject *expr, Int modeFlags, Bool invert, LexerPosition lexerPosition)
{
	ParseError parseError;
	SmileObject condition;
	SmileObject thenBody, elseBody;
	Token token;

	// Parse the condition.
	parseError = Parser_ParseOpEquals(parser, &condition, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (parseError != NULL) {
		// Bad condition.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_THEN) {

			// Bad condition, but we recovered to a 'then', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			condition = NullObject;
		}
		else {
			*expr = NullObject;
			return parseError;
		}
	}

	// Consume the 'then' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_THEN)) {
		// Missing 'then' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'then' keyword after '%s'.", invert ? "unless" : "if");
		Lexer_Unget(parser->lexer);
	}

	// Parse the then-body.
	parseError = Parser_ParseExpr(parser, &thenBody, modeFlags);
	if (parseError != NULL) {
		// Bad then-body.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_ELSE) {

			// Bad condition, but we recovered to an 'else', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			condition = NullObject;
		}
		else {
			*expr = NullObject;
			return parseError;
		}
	}

	// Consume an optional 'else' keyword.
	if (((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_ELSE) {

		// Parse the else-body.
		parseError = Parser_ParseExpr(parser, &elseBody, modeFlags);
		if (parseError != NULL) {
			// No recovery past this point; there are no more tokens we can consume in the grammar.
			*expr = NullObject;
			return parseError;
		}
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
		*expr = (SmileObject)SmileList_CreateFourWithSource(Smile_KnownObjects._ifSymbol, condition, thenBody, elseBody, lexerPosition);
	}
	else {
		// Make an [$if cond then] construct.
		*expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._ifSymbol, condition, thenBody, lexerPosition);
	}
	return NULL;
}

// do_while ::= DO . expr WHILE arith
//            | DO . expr UNTIL arith
ParseError Parser_ParseDo(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition)
{
	ParseError parseError;
	SmileObject condition;
	SmileObject body;
	Token token;
	Bool invert;

	// Parse the body.
	parseError = Parser_ParseExpr(parser, &body, modeFlags);
	if (parseError != NULL) {
		// Bad body.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& (recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_WHILE || recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_UNTIL)) {

			// Bad condition, but we recovered to a 'while' or an 'until', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			condition = NullObject;
		}
		else {
			*expr = NullObject;
			return parseError;
		}
	}

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
	parseError = Parser_ParseOpEquals(parser, &condition, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (parseError != NULL) {
		// No recovery past this point; there are no more tokens we can consume in the grammar.
		*expr = NullObject;
		return parseError;
	}

	// If we're an 'until' form, add a [$not] around the condition.
	if (invert) {
		condition = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._notSymbol, condition, lexerPosition);
	}

	// Now make the resulting form: [$while body condition null]
	*expr = (SmileObject)SmileList_CreateFourWithSource(Smile_KnownObjects._whileSymbol, body, condition, NullObject, lexerPosition);
	return NULL;
}

// while_do ::= WHILE . arith DO expr
//            | UNTIL . arith DO expr
ParseError Parser_ParseWhileUntil(Parser parser, SmileObject *expr, Int modeFlags, Bool invert, LexerPosition lexerPosition)
{
	ParseError parseError;
	SmileObject condition;
	SmileObject body;
	Token token;

	// Parse the condition.
	parseError = Parser_ParseOpEquals(parser, &condition, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (parseError != NULL) {
		// Bad condition.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_DO) {

			// Bad condition, but we recovered to a 'do', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			condition = NullObject;
		}
		else {
			*expr = NullObject;
			return parseError;
		}
	}

	// Consume the 'do' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_DO)) {
		// Missing 'do' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'do' keyword after '%s'.", invert ? "until" : "while");
		Lexer_Unget(parser->lexer);
	}

	// Parse the then-body.
	parseError = Parser_ParseExpr(parser, &body, modeFlags);
	if (parseError != NULL) {
		// No recovery past this point; there are no more tokens we can consume in the grammar.
		*expr = NullObject;
		return parseError;
	}

	// If we're an 'until' form, add a [$not] around the condition.
	if (invert) {
		condition = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._notSymbol, condition, lexerPosition);
	}

	// Now make the resulting form: [$while condition body]
	*expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._whileSymbol, condition, body, lexerPosition);
	return NULL;
}

// return ::= RETURN . arith
ParseError Parser_ParseReturn(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition)
{
	ParseError parseError;
	SmileObject result;

	// Parse the result.
	parseError = Parser_ParseOpEquals(parser, &result, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (parseError != NULL) {
		*expr = NullObject;
		return parseError;
	}

	// Make a [$return result] construct.
	*expr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._returnSymbol, result, lexerPosition);
	return NULL;
}

// try_catch ::= TRY . expr CATCH func
ParseError Parser_ParseTry(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition)
{
	ParseError parseError;
	SmileObject handler;
	SmileObject body;
	Token token;
	Int tokenKind;

	// Parse the body.
	parseError = Parser_ParseExpr(parser, &body, (modeFlags & ~BINARYLINEBREAKS_MASK) | BINARYLINEBREAKS_ALLOWED);
	if (parseError != NULL) {
		// Bad condition.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_CATCH) {

			// Bad body, but we recovered to a 'catch', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			body = NullObject;
		}
		else {
			*expr = NullObject;
			return parseError;
		}
	}

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
		parseError = Parser_ParseFunc(parser, &handler, modeFlags);
		if (parseError != NULL) {
			// No recovery past this point; there are no more tokens we can consume in the grammar.
			*expr = NullObject;
			return parseError;
		}
	}
	else if (tokenKind == TOKEN_LEFTBRACKET) {
		// Might be [$fn ...], so try to parse that.
		Lexer_Unget(parser->lexer);
		parseError = Parser_ParseTerm(parser, &handler, modeFlags, NULL);
		if (parseError != NULL) {
			// No recovery past this point; there are no more tokens we can consume in the grammar.
			*expr = NullObject;
			return parseError;
		}
		if (SMILE_KIND(handler) != SMILE_KIND_LIST
			|| SMILE_KIND(LIST_FIRST((SmileList)handler)) != SMILE_KIND_SYMBOL
			|| ((SmileSymbol)LIST_FIRST((SmileList)handler))->symbol != SMILE_SPECIAL_SYMBOL__FN) {
			// No recovery past this point; there are no more tokens we can consume in the grammar.
			Lexer_Unget(parser->lexer);
			*expr = NullObject;
			return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("'catch' handler must be a function."));
		}
	}
	else {
		// No recovery past this point; there are no more tokens we can consume in the grammar.
		Lexer_Unget(parser->lexer);
		*expr = NullObject;
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("'catch' handler must be a function."));
	}

	// Now make the resulting form: [$catch body handler]
	*expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._catchSymbol, body, handler, lexerPosition);
	return NULL;
}

// till_do ::= TILL . till_names DO expr whens_opt
ParseError Parser_ParseTill(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition)
{
	ParseError parseError;
	Int32Int32Dict tillFlags;
	SmileList names;
	SmileObject body;
	Token token;
	SmileObject whens;

	Parser_BeginScope(parser, PARSESCOPE_TILLDO);
	tillFlags = parser->currentScope->symbolDict;

	// Parse the till-names first.
	if ((parseError = Parser_ParseTillNames(parser, &names)) != NULL) {
		// Bad flags.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_DO) {

			// Bad names, but we recovered to a 'do', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			names = NullList;
		}
		else {
			Parser_EndScope(parser);
			*expr = NullObject;
			return parseError;
		}
	}

	// Consume the 'do' keyword.
	if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_DO)) {
		// Missing 'do' keyword.
		// We assume that's an error of omission, so we just rewind back a token and then try to keep going.
		Parser_AddError(parser, Token_GetPosition(token), "Missing 'do' keyword after 'till'.");
		Lexer_Unget(parser->lexer);
	}

	// Parse the loop body.
	parseError = Parser_ParseExpr(parser, &body, modeFlags);
	if (parseError != NULL) {
		// Bad flags.
		Token recoveryToken = Parser_Recover(parser, Parser_BracesBracketsParenthesesBarName_Recovery,
			Parser_BracesBracketsParenthesesBarName_Count);
		if ((recoveryToken->kind == TOKEN_ALPHANAME || recoveryToken->kind == TOKEN_UNKNOWNALPHANAME)
			&& recoveryToken->data.symbol == SMILE_SPECIAL_SYMBOL_WHEN) {

			// Bad body, but we recovered to a 'when', so try to keep going.
			Parser_NextToken(parser);
			Parser_AddMessage(parser, parseError);
			names = NullList;
		}
		else {
			Parser_EndScope(parser);
			*expr = NullObject;
			return parseError;
		}
	}

	Parser_EndScope(parser);

	// Parse any 'when' clauses hanging off the bottom of the loop.
	parseError = Parser_ParseWhens(parser, &whens, tillFlags, modeFlags);
	if (parseError != NULL) {
		Parser_AddMessage(parser, parseError);
		whens = NullObject;
	}

	// Now make the resulting form: [$till flags body whens]
	if (SMILE_KIND(whens) != SMILE_KIND_NULL) {
		*expr = (SmileObject)SmileList_CreateFourWithSource(Smile_KnownObjects._tillSymbol, names, body, whens, lexerPosition);
	}
	else {
		*expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._tillSymbol, names, body, lexerPosition);
	}
	return NULL;
}

// till_names ::= . anyname | . till_names COMMA anyname
ParseError Parser_ParseTillNames(Parser parser, SmileList *names)
{
	SmileObject decl;
	ParseError error;
	SmileList head, tail;

	// Parse the first name, which results in a symbol like 'x'.
	error = Parser_ParseTillName(parser, &decl);
	if (error != NULL) return error;

	// Wrap it in a list, so it becomes [x].
	LIST_INIT(head, tail);
	if (decl->kind != SMILE_KIND_NULL) {
		LIST_APPEND_WITH_SOURCE(head, tail, decl, ((struct SmileListWithSourceInt *)decl)->position);
	}

	// Every time we see a comma, parse the next name, and add it to the list.
	while (Parser_NextToken(parser)->kind == TOKEN_COMMA) {

		error = Parser_ParseTillName(parser, &decl);
		if (error != NULL) return error;

		if (decl->kind != SMILE_KIND_NULL) {
			LIST_APPEND_WITH_SOURCE(head, tail, decl, ((struct SmileListWithSourceInt *)decl)->position);
		}
	}

	// Don't overconsume at the end.
	Lexer_Unget(parser->lexer);

	// Return the list of names.
	*names = head;
	return NULL;
}

// anyname ::= NAME
ParseError Parser_ParseTillName(Parser parser, SmileObject *expr)
{
	Token token;
	ParseError error;
	Symbol symbol;

	// Get the name.
	if ((token = Parser_NextToken(parser))->kind != TOKEN_UNKNOWNALPHANAME
		&& token->kind != TOKEN_ALPHANAME
		&& token->kind != TOKEN_UNKNOWNPUNCTNAME
		&& token->kind != TOKEN_PUNCTNAME) {

		// No variable name?  That's an error.
		*expr = NullObject;
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("Missing flag name after 'till'."));
		return error;
	}

	// Declare it in the current scope.
	symbol = token->data.symbol;
	error = ParseScope_Declare(parser->currentScope, symbol, PARSEDECL_TILL, Token_GetPosition(token), NULL);
	if (error != NULL)
		return error;

	*expr = (SmileObject)SmileSymbol_Create(symbol);
	return NULL;
}

// whens_opt ::= whens |
// whens ::= when whens | when
// when ::= WHEN name expr
ParseError Parser_ParseWhens(Parser parser, SmileObject *expr, Int32Int32Dict tillFlags, Int modeFlags)
{
	ParseError parseError;
	Token token;
	Int32Dict usedSymbolDict;
	SmileList whenHead, whenTail;
	Symbol flagName;
	SmileObject whenBody;
	LexerPosition position;
	SmileObject whenClause;

	usedSymbolDict = Int32Dict_Create();

	whenHead = whenTail = NullList;

	// Parse any 'when' constructs that follow.
	for (;;) {

		// Consume a 'when' keyword.
		if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
			&& token->data.symbol == SMILE_SPECIAL_SYMBOL_WHEN)) {
			// No 'when' keyword, so we're done.
			Lexer_Unget(parser->lexer);
			break;
		}
		position = Token_GetPosition(token);

		// There should be a symbol that follows it.
		if (!(((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME
			|| token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_UNKNOWNPUNCTNAME))) {
			// Not a symbol, so complain, but rewind a token and keep going if we can.
			Parser_AddError(parser, Token_GetPosition(token), "Missing till-flag name after 'when'.");
			Lexer_Unget(parser->lexer);
			flagName = 0;
		}
		else {
			flagName = token->data.symbol;

			// That symbol should be one of the flag names declared in the 'till'.
			if (!Int32Int32Dict_ContainsKey(tillFlags, flagName)) {
				Parser_AddError(parser, Token_GetPosition(token), "'%S' is not a name declared in this 'till' loop.",
					SymbolTable_GetName(Smile_SymbolTable, flagName));
				// Try to keep going anyway.
			}
			else {
				// That symbol shouldn't have been used already for this 'till'.
				if (!Int32Dict_Add(usedSymbolDict, flagName, NULL)) {
					Parser_AddError(parser, Token_GetPosition(token), "'when %S' was used multiple times for the same 'till' loop.",
						SymbolTable_GetName(Smile_SymbolTable, flagName));
					// Keep going with the parse.
				}
			}
		}

		// There should be a body after the 'when name' clause.
		parseError = Parser_ParseExpr(parser, &whenBody, modeFlags);
		if (parseError != NULL) {
			Parser_AddMessage(parser, parseError);
		}

		// Construct the when clause itself:  [flag body]
		whenClause = (SmileObject)SmileList_CreateTwoWithSource(flagName ? (SmileObject)SmileSymbol_Create(flagName) : NullObject, whenBody, position);

		// Add it to the result list.
		LIST_APPEND_WITH_SOURCE(whenHead, whenTail, whenClause, position);
	}

	*expr = (SmileObject)whenHead;
	return NULL;
}

// orexpr ::= . orexpr OR andexpr | . andexpr
ParseError Parser_ParseOrExpr(Parser parser, SmileObject *expr, Int modeFlags)
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

	parseError = Parser_ParseAndExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	while (((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_OR
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseAndExpr(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		if (isFirst) {
			*expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._orSymbol, *expr, rvalue, lexerPosition);
			isFirst = False;
		}
		else {
			tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));
		}
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// andexpr :: = . andexpr AND notexpr | . notexpr
ParseError Parser_ParseAndExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileList tail = NullList;
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	Bool isFirst = True;

	parseError = Parser_ParseNotExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	while (((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME || token->kind == TOKEN_UNKNOWNALPHANAME)
		&& token->data.symbol == SMILE_SPECIAL_SYMBOL_AND
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseNotExpr(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		if (isFirst) {
			*expr = (SmileObject)SmileList_CreateThreeWithSource(Smile_KnownObjects._andSymbol, *expr, rvalue, lexerPosition);
			isFirst = False;
		}
		else {
			tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));
		}
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// notexpr :: = . NOT notexpr | . cmpexpr
ParseError Parser_ParseNotExpr(Parser parser, SmileObject *expr, Int modeFlags)
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
		// Not is a special built-in form:  [$not x]
		*expr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._notSymbol, *expr, lexerPosition);
	}

	return NULL;
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

	if (Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE)) {
		// This might not be possible, but if this is something like >==, then don't allow it to be consumed as a binary operator.
		return NULL;
	}

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
			*expr = (SmileObject)SmileList_CreateThreeWithSource(symbolObject, *expr, rvalue, lexerPosition);
		}
		else {
			*expr = (SmileObject)SmileList_CreateTwoWithSource(
				SmileList_CreateDotWithSource(*expr, symbolObject, lexerPosition), rvalue, lexerPosition);
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

	if (Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE)) {
		// If this is something like +=, then don't allow it to be consumed as a binary operator.
		return NULL;
	}

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

		*expr = (SmileObject)SmileList_CreateTwoWithSource(
			SmileList_CreateDotWithSource(*expr,
				symbol == SMILE_SPECIAL_SYMBOL_PLUS ? Smile_KnownObjects.plusSymbol : Smile_KnownObjects.minusSymbol,
				lexerPosition),
			rvalue,
			lexerPosition
		);

		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// mulexpr ::= . mulexpr STAR binaryexpr | . mulexpr SLASH binaryexpr | . binaryexpr
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

	parseError = Parser_ParseBinaryExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

parseNextOperator:

	if (Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE)) {
		// If this is something like *=, then don't allow it to be consumed as a binary operator.
		return NULL;
	}

	if ((customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_MULEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_BINARYEXPR, &parseError))
		!= CustomSyntaxResult_NotMatchedAndNoTokensConsumed) {

		if (customSyntaxResult == CustomSyntaxResult_PartialApplicationWithError)
			return parseError;

		goto parseNextOperator;
	}

	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNPUNCTNAME || token->kind == TOKEN_PUNCTNAME)
		&& ((symbol = token->data.symbol) == SMILE_SPECIAL_SYMBOL_STAR || symbol == SMILE_SPECIAL_SYMBOL_SLASH)
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseBinaryExpr(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		*expr = (SmileObject)SmileList_CreateTwoWithSource(
			SmileList_CreateDotWithSource(*expr,
				symbol == SMILE_SPECIAL_SYMBOL_STAR ? Smile_KnownObjects.starSymbol : Smile_KnownObjects.slashSymbol,
				lexerPosition),
			rvalue,
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

// binaryexpr ::= . binaryexpr UNKNOWN_PUNCT_NAME binary_args
// 		| . binaryexpr UNKNOWN_ALPHA_NAME binary_args
// 		| . colonexpr
// binary_args ::= binary_args COMMA colonexpr | colonexpr
ParseError Parser_ParseBinaryExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileList binaryExpr, tail;
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;
	Symbol symbol;
	CustomSyntaxResult customSyntaxResult;

	customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_BINARYEXPR, SYNTAXROOT_KEYWORD, 0, &parseError);
	if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
		return parseError;

	parseError = Parser_ParseColonExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

parseNextOperator:

	if (Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE)) {
		// If this is something like +=, then don't allow it to be consumed as a binary operator.
		return NULL;
	}

	if ((customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_BINARYEXPR, SYNTAXROOT_NONTERMINAL, SMILE_SPECIAL_SYMBOL_COLONEXPR, &parseError))
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

		parseError = Parser_ParseColonExpr(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		binaryExpr = tail = SmileList_CreateOneWithSource(
			SmileList_CreateDotWithSource(*expr, SmileSymbol_Create(symbol), lexerPosition), lexerPosition);

		*expr = (SmileObject)binaryExpr;

		tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));

		if ((modeFlags & COMMAMODE_MASK) == COMMAMODE_NORMAL) {
			while (Lexer_Peek(parser->lexer) == TOKEN_COMMA) {
				Lexer_Next(parser->lexer);

				lexerPosition = Token_GetPosition(parser->lexer->token);

				parseError = Parser_ParseColonExpr(parser, &rvalue, modeFlags);
				if (parseError != NULL)
					return parseError;

				tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));
			}
		}

		goto parseNextOperator;
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// colonexpr :: = . colonexpr COLON rangeexpr | . rangeexpr
ParseError Parser_ParseColonExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;

	parseError = Parser_ParseRangeExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	if ((modeFlags & COLONMODE_MASK) == COLONMODE_MEMBERDECL)
		return NULL;

	while ((token = Parser_NextToken(parser))->kind == TOKEN_COLON
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParseRangeExpr(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		*expr = (SmileObject)SmileList_CreateIndexWithSource(*expr, rvalue, lexerPosition);
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// rangeexpr ::= . prefixexpr DOTDOT prefixexpr | . prefixexpr
ParseError Parser_ParseRangeExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	SmileObject rvalue;
	ParseError parseError;
	Token token;
	LexerPosition lexerPosition;

	parseError = Parser_ParsePrefixExpr(parser, expr, modeFlags);
	if (parseError != NULL)
		return parseError;

	if ((token = Parser_NextToken(parser))->kind == TOKEN_DOTDOT
		&& ((modeFlags & BINARYLINEBREAKS_MASK) == BINARYLINEBREAKS_ALLOWED || !token->isFirstContentOnLine)) {

		lexerPosition = Token_GetPosition(token);

		parseError = Parser_ParsePrefixExpr(parser, &rvalue, modeFlags);
		if (parseError != NULL)
			return parseError;

		*expr = (SmileObject)SmileList_CreateTwoWithSource(
			SmileList_CreateDotWithSource(*expr, Smile_KnownObjects.rangeToSymbol, lexerPosition),
			rvalue,
			lexerPosition
		);
	}
	else {
		Lexer_Unget(parser->lexer);
	}

	return NULL;
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
ParseError Parser_ParsePrefixExpr(Parser parser, SmileObject *expr, Int modeFlags)
{
	LexerPosition position;
	ParseError parseError;
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
	if (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
		&& Parser_IsAcceptableArbitraryPrefixOperator(token->data.symbol)) {

		// Record which symbol came last, for error-reporting.
		lastUnaryTokenSymbol = token->data.symbol;

		MemCpy(unaryOperators + numOperators++, token, sizeof(struct TokenStruct));
		firstUnaryTokenForErrorReporting = &unaryOperators[0];

		// Collect up any successive unary prefix operators in the unaryOperators array.
		while (((token = Parser_NextToken(parser))->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
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
			return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Expected an expression term on the same line after unary operator '%S'",
					SymbolTable_GetName(Smile_SymbolTable, lastUnaryTokenSymbol)));
		}
	}

	Lexer_Unget(parser->lexer);

	// We now have all of the unary prefix operators.  Now go parse the term itself.
	parseError = Parser_ParseNewExpr(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
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
			// Typeof is a special built-in form:  [$typeof x]
			*expr = (SmileObject)SmileList_CreateTwoWithSource(Smile_KnownObjects._typeofSymbol, *expr, position);
		}
		else {
			// Construct an expression of the unary method-call form:  [(expr.unary)]
			*expr = (SmileObject)SmileList_CreateOneWithSource(
				SmileList_CreateDotWithSource(*expr, (SmileObject)SmileSymbol_Create(symbol), position),
				position
			);
		}
	}
	return NULL;
}

ParseError Parser_ParsePostfixExpr(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	ParseError parseError;
	CustomSyntaxResult customSyntaxResult;

	customSyntaxResult = Parser_ApplyCustomSyntax(parser, expr, modeFlags, SMILE_SPECIAL_SYMBOL_POSTFIXEXPR, SYNTAXROOT_KEYWORD, 0, &parseError);
	if (customSyntaxResult != CustomSyntaxResult_NotMatchedAndNoTokensConsumed)
		return parseError;

	return Parser_ParseConsExpr(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
}

// consexpr ::= . dotexpr DOUBLEHASH consexpr | . dotexpr
ParseError Parser_ParseConsExpr(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	SmileList tail;
	SmileObject rvalue, nextRValue;
	ParseError parseError;
	LexerPosition lexerPosition;
	Bool isFirst;

	parseError = Parser_ParseDotExpr(parser, expr, modeFlags, firstUnaryTokenForErrorReporting);
	if (parseError != NULL)
		return parseError;

	if (Lexer_Next(parser->lexer) == TOKEN_DOUBLEHASH) {

		lexerPosition = Token_GetPosition(parser->lexer->token);
		tail = SmileList_CreateOneWithSource(*expr, lexerPosition);
		*expr = (SmileObject)tail;
		isFirst = True;
		rvalue = NullObject;

		do {
			parseError = Parser_ParseDotExpr(parser, &nextRValue, modeFlags, firstUnaryTokenForErrorReporting);
			if (parseError != NULL)
				return parseError;

			if (!isFirst) {
				tail = (SmileList)(tail->d = (SmileObject)SmileList_CreateOneWithSource(rvalue, lexerPosition));
			}

			rvalue = nextRValue;
			isFirst = False;

		} while (Lexer_Next(parser->lexer) == TOKEN_DOUBLEHASH);

		tail->d = rvalue;
	}

	Lexer_Unget(parser->lexer);

	return NULL;
}

// dotexpr ::= . dotexpr DOT any_name | . term
ParseError Parser_ParseDotExpr(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting)
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

			*expr = (SmileObject)SmileList_CreateDotWithSource(*expr, (SmileObject)SmileSymbol_Create(symbol), lexerPosition);
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
	if (*result == Parser_IgnorableObject) *result = NullObject;

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
