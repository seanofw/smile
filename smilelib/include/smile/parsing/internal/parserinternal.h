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

//-------------------------------------------------------------------------------------------------
// Types and helpers for functions returning templates.

typedef enum {
	TemplateKind_None = 0,
	TemplateKind_Template = 1,
	TemplateKind_TemplateWithSplicing = 2,
} TemplateKind;

typedef struct TemplateResultStruct {
	ParseResult parseResult;
	Int templateKind;
} TemplateResult;

#define TEMPLATE_RESULT(__parseResult__, __templateKind__) \
	(TemplateResult_Create((__parseResult__), (__templateKind__)))

Inline TemplateResult TemplateResult_Create(ParseResult parseResult, Int templateKind)
{
	TemplateResult templateResult;
	templateResult.parseResult = parseResult;
	templateResult.templateKind = templateKind;
	return templateResult;
}

// If a parse error has been raised, pass it up to the caller, or abort (if necessary).
#define RETURN_TEMPLATE_PARSE_ERROR(__result__) do { \
		if ((__result__).status == ParseStatus_PartialParseWithError) \
			return TEMPLATE_RESULT((__result__), TemplateKind_None); \
		else if ((__result__).status == ParseStatus_NotMatchedAndNoTokensConsumed) \
			Smile_Abort_FatalError(String_ToC(String_Format("Unexpected ParseStatus_NotMatchedAndNoTokensConsumed in %s().", __FUNCTION__))); \
		else \
			Smile_Abort_FatalError(String_ToC(String_Format("Unknown ParseStatus value in %s().", __FUNCTION__))); \
	} while (0)

//-------------------------------------------------------------------------------------------------
// Helpers for generating the ParseResult structures returned by each parse function.

// Return a parse result that contains an error that has not yet been recovered from.
#define ERROR_RESULT(__error__) (ParseResult_Create(ParseStatus_PartialParseWithError, NULL, (__error__)))

// Return a parse result that contains a successfully-parsed expression.
#define EXPR_RESULT(__expr__) (ParseResult_Create(ParseStatus_SuccessfulWithResult, (SmileObject)(__expr__), NULL))

// Return a parse result that contains no result, as it is a recovery-from-error.
#define RECOVERY_RESULT() (ParseResult_Create(ParseStatus_ErroredButRecovered, NullObject, NULL))

// Return a parse result that indicates nothing was consumed and nothing resulted from it.
#define NOMATCH_RESULT() ((ParseResult){ .status = ParseStatus_NotMatchedAndNoTokensConsumed, NULL, NULL })

// Return a parse result that indicates nothing was consumed and nothing resulted from it.
#define NULL_RESULT() (ParseResult_Create(ParseStatus_SuccessfulWithNoResult, NullObject, NULL))

// Determine whether this result represents an error.
#define IS_PARSE_ERROR(__result__) ((__result__).status <= 0)

// If a parse error has been raised, pass it up to the caller, or abort (if necessary).
#define RETURN_PARSE_ERROR(__result__) do { \
		if ((__result__).status == ParseStatus_PartialParseWithError) \
			return (__result__); \
		else if ((__result__).status == ParseStatus_NotMatchedAndNoTokensConsumed) \
			Smile_Abort_FatalError(String_ToC(String_Format("Unexpected ParseStatus_NotMatchedAndNoTokensConsumed in %s().", __FUNCTION__))); \
		else \
			Smile_Abort_FatalError(String_ToC(String_Format("Unknown ParseStatus value in %s().", __FUNCTION__))); \
	} while (0)

// If a parse error has been raised, log it, or abort (if necessary), resulting in a clean parser.
#define HANDLE_PARSE_ERROR(__parser__, __result__) do { \
		if ((__result__).status == ParseStatus_PartialParseWithError) \
			Parser_AddMessage((__parser__), (__result__).error); \
		else if ((__result__).status == ParseStatus_NotMatchedAndNoTokensConsumed) \
			Smile_Abort_FatalError(String_ToC(String_Format("Unexpected ParseStatus_NotMatchedAndNoTokensConsumed in %s().", __FUNCTION__))); \
		else \
			Smile_Abort_FatalError(String_ToC(String_Format("Unknown ParseStatus value in %s().", __FUNCTION__))); \
	} while (0)

//-------------------------------------------------------------------------------------------------
// Parser-internal methods

