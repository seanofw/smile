#ifndef __SMILE_PARSING_PARSETYPES_H__
#define __SMILE_PARSING_PARSETYPES_H__

typedef struct LexerStruct *Lexer;

typedef struct TokenStruct *Token;
enum TokenKind;

typedef struct ParserSyntaxNodeStruct *ParserSyntaxNode;
typedef struct ParserSyntaxClassStruct *ParserSyntaxClass;
typedef struct ParserSyntaxTableStruct *ParserSyntaxTable;

// Alternate name for a ParseMessage, which is a useful alternate name when talking about errors.
typedef struct ParseMessageStruct *ParseError;

#endif