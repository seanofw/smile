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

#include <stdlib.h>

#include <smile/types.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/stringbuilder.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/identkind.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/internal/staticstring.h>

STATIC_STRING(MissingSyntaxClassName, "Expected a valid syntax nonterminal name after #syntax.");
STATIC_STRING(MissingSyntaxColon, "Expected a ':' after the syntax nonterminal name.");
STATIC_STRING(MissingSyntaxLeftBracket, "Expected a '[' to begin the syntax pattern after the syntax nonterminal name.");
STATIC_STRING(MissingSyntaxRightBracket, "Expected a ']' after the syntax pattern.");
STATIC_STRING(MissingSyntaxImpliesSymbol, "Expected a '=>' (implies symbol) to separate the syntax pattern from its substitution.");
STATIC_STRING(MalformedSyntaxPatternKeyword, "Invalid syntax pattern: Missing keyword.");
STATIC_STRING(MalformedSyntaxPatternMismatchedParentheses, "Invalid syntax pattern: Mismatched parentheses in rule.");
STATIC_STRING(MalformedSyntaxPatternMismatchedBraces, "Invalid syntax pattern: Mismatched curly braces in rule.");
STATIC_STRING(MalformedSyntaxPatternMismatchedBrackets, "Invalid syntax pattern: Mismatched square brackets in rule.");
STATIC_STRING(MalformedSyntaxPatternIllegalNonterminal, "Invalid syntax pattern: Nonterminals must be named identifiers, not \"%S\".");
STATIC_STRING(MalformedSyntaxPatternIllegalNonterminalName, "Invalid syntax pattern: Nonterminal variables must be named identifiers, not \"%S\".");
STATIC_STRING(MalformedSyntaxPatternIllegalNonterminalRepeat, "Invalid syntax pattern: Unknown nonterminal repeat kind '%S'.");
STATIC_STRING(MalformedSyntaxPatternNonterminalRepeatSeparatorMismatch, "Invalid syntax pattern: Nonterminal has a separator '%S' but does not repeat.");
STATIC_STRING(MalformedSyntaxPatternMissingNameAfterWith, "Invalid syntax pattern: Invalid name after 'with' in nonterminal.");

STATIC_STRING(InvalidPatternError, "Syntax patterns must be non-empty lists.");
STATIC_STRING(BrokenPatternError, "Internal error: Syntax patterns must only consist of symbols and nonterminals.");

STATIC_STRING(InvalidSealedPatternError, "Syntax patterns may not be added to the special %s class.");
STATIC_STRING(InvalidClassError, "Unknown or illegal syntax class '%s' (did you forget a hyphen?)");
STATIC_STRING(InvalidKeywordPatternError, "Syntax patterns in the %s class must start with a keyword.");

STATIC_STRING(InvalidCmpExprPatternError, "Syntax patterns in the CMPEXPR class must either start with a keyword, or with an ADDEXPR nonterminal followed by a keyword that is none of the nine standard comparison operators.");
STATIC_STRING(InvalidAddExprPatternError, "Syntax patterns in the ADDEXPR class must either start with a keyword, or with a MULEXPR nonterminal followed by a keyword that is neither '+' or '-'.");
STATIC_STRING(InvalidMulExprPatternError, "Syntax patterns in the MULEXPR class must either start with a keyword, or with a BINARYEXPR nonterminal followed by a keyword that is neither '*' or '/'.");
STATIC_STRING(InvalidBinaryExprPatternError, "Syntax patterns in the BINARYEXPR class must either start with a keyword, or with a COLONEXPR nonterminal followed by a keyword.");
STATIC_STRING(InvalidPostfixExprPatternError, "Syntax patterns in the POSTFIXEXPR class must either start with a keyword, or with a CONSEXPR nonterminal followed by a keyword.");

/// <summary>
/// Tokens that are used during common recovery scenarios to determine the
/// likely end of the current error:  {  }  [  ]  (  )
/// </summary>
static Int _syntaxRecover[] = {
	TOKEN_RIGHTBRACKET,
	TOKEN_RIGHTBRACE,
	TOKEN_RIGHTPARENTHESIS,
	TOKEN_LEFTBRACKET,
	TOKEN_LEFTBRACE,
	TOKEN_LEFTPARENTHESIS,
};
static Int _syntaxRecoverCount = sizeof(_syntaxRecover) / sizeof(Int);

