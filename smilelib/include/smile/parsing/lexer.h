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
#ifndef __SMILE_REGEX_H__
#include <smile/regex.h>
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
#ifndef __SMILE_PARSING_PARSERTYPES_H__
#include <smile/parsing/parsertypes.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A position within a source file.  This represents the start of a token within that source
/// file, and the length of that token.
/// </summary>
struct LexerPositionStruct {
	String filename;	// Which source file this position is in.
	Int32 line;			// On which line this position begins.
	Int32 column;		// The column within that line (note: tabs count as 1 char)
	Int32 lineStart;	// The offset of the start of this line from the start of the file.
	Int32 length;		// The length of the span of content, in characters.
};

/// <summary>
/// A single token from the input.
/// </summary>
struct TokenStruct {

	enum TokenKind kind;					// What kind of token this is (see tokenkind.h)
		
	struct LexerPositionStruct _position;	// The position where the start of this token was found.
	Bool isFirstContentOnLine;				// Whether this token is the first content on the line.
	Bool hasEscapes;						// Whether this token's text used escape codes (mainly needed for symbols).
		
	String text;							// The text of this token (for strings/numbers/#loanwords).

	// The various kinds of data this token can represent ('kind' determines which of these is valid).
	union {
		Symbol symbol;		// The symbol for this token (for identifiers).
		Byte byte;			// A 8-bit (unsigned) byte value for this token.
		Int16 int16;		// A 16-bit integer value for this token.
		Int32 int32;		// A 32-bit integer value for this token.
		Int64 int64;		// A 64-bit integer value for this token.
		Float32 float32;	// A 32-bit float value for this token.
		Float64 float64;	// A 64-bit float value for this token.
		Real32 real32;		// A 32-bit real value for this token.
		Real64 real64;		// A 64-bit real value for this token.
		Real128 real128;	// A 128-bit real value for this token.
		Byte ch;			// An 8-bit character value for this token.
		UInt32 uni;			// A 32-bit Unicode code point for this token.
		void *ptr;			// Pointer to nontrivial/complex/compound #loanwords.
	} data;
};

/// <summary>
/// The Smile lexical analyzer.
/// </summary>
struct LexerStruct {

	// The actual input, and current position within it.
	String stringInput;			// The original input, as an immutable string.
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
};

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Lexer Lexer_Create(String input, Int start, Int length, String filename, Int firstLine, Int firstColumn);
SMILE_API_FUNC Int Lexer_Next(Lexer lexer);
SMILE_API_FUNC Int Lexer_DecodeEscapeCode(const Byte **input, const Byte *end, Bool allowUnknowns);
SMILE_API_FUNC Bool Lexer_ConsumeWhitespaceOnThisLine(Lexer lexer);
SMILE_API_FUNC RegexMatch Lexer_ConsumeRegex(Lexer lexer, Regex regex);
SMILE_API_FUNC LexerPosition Lexer_GetPosition(Lexer lexer);
SMILE_API_FUNC LexerPosition LexerPosition_Clone(LexerPosition position);
SMILE_API_FUNC Token Token_Clone(Token token);
SMILE_API_FUNC Bool LexerPosition_Equals(LexerPosition a, LexerPosition b);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Create a position object for a token that describes that token's found location.
/// </summary>
/// <param name="token">The token to create a position for.</param>
/// <returns>The position of that token in the input stream (but with the data located on the GC heap).</returns>
Inline LexerPosition Token_GetPosition(Token token)
{
	return LexerPosition_Clone(&token->_position);
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