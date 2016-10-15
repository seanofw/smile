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

STATIC_STRING(InvalidPatternError, "Syntax patterns must be non-empty lists.");
STATIC_STRING(BrokenPatternError, "Internal error: Syntax patterns must only consist of symbols and nonterminals.");

STATIC_STRING(InvalidSealedPatternError, "Syntax patterns may not be added to the special %s class.");
STATIC_STRING(InvalidClassError, "Unknown or illegal syntax class '%s' (did you forget a hyphen?)");
STATIC_STRING(InvalidKeywordPatternError, "Syntax patterns in the %s class must start with a keyword.");

STATIC_STRING(InvalidCmpExprPatternError, "Syntax patterns in the CMPEXPR class must either start with a keyword, or with an ADDEXPR nonterminal followed by a keyword that is none of the nine standard comparison operators.");
STATIC_STRING(InvalidAddExprPatternError, "Syntax patterns in the ADDEXPR class must either start with a keyword, or with a MULEXPR nonterminal followed by a keyword that is neither '+' or '-'.");
STATIC_STRING(InvalidMulExprPatternError, "Syntax patterns in the MULEXPR class must either start with a keyword, or with a BINARYEXPR nonterminal followed by a keyword that is neither '*' or '/'.");
STATIC_STRING(InvalidBinaryExprPatternError, "Syntax patterns in the BINARYEXPR class must either start with a keyword, or with a COLONEXPR nonterminal followed by a keyword.");
STATIC_STRING(InvalidPostfixPatternError, "Syntax patterns in the POSTFIX class must either start with a keyword, or with a DOUBLEHASH nonterminal followed by a keyword.");

STATIC_STRING(String_Plus, "+");
STATIC_STRING(String_Star, "*");
STATIC_STRING(String_QuestionMark, "?");
STATIC_STRING(String_Comma, ",");
STATIC_STRING(String_Semicolon, ";");

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
	"CHAR", "CMPEXPR", "COLONEXPR",
	"DOT", "DOUBLEHASH", "DYNSTRING",
	"EXPR", "EXPRS",
	"FLOAT", "FLOAT128", "FLOAT16", "FLOAT32", "FLOAT64", "FLOAT8", "FUNC",
	"INT", "INT128", "INT16", "INT32", "INT64", "INT8",
	"MULEXPR",
	"NAME", "NEW", "NUMBER",
	"POSTFIX", "PUNCTNAME",
	"RANGE", "RAWLIST", "RAWLISTTERM", "RAWSTRING", "REAL", "REAL128", "REAL16", "REAL32", "REAL64", "REAL8",
	"SCOPE", "STMT", "STRING",
	"TERM",
	"UNARY",
	"VARIABLE",
};
static Int _reservedClassNameLength = sizeof(_reservedClassNames) / sizeof(const char *);

static ParseError Parser_ValidateSpecialSyntaxClasses(Symbol nonterminal, SmileList pattern, LexerPosition position);
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

	// Make sure that if this is one of the special (known) syntax classes, that the pattern is
	// a valid form.
	parseError = Parser_ValidateSpecialSyntaxClasses(nonterminal, pattern, rulePosition);
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

		case SMILE_SPECIAL_SYMBOL_UNARY:
			// UNARY must always start with a symbol (keyword).
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidKeywordPatternError, "UNARY"));
				return parseError;
			}
			return NULL;

		case SMILE_SPECIAL_SYMBOL_POSTFIX:
			// POSTFIX must always start with either a symbol (keyword) or a DOUBLEHASH nonterminal followed by a
			// symbol (keyword).
			if (LIST_FIRST(pattern)->kind == SMILE_KIND_SYMBOL) {
				return NULL;
			}
			if (LIST_FIRST(pattern)->kind != SMILE_KIND_NONTERMINAL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, BrokenPatternError);
				return parseError;
			}
			nonterminal = (SmileNonterminal)(LIST_FIRST(pattern));
			if (nonterminal->nonterminal != SMILE_SPECIAL_SYMBOL_DOUBLEHASH) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidPostfixPatternError);
				return parseError;
			}
			if (LIST_SECOND(pattern)->kind != SMILE_KIND_SYMBOL) {
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, InvalidPostfixPatternError);
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
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidSealedPatternError, "DOT"));
				return parseError;
			}
			else {
				// ...another kind of error message if they're just doin' it wrong.
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FormatString(InvalidClassError, clsString));
				return parseError;
			}
	}
}
