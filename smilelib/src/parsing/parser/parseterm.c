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
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static Bool Parser_TryParseSpecialForm(Parser parser, LexerPosition startPosition, SmileObject *result, ParseError *error);

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
//         | . FLOAT
//         | . REAL
ParseError Parser_ParseTerm(Parser parser, SmileObject *result, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	ParseDecl parseDecl;
	Token token = Parser_NextTokenWithDeclaration(parser, &parseDecl);
	LexerPosition startPosition;
	ParseError error;
	SmileList head, tail;

	switch (token->kind) {

	case TOKEN_LEFTPARENTHESIS:
		Lexer_Unget(parser->lexer);
		return Parser_ParseParentheses(parser, result, modeFlags);

	case TOKEN_LEFTBRACKET:
		startPosition = Token_GetPosition(token);
		if (Parser_TryParseSpecialForm(parser, startPosition, result, &error))
			return error;

		head = NullList, tail = NullList;
		Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

		if ((error = Parser_ExpectRightBracket(parser, result, firstUnaryTokenForErrorReporting, "list", startPosition)) != NULL)
			return error;

		*result = (SmileObject)head;
		return NULL;

	case TOKEN_BAR:
		error = Parser_ParseFunc(parser, result, modeFlags);
		return error;

	case TOKEN_BACKTICK:
		error = Parser_ParseQuoteBody(parser, result, modeFlags, Token_GetPosition(token));
		return error;

	case TOKEN_LEFTBRACE:
		Lexer_Unget(parser->lexer);
		error = Parser_ParseScope(parser, result);
		return error;

	case TOKEN_ALPHANAME:
	case TOKEN_PUNCTNAME:
		if (parseDecl->declKind == PARSEDECL_KEYWORD) {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR,
				firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : Token_GetPosition(token),
				String_Format("\"%S\" is a keyword and cannot be used as a variable or operator", token->text));
			return error;
		}
		*result = (SmileObject)SmileSymbol_Create(token->data.symbol);
		return NULL;

	case TOKEN_RAWSTRING:
		*result = (SmileObject)token->text;
		return NULL;

	case TOKEN_DYNSTRING:
		return Parser_ParseDynamicString(parser, result, token->text, Token_GetPosition(token));

	case TOKEN_CHAR:
		*result = (SmileObject)SmileByte_Create(token->data.byte);
		return NULL;

	case TOKEN_BYTE:
		*result = (SmileObject)SmileByte_Create(token->data.byte);
		return NULL;

	case TOKEN_INTEGER16:
		*result = (SmileObject)SmileInteger16_Create(token->data.int16);
		return NULL;

	case TOKEN_INTEGER32:
		*result = (SmileObject)SmileInteger32_Create(token->data.int32);
		return NULL;

	case TOKEN_INTEGER64:
		*result = (SmileObject)SmileInteger64_Create(token->data.int64);
		return NULL;

	case TOKEN_REAL64:
		*result = (SmileObject)SmileReal64_Create(token->data.real64);
		return NULL;

	case TOKEN_UNKNOWNALPHANAME:
	case TOKEN_UNKNOWNPUNCTNAME:
		// If we get an operator name instead of a variable name, we can't use it as a term.
		error = ParseMessage_Create(PARSEMESSAGE_ERROR,
			firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : Token_GetPosition(token),
			String_Format("\"%S\" is not a known variable name", token->text));
		return error;
	
	case TOKEN_LOANWORD_SYNTAX:
		// Parse the new syntax rule.
		error = Parser_ParseSyntax(parser, result, modeFlags);
		if (error != NULL)
			return error;
	
		// Add the syntax rule to the table of syntax rules for the current scope.
		if (!ParserSyntaxTable_AddRule(parser, &parser->currentScope->syntaxTable, (SmileSyntax)*result)) {
			*result = NullObject;
		}
		return NULL;

	default:
		// We got an unknown token that can't be turned into a term.  So we're going to generate
		// an error message, but we do our best to specialize that message according to the most
		// common mistakes people make.
		if (firstUnaryTokenForErrorReporting != NULL) {
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(firstUnaryTokenForErrorReporting),
				String_Format("\"%S\" is not a known variable name", firstUnaryTokenForErrorReporting->text));
		}
		else if (token->kind == TOKEN_SEMICOLON) {
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

/// <summary>
/// Parse exactly one name (any name).
/// </summary>
ParseError Parser_ParseAnyName(Parser parser, SmileObject *expr)
{
	Token token = Parser_NextToken(parser);
	ParseError error;

	if (token->kind == TOKEN_ALPHANAME || token->kind == TOKEN_PUNCTNAME
		|| token->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME) {
		*expr =(SmileObject)SmileSymbol_Create(token->data.symbol);
		return NULL;
	}

	error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
		String_Format("Expected a name, not \"%S\".", TokenKind_ToString(token->kind)));
	return error;
}

