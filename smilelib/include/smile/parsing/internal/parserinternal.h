#ifndef __SMILE_PARSING_INTERNAL_PARSERINTERNAL_H__
#define __SMILE_PARSING_INTERNAL_PARSERINTERNAL_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_PARSING_PARSER_H__
#include <smile/parsing/parser.h>
#endif

//-------------------------------------------------------------------------------------------------
// Types and constants

// Mode flags.

#define BINARYLINEBREAKS_DISALLOWED		0			// Declare that line breaks are disallowed before a binary operator, causing the start of a new expression.
#define BINARYLINEBREAKS_ALLOWED		(1 << 0)	// Declare that line breaks are allowed before a binary operator, allowing the expression to cross line breaks.
#define BINARYLINEBREAKS_MASK			(1 << 0)
		
#define COMMAMODE_NORMAL				0			// Declare that commas delineate successive operands in N-ary operations.
#define COMMAMODE_VARIABLEDECLARATION	(1 << 1)	// Declare that commas are being used to separate successive variable declarations.
#define COMMAMODE_MASK					(1 << 1)
		
#define COLONMODE_MEMBERACCESS			0			// Declare that colons are used for member-retrieval.
#define COLONMODE_MEMBERDECL			(1 << 2)	// Declare that colons are used for member-declaration.
#define COLONMODE_MASK					(1 << 2)	
		
#define SYNTAXROOT_ASIS					0			// Parse the custom syntax rule as-is.
#define SYNTAXROOT_NONTERMINAL			1			// Parse the custom syntax rule, skipping a preexisting nonterminal.
#define SYNTAXROOT_KEYWORD				2			// Parse the custom syntax rule, but only rules starting with an initial keyword.
#define SYNTAXROOT_RECURSE				3			// Parse the custom syntax rule, recursing from a parent custom syntax rule.

typedef enum {
	TemplateKind_None = 0,
	TemplateKind_Template = 1,
	TemplateKind_TemplateWithSplicing = 2,
} TemplateKind;

// Helpers for generating the ParseResult structures returned by each parse function.

// Return a parse result that contains an error.
Inline ParseResult ERROR_RESULT(ParseMessage error)
{
	ParseResult parseResult;
	parseResult.status = ParseStatus_PartialParseWithError;
	parseResult.expr = NULL;
	parseResult.error = error;
	return parseResult;
};

// Return a parse result that contains a successfully-parsed expression.
Inline ParseResult SUCCESS_RESULT(SmileObject expr)
{
	ParseResult parseResult;
	parseResult.status = ParseStatus_SuccessfullyParsed;
	parseResult.expr = expr;
	parseResult.error = NULL;
	return parseResult;
}

// Return a parse result that indicates nothing was consumed and nothing resulted from it.
Inline ParseResult NOMATCH_RESULT(void)
{
	ParseResult parseResult;
	parseResult.status = ParseStatus_NotMatchedAndNoTokensConsumed;
	parseResult.expr = NULL;
	parseResult.error = NULL;
	return parseResult;
}

//-------------------------------------------------------------------------------------------------
// Parser-internal methods

