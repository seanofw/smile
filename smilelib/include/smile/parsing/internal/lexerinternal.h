
#ifndef __SMILE_PARSING_INTERNAL_LEXERINTERNAL_H__
#define __SMILE_PARSING_INTERNAL_LEXERINTERNAL_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_PARSING_LEXER_H__
#include <smile/parsing/lexer.h>
#endif

SMILE_INTERNAL_FUNC Int Lexer_ParseName(Lexer lexer);
SMILE_INTERNAL_FUNC Int Lexer_ParsePunctuation(Lexer lexer);

SMILE_INTERNAL_FUNC Int Lexer_ParseRawString(Lexer lexer);
SMILE_INTERNAL_FUNC Int Lexer_ParseChar(Lexer lexer);
SMILE_INTERNAL_FUNC Int Lexer_ParseDynamicString(Lexer lexer);

SMILE_INTERNAL_FUNC Int Lexer_ParseZero(Lexer lexer);
SMILE_INTERNAL_FUNC Int Lexer_ParseDigit(Lexer lexer);
SMILE_INTERNAL_FUNC Int Lexer_ParseReal(Lexer lexer);

SMILE_INTERNAL_FUNC Int Lexer_ParseLoanword(Lexer lexer);
SMILE_INTERNAL_FUNC Int Lexer_ParseRegex(Lexer lexer);

SMILE_INTERNAL_FUNC Int Lexer_ParseSlash(Lexer lexer);
SMILE_INTERNAL_FUNC Int Lexer_ParseDot(Lexer lexer);
SMILE_INTERNAL_FUNC Int Lexer_ParseHyphenOrEquals(Lexer lexer, Int initialChar);

SMILE_INTERNAL_FUNC Bool Lexer_IsValidRestartCharacter(const Byte *src, const Byte *end);

//---------------------------------------------------------------------------
//  Tokenization macros.

// Begin a token at the given start pointer.
#define START_TOKEN(__startPtr__) \
	((token->_position.filename = lexer->filename), \
	 (token->_position.line = (Int32)lexer->line), \
	 (token->_position.lineStart = (Int32)(lexer->lineStart - lexer->input)), \
	 (token->_position.column = (Int32)((lexer->tokenStart = (__startPtr__)) - lexer->lineStart)), \
	 (token->isFirstContentOnLine = lexer->_isFirstContentOnLine), \
	 (token->hasEscapes = False))

// End the current token as the given kind.
#define END_TOKEN(__kind__) \
	((lexer->src = src), \
	 (token->_position.length = (Int32)(src - lexer->tokenStart)), \
	 (token->kind = (__kind__)))

// Construct a simple token, a token with no meaningful properties except its kind and position.
#define SIMPLE_TOKEN(__startPtr__, __kind__) \
	(START_TOKEN(__startPtr__), \
	 END_TOKEN(__kind__))

#define IS_KEYWORD_CHAR(__index__, __ch__) \
	(src+(__index__) < end && src[__index__] == (__ch__))

#define IS_KEYWORD_END(__index__) \
	((SmileIdentifierKind(src[__index__]) & (IDENTKIND_STARTLETTER | IDENTKIND_MIDDLELETTER)) == 0)

#endif
