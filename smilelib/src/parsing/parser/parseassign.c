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
#include <smile/internal/staticstring.h>

STATIC_STRING(MissingVarNameMessage, "Expected a variable name after 'var'");
STATIC_STRING(MissingConstNameMessage, "Expected a constant name after 'const'");
STATIC_STRING(MissingAutoNameMessage, "Expected a auto-cleanup variable name after 'auto'");
STATIC_STRING(MissingConstRValueMessage, "Expected a constant-value assignment after 'const'");
STATIC_STRING(MissingAutoRValueMessage, "Expected an auto-value assignment after 'auto'");
STATIC_STRING(MissingKeywordNameMessage, "Expected a keyword name after 'keyword'");
STATIC_STRING(InternalErrorMessage, "Internal error while parsing variable declarations");

static ParseError EnsureLValueIsAssignable(Parser parser, SmileObject lvalue, LexerPosition startPosition);

//-------------------------------------------------------------------------------------------------
// Assignment, opequals, and variable declarations

//  var_decl ::= KEYWORD . keyword_decls
//  keyword_decls ::= name | keyword_decls COMMA name
ParseResult Parser_ParseKeywordList(Parser parser)
{
	ParseError error;
	Token token;

	do {
		token = Parser_NextToken(parser);
		if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME
			&& token->kind != TOKEN_PUNCTNAME && token->kind != TOKEN_UNKNOWNPUNCTNAME) {

			// No keyword name?  That's an error.
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				MissingKeywordNameMessage));
		}

		error = ParseScope_DeclareHere(parser->currentScope, token->data.symbol, PARSEDECL_KEYWORD, Token_GetPosition(token), NULL);
		if (error != NULL)
			return ERROR_RESULT(error);
	
	} while (Lexer_Next(parser->lexer) == TOKEN_COMMA);

	Lexer_Unget(parser->lexer);

	// There's literally nothing to return on a successful parse.
	return NULL_RESULT();
}

//  var_decl ::= VAR . decls
//  decls ::= decl | decls COMMA decl
ParseResult Parser_ParseVarDecls(Parser parser, Int modeFlags, Int declKind)
{
	SmileList head, tail;
	ParseResult declResult;

	LIST_INIT(head, tail);

	// Parse the first declaration, which will result in either nothing or a list like [\= x 5].
	declResult = Parser_ParseDecl(parser, modeFlags, declKind);
	switch (declResult.status)
	{
		case ParseStatus_PartialParseWithError:
			return declResult;
		case ParseStatus_SuccessfulWithResult:
			// Attach it to the list as a nested list, so it becomes [[\ = x 5]].
			LIST_APPEND_WITH_SOURCE(head, tail, declResult.expr, ((struct SmileListWithSourceInt*)declResult.expr)->position);
			break;
		case ParseStatus_SuccessfulWithNoResult:
		case ParseStatus_ErroredButRecovered:
			break;
		case ParseStatus_NotMatchedAndNoTokensConsumed:
			// Shouldn't be able to get this state, but flexible parsing is weird, so we have
			// to assume it's at least possible and handle it sensibly.
			return declResult;
		default:
			Smile_Abort_FatalError("Unhandled ParseStatus in Parser_ParseVarDecls().");
	}

	// Every time we see a comma, parse the next declaration, and add it to the list if it
	// is any form of assignment:  [[\= x 5] [\= y 8] [\= z 10] ...]
	while (Parser_NextToken(parser)->kind == TOKEN_COMMA) {

		declResult = Parser_ParseDecl(parser, modeFlags, declKind);
		switch (declResult.status)
		{
			case ParseStatus_PartialParseWithError:
				return declResult;
			case ParseStatus_SuccessfulWithResult:
				LIST_APPEND_WITH_SOURCE(head, tail, declResult.expr, ((struct SmileListWithSourceInt*)declResult.expr)->position);
				break;
			case ParseStatus_SuccessfulWithNoResult:
			case ParseStatus_ErroredButRecovered:
				break;
			case ParseStatus_NotMatchedAndNoTokensConsumed:
				// Shouldn't be able to get this state, but flexible parsing is weird, so we have
				// to assume it's at least possible and handle it sensibly.
				return declResult;
			default:
				Smile_Abort_FatalError("Unhandled ParseStatus in Parser_ParseVarDecls().");
		}
	}

	// Don't overconsume at the end.
	Lexer_Unget(parser->lexer);

	// Finally, take our pile of assignments (if they exist), and turn them into an executable
	// form by prefixing them with '$progn':  [$progn [\= x 5] [\= y 8] [\= z 10] ...]
	return (head->kind == SMILE_KIND_NULL)
		? NULL_RESULT()
		: EXPR_RESULT((SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._prognSymbol, (SmileObject)head,
			((struct SmileListWithSourceInt *)head)->position));
}

