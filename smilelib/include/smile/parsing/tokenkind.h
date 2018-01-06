
#ifndef __SMILE_PARSING_TOKENKIND_H__
#define __SMILE_PARSING_TOKENKIND_H__

enum TokenKind {
	TOKEN_ERROR = -2,
	TOKEN_NONE = -1,

	TOKEN_EOI = 0,

	TOKEN_ALPHANAME,
	TOKEN_PUNCTNAME,

	TOKEN_CHAR,
	TOKEN_UNI,
	TOKEN_BYTE,
	TOKEN_INTEGER16,
	TOKEN_INTEGER32,
	TOKEN_INTEGER64,
	TOKEN_INTEGER128,
	TOKEN_REAL32,
	TOKEN_REAL64,
	TOKEN_REAL128,
	TOKEN_FLOAT32,
	TOKEN_FLOAT64,
	TOKEN_FLOAT128,

	TOKEN_RAWSTRING,
	TOKEN_DYNSTRING,

	TOKEN_LEFTBRACE,
	TOKEN_RIGHTBRACE,
	TOKEN_LEFTPARENTHESIS,
	TOKEN_RIGHTPARENTHESIS,
	TOKEN_LEFTBRACKET,
	TOKEN_RIGHTBRACKET,

	TOKEN_BAR,
	TOKEN_EQUAL,
	TOKEN_EQUALWITHOUTWHITESPACE,
	TOKEN_BACKTICK,
	TOKEN_DOUBLEHASH,
	TOKEN_DOT,
	TOKEN_DOTDOT,
	TOKEN_DOTDOTDOT,
	TOKEN_COLON,
	TOKEN_COMMA,
	TOKEN_SEMICOLON,
	TOKEN_AT,
	TOKEN_ATAT,

	// Loanwords.
	TOKEN_LOANWORD_INCLUDE,
	TOKEN_LOANWORD_REGEX,
	TOKEN_LOANWORD_XML,
	TOKEN_LOANWORD_JSON,
	TOKEN_LOANWORD_BRK,
	TOKEN_LOANWORD_SYNTAX,

	// These kinds are required by the parser, but are not directly returned by the lexer.
	TOKEN_KNOWNNAME,
	TOKEN_UNKNOWNALPHANAME,
	TOKEN_UNKNOWNPUNCTNAME,
};

SMILE_API_FUNC String TokenKind_ToString(Int tokenKind);

#endif