/// <summary>
/// Reserved class names: These are names that are used in the core grammar currently or that may be used in the future.
/// This list must be in alphabetical (ASCIIbetical) order.
/// </summary>
static const char *_reservedClassNames[] = {
	"ADDEXPR", "ALPHANAME", "ASSIGN",
	"BINARYEXPR", "BOOL", "BYTE",
	"CHAR", "CMPEXPR", "COLONEXPR", "CONSEXPR",
	"DOTEXPR", "DYNSTRING",
	"EXPR", "EXPRS",
	"FLOAT", "FLOAT128", "FLOAT16", "FLOAT32", "FLOAT64", "FLOAT8", "FUNC",
	"INT", "INT128", "INT16", "INT32", "INT64", "INT8",
	"MULEXPR",
	"NAME", "NEWEXPR", "NUMBER",
	"PREFIXEXPR", "POSTFIXEXPR", "PUNCTNAME",
	"RANGEEXPR", "RAWLIST", "RAWLISTTERM", "RAWSTRING", "REAL", "REAL128", "REAL16", "REAL32", "REAL64", "REAL8",
	"SCOPE", "STMT", "STRING",
	"TERM",
	"VARIABLE",
};
static Int _reservedClassNameLength = sizeof(_reservedClassNames) / sizeof(const char *);

static ParseError Parser_ValidateSpecialSyntaxClasses(Symbol nonterminal, SmileList pattern, LexerPosition position);
static ParseResult Parser_ParseSyntaxPattern(Parser parser, SmileList **tailRef);
static ParseResult Parser_ParseSyntaxTerminal(Parser parser, SmileList **tailRef);
static ParseResult Parser_ParseSyntaxNonterminal(Parser parser, SmileList **tailRef);
static ParseResult Parser_ParseSyntaxNonterminalWithDeclarations(Parser parser, Int *numWithSymbols, Symbol **withSymbols);
static void Parser_DeclareNonterminals(SmileList pattern, ParseScope scope, LexerPosition position);

// syntax_expr :: = . syntax_level COLON LBRACKET syntax_pattern RBRACKET IMPLIES raw_list_term
SMILE_INTERNAL_FUNC ParseResult Parser_ParseSyntax(Parser parser, Int modeFlags)
{
	Token token;
	Symbol nonterminal;
	SmileList pattern;
	SmileList *patternTail;
	SmileObject replacement;
	ParseResult parseResult;
	LexerPosition rulePosition, impliesPosition;
	Int templateKind;
	SmileSyntax syntax;
	TemplateResult templateResult;
	ParseError parseError;

	// First, read the syntax predicate's leading nonterminal.
	token = Parser_NextToken(parser);
	rulePosition = Token_GetPosition(token);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_PUNCTNAME
		&& token->kind != TOKEN_UNKNOWNALPHANAME && token->kind != TOKEN_UNKNOWNPUNCTNAME) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, rulePosition, MissingSyntaxClassName));
	}
	nonterminal = token->data.symbol;

	// If the leading nonterminal is the special keyword 'reexport', then flag this scope as
	// reexporting its imported rules, and we're done.
	if (nonterminal == Smile_KnownSymbols.reexport) {
		parser->currentScope->reexport = True;
		return NULL_RESULT();
	}

	// There must be a colon next.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_COLON)
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingSyntaxColon));

	// There must be a left bracket next to start the pattern.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_LEFTBRACKET)
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingSyntaxLeftBracket));

	// Parse the pattern.
	patternTail = &pattern;
	parseResult = Parser_ParseSyntaxPattern(parser, &patternTail);
	if (IS_PARSE_ERROR(parseResult))
		return parseResult;

	// There must be a right bracket to end the pattern.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_RIGHTBRACKET)
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingSyntaxRightBracket));

	// Now, ensure that the special '=>' (implies) symbol exists.
	token = Parser_NextToken(parser);
	if (!(token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
		|| token->data.symbol != Smile_KnownSymbols.implies) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingSyntaxImpliesSymbol));
	}
	impliesPosition = Token_GetPosition(token);

	// Create a new scope for the syntax rule's substitution expression.
	Parser_BeginScope(parser, PARSESCOPE_SYNTAX);

	// Make sure the subsitution expression can "see" the nonterminal variables in scope.
	Parser_DeclareNonterminals(pattern, parser->currentScope, rulePosition);

	// Parse the substitution expression in the syntax rule's scope.
	templateResult = Parser_ParseRawListTerm(parser, modeFlags);
	Parser_EndScope(parser, False);
	if (IS_PARSE_ERROR(templateResult.parseResult))
		return templateResult.parseResult;
	replacement = templateResult.parseResult.expr;
	templateKind = templateResult.templateKind;

	// Make sure the template is an evaluable expression form, not just a raw term.
	replacement = Parser_ConvertItemToTemplateIfNeeded(replacement, templateKind, impliesPosition);

	// Now make sure that the template is evaluable at parse-time.  The set of supported
	// parse-time template-evaluation forms is intentionally limited:
	//
	//   - n (where n is any nonterminal symbol)
	//   - Raw numeric constants, strings, and chars/unis
	//   - [] (i.e., null)
	//   - [$quote x] (for any x)
	//   - [List.of x y z ...]
	//   - [List.join x y z ...]
	//   - [List.cons x y]
	//
	// All other computation is expressly prohibited and must be implemented using macros.
	Parser_VerifySyntaxTemplateIsEvaluableAtParseTime(parser, replacement);

	// Make sure that if this is one of the special (known) syntax classes, that the pattern is
	// a valid form.
	parseError = Parser_ValidateSpecialSyntaxClasses(nonterminal, pattern, rulePosition);
	if (parseError != NULL)
		return ERROR_RESULT(parseError);

	// Everything passes muster, so create the new syntax object.
	syntax = SmileSyntax_Create(nonterminal, pattern, replacement, rulePosition);

	// Everything is all set up, so return the finished syntax object.
	return EXPR_RESULT(syntax);
}