//  decl ::= . name | . name EQUAL arith | . name EQUAL_NOSPACE arith
ParseResult Parser_ParseDecl(Parser parser, Int modeFlags, Int declKind)
{
	Token token;
	ParseError error;
	Symbol symbol;
	SmileObject rvalue, expr;
	LexerPosition lexerPosition;
	ParseResult parseResult;

	// Get the variable name.
	if ((token = Parser_NextToken(parser))->kind != TOKEN_UNKNOWNALPHANAME
		&& token->kind != TOKEN_ALPHANAME
		&& token->kind != TOKEN_UNKNOWNPUNCTNAME
		&& token->kind != TOKEN_PUNCTNAME) {

		// No variable name?  That's an error.
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			declKind == PARSEDECL_AUTO ? MissingAutoNameMessage
			: declKind == PARSEDECL_CONST ? MissingConstNameMessage
			: declKind == PARSEDECL_VARIABLE ? MissingVarNameMessage
			: InternalErrorMessage));
	}

	symbol = token->data.symbol;

	// If it's followed by an equal sign (in variable mode), or *when* it's followed
	// by an equal sign (in const/auto mode), collect the assignment value.
	token = Parser_NextToken(parser);
	if (token->kind == TOKEN_EQUAL || token->kind == TOKEN_EQUALWITHOUTWHITESPACE) {

		// This is an assignment, so collect up the assignment value.
		parseResult = Parser_ParseOpEquals(parser, (modeFlags & ~COMMAMODE_MASK) | COMMAMODE_VARIABLEDECLARATION);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		// Now that we've parsed the value, we can safely declare it in the current scope.
		error = ParseScope_Declare(parser->currentScope, symbol, declKind, Token_GetPosition(token), NULL);
		if (error != NULL)
			return ERROR_RESULT(error);

		// Build the result, which is a list shaped like [$set symbol rvalue]
		lexerPosition = Token_GetPosition(token);
		expr =
			(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._setSymbol,
				(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(symbol),
					(SmileObject)SmileList_ConsWithSource(rvalue,
						NullObject,
						lexerPosition),
					lexerPosition),
				lexerPosition);

		return EXPR_RESULT(expr);
	}
	else if (declKind == PARSEDECL_AUTO || declKind == PARSEDECL_CONST) {

		// It's 'auto' or 'const', but with no assignment.  That's an error.
		Lexer_Unget(parser->lexer);
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			declKind == PARSEDECL_AUTO ? MissingAutoRValueMessage : MissingConstRValueMessage));
	}
	else {

		// Declare it in the current scope.
		error = ParseScope_Declare(parser->currentScope, symbol, declKind, Token_GetPosition(token), NULL);
		if (error != NULL)
			return ERROR_RESULT(error);

		// It's 'var', without an assignment.  That is fully allowed, so we return the
		// nonexistent assignment (i.e., null).
		Lexer_Unget(parser->lexer);
		return NULL_RESULT();
	}
}

