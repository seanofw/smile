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
#include <smile/smiletypes/smilesyntax.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/stringbuilder.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/identkind.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

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

STATIC_STRING(String_Plus, "+");
STATIC_STRING(String_Star, "*");
STATIC_STRING(String_QuestionMark, "?");
STATIC_STRING(String_Comma, ",");
STATIC_STRING(String_Semicolon, ";");

static Int _syntaxRecover[] = {
	TOKEN_RIGHTBRACKET,
	TOKEN_RIGHTBRACE,
	TOKEN_RIGHTPARENTHESIS,
	TOKEN_LEFTBRACKET,
	TOKEN_LEFTBRACE,
	TOKEN_LEFTPARENTHESIS,
};

static Int _syntaxRecoverCount = sizeof(_syntaxRecover) / sizeof(Int);

static ParseError Parser_ParseSyntaxPattern(Parser parser, SmileList **tailRef);
static ParseError Parser_ParseSyntaxTerminal(Parser parser, SmileList **tailRef);
static ParseError Parser_ParseSyntaxNonterminal(Parser parser, SmileList **tailRef);
static void Parser_DeclareNonterminals(SmileList pattern, ParseScope scope, LexerPosition position);

// syntax_expr :: = . syntax_level COLON LBRACKET syntax_pattern RBRACKET IMPLIES expr
SMILE_INTERNAL_FUNC ParseError Parser_ParseSyntax(Parser parser, SmileObject *expr, Int modeFlags)
{
	Token token;
	Symbol nonterminal;
	SmileList pattern;
	SmileList *patternTail;
	SmileObject replacement;
	ParseError parseError;
	LexerPosition rulePosition;
	Bool isTemplate;

	// First, read the syntax predicate's leading nonterminal.
	token = Parser_NextToken(parser);
	rulePosition = Token_GetPosition(token);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_PUNCTNAME
		&& token->kind != TOKEN_UNKNOWNALPHANAME && token->kind != TOKEN_UNKNOWNPUNCTNAME) {
		*expr = NullObject;
		return ParseMessage_Create(PARSEMESSAGE_ERROR, rulePosition, MissingSyntaxClassName);
	}
	nonterminal = token->data.symbol;

	// There must be a colon next.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_COLON) {
		*expr = NullObject;
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingSyntaxColon);
	}

	// There must be a left bracket next to start the pattern.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_LEFTBRACKET) {
		*expr = NullObject;
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingSyntaxLeftBracket);
	}

	// Parse the pattern.
	patternTail = &pattern;
	parseError = Parser_ParseSyntaxPattern(parser, &patternTail);
	if (parseError != NULL) {
		*expr = NullObject;
		return parseError;
	}

	// There must be a right bracket to end the pattern.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_RIGHTBRACKET) {
		*expr = NullObject;
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingSyntaxRightBracket);
	}

	// Now, ensure that the special '=>' (implies) symbol exists.
	token = Parser_NextToken(parser);
	if (!(token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
		|| token->data.symbol != Smile_KnownSymbols.implies) {
		*expr = NullObject;
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingSyntaxImpliesSymbol);
	}

	// Create a new scope for the syntax rule's substitution expression.
	Parser_BeginScope(parser, PARSESCOPE_SYNTAX);

	// Make sure the subsitution expression can "see" the nonterminal variables in scope.
	Parser_DeclareNonterminals(pattern, parser->currentScope, rulePosition);

	// Parse the substitution expression in the syntax rule's scope.
	parseError = Parser_ParseRawListTerm(parser, &replacement, &isTemplate, modeFlags);
	Parser_EndScope(parser);
	if (parseError != NULL) {
		*expr = NullObject;
		return parseError;
	}

	// Everything passes muster, so create and return the new syntax object.
	*expr = (SmileObject)SmileSyntax_Create(nonterminal, pattern, replacement, rulePosition);
	return NULL;
}

// syntax_pattern :: = syntax_element | syntax_element syntax_pattern
// syntax_element :: = syntax_term | syntax_nonterm
static ParseError Parser_ParseSyntaxPattern(Parser parser, SmileList **tailRef)
{
	ParseError parseError;

	for (;;) {
		switch (Lexer_Peek(parser->lexer)) {
			case TOKEN_LEFTBRACKET:
				parseError = Parser_ParseSyntaxNonterminal(parser, tailRef);
				if (parseError != NULL)
					return parseError;
				break;

			case TOKEN_RIGHTBRACKET:
			case TOKEN_RIGHTPARENTHESIS:
			case TOKEN_RIGHTBRACE:
				return NULL;

			default:
				parseError = Parser_ParseSyntaxTerminal(parser, tailRef);
				if (parseError != NULL)
					return parseError;
				break;
		}
	}
}