static Int Parser_VerifyArgumentsAreEvaluableAtParseTime(Parser parser, SmileList list)
{
	Bool passed = True;
	Int numArgs = 0;

	for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list), numArgs++) {
		passed &= Parser_VerifySyntaxTemplateIsEvaluableAtParseTime(parser, list->a);
	}

	return SMILE_KIND(list) == SMILE_KIND_NULL ? numArgs : -1;
}

/// <summary>
/// Make sure that the given template expression is evaluable at parse-time.  The set of
/// supported parse-time template-evaluation forms is intentionally very limited:
///
///   - n (where n is any nonterminal symbol)
///   - 123 (all numeric constant values)
///   - "foo" (all string constant values)
///   - 'x' (all character/uni constant values)
///   - [] (i.e., null)
///   - [$quote x] (for any x)
///   - [[$dot List cons] x y]
///   - [[$dot List of] x y z ...]
///   - [[$dot List combine] x y z ...]
///
/// All other computation is expressly prohibited and must be implemented using macros.
/// </summary>
Bool Parser_VerifySyntaxTemplateIsEvaluableAtParseTime(Parser parser, SmileObject expr)
{
	SmileList list;

	switch (SMILE_KIND(expr)) {
		case SMILE_KIND_BOOL:
		case SMILE_KIND_CHAR:
		case SMILE_KIND_UNI:
		case SMILE_KIND_BYTE:
		case SMILE_KIND_INTEGER16:
		case SMILE_KIND_INTEGER32:
		case SMILE_KIND_INTEGER64:
		case SMILE_KIND_INTEGER128:
		case SMILE_KIND_REAL32:
		case SMILE_KIND_REAL64:
		case SMILE_KIND_REAL128:
		case SMILE_KIND_FLOAT32:
		case SMILE_KIND_FLOAT64:
		case SMILE_KIND_FLOAT128:
		case SMILE_KIND_BIGINT:
		case SMILE_KIND_BIGREAL:
		case SMILE_KIND_BIGFLOAT:
			return True;

		case SMILE_KIND_NULL:
			return True;

		case SMILE_KIND_SYMBOL:
			return True;

		case SMILE_KIND_LIST:
			list = (SmileList)expr;

			if (SMILE_KIND(list->a) == SMILE_KIND_SYMBOL) {
				SmileSymbol symbol = (SmileSymbol)list->a;
				if (symbol->symbol == SMILE_SPECIAL_SYMBOL__QUOTE) {
					if (SmileList_SafeLength(list) == 2) return True;

					Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(list, getSourceLocation),
						String_Format("[$quote] is mis-formed in this syntax template; it must have exactly one argument.")));
					return False;
				}
				else {
					Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(list, getSourceLocation),
						String_Format("'%S' is not allowed here in this syntax template (are you incorrectly"
							" trying to call a function inside a syntax template?).",
							SymbolTable_GetName(Smile_SymbolTable, symbol->symbol))));
					return False;
				}
			}

			if (SmileObject_IsCallToSymbol(SMILE_SPECIAL_SYMBOL__DOT, list->a)) {
				SmileList dotList = (SmileList)list->a;

				if (SMILE_KIND(LIST_SECOND(dotList)) == SMILE_KIND_SYMBOL && SMILE_KIND(LIST_THIRD(dotList)) == SMILE_KIND_SYMBOL) {

					SmileSymbol objSymbol = (SmileSymbol)LIST_SECOND(dotList);
					SmileSymbol methodSymbol = (SmileSymbol)LIST_THIRD(dotList);

					if (objSymbol->symbol == Smile_KnownSymbols.List_) {
						if (methodSymbol->symbol == Smile_KnownSymbols.of
							|| methodSymbol->symbol == Smile_KnownSymbols.combine) {
							return Parser_VerifyArgumentsAreEvaluableAtParseTime(parser, LIST_REST(list)) >= 0;
						}
						else if (methodSymbol->symbol == Smile_KnownSymbols.cons) {
							if (Parser_VerifyArgumentsAreEvaluableAtParseTime(parser, LIST_REST(list)) != 2) {
								Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(list, getSourceLocation),
									String_FromC("'List.cons' must be called with exactly two arguments.")));
								return False;
							}
							return True;
						}
						else {
							Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(list, getSourceLocation),
								String_Format("'List.%S' is not allowed here in this syntax template (only List.cons,"
									" List.of, and List.combine are supported in syntax templates).",
									SymbolTable_GetName(Smile_SymbolTable, methodSymbol->symbol))));
							return False;
						}
					}
				}
			}

			// For all other forms, fall-through to default error message.

		default:
			Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(expr, getSourceLocation),
				String_FromC("Syntax templates may only consist of nonterminals, the empty list, [$quote], and certain [List.*] method calls.")));
			return False;
	}
}

