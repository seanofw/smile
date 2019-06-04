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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/regex.h>

static ParseResult Parser_TryParseSpecialForm(Parser parser, LexerPosition startPosition);

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
//         | . LOANWORD_SYNTAX
//         | . LOANWORD_REGEX
ParseResult Parser_ParseTerm(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting)
{
	ParseDecl parseDecl;
	Token token = Parser_NextTokenWithDeclaration(parser, &parseDecl);
	LexerPosition startPosition;
	ParseResult parseResult;
	SmileList head, tail;

	switch (token->kind) {

	case TOKEN_LEFTPARENTHESIS:
		Lexer_Unget(parser->lexer);
		return Parser_ParseParentheses(parser, modeFlags);

	case TOKEN_LEFTBRACKET:
		startPosition = Token_GetPosition(token);
		parseResult = Parser_TryParseSpecialForm(parser, startPosition);
		if (parseResult.status != ParseStatus_NotMatchedAndNoTokensConsumed) {
			if (IS_PARSE_ERROR(parseResult))
				RETURN_PARSE_ERROR(parseResult);
			else return parseResult;
		}

		head = NullList, tail = NullList;
		Parser_ParseCallArgsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

		parseResult = Parser_ExpectRightBracket(parser, firstUnaryTokenForErrorReporting, "list", startPosition);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);

		return EXPR_RESULT(head);

	case TOKEN_BAR:
		return Parser_ParseFunc(parser, modeFlags);

	case TOKEN_BACKTICK:
		return Parser_ParseQuoteBody(parser, modeFlags, Token_GetPosition(token));

	case TOKEN_LEFTBRACE:
		Lexer_Unget(parser->lexer);
		return Parser_ParseScope(parser);

	case TOKEN_ALPHANAME:
	case TOKEN_PUNCTNAME:
		if (parseDecl->declKind == PARSEDECL_KEYWORD) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR,
				firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : Token_GetPosition(token),
				String_Format("\"%S\" is a keyword and cannot be used as a variable or operator", token->text)));
		}
		return EXPR_RESULT(SmileSymbol_Create(token->data.symbol));

	case TOKEN_RAWSTRING:
		return EXPR_RESULT(token->text);

	case TOKEN_DYNSTRING:
		return Parser_ParseDynamicString(parser, token->text, Token_GetPosition(token));

	case TOKEN_CHAR:
		return EXPR_RESULT(SmileChar_Create(token->data.ch));

	case TOKEN_UNI:
		return EXPR_RESULT(SmileUni_Create(token->data.uni));

	case TOKEN_BYTE:
		return EXPR_RESULT(SmileByte_Create(token->data.byte));

	case TOKEN_INTEGER16:
		return EXPR_RESULT(SmileInteger16_Create(token->data.int16));

	case TOKEN_INTEGER32:
		return EXPR_RESULT(SmileInteger32_Create(token->data.int32));

	case TOKEN_INTEGER64:
		return EXPR_RESULT(SmileInteger64_Create(token->data.int64));

	case TOKEN_REAL64:
		return EXPR_RESULT(SmileReal64_Create(token->data.real64));

	case TOKEN_REAL32:
		return EXPR_RESULT(SmileReal32_Create(token->data.real32));

	case TOKEN_FLOAT64:
		return EXPR_RESULT(SmileFloat64_Create(token->data.float64));

	case TOKEN_FLOAT32:
		return EXPR_RESULT(SmileFloat32_Create(token->data.float32));

	case TOKEN_UNKNOWNALPHANAME:
	case TOKEN_UNKNOWNPUNCTNAME:
		// If we get an operator name instead of a variable name, we can't use it as a term.
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR,
			firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : Token_GetPosition(token),
			String_Format("\"%S\" is not a known variable name", token->text)));

	case TOKEN_LOANWORD_REGEX:
		{
			LexerPosition position = Token_GetPosition(token);
		
			// Get the regex itself.
			Regex regex = (Regex)token->data.ptr;

			// Return it as a pre-built list of the form [Regex.of pattern-string options-string].
			// We'll actually end up discarding this regex and creating a new one, but that's okay, because
			// it still validates the correctness of the regex:  It parses it fully at lexical-analysis time,
			// so we can be certain it's correct before letting the program run.  Second, even though we've
			// discarded it, the compiled copy of the regex is still sitting in the global regex cache, so when
			// the program runs, it won't need to compile the regex again; it'll just locate the correct
			// compiled regex object by matching strings to a known cache entry.  (The compiler may be able to
			// even optimize this further, by recognizing patterns of the form [Regex.of pattern options], and
			// simply transforming those into a load of a known handle.  But that work doesn't belong here,
			// since #/.../ is only a shorthand for writing [Regex.of pattern options].)
			SmileList creationCall =
				SmileList_ConsWithSource(
					(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._dotSymbol,
						(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects.RegexSymbol,
							(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects.ofSymbol,
								NullObject,
								position),
							position),
						position),
					(SmileObject)SmileList_ConsWithSource((SmileObject)regex->pattern,
						(SmileObject)SmileList_ConsWithSource((SmileObject)regex->flags,
							NullObject,
							position),
						position),
					position);

			return EXPR_RESULT(creationCall);
		}

	case TOKEN_LOANWORD_SYNTAX:
		// Parse the new syntax rule.
		parseResult = Parser_ParseSyntax(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
	
		// Add the syntax rule to the table of syntax rules for the current scope.
		if (!ParserSyntaxTable_AddRule(parser, &parser->currentScope->syntaxTable, (SmileSyntax)parseResult.expr)) {
			parseResult = EXPR_RESULT(NullObject);
		}
		ParseScope_AddSyntax(parser->currentScope, (SmileSyntax)parseResult.expr);
		return EXPR_RESULT(parseResult.expr);

	case TOKEN_LOANWORD_LOANWORD:
		parseResult = Parser_ParseLoanword(parser, modeFlags);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);

		// Add the loanword rule to the table of loanword rules for the current scope.
		if (!ParserLoanwordTable_AddRule(parser, &parser->currentScope->loanwordTable, (SmileLoanword)parseResult.expr)) {
			parseResult = EXPR_RESULT(NullObject);
		}
		ParseScope_AddLoanword(parser->currentScope, (SmileLoanword)parseResult.expr);
		return EXPR_RESULT(parseResult.expr);

	case TOKEN_LOANWORD_CUSTOM:
		// Lexer doesn't know this loanword, so we have to see if it's in the current loanword table.
		// If so, we use its regex to consume any subsequent characters it requires, and then transform
		// the regex match into a template substitution.
		return Parser_ApplyCustomLoanword(parser, token);

	default:
		// We got an unknown token that can't be turned into a term.  So we're going to generate
		// an error message, but we do our best to specialize that message according to the most
		// common mistakes people make.
		if (firstUnaryTokenForErrorReporting != NULL) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(firstUnaryTokenForErrorReporting),
				String_Format("\"%S\" is not a known variable name", firstUnaryTokenForErrorReporting->text)));
		}
		else if (token->kind == TOKEN_SEMICOLON) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_FromC("Expected a variable or number or other legal expression term, not a semicolon (remember, semicolons don't terminate statements in Smile!)")));
		}
		else if (token->kind == TOKEN_COMMA) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_FromC("Expected a variable or number or other legal expression term, not a comma (did you mistakenly put commas in a list?)")));
		}
		else if (token->kind == TOKEN_ERROR) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), token->text));
		}
		else {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_Format("Expected a variable or number or other legal expression term, not \"%S\".", TokenKind_ToString(token->kind))));
		}
	}
}