SMILE_INTERNAL_FUNC ParseError Parser_ParseScope(Parser parser, SmileObject *expr);
SMILE_INTERNAL_FUNC SmileObject Parser_ParseScopeBody(Parser parser, ParseScope *parseScope);
SMILE_INTERNAL_FUNC ParseError Parser_ParseStmt(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC void Parser_ImportExternalVars(Parser parser, SmileList *head, SmileList *tail);

SMILE_INTERNAL_FUNC ParseError Parser_ParseInclude(Parser parser, SmileObject *expr);

SMILE_INTERNAL_FUNC ParseError Parser_ParseIfUnless(Parser parser, SmileObject *expr, Int modeFlags, Bool invert, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseDo(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseWhileUntil(Parser parser, SmileObject *expr, Int modeFlags, Bool invert, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseReturn(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseTry(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseTill(Parser parser, SmileObject *expr, Int modeFlags, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseTillNames(Parser parser, SmileList *names);
SMILE_INTERNAL_FUNC ParseError Parser_ParseTillName(Parser parser, SmileObject *expr);
SMILE_INTERNAL_FUNC ParseError Parser_ParseWhens(Parser parser, SmileObject *expr, Int32Int32Dict tillFlags, Int modeFlags);

SMILE_INTERNAL_FUNC ParseError Parser_ParseVarDecls(Parser parser, SmileObject *expr, Int modeFlags, Int declKind);
SMILE_INTERNAL_FUNC ParseError Parser_ParseKeywordList(Parser parser, SmileObject *expr);
SMILE_INTERNAL_FUNC ParseError Parser_ParseDecl(Parser parser, SmileObject *expr, Int modeFlags, Int declKind);
SMILE_INTERNAL_FUNC ParseError Parser_ParseOpEquals(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseEquals(Parser parser, SmileObject *expr, Int modeFlags);

SMILE_INTERNAL_FUNC ParseError Parser_ParseOrExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseAndExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseNotExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseCmpExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseAddExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseMulExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseBinaryExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseColonExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseRangeExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParsePrefixExpr(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseNewExpr(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC Bool Parser_ParseMembers(Parser parser, SmileObject *expr);
SMILE_INTERNAL_FUNC ParseError Parser_ParsePostfixExpr(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseError Parser_ParseConsExpr(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseError Parser_ParseDotExpr(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseError Parser_ParseTerm(Parser parser, SmileObject *expr, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseError Parser_ParseParentheses(Parser parser, SmileObject *result, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseAnyName(Parser parser, SmileObject *expr);

SMILE_INTERNAL_FUNC void Parser_ParseCallArgsOpt(Parser parser, SmileList *head, SmileList *tail, Int modeFlags);

SMILE_INTERNAL_FUNC ParseError Parser_ParseQuotedTerm(Parser parser, SmileObject *result, Int modeFlags, LexerPosition position);
SMILE_INTERNAL_FUNC ParseError Parser_ParseRawListTerm(Parser parser, SmileObject *result, Int *templateKind, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseRawListItemsOpt(Parser parser, SmileList *head, SmileList *tail, Int *templateKind, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseRawListDotExpr(Parser parser, SmileObject *result, Int *templateKind, Int modeFlags);

SMILE_INTERNAL_FUNC ParseError Parser_ParseDynamicString(Parser parser, SmileObject *expr, String text, LexerPosition startPosition);

SMILE_INTERNAL_FUNC ParseError Parser_ParseFunc(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseParamsOpt(Parser parser, SmileList *params);
SMILE_INTERNAL_FUNC ParseError Parser_ParseParam(Parser parser, SmileObject *param, LexerPosition *position);
SMILE_INTERNAL_FUNC ParseError Parser_ParseParamType(Parser parser, SmileObject *type);

SMILE_INTERNAL_FUNC ParseError Parser_ParseSyntax(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseError Parser_ParseLoanword(Parser parser, SmileObject *expr, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ApplyCustomSyntax(Parser parser, Int modeFlags, Symbol syntaxClassSymbol,
	Int syntaxRootMode, Symbol rootSkipSymbol);
SMILE_INTERNAL_FUNC ParseError Parser_ApplyCustomLoanword(Parser parser, Token token, SmileObject *result);
SMILE_INTERNAL_FUNC SmileObject Parser_RecursivelyApplyTemplate(Parser parser, SmileObject expr, Int32Dict replacements, LexerPosition lexerPosition);

SMILE_INTERNAL_FUNC ParseError Parser_ParseClassicFn(Parser parser, SmileObject *result, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseQuoteBody(Parser parser, SmileObject *result, Int modeFlags, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseClassicQuote(Parser parser, SmileObject *result, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseClassicScope(Parser parser, SmileObject *result, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseClassicTill(Parser parser, SmileObject *result, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseClassicNew(Parser parser, SmileObject *result, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ParseClassicSet(Parser parser, SmileObject *result, LexerPosition startPosition);

SMILE_INTERNAL_FUNC SmileObject Parser_WrapTemplateForSplicing(SmileObject obj);
SMILE_INTERNAL_FUNC SmileObject Parser_ConvertItemToTemplateIfNeeded(SmileObject expr, Int itemTemplateKind, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC Bool Parser_VerifySyntaxTemplateIsEvaluableAtParseTime(Parser parser, SmileObject expr);

SMILE_INTERNAL_FUNC Token Parser_Recover(Parser parser, Int *tokenKinds, Int numTokenKinds);
SMILE_INTERNAL_FUNC Bool Parser_IsLValue(SmileObject obj);
SMILE_INTERNAL_FUNC Bool Parser_HasEqualLookahead(Parser parser);
SMILE_INTERNAL_FUNC Bool Parser_HasEqualOrColonLookahead(Parser parser);
SMILE_INTERNAL_FUNC Bool Parser_HasLookahead(Parser parser, Int tokenKind);
SMILE_INTERNAL_FUNC Bool Parser_Has2Lookahead(Parser parser, Int tokenKind1, Int tokenKind2);
SMILE_INTERNAL_FUNC Bool Parser_Peek2(Parser parser, Token *token1, Token *token2);
SMILE_INTERNAL_FUNC ParseError Parser_ExpectLeftBracket(Parser parser, SmileObject *result,
	Token firstUnaryTokenForErrorReporting, const char *name, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseError Parser_ExpectRightBracket(Parser parser, SmileObject *result,
	Token firstUnaryTokenForErrorReporting, const char *name, LexerPosition startPosition);

SMILE_INTERNAL_DATA Int *Parser_BracesBracketsParenthesesBar_Recovery;
SMILE_INTERNAL_DATA Int Parser_BracesBracketsParenthesesBar_Count;
SMILE_INTERNAL_DATA Int *Parser_RightBracesBracketsParentheses_Recovery;
SMILE_INTERNAL_DATA Int Parser_RightBracesBracketsParentheses_Count;
SMILE_INTERNAL_DATA Int *Parser_BracesBracketsParenthesesBarName_Recovery;
SMILE_INTERNAL_DATA Int Parser_BracesBracketsParenthesesBarName_Count;

SMILE_INTERNAL_DATA SmileObject Parser_IgnorableObject;

//-------------------------------------------------------------------------------------------------
// Inline helper methods

/// <summary>
/// Determine if the given haystack of integers contains the given needle.
/// </summary>
/// <param name="needle">The integer to search for.</param>
/// <param name="haystack">The base pointer of the haystack that may or may not contain the
/// needle.</param>
/// <param name="count">The number of items in the haystack to test.</param>
/// <returns>True if the given needle can be found in the haystack; False if the haystack does
/// not contain the needle.</returns>
Inline Bool IntArrayContains(Int needle, Int *haystack, Int count)
{
	while (count--) {
		if (*haystack++ == needle)
			return True;
	}
	return False;
}

/// <summary>
/// Read the next token from the input stream.  If the token is an identifier, correctly map
/// it to its declaration (or lack thereof) in the current scope.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>The next token in the input stream.</returns>
Inline Token Parser_NextToken(Parser parser)
{
	Token token;
	Int tokenKind;
	Symbol symbol;

	tokenKind = Lexer_Next(parser->lexer);
	token = parser->lexer->token;

	switch (tokenKind) {

		case TOKEN_ALPHANAME:
		case TOKEN_UNKNOWNALPHANAME:
			if (token->data.symbol == 0) {
				token->data.symbol = symbol = SymbolTable_GetSymbol(Smile_SymbolTable, token->text);
			}
			else {
				symbol = token->data.symbol;
			}
			token->kind = ParseScope_IsDeclared(parser->currentScope, token->data.symbol) ? TOKEN_ALPHANAME : TOKEN_UNKNOWNALPHANAME;
			break;

		case TOKEN_PUNCTNAME:
		case TOKEN_UNKNOWNPUNCTNAME:
			if (token->data.symbol == 0) {
				token->data.symbol = symbol = SymbolTable_GetSymbol(Smile_SymbolTable, token->text);
			}
			else {
				symbol = token->data.symbol;
			}
			token->kind = ParseScope_IsDeclared(parser->currentScope, token->data.symbol) ? TOKEN_PUNCTNAME : TOKEN_UNKNOWNPUNCTNAME;
			break;
	}

	return token;
}

/// <summary>
/// Read the next token from the input stream.  If the token is an identifier, correctly map
/// it to its declaration (or lack thereof) in the current scope.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <param name="parseDecl">The declaration for this token, if it is a declared token.</param>
/// <returns>The next token in the input stream.</returns>
Inline Token Parser_NextTokenWithDeclaration(Parser parser, ParseDecl *parseDecl)
{
	Token token;
	Int tokenKind;
	Symbol symbol;
	ParseDecl decl;

	tokenKind = Lexer_Next(parser->lexer);
	token = parser->lexer->token;

	switch (tokenKind) {

	case TOKEN_ALPHANAME:
	case TOKEN_UNKNOWNALPHANAME:
		if (token->data.symbol == 0) {
			token->data.symbol = symbol = SymbolTable_GetSymbol(Smile_SymbolTable, token->text);
		}
		else {
			symbol = token->data.symbol;
		}
		decl = ParseScope_FindDeclaration(parser->currentScope, token->data.symbol);
		token->kind = decl != NULL ? TOKEN_ALPHANAME : TOKEN_UNKNOWNALPHANAME;
		*parseDecl = decl;
		break;

	case TOKEN_PUNCTNAME:
	case TOKEN_UNKNOWNPUNCTNAME:
		if (token->data.symbol == 0) {
			token->data.symbol = symbol = SymbolTable_GetSymbol(Smile_SymbolTable, token->text);
		}
		else {
			symbol = token->data.symbol;
		}
		decl = ParseScope_FindDeclaration(parser->currentScope, token->data.symbol);
		token->kind = decl != NULL ? TOKEN_PUNCTNAME : TOKEN_UNKNOWNPUNCTNAME;
		*parseDecl = decl;
		break;
	
	default:
		*parseDecl = NULL;
		break;
	}

	return token;
}

/// <summary>
/// Enter a new parsing scope (a function, for example), in which local declarations will be contained.
/// </summary>
/// <param name="parser">The parser that is about to enter a new parsing scope.</param>
/// <param name="parseScopeKind">What kind of parsing scope to enter (one of the PARSESCOPE_* values).</param>
Inline void Parser_BeginScope(Parser parser, Int parseScopeKind)
{
	ParseScope newScope = ParseScope_CreateChild(parser->currentScope, parseScopeKind);
	parser->currentScope = newScope;
}

/// <summary>
/// End the current parsing scope (a function, for example), in which local declarations are contained.
/// </summary>
/// <param name="parser">The parser that is ending its current parsing scope.</param>
Inline void Parser_EndScope(Parser parser, Bool keepScopeData)
{
	ParseScope currentScope = parser->currentScope;
	parser->currentScope = currentScope->parentScope;
	if (!keepScopeData)
		ParseScope_Finish(currentScope);
}

#endif