/// <summary>
/// Transform the provided expression, which may be any of the three different kinds of
/// template forms, into a spliced-template form so that it may be acted on uniformly.
/// </summary>
SmileObject Parser_ConvertItemToTemplateIfNeeded(SmileObject expr, Int itemTemplateKind, LexerPosition lexerPosition)
{
	// This is a templated list, but not a templated item (or not templated enough).
	// So we need to wrap/quote it before adding it to the list.
	switch (itemTemplateKind) {
		case TemplateKind_None:
			return (SmileObject)SmileList_ConsWithSource(
				(SmileObject)Smile_KnownObjects._quoteSymbol,
				(SmileObject)SmileList_ConsWithSource(expr, NullObject, lexerPosition),
				lexerPosition
			);

		default:
			return expr;
	}
}

// syntax_pattern :: = . syntax_element | . syntax_element syntax_pattern
// syntax_element :: = . syntax_term | . syntax_nonterm
static ParseResult Parser_ParseSyntaxPattern(Parser parser, SmileList **tailRef)
{
	ParseResult parseResult;

	for (;;) {
		switch (Lexer_Peek(parser->lexer)) {
			case TOKEN_LEFTBRACKET:
				parseResult = Parser_ParseSyntaxNonterminal(parser, tailRef);
				if (IS_PARSE_ERROR(parseResult))
					return parseResult;
				break;

			case TOKEN_RIGHTBRACKET:
			case TOKEN_RIGHTPARENTHESIS:
			case TOKEN_RIGHTBRACE:
				return NULL_RESULT();

			default:
				parseResult = Parser_ParseSyntaxTerminal(parser, tailRef);
				if (IS_PARSE_ERROR(parseResult))
					return parseResult;
				break;
		}
	}
}

// syntax_term :: = . any_name | . COMMA | . SEMICOLON | . COLON | . LPAREN syntax_pattern RPAREN | . LBRACE syntax_pattern RBRACE
static ParseResult Parser_ParseSyntaxTerminal(Parser parser, SmileList **tailRef)
{
	SmileList *tail = *tailRef;
	ParseError parseError;
	ParseResult parseResult;

	switch (Lexer_Next(parser->lexer)) {

		case TOKEN_ALPHANAME:
		case TOKEN_PUNCTNAME:
		case TOKEN_UNKNOWNALPHANAME:
		case TOKEN_UNKNOWNPUNCTNAME:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(parser->lexer->token->data.symbol), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL_RESULT();

		case TOKEN_COMMA:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.comma), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL_RESULT();
		case TOKEN_SEMICOLON:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.semicolon), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL_RESULT();
		case TOKEN_COLON:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.colon), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL_RESULT();

		case TOKEN_LEFTPARENTHESIS:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.left_parenthesis), NullObject);
			tail = (SmileList *)&((*tail)->d);

			*tailRef = tail;
			parseResult = Parser_ParseSyntaxPattern(parser, tailRef);
			if (IS_PARSE_ERROR(parseResult))
				return parseResult;
			tail = *tailRef;

			if (Lexer_Next(parser->lexer) != TOKEN_RIGHTPARENTHESIS) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternMismatchedParentheses);
				Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
				return ERROR_RESULT(parseError);
			}

			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.right_parenthesis), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL_RESULT();

		case TOKEN_LEFTBRACE:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.left_brace), NullObject);
			tail = (SmileList *)&((*tail)->d);

			*tailRef = tail;
			parseResult = Parser_ParseSyntaxPattern(parser, tailRef);
			if (IS_PARSE_ERROR(parseResult))
				return parseResult;
			tail = *tailRef;

			if (Lexer_Next(parser->lexer) != TOKEN_RIGHTBRACE) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternMismatchedBraces);
				Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
				return ERROR_RESULT(parseError);
			}

			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.right_brace), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL_RESULT();

		default:
			*tailRef = tail;
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternKeyword));
	}
}

