
#ifndef __SMILE_PARSING_INTERNAL_PARSERINTERNAL_H__
#define __SMILE_PARSING_INTERNAL_PARSERINTERNAL_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_PARSING_PARSER_H__
#include <smile/parsing/parser.h>
#endif

// Binary line-break modes.
#define BINARYLINEBREAKS_ALLOWED 1			// Declare that line breaks are allowed before a binary operator, allowing the expression to cross line breaks.
#define BINARYLINEBREAKS_DISALLOWED 0		// Declare that line breaks are disallowed before a binary operator, causing the start of a new expression.

// Comma-parsing modes.
#define COMMAMODE_NORMAL 0					// Declare that commas delineate successive operands in N-ary operations.
#define COMMAMODE_VARIABLEDECLARATION 1		// Declare that commas are being used to separate successive variable declarations.

#endif
