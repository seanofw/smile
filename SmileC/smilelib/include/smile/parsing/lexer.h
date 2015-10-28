#ifndef __SMILE_PARSING_LEXER_H__
#define __SMILE_PARSING_LEXER_H__

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

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A position within a source file.  This represents the start of a token within that source
/// file, and the length of that token.
/// </summary>
typedef struct LexerPositionStruct {

	String filename;						// Which source file this position is in.
	Int32 line;								// On which line this position begins.
	Int32 column;							// The column within that line (note: tabs count as 1 char)
	Int32 lineStart;						// The offset of the start of this line from the start of the file.
	Int32 length;							// The length of the span of content, in characters.

} *LexerPosition;

/// <summary>
/// A single token from the input.
/// </summary>
typedef struct TokenStruct {

	enum TokenKind kind;					// What kind of token this is (see tokenkind.h)

	struct LexerPositionStruct position;	// The position where the start of this token was found.
	Bool isFirstContentOnLine;				// Whether this token is the first content on the line.

	String text;							// The text of this token (for strings/numbers/#loanwords).

	// The various kinds of data this token can represent ('kind' determines which of these is valid).
	union {
		Symbol symbol;						// The symbol for this token (for identifiers).
		Int32 i;							// The integer value of this token (char/byte/int16/int32).
		Int64 int64;						// A 64-bit integer value for this token.
	} data;

} *Token;

/// <summary>
/// The Smile lexical analyzer.
/// </summary>
typedef struct LexerStruct {

	// The actual input, and current position within it.
	const Byte *input;			// The input (source file) itself.
	const Byte *src;			// The current read pointer within the input.
	const Byte *end;			// The end of the input (one past the last valid byte).
	const Byte *lineStart;		// The start of the current line of input (for computing the current char index).

	// The current filename/line number, for computing LexerPositions.
	String filename;			// The current filename.
	Int line;					// The current line number.

	// The most-recently-read token, and a stack of previously-read tokens for ungetting.
	struct TokenStruct tokenBuffer[16];
	Token token;				// The current read pointer within the tokenBuffer.
	Int tokenIndex;				// The current index of the token pointer within the token ring buffer.  Cached for speed.
	Int ungetCount;				// The number of tokens we have ungotten.  This maxes out at 15 levels deep.
	const Byte *tokenStart;		// The starting pointer of the current token.

	// External helper constructs.
	SymbolTable symbolTable;	// The symbol table, for resolving identifiers.

} *Lexer;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API Lexer Lexer_Create(String input, Int start, Int length, String filename, Int firstLine, Int firstColumn);
SMILE_API Int Lexer_Next(Lexer lexer);
SMILE_API Int Lexer_DecodeEscapeCode(const Byte **input, const Byte *end, Bool allowUnknowns);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Make a safe clone of a token on the heap.
/// </summary>
/// <param name="token">The token to clone.</param>
/// <returns>A copy of the provided token, located on the GC heap.</returns>
Inline Token Token_Clone(Token token)
{
	Token newToken = GC_MALLOC_STRUCT(struct TokenStruct);
	MemCpy(newToken, token, sizeof(struct TokenStruct));
	return newToken;
}

/// <summary>
/// Assign a symbol table for the lexer to use; if this is provided, any identifiers collected
/// will be added to the symbol table and their Symbol value will be returned.
/// </summary>
Inline void Lexer_SetSymbolTable(Lexer lexer, SymbolTable symbolTable)
{
	lexer->symbolTable = symbolTable;
}

/// <summary>
/// Get the most-recently-read token.  Call Lexer_Next() to read a token, and then call this to
/// see what its data is.  Note that this is only valid data until the next call to any of the
/// Lexer_*() functions, so if you need this token's contents, you must copy them somewhere else.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
/// <returns>A temporary pointer to the most-recently-read token in the lexical analyzer.</returns>
Inline Token Lexer_Token(Lexer lexer)
{
	return lexer->token;
}

/// <summary>
/// Unget a token read by Lexer_Next() --- i.e., push it back on the input.  You may call this up to 15 times
/// before exhausting the unget stack.  This runs in constant (O(1)) time and space.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
Inline void Lexer_Unget(Lexer lexer)
{
	lexer->ungetCount++;
	lexer->token = lexer->tokenBuffer + (--lexer->tokenIndex & 15);
}

/// <summary>
/// Peek at the next token in the input without actually consuming it.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
/// <returns>The type of token next in the input.</returns>
Inline Int Lexer_Peek(Lexer lexer)
{
	Int kind = Lexer_Next(lexer);
	Lexer_Unget(lexer);
	return kind;
}

#endif