// syntax_nonterm :: = . LBRACKET syntax_with_opt any_name any_name syntax_sep_opt RBRACKET
// syntax_with_opt :: = WITH any_names COLON | 
static ParseResult Parser_ParseSyntaxNonterminal(Parser parser, SmileList **tailRef)
{
	ParseError parseError;
	ParseResult parseResult;
	Symbol nonterminal;
	Symbol name;
	Symbol repeat;
	Symbol separator;
	String punctuationTail;
	Token token;
	Int tokenKind;
	Int numWithSymbols;
	Symbol *withSymbols;

	STATIC_STRING(withKeyword, "with");

	// Left bracket first.
	if (Lexer_Next(parser->lexer) != TOKEN_LEFTBRACKET) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternMismatchedBrackets);
		Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
		return ERROR_RESULT(parseError);
	}

	// Read the next thing, which should be the nonterminal name.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			String_FormatString(MalformedSyntaxPatternIllegalNonterminal, token->text));
		Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
		return ERROR_RESULT(parseError);
	}

	// If the nonterminal name is the special keyword 'with', go parse the list of "with" declarations,
	// and then collect another nonterminal name.
	if (String_Equals(token->text, withKeyword)) {

		// Go parse the with declarations, until we reach a ':' token.
		parseResult = Parser_ParseSyntaxNonterminalWithDeclarations(parser, &numWithSymbols, &withSymbols);
		if (IS_PARSE_ERROR(parseResult)) {
			Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
			return parseResult;
		}

		// Now get the real nonterminal name.
		token = Parser_NextToken(parser);
		if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME) {
			parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_FormatString(MalformedSyntaxPatternIllegalNonterminal, token->text));
			Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
			return ERROR_RESULT(parseError);
		}
	}
	else {
		// No 'with' symbol list.
		withSymbols = NULL;
		numWithSymbols = 0;
	}

	if (String_EndsWith(token->text, String_QuestionMark)) {
		// This ends with a question mark, so strip off the question mark to get the real nonterminal.
		nonterminal = SymbolTable_GetSymbol(Smile_SymbolTable, String_Substring(token->text, 0, String_Length(token->text) - 1));
		repeat = Smile_KnownSymbols.question_mark;
	}
	else {
		nonterminal = SymbolTable_GetSymbol(Smile_SymbolTable, token->text);

		// If the next thing is a punctuation identifier, it must be a repeat symbol.
		token = Parser_NextToken(parser);
		if (token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_UNKNOWNPUNCTNAME) {
			punctuationTail = token->text;

			// Decode the "tail" as the repeat, which must be '?', '*', or '+'.
			if (String_Equals(punctuationTail, String_Plus)) {
				repeat = Smile_KnownSymbols.plus;
			}
			else if (String_Equals(punctuationTail, String_Star)) {
				repeat = Smile_KnownSymbols.star;
			}
			else if (String_Equals(punctuationTail, String_QuestionMark)) {
				repeat = Smile_KnownSymbols.question_mark;
			}
			else {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
					String_FormatString(MalformedSyntaxPatternIllegalNonterminalRepeat, punctuationTail));
				Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
				return ERROR_RESULT(parseError);
			}
		}
		else {
			// No repeat symbol.
			Lexer_Unget(parser->lexer);
			repeat = 0;
		}
	}

	// Read the next thing, which should be the substitution variable name.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			String_FormatString(MalformedSyntaxPatternIllegalNonterminalName, token->text));
		Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
		return ERROR_RESULT(parseError);
	}
	name = SymbolTable_GetSymbol(Smile_SymbolTable, parser->lexer->token->text);

	// Last, but not least, allow an optional trailing ',' or ';' to express the notion of a separator character.
	if ((tokenKind = Lexer_Next(parser->lexer)) == TOKEN_COMMA || tokenKind == TOKEN_SEMICOLON) {

		if (!repeat) {
			parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_FormatString(MalformedSyntaxPatternNonterminalRepeatSeparatorMismatch,
					tokenKind == TOKEN_COMMA ? String_Comma : String_Semicolon));
			Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
			return ERROR_RESULT(parseError);
		}

		separator = (tokenKind == TOKEN_COMMA ? Smile_KnownSymbols.comma : Smile_KnownSymbols.semicolon);
	}
	else {
		Lexer_Unget(parser->lexer);
		separator = 0;
	}

	// Finally, this must finish with a right bracket.
	if (Lexer_Next(parser->lexer) != TOKEN_RIGHTBRACKET) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternMismatchedBrackets);
		Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
		return ERROR_RESULT(parseError);
	}

	// Now construct the nonterminal, since we know it's legal.
	**tailRef = SmileList_Cons((SmileObject)SmileNonterminal_Create(nonterminal, name, repeat, separator, numWithSymbols, withSymbols), NullObject);
	*tailRef = (SmileList *)&((**tailRef)->d);
	return NULL_RESULT();
}