/// <summary>
/// Parse exactly one name (any name).
/// </summary>
ParseResult Parser_ParseAnyName(Parser parser)
{
	Token token = Parser_NextToken(parser);

	if (token->kind == TOKEN_ALPHANAME || token->kind == TOKEN_PUNCTNAME
		|| token->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME) {
		return EXPR_RESULT(SmileSymbol_Create(token->data.symbol));
	}

	return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
		String_Format("Expected a name, not \"%S\".", TokenKind_ToString(token->kind))));
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
static ParseResult Parser_TryParseSpecialForm(Parser parser, LexerPosition startPosition)
{
	Token token;
	if ((token = Parser_NextToken(parser))->kind == TOKEN_ALPHANAME) {
	
		switch (token->data.symbol) {

			case SMILE_SPECIAL_SYMBOL__QUOTE:
				return Parser_ParseClassicQuote(parser, startPosition);
			
			case SMILE_SPECIAL_SYMBOL__SCOPE:
				return Parser_ParseClassicScope(parser, startPosition);
			
			case SMILE_SPECIAL_SYMBOL__FN:
				return Parser_ParseClassicFn(parser, startPosition);
			
			case SMILE_SPECIAL_SYMBOL__TILL:
				return Parser_ParseClassicTill(parser, startPosition);
			
			case SMILE_SPECIAL_SYMBOL__NEW:
				return Parser_ParseClassicNew(parser, startPosition);
			
			case SMILE_SPECIAL_SYMBOL__SET:
				return Parser_ParseClassicSet(parser, startPosition);
		}
	}

	Lexer_Unget(parser->lexer);
	return NOMATCH_RESULT();
}

ParseResult Parser_ExpectLeftBracket(Parser parser, Token firstUnaryTokenForErrorReporting, const char *name, LexerPosition startPosition)
{
	ParseResult parseResult;

	if (!Parser_HasLookahead(parser, TOKEN_LEFTBRACKET)) {
		parseResult = ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR,
			firstUnaryTokenForErrorReporting != NULL ? Token_GetPosition(firstUnaryTokenForErrorReporting) : startPosition,
			String_Format("Missing '[' in %s starting on line %d.", name, startPosition->line)));
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		return parseResult;
	}
	Parser_NextToken(parser);

	return NULL_RESULT();
}

ParseResult Parser_ExpectRightBracket(Parser parser, Token firstUnaryTokenForErrorReporting, const char *name, LexerPosition startPosition)
{
	UNUSED(firstUnaryTokenForErrorReporting);

	if (!Parser_HasLookahead(parser, TOKEN_RIGHTBRACKET)) {
		LexerPosition lexerPosition = Token_GetPosition(parser->lexer->token);
		ParseResult parseResult = ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, lexerPosition,
			String_Format("Missing ']' in %s starting on line %d.", name, startPosition->line)));
		Parser_Recover(parser, Parser_RightBracesBracketsParentheses_Recovery, Parser_RightBracesBracketsParentheses_Count);
		return parseResult;
	}
	Parser_NextToken(parser);

	return NULL_RESULT();
}