// syntax_term :: = any_name | COMMA | SEMICOLON | COLON | LPAREN syntax_pattern RPAREN | LBRACE syntax_pattern RBRACE
static ParseError Parser_ParseSyntaxTerminal(Parser parser, SmileList **tailRef)
{
	SmileList *tail = *tailRef;
	ParseError parseError;

	switch (Lexer_Next(parser->lexer)) {

		case TOKEN_ALPHANAME:
		case TOKEN_PUNCTNAME:
		case TOKEN_UNKNOWNALPHANAME:
		case TOKEN_UNKNOWNPUNCTNAME:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(parser->lexer->token->data.symbol), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL;

		case TOKEN_COMMA:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.comma), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL;
		case TOKEN_SEMICOLON:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.semicolon), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL;
		case TOKEN_COLON:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.colon), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL;

		case TOKEN_LEFTPARENTHESIS:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.left_parenthesis), NullObject);
			tail = (SmileList *)&((*tail)->d);

			*tailRef = tail;
			parseError = Parser_ParseSyntaxPattern(parser, tailRef);
			if (parseError != NULL)
				return parseError;
			tail = *tailRef;

			if (Lexer_Next(parser->lexer) != TOKEN_RIGHTPARENTHESIS) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternMismatchedParentheses);
				Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
				return parseError;
			}

			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.right_parenthesis), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL;

		case TOKEN_LEFTBRACE:
			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.left_brace), NullObject);
			tail = (SmileList *)&((*tail)->d);

			*tailRef = tail;
			parseError = Parser_ParseSyntaxPattern(parser, tailRef);
			if (parseError != NULL)
				return parseError;
			tail = *tailRef;

			if (Lexer_Next(parser->lexer) != TOKEN_RIGHTBRACE) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternMismatchedBraces);
				Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
				return parseError;
			}

			*tail = SmileList_Cons((SmileObject)SmileSymbol_Create(Smile_KnownSymbols.right_brace), NullObject);
			tail = (SmileList *)&((*tail)->d);
			*tailRef = tail;
			return NULL;

		default:
			*tailRef = tail;
			return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternKeyword);
	}
}

// syntax_nonterm :: = LBRACKET any_name any_name syntax_sep_opt RBRACKET
static ParseError Parser_ParseSyntaxNonterminal(Parser parser, SmileList **tailRef)
{
	ParseError parseError;
	Symbol nonterminal;
	Symbol name;
	Symbol repeat;
	Symbol separator;
	String punctuationTail;
	Token token;
	Int tokenKind;

	// Left bracket first.
	if (Lexer_Next(parser->lexer) != TOKEN_LEFTBRACKET) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternMismatchedBrackets);
		Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
		return parseError;
	}

	// Read the next thing, which should be the nonterminal name.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			String_FormatString(MalformedSyntaxPatternIllegalNonterminal, token->text));
		Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
		return parseError;
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
				return parseError;
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
		return parseError;
	}
	name = SymbolTable_GetSymbol(Smile_SymbolTable, parser->lexer->token->text);

	// Last, but not least, allow an optional trailing ',' or ';' to express the notion of a separator character.
	if ((tokenKind = Lexer_Peek(parser->lexer)) == TOKEN_COMMA || tokenKind == TOKEN_SEMICOLON) {
		Lexer_Next(parser->lexer);

		if (!repeat) {
			parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_FormatString(MalformedSyntaxPatternNonterminalRepeatSeparatorMismatch,
					tokenKind == TOKEN_COMMA ? String_Comma : String_Semicolon));
			Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
			return parseError;
		}

		separator = (tokenKind == TOKEN_COMMA ? Smile_KnownSymbols.comma : Smile_KnownSymbols.semicolon);
	}
	else separator = 0;

	// Finally, this must finish with a right bracket.
	if (Lexer_Next(parser->lexer) != TOKEN_RIGHTBRACKET) {
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token), MalformedSyntaxPatternMismatchedBrackets);
		Parser_Recover(parser, _syntaxRecover, _syntaxRecoverCount);
		return parseError;
	}

	// Now construct the nonterminal, since we know it's legal.
	**tailRef = SmileList_Cons((SmileObject)SmileNonterminal_Create(nonterminal, name, repeat, separator), NullObject);
	*tailRef = (SmileList *)&((**tailRef)->d);
	return NULL;
}

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