/// <summary>
/// See if this is a special form that requires "unique" parsing, and if so, parse it
/// as one of those special forms.
/// </summary>
/// <remarks>
/// The special forms that require unusual parsing are those which have nonstandard
/// variable-evaluation rules --- and those are primarily forms that are able to
/// declare variables in a new scope.  Specifically this means we check for:
///
///     [$quote ...]
///     [$scope [vars...] ... ]
///     [$fn [args...] ... ]
///     [$till [flags...] ... ]
///     [$new base [members...]]
///     [$set name value]
///
/// Everything else can be parsed normally, but those are necessarily unique.
///
/// Note that [$set], like '=', can construct new variables in the current scope, if
/// the scope is one that allows variable construction.
///
/// Note also that [$scope] is much more like Lisp (let) than it is like {braces}, in
/// that its given local-variable list contains the names of the variables within it,
/// and it cannot be modified beyond that set of local variables, either by [$set] or
/// by '='.
///
/// Note finally that 'const' is nothing more than sleight-of-hand by the parser, in
/// that it simply prohibits [$set] or '=' to have a 'const' variable as its lvalue:
/// Once everything has been compiled to Lisp forms, there's no such thing as 'const'.
/// </remarks>
static Bool Parser_TryParseSpecialForm(Parser parser, LexerPosition startPosition, SmileObject *result, ParseError *error)
{
	Token token;
	if ((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME) {
	
		switch (token->data.symbol) {

			case SMILE_SPECIAL_SYMBOL__QUOTE:
				*error = Parser_ParseClassicQuote(parser, result, startPosition);
				return True;
			
			case SMILE_SPECIAL_SYMBOL__SCOPE:
				*error = Parser_ParseClassicScope(parser, result, startPosition);
				return True;
			
			case SMILE_SPECIAL_SYMBOL__FN:
				*error = Parser_ParseClassicFn(parser, result, startPosition);
				return True;
			
			case SMILE_SPECIAL_SYMBOL__TILL:
				*error = Parser_ParseClassicTill(parser, result, startPosition);
				return True;
			
			case SMILE_SPECIAL_SYMBOL__NEW:
				*error = Parser_ParseClassicNew(parser, result, startPosition);
				return True;
			
			case SMILE_SPECIAL_SYMBOL__SET:
				*error = Parser_ParseClassicSet(parser, result, startPosition);
				return True;
		}
	}

	Lexer_Unget(parser->lexer);
	return False;
}

ParseError Parser_ExpectLeftBracket(Parser parser, SmileObject *result, Token firstUnaryTokenForErrorReporting, const char *name, LexerPosition startPosition)
{
	ParseError error;

	if (!Parser_HasLookahead(parser, TOKEN_LEFTBRACKET)) {
		error = ParseMessage_Create(PARSEMESSAGE_ERROR,
			firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : startPosition,
			String_Format("Missing '[' in %s starting on line %d.", name, startPosition->line));
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		*result = NullObject;
		return error;
	}
	Parser_NextToken(parser);

	return NULL;
}

ParseError Parser_ExpectRightBracket(Parser parser, SmileObject *result, Token firstUnaryTokenForErrorReporting, const char *name, LexerPosition startPosition)
{
	ParseError error;

	if (!Parser_HasLookahead(parser, TOKEN_RIGHTBRACKET)) {
		error = ParseMessage_Create(PARSEMESSAGE_ERROR,
			firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : startPosition,
			String_Format("Missing ']' in %s starting on line %d.", name, startPosition->line));
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		*result = NullObject;
		return error;
	}
	Parser_NextToken(parser);

	return NULL;
}