//  arith ::= . lvalue unknown_name EQUAL_NOSPACE arith | . assign
ParseResult Parser_ParseOpEquals(Parser parser, Int modeFlags)
{
	ParseResult parseResult;
	Token opToken;
	SmileObject lvalue, rvalue, expr;
	LexerPosition lexerPosition;

	parseResult = Parser_ParseEquals(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	lvalue = parseResult.expr;

	if (!Parser_IsLValue(lvalue)
		|| (!Parser_Has2Lookahead(parser, TOKEN_UNKNOWNALPHANAME, TOKEN_EQUALWITHOUTWHITESPACE)
			&& !Parser_Has2Lookahead(parser, TOKEN_UNKNOWNPUNCTNAME, TOKEN_EQUALWITHOUTWHITESPACE))) {
		return EXPR_RESULT(lvalue);
	}

	// Op-equal assignment to a known variable.

	// First, consume the op-equals part.
	opToken = Parser_NextToken(parser);		// Consume the operator name.
	lexerPosition = Token_GetPosition(opToken);
	Parser_NextToken(parser);				// Consume the TOKEN_EQUALWITHOUTWHITESPACE.

	// Collect the rvalue (recursively).
	parseResult = Parser_ParseOpEquals(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);
	rvalue = parseResult.expr;

	// Build the result, which is a list shaped like [$opset operator lvalue rvalue]
	expr =
		(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._opsetSymbol,
			(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(opToken->data.symbol),
				(SmileObject)SmileList_ConsWithSource(lvalue,
					(SmileObject)SmileList_ConsWithSource(rvalue,
						NullObject,
						lexerPosition),
					lexerPosition),
				lexerPosition),
			lexerPosition);

	return EXPR_RESULT(expr);
}

//  assign ::= . lvalue EQUAL assign
//			| . lvalue EQUAL_NOSPACE assign
//          | . unknown_name EQUAL assign
//          | . unknown_name EQUAL_NOSPACE assign
//          | . or
ParseResult Parser_ParseEquals(Parser parser, Int modeFlags)
{
	Token token, token2;
	LexerPosition position, position2;
	SmileObject lvalue, rvalue, expr;
	ParseResult parseResult;
	ParseError error;
	Symbol name;

	// Handle the "implicit variable declaration" production:
	//   assign ::= . unknown_name EQUAL assign | . unknown_name EQUAL_NOSPACE assign
	if (Parser_Peek2(parser, &token, &token2)
		&& (token->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
		&& (token2->kind == TOKEN_EQUALWITHOUTWHITESPACE || token2->kind == TOKEN_EQUAL)) {

		// This is a variable declaration-and-assignment statement.

		Parser_NextToken(parser);		// Consume the variable name.
		position = Token_GetPosition(token);

		Parser_NextToken(parser);		// Consume the equal sign.
		position2 = Token_GetPosition(token2);

		// Declare the variable name in this scope.  (We don't need to subsequently check if
		// we can assign it in this scope, since we just declared it.  If this scope is, say,
		// constant or special or something, the attempt to declare it will fail here, so we
		// only need to check if the attempt to declare it failed.)
		error = ParseScope_Declare(parser->currentScope, name = token->data.symbol, PARSEDECL_VARIABLE, position, NULL);
		if (error != NULL)
			return ERROR_RESULT(error);

		// Collect the rvalue recursively, *after* the declaration in this scope.  The rule
		// is that variables in a scope exist for the entire extent of the scope, so 'x = x + 1'
		// must always refer to the same 'x', even if the 'x =' part is the first time we've
		// seen 'x'.  Thus in a new scope, the second 'x' in 'x = x + 1' must behave as a
		// logical error (a read from an unassigned variable), not as a reference to an 'x'
		// variable from an outside scope.
		parseResult = Parser_ParseEquals(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;

		// Construct the resulting list, which will look like [$set name rvalue], but invisibly
		// annotated with source locations.
		expr =
			(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._setSymbol,
				(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(name),
					(SmileObject)SmileList_ConsWithSource(rvalue, NullObject,
					position2),
				position2),
			position2);
		return EXPR_RESULT(expr);
	}
	else {
		// Not an implicit variable declaration, so handle the other three productions:
		//   assign ::= . lvalue EQUAL assign | . lvalue EQUAL_NOSPACE assign | . or

		// Try reading an 'orexpr'-level nonterminal, whatever it might turn out to be.
		parseResult = Parser_ParseOrExpr(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		lvalue = parseResult.expr;

		// See if it's actually an lvalue followed by an equal sign.
		if (!Parser_IsLValue(lvalue) || !Parser_HasEqualLookahead(parser)) {

			// It's just a plain lvalue.
			return EXPR_RESULT(lvalue);
		}

		// It is an lvalue followed by an equal sign, so this is a variable assignment.
		token2 = Parser_NextToken(parser);
		position2 = Token_GetPosition(token2);

		// Collect the rvalue, recursively.
		parseResult = Parser_ParseEquals(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		rvalue = parseResult.expr;
	
		// If the target is a variable, ensure proper variable-declaration semantics are followed by looking up
		// the variable's current declaration.
		if ((error = EnsureLValueIsAssignable(parser, lvalue, position2)) != NULL)
			return ERROR_RESULT(error);

		// Construct the resulting list, which will look like [$set lvalue rvalue], but invisibly
		// annotated with source locations.
		expr =
			(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._setSymbol,
				(SmileObject)SmileList_ConsWithSource(lvalue,
					(SmileObject)SmileList_ConsWithSource(rvalue, NullObject,
					position2),
				position2),
			position2);

		return EXPR_RESULT(expr);
	}
}

// term ::= '[' '$set' . lvalue rvalue ']'
//
// Note:  Because 'const' is just sleight-of-hand by the parser, it is impossible for
// a [$set] form to ever assign to a 'const' value.
ParseResult Parser_ParseClassicSet(Parser parser, LexerPosition startPosition)
{
	ParseResult parseResult;
	Token token;
	SmileObject lvalue, rvalue, expr;
	ParseDecl decl = NULL;
	ParseError error;

	// Make sure an acceptable lvalue follows.
	token = Parser_NextToken(parser);
	if (token->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
	{
		// Never seen this before, so declare it on the spot, if we can do that in this scope.
		lvalue = (SmileObject)SmileSymbol_Create(token->data.symbol);
	
		// Declare the variable name in this scope.
		if ((error = ParseScope_Declare(parser->currentScope, token->data.symbol, PARSEDECL_VARIABLE, Token_GetPosition(token), &decl)) != NULL)
			return ERROR_RESULT(error);
	}
	else {
		Lexer_Unget(parser->lexer);

		// Try reading an 'orexpr'-level nonterminal, whatever that might turn out to be.
		parseResult = Parser_ParseOrExpr(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		lvalue = parseResult.expr;
	}

	// Parse the subsequent rvalue expression.
	parseResult = Parser_ParseExpr(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (IS_PARSE_ERROR(parseResult)) {
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		RETURN_PARSE_ERROR(parseResult);
	}
	rvalue = parseResult.expr;

	parseResult = Parser_ExpectRightBracket(parser, NULL, "[$set] form", startPosition);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);

	// Make sure the first argument is actually an lvalue; even if the form is syntactically valid in
	// general, it's not a valid [$set] form without an lvalue in its first argument position.
	if (!Parser_IsLValue(lvalue)) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition,
			String_Format("Invalid lvalue in [$set] starting on line %d.", startPosition->line)));
	}

	// If the target is a variable, ensure proper variable-declaration semantics are followed by looking up
	// the variable's current declaration.
	if ((error = EnsureLValueIsAssignable(parser, lvalue, startPosition)) != NULL)
		return ERROR_RESULT(error);

	// Construct the resulting [$set lvalue rvalue] form.
	expr =
		(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__SET),
			(SmileObject)SmileList_ConsWithSource(lvalue,
				(SmileObject)SmileList_ConsWithSource(rvalue, NullObject, startPosition),
			startPosition),
		startPosition);
	return EXPR_RESULT(expr);
}

/// <summary>
/// If the lvalue is a variable, ensure proper variable-declaration semantics are followed by looking up
/// the variable's current declaration.  If the declaration is an argument, a variable, or a global, we
/// allow it.  If it's const or auto or a till flag or something like that, we generate an error and return
/// False.
/// </summary>
static ParseError EnsureLValueIsAssignable(Parser parser, SmileObject lvalue, LexerPosition startPosition)
{
	ParseDecl decl;

	if (SMILE_KIND(lvalue) != SMILE_KIND_SYMBOL)
		return NULL;

	decl = ParseScope_FindDeclaration(parser->currentScope, ((SmileSymbol)lvalue)->symbol);
	if (decl == NULL) {
		return ParseMessage_Create(PARSEMESSAGE_FATAL, startPosition,
			String_Format("Lvalue in [$set] starting on line %d is a known symbol, but its scope declaration cannot be found (this is probably a bug!).", startPosition->line));
	}

	if (decl->declKind != PARSEDECL_VARIABLE && decl->declKind != PARSEDECL_ARGUMENT
		&& decl->declKind != PARSEDECL_GLOBAL) {
		if (decl->declKind == PARSEDECL_CONST) {
			return ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition,
				String_Format("\"%S\" is a const variable and already has a value.",
					SymbolTable_GetName(Smile_SymbolTable, ((SmileSymbol)lvalue)->symbol), startPosition->line));
		}
		else if (decl->declKind == PARSEDECL_AUTO) {
			return ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition,
				String_Format("\"%S\" is a auto variable and already has a value.",
					SymbolTable_GetName(Smile_SymbolTable, ((SmileSymbol)lvalue)->symbol), startPosition->line));
		}
		else {
			return ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition,
				String_Format("\"%S\" is not an assignable variable.",
				SymbolTable_GetName(Smile_SymbolTable, ((SmileSymbol)lvalue)->symbol), startPosition->line));
		}
	}

	return NULL;
}