/// <summary>
/// Skim through the given pattern, find all its nonterminals, and declare them as variables in the given scope.
/// </summary>
/// <param name="pattern">The pattern to extract nonterminals from.</param>
/// <param name="scope">The parse scope in which the new variables will be declared.</param>
/// <param name="position">The lexical position where the nonterminals were found, so that the new
/// variables will be considered to have been declared *there*.</param>
static void Parser_DeclareNonterminals(SmileList pattern, ParseScope scope, LexerPosition position)
{
	SmileNonterminal nonterminal;
	ParseDecl decl;

	for (; pattern != NullList; pattern = SmileList_Rest(pattern)) {
		if (SMILE_KIND(pattern->a) == SMILE_KIND_NONTERMINAL) {
			nonterminal = (SmileNonterminal)pattern->a;
			ParseScope_Declare(scope, nonterminal->name, PARSEDECL_VARIABLE, position, &decl);
		}
	}
}

/// <summary>
/// Determine whether the given string refers to a reserved classname.
/// </summary>
/// <param name="clsString">The name of the classname to check.</param>
/// <returns>True if that name is reserved (a known name), false if that name is not reserved (an unknown name).</returns>
static Bool Parser_IsReservedClassName(String clsString)
{
	const char *clsText;
	Int clsLength;
	Int start, mid, end;
	int cmp;

	clsText = String_ToC(clsString);
	clsLength = String_Length(clsString);

	start = 0;
	end = _reservedClassNameLength;

	// Binary search through the _reservedClassNames for the given symbol.
	// This isn't the most elegant solution we could use, but it's fairly fast, has
	// good worst-case performance, and makes the reserved-names list easy to change.
	while (start < end) {
		mid = (start + end) / 2;
		cmp = strcmp(clsText, _reservedClassNames[mid]);
		if (cmp == 0)
			return True;
		if (cmp < 0) {
			end = mid;
		}
		else {
			start = mid + 1;
		}
	}

	// Didn't find it in the reserved set, so it's not reserved.
	return False;
}

/// <summary>
/// Determine whether the given syntax pattern is valid for the given syntax class.
/// </summary>
/// <param name="cls">The name of the class that is about to have the given pattern added to it.</param>
/// <param name="pattern">The parsed syntax pattern to check.</param>
/// <param name="position">The pattern's position in the source, for error-reporting purposes.</param>
/// <returns>NULL if no errors were found, or a ParseError if an error was found in the pattern or
/// in the class it's about to be added to.</returns>
static ParseError Parser_ValidateSpecialSyntaxClasses(Symbol cls, SmileList pattern, LexerPosition position)
{
	ParseError parseError;
	SmileNonterminal nonterminal;
	SmileSymbol smileSymbol;
	String clsString;

	// Invalid patterns early-out.
	if (SMILE_KIND(pattern) != SMILE_KIND_LIST) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidPatternError);
		return parseError;
	}

	// Any class with a hyphen ('-') in its name is custom (i.e., always valid).
	clsString = SymbolTable_GetName(Smile_SymbolTable, cls);
	if (String_IndexOfChar(clsString, '-', 0) >= 0)
		return NULL;

	switch (cls) {
		case SMILE_SPECIAL_SYMBOL_STMT:
			// STMT must always start with a symbol (keyword).
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidKeywordPatternError, "STMT"));
				return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_EXPR:
			// EXPR must always start with a symbol (keyword).
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidKeywordPatternError, "EXPR"));
				return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_CMPEXPR:
			// CMPEXPR must always start with either a symbol (keyword) or an ADDEXPR nonterminal followed by a
			// symbol (keyword) that is not '<' or '>' or '<=' or '>=' or '==' or '!=' or '===' or '!===' or 'is'.
			if (LIST_FIRST(pattern)->kind == SMILE_KIND_SYMBOL)
				return NULL;
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_NONTERMINAL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, BrokenPatternError);
				return parseError;
			}
			nonterminal = (SmileNonterminal)(LIST_FIRST(pattern));
			if (nonterminal->nonterminal != SMILE_SPECIAL_SYMBOL_ADDEXPR) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidCmpExprPatternError);
				return parseError;
			}
			if (LIST_SECOND(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidCmpExprPatternError);
				return parseError;
			}
			smileSymbol = (SmileSymbol)(LIST_SECOND(pattern));
			switch (smileSymbol->symbol) {
				case SMILE_SPECIAL_SYMBOL_LT:
				case SMILE_SPECIAL_SYMBOL_LE:
				case SMILE_SPECIAL_SYMBOL_GT:
				case SMILE_SPECIAL_SYMBOL_GE:
				case SMILE_SPECIAL_SYMBOL_EQ:
				case SMILE_SPECIAL_SYMBOL_NE:
				case SMILE_SPECIAL_SYMBOL_SUPEREQ:
				case SMILE_SPECIAL_SYMBOL_SUPERNE:
				case SMILE_SPECIAL_SYMBOL_IS:
					parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidCmpExprPatternError);
					return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_ADDEXPR:
			// ADDEXPR must always start with either a symbol (keyword) or a MULEXPR nonterminal followed by a
			// symbol (keyword) that is not '+' or '-'.
			if (LIST_FIRST(pattern)->kind == SMILE_KIND_SYMBOL)
				return NULL;
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_NONTERMINAL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, BrokenPatternError);
				return parseError;
			}
			nonterminal = (SmileNonterminal)(LIST_FIRST(pattern));
			if (nonterminal->nonterminal != SMILE_SPECIAL_SYMBOL_MULEXPR) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidAddExprPatternError);
				return parseError;
			}
			if (LIST_SECOND(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidAddExprPatternError);
				return parseError;
			}
			smileSymbol = (SmileSymbol)(LIST_SECOND(pattern));
			if (smileSymbol->symbol == SMILE_SPECIAL_SYMBOL_PLUS || smileSymbol->symbol == SMILE_SPECIAL_SYMBOL_MINUS) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidAddExprPatternError);
				return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_MULEXPR:
			// MULEXPR must always start with either a symbol (keyword) or a BINARYEXPR nonterminal followed by a
			// symbol (keyword) that is not '*' or '/'.
			if (LIST_FIRST(pattern)->kind == SMILE_KIND_SYMBOL)
				return NULL;
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_NONTERMINAL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, BrokenPatternError);
				return parseError;
			}
			nonterminal = (SmileNonterminal)(LIST_FIRST(pattern));
			if (nonterminal->nonterminal != SMILE_SPECIAL_SYMBOL_BINARYEXPR) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidMulExprPatternError);
				return parseError;
			}
			if (LIST_SECOND(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidMulExprPatternError);
				return parseError;
			}
			smileSymbol = (SmileSymbol)(LIST_SECOND(pattern));
			if (smileSymbol->symbol == SMILE_SPECIAL_SYMBOL_STAR || smileSymbol->symbol == SMILE_SPECIAL_SYMBOL_SLASH) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidMulExprPatternError);
				return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_BINARYEXPR:
			// BINARYEXPR must always start with either a symbol (keyword) or a COLONEXPR nonterminal followed by a
			// symbol (keyword).
			if (LIST_FIRST(pattern)->kind == SMILE_KIND_SYMBOL)
				return NULL;
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_NONTERMINAL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, BrokenPatternError);
				return parseError;
			}
			nonterminal = (SmileNonterminal)(LIST_FIRST(pattern));
			if (nonterminal->nonterminal != SMILE_SPECIAL_SYMBOL_COLONEXPR) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidBinaryExprPatternError);
				return parseError;
			}
			if (LIST_SECOND(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidBinaryExprPatternError);
				return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_PREFIXEXPR:
			// PREFIXEXPR must always start with a symbol (keyword).
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidKeywordPatternError, "PREFIXEXPR"));
				return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_POSTFIXEXPR:
			// POSTFIXEXPR must always start with either a symbol (keyword) or a CONSEXPR nonterminal followed by a
			// symbol (keyword).
			if (LIST_FIRST(pattern)->kind == SMILE_KIND_SYMBOL) {
				return NULL;
			}
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_NONTERMINAL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, BrokenPatternError);
				return parseError;
			}
			nonterminal = (SmileNonterminal)(LIST_FIRST(pattern));
			if (nonterminal->nonterminal != SMILE_SPECIAL_SYMBOL_CONSEXPR) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidPostfixExprPatternError);
				return parseError;
			}
			if (LIST_SECOND(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidPostfixExprPatternError);
				return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_TERM:
			// TERM must always start with a symbol (keyword).
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidKeywordPatternError, "TERM"));
				return parseError;
			}
			return NULL;

		default:
			// One kind of error message for known reserved class names...
			if (Parser_IsReservedClassName(clsString)) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidSealedPatternError, String_ToC(clsString)));
				return parseError;
			}
			else {
				// ...another kind of error message if they're just doin' it wrong.
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidClassError, String_ToC(clsString)));
				return parseError;
			}
	}
}

