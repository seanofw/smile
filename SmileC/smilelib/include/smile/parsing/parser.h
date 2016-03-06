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

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// The Smile parser.
/// </summary>
typedef struct ParserStruct {

	Lexer lexer;						// The lexer, which provides the source token stream.

	ParseScope currentScope;			// The current parsing scope.

	SmileList firstError, lastError;	// A list of errors generated during the parse.

} *Parser;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Parser Parser_Create(void);
SMILE_API_FUNC SmileObject Parse(Parser parser, Lexer lexer, ParseScope scope);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

#endif