SMILE_INTERNAL_FUNC Token Parser_NextToken(Parser parser);
SMILE_INTERNAL_FUNC Token Parser_NextTokenWithDeclaration(Parser parser, ParseDecl* parseDecl);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseScope(Parser parser);
SMILE_INTERNAL_FUNC SmileObject Parser_ParseScopeBody(Parser parser, ParseScope *parseScope);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseStmt(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC void Parser_ImportExternalVars(Parser parser, SmileList *head, SmileList *tail);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseInclude(Parser parser);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseIfUnless(Parser parser, Int modeFlags, Bool invert, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseDo(Parser parser, Int modeFlags, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseWhileUntil(Parser parser, Int modeFlags, Bool invert, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseReturn(Parser parser, Int modeFlags, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseTry(Parser parser, Int modeFlags, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseTill(Parser parser, Int modeFlags, LexerPosition lexerPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseTillNames(Parser parser);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseTillName(Parser parser);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseWhens(Parser parser, Int32Int32Dict tillFlags, Int modeFlags);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseVarDecls(Parser parser, Int modeFlags, Int declKind);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseKeywordList(Parser parser);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseDecl(Parser parser, Int modeFlags, Int declKind);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseOpEquals(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseEquals(Parser parser, Int modeFlags);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseOrExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseAndExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseNotExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseCmpExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseAddExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseMulExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseBinaryExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseColonExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseRangeExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParsePrefixExpr(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseNewExpr(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseMembers(Parser parser);
SMILE_INTERNAL_FUNC ParseResult Parser_ParsePostfixExpr(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseConsExpr(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseDotExpr(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseTerm(Parser parser, Int modeFlags, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseParentheses(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseAnyName(Parser parser);

SMILE_INTERNAL_FUNC void Parser_ParseCallArgsOpt(Parser parser, SmileList *head, SmileList *tail, Int modeFlags);

SMILE_INTERNAL_FUNC TemplateResult Parser_ParseQuotedTerm(Parser parser, Int modeFlags, LexerPosition position);
SMILE_INTERNAL_FUNC TemplateResult Parser_ParseRawListTerm(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC TemplateResult Parser_ParseRawListItemsOpt(Parser parser, Int modeFlags, SmileList *head, SmileList *tail);
SMILE_INTERNAL_FUNC TemplateResult Parser_ParseRawListDotExpr(Parser parser, Int modeFlags);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseDynamicString(Parser parser, String text, LexerPosition startPosition);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseFunc(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseParamsOpt(Parser parser);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseParam(Parser parser, LexerPosition *position);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseParamType(Parser parser);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseSyntax(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseLoanword(Parser parser, Int modeFlags);
SMILE_INTERNAL_FUNC ParseResult Parser_ApplyCustomSyntax(Parser parser, Int modeFlags, Symbol syntaxClassSymbol,
	Int syntaxRootMode, Symbol rootSkipSymbol);
SMILE_INTERNAL_FUNC ParseResult Parser_ApplyCustomLoanword(Parser parser, Token token);
SMILE_INTERNAL_FUNC SmileObject Parser_RecursivelyApplyTemplate(Parser parser, SmileObject expr, Int32Dict replacements, LexerPosition lexerPosition);

SMILE_INTERNAL_FUNC ParseResult Parser_ParseClassicFn(Parser parser, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseQuoteBody(Parser parser, Int modeFlags, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseClassicQuote(Parser parser, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseClassicScope(Parser parser, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseClassicTill(Parser parser, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseClassicNew(Parser parser, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ParseClassicSet(Parser parser, LexerPosition startPosition);

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
SMILE_INTERNAL_FUNC ParseResult Parser_ExpectLeftBracket(Parser parser,
	Token firstUnaryTokenForErrorReporting, const char *name, LexerPosition startPosition);
SMILE_INTERNAL_FUNC ParseResult Parser_ExpectRightBracket(Parser parser,
	Token firstUnaryTokenForErrorReporting, const char *name, LexerPosition startPosition);

SMILE_INTERNAL_DATA Int *Parser_BracesBracketsParenthesesBar_Recovery;
SMILE_INTERNAL_DATA Int Parser_BracesBracketsParenthesesBar_Count;
SMILE_INTERNAL_DATA Int *Parser_RightBracesBracketsParentheses_Recovery;
SMILE_INTERNAL_DATA Int Parser_RightBracesBracketsParentheses_Count;
SMILE_INTERNAL_DATA Int *Parser_BracesBracketsParenthesesBarName_Recovery;
SMILE_INTERNAL_DATA Int Parser_BracesBracketsParenthesesBarName_Count;

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
