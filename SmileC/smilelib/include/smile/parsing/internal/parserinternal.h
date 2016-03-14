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

// Binary line-break modes.
#define BINARYLINEBREAKS_ALLOWED 1			// Declare that line breaks are allowed before a binary operator, allowing the expression to cross line breaks.
#define BINARYLINEBREAKS_DISALLOWED 0		// Declare that line breaks are disallowed before a binary operator, causing the start of a new expression.

// Comma-parsing modes.
#define COMMAMODE_NORMAL 0					// Declare that commas delineate successive operands in N-ary operations.
#define COMMAMODE_VARIABLEDECLARATION 1		// Declare that commas are being used to separate successive variable declarations.

//-------------------------------------------------------------------------------------------------
// Parser-internal methods

SMILE_INTERNAL_FUNC ParseError Parser_ParseScope(Parser parser, SmileObject *expr, Int binaryLineBreaks);
SMILE_INTERNAL_FUNC void Parser_ParseExprsOpt(Parser parser, SmileList *head, SmileList *tail, Int binaryLineBreaks);
SMILE_INTERNAL_FUNC ParseError Parser_ParseExpr(Parser parser, SmileObject *expr, Int binaryLineBreaks);
SMILE_INTERNAL_FUNC ParseError Parser_ParseBaseExpr(Parser parser, SmileObject *expr, Int binaryLineBreaks);

SMILE_INTERNAL_FUNC ParseError Parser_ParseVarDecls(Parser parser, SmileObject *expr, Int binaryLineBreaks, Int declKind);
SMILE_INTERNAL_FUNC ParseError Parser_ParseDecl(Parser parser, SmileObject *expr, Int binaryLineBreaks, Int declKind);
SMILE_INTERNAL_FUNC ParseError Parser_ParseOpEquals(Parser parser, SmileObject *expr, Int binaryLineBreaks, Int commaMode);
SMILE_INTERNAL_FUNC ParseError Parser_ParseEquals(Parser parser, SmileObject *expr, Int binaryLineBreaks, Int commaMode);

SMILE_INTERNAL_FUNC ParseError Parser_ParseOr(Parser parser, SmileObject *expr, Int binaryLineBreaks, Int commaMode);

SMILE_INTERNAL_FUNC ParseError Parser_ParseTerm(Parser parser, SmileObject *expr, Int binaryLineBreaks, Token firstUnaryTokenForErrorReporting);
SMILE_INTERNAL_FUNC ParseError Parser_ParseDynamicString(Parser parser, SmileObject *expr, Int binaryLineBreaks, String text, LexerPosition startPosition);

SMILE_INTERNAL_FUNC Token Parser_Recover(Parser parser, Int *tokenKinds, Int numTokenKinds);
SMILE_INTERNAL_FUNC Bool Parser_IsLValue(SmileObject obj);
SMILE_INTERNAL_FUNC Bool Parser_HasEqualLookahead(Parser parser);
SMILE_INTERNAL_FUNC Bool Parser_HasEqualOrColonLookahead(Parser parser);
SMILE_INTERNAL_FUNC Bool Parser_HasLookahead(Parser parser, Int tokenKind);
SMILE_INTERNAL_FUNC Bool Parser_Has2Lookahead(Parser parser, Int tokenKind1, Int tokenKind2);
SMILE_INTERNAL_FUNC Bool Parser_Peek2(Parser parser, Token *token1, Token *token2);

SMILE_INTERNAL_DATA Int Parser_BracesBracketsParenthesesBar_Recovery[];
SMILE_INTERNAL_DATA Int Parser_BracesBracketsParenthesesBar_Count;
SMILE_INTERNAL_DATA Int Parser_RightBracesBracketsParentheses_Recovery[];
SMILE_INTERNAL_DATA Int Parser_RightBracesBracketsParentheses_Count;

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
	Symbol symbol;

	Lexer_Next(parser->lexer);
	token = parser->lexer->token;

	switch (token->kind) {

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

#endif
