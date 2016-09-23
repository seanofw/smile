#ifndef __SMILE_PARSING_PARSER_H__
#define __SMILE_PARSING_PARSER_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_GC_H__
#include <smile/gc.h>
#endif
#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif
#ifndef __SMILE_ENV_SYMBOLTABLE_H__
#include <smile/env/symboltable.h>
#endif
#ifndef __SMILE_SMILETYPES_OBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif
#ifndef __SMILE_PARSING_TOKENKIND_H__
#include <smile/parsing/tokenkind.h>
#endif
#ifndef __SMILE_PARSING_LEXER_H__
#include <smile/parsing/lexer.h>
#endif
#ifndef __SMILE_PARSING_INTERNAL_PARSESCOPE_H__
#include <smile/parsing/internal/parsescope.h>
#endif
#ifndef __SMILE_ENV_ENV_H__
#include <smile/env/env.h>
#endif
#ifndef __SMILE_PARSING_PARSEMESSAGE_H__
#include <smile/parsing/parsemessage.h>
#endif
//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// The Smile parser.
/// </summary>
struct ParserStruct {
	Lexer lexer;	// The lexer, which provides the source token stream.
		
	ParseScope currentScope;	// The current parsing scope.
	Int32Int32Dict customFollowSet;	// The set of tokens that follow in the current custom syntax rule.
		
	SmileList firstMessage, lastMessage;	// A list of messages (errors/warnings) generated during the parse.
};

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Parser Parser_Create(void);
SMILE_API_FUNC SmileList Parser_Parse(Parser parser, Lexer lexer, ParseScope scope);
SMILE_API_FUNC SmileList Parser_ParseFromC(Parser parser, ParseScope scope, const char *text);
SMILE_API_FUNC SmileList Parser_ParseString(Parser parser, ParseScope scope, String text);
SMILE_API_FUNC ParseError Parser_ParseOneExpressionFromText(Parser parser, SmileObject *expr, String string, LexerPosition startPosition);

SMILE_API_FUNC void Parser_AddMessage(Parser parser, ParseMessage message);
SMILE_API_FUNC void Parser_AddFatalError(Parser parser, LexerPosition position, const char *message, ...);
SMILE_API_FUNC void Parser_AddFatalErrorv(Parser parser, LexerPosition position, const char *message, va_list v);
SMILE_API_FUNC void Parser_AddError(Parser parser, LexerPosition position, const char *message, ...);
SMILE_API_FUNC void Parser_AddErrorv(Parser parser, LexerPosition position, const char *message, va_list v);
SMILE_API_FUNC void Parser_AddWarning(Parser parser, LexerPosition position, const char *message, ...);
SMILE_API_FUNC void Parser_AddWarningv(Parser parser, LexerPosition position, const char *message, va_list v);
SMILE_API_FUNC void Parser_AddInfo(Parser parser, LexerPosition position, const char *message, ...);
SMILE_API_FUNC void Parser_AddInfov(Parser parser, LexerPosition position, const char *message, va_list v);

SMILE_API_FUNC int Parser_GetWarningCount(Parser parser);
SMILE_API_FUNC int Parser_GetErrorCount(Parser parser);
SMILE_API_FUNC int Parser_GetFatalErrorCount(Parser parser);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

#endif