/// <summary>
/// Lexicographically compare the text of two symbols for ordering.  This uses
/// an ASCII-betical sort order.  Returns 0 if a==b, &gt;0 if a&gt;b, and &lt;0 if a&lt;b.
/// </summary>
Inline Int CompareSymbols(Symbol a, Symbol b)
{
	return String_Compare(
		SymbolTable_GetName(Smile_SymbolTable, a),
		SymbolTable_GetName(Smile_SymbolTable, b)
	);
}

/// <summary>
/// Helper comparison function for qsort that invokes CompareSymbols to do the real work.
/// </summary>
static int qsort_CompareSymbols(const void *a, const void *b)
{
	return (int)CompareSymbols(*(Symbol *)a, *(Symbol *)b);
}

/// <summary>
/// Lexicographically sort an array of symbols by their text, in-place.  This uses
/// an ASCII-betical sort order.
/// </summary>
Inline void SortSymbols(Int numSymbols, Symbol *symbols)
{
	qsort((void *)symbols, (size_t)numSymbols, sizeof(Symbol), qsort_CompareSymbols);
}

// syntax_with_opt :: = WITH . syntax_with_names COLON |
// syntax_with_names :: = . syntax_with_names COMMA any_name | . syntax_with_names any_name | . any_name
static ParseResult Parser_ParseSyntaxNonterminalWithDeclarations(Parser parser, Int *numWithSymbolsReturn, Symbol **withSymbolsReturn)
{
	Int numWithSymbols, maxWithSymbols;
	Symbol *withSymbols;
	Token token;
	ParseError parseError;
	Int tokenKind;

	// Create an initial chunk of space for the symbols (up to 16, which is plenty for nearly everything).
	numWithSymbols = 0;
	maxWithSymbols = 16;
	withSymbols = (Symbol *)GC_MALLOC_ATOMIC(sizeof(Symbol) * maxWithSymbols);
	if (withSymbols == NULL)
		Smile_Abort_OutOfMemory();

	do {
		// Read the next thing, which should be a name of some kind.
		token = Parser_NextToken(parser);
		if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME) {
			parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_FormatString(MalformedSyntaxPatternMissingNameAfterWith, token->text));
			Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
			return ERROR_RESULT(parseError);
		}

		// Make enough room in the collection of names for another entry, growing the array if necessary.
		if (numWithSymbols >= maxWithSymbols) {
			Int newMax;
			Symbol *newSymbols;

			newMax = maxWithSymbols * 2;
			newSymbols = (Symbol *)GC_MALLOC_ATOMIC(sizeof(Symbol) * newMax);
			if (newSymbols == NULL)
				Smile_Abort_OutOfMemory();

			MemCpy(newSymbols, withSymbols, sizeof(Symbol) * numWithSymbols);
			withSymbols = newSymbols;
			maxWithSymbols = newMax;
		}

		// Add the new name to the collection of names.
		withSymbols[numWithSymbols++] = token->data.symbol;

		// Optionally consume a comma.
		tokenKind = Lexer_Peek(parser->lexer);
		if (tokenKind == TOKEN_COMMA) {
			Lexer_Next(parser->lexer);
		}

		// Keep going until we see a colon.
	} while (tokenKind != TOKEN_COLON);

	// Got a colon, so consume it.
	Lexer_Next(parser->lexer);

	// Sort the symbols ASCII-betically so that symbol arrays may be easily compared and diff'ed.
	SortSymbols(numWithSymbols, withSymbols);

	// Return the resulting collection of names.
	*numWithSymbolsReturn = numWithSymbols;
	*withSymbolsReturn = withSymbols;
	return NULL_RESULT();
}
