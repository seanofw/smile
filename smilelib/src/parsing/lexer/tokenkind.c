//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <smile/string.h>
#include <smile/parsing/tokenkind.h>
#include <smile/internal/staticstring.h>

STATIC_STRING(TokenString_Error, "<error>");
STATIC_STRING(TokenString_None, "<none>");

STATIC_STRING(TokenString_Eoi, "end-of-file");

STATIC_STRING(TokenString_AlphaName, "alpha name");
STATIC_STRING(TokenString_PunctName, "punct name");

STATIC_STRING(TokenString_Char, "char");
STATIC_STRING(TokenString_Uni, "uni");
STATIC_STRING(TokenString_Byte, "byte");
STATIC_STRING(TokenString_Integer16, "int16");
STATIC_STRING(TokenString_Integer32, "int32");
STATIC_STRING(TokenString_Integer64, "int64");
STATIC_STRING(TokenString_Integer128, "int128");
STATIC_STRING(TokenString_Real32, "real32");
STATIC_STRING(TokenString_Real64, "real64");
STATIC_STRING(TokenString_Real128, "real128");
STATIC_STRING(TokenString_Float32, "float32");
STATIC_STRING(TokenString_Float64, "float64");
STATIC_STRING(TokenString_Float128, "float128");

STATIC_STRING(TokenString_RawString, "raw string");
STATIC_STRING(TokenString_DynString, "string");

STATIC_STRING(TokenString_DoubleHash, "##");
STATIC_STRING(TokenString_DotDot, "..");
STATIC_STRING(TokenString_DotDotDot, "...");

STATIC_STRING(TokenString_At, "@");
STATIC_STRING(TokenString_AtAt, "@@");

STATIC_STRING(TokenString_SuperEq, "===");
STATIC_STRING(TokenString_SuperNe, "!==");
STATIC_STRING(TokenString_Eq, "==");
STATIC_STRING(TokenString_Ne, "!=");
STATIC_STRING(TokenString_Le, "<=");
STATIC_STRING(TokenString_Ge, ">=");

STATIC_STRING(TokenString_Var, "var");
STATIC_STRING(TokenString_Auto, "auto");
STATIC_STRING(TokenString_Const, "const");

STATIC_STRING(TokenString_And, "and");
STATIC_STRING(TokenString_Or, "or");
STATIC_STRING(TokenString_Not, "not");

STATIC_STRING(TokenString_New, "new");
STATIC_STRING(TokenString_Is, "is");
STATIC_STRING(TokenString_TypeOf, "typeof");

STATIC_STRING(TokenString_LoanWord_Include, "#include");
STATIC_STRING(TokenString_LoanWord_Regex, "#/.../");
STATIC_STRING(TokenString_LoanWord_Xml, "#xml");
STATIC_STRING(TokenString_LoanWord_Json, "#json");
STATIC_STRING(TokenString_LoanWord_Brk, "#brk");
STATIC_STRING(TokenString_LoanWord_Syntax, "#syntax");

STATIC_STRING(TokenString_KnownName, "variable");
STATIC_STRING(TokenString_UnknownAlphaName, "alpha-operator");
STATIC_STRING(TokenString_UnknownPunctName, "punct-operator");

SMILE_API_FUNC String TokenKind_ToString(Int tokenKind)
{
	switch (tokenKind) {

		case TOKEN_ERROR: return TokenString_Error;
		case TOKEN_NONE: return TokenString_None;

		case TOKEN_EOI: return TokenString_Eoi;

		case TOKEN_ALPHANAME: return TokenString_AlphaName;
		case TOKEN_PUNCTNAME: return TokenString_PunctName;

		case TOKEN_CHAR: return TokenString_Char;
		case TOKEN_UNI: return TokenString_Uni;
		case TOKEN_BYTE: return TokenString_Byte;
		case TOKEN_INTEGER16: return TokenString_Integer16;
		case TOKEN_INTEGER32: return TokenString_Integer32;
		case TOKEN_INTEGER64: return TokenString_Integer64;
		case TOKEN_INTEGER128: return TokenString_Integer128;
		case TOKEN_REAL32: return TokenString_Real32;
		case TOKEN_REAL64: return TokenString_Real64;
		case TOKEN_REAL128: return TokenString_Real128;
		case TOKEN_FLOAT32: return TokenString_Float32;
		case TOKEN_FLOAT64: return TokenString_Float64;
		case TOKEN_FLOAT128: return TokenString_Float128;

		case TOKEN_RAWSTRING: return TokenString_RawString;
		case TOKEN_DYNSTRING: return TokenString_DynString;

		case TOKEN_LEFTBRACE: return String_LeftBrace;
		case TOKEN_RIGHTBRACE: return String_RightBrace;
		case TOKEN_LEFTPARENTHESIS: return String_LeftParenthesis;
		case TOKEN_RIGHTPARENTHESIS: return String_RightParenthesis;
		case TOKEN_LEFTBRACKET: return String_LeftBracket;
		case TOKEN_RIGHTBRACKET: return String_RightBracket;

		case TOKEN_BAR: return String_VerticalBar;
		case TOKEN_EQUAL: return String_Equal;
		case TOKEN_EQUALWITHOUTWHITESPACE: return String_Equal;
		case TOKEN_BACKTICK: return String_Backtick;
		case TOKEN_DOUBLEHASH: return TokenString_DoubleHash;
		case TOKEN_DOT: return String_Period;
		case TOKEN_DOTDOT: return TokenString_DotDot;
		case TOKEN_DOTDOTDOT: return TokenString_DotDotDot;
		case TOKEN_COLON: return String_Colon;
		case TOKEN_COMMA: return String_Comma;
		case TOKEN_SEMICOLON: return String_Semicolon;
		case TOKEN_AT: return TokenString_At;
		case TOKEN_ATAT: return TokenString_AtAt;

		case TOKEN_LOANWORD_INCLUDE: return TokenString_LoanWord_Include;
		case TOKEN_LOANWORD_REGEX: return TokenString_LoanWord_Regex;
		case TOKEN_LOANWORD_XML: return TokenString_LoanWord_Xml;
		case TOKEN_LOANWORD_JSON: return TokenString_LoanWord_Json;
		case TOKEN_LOANWORD_BRK: return TokenString_LoanWord_Brk;
		case TOKEN_LOANWORD_SYNTAX: return TokenString_LoanWord_Syntax;

		case TOKEN_KNOWNNAME: return TokenString_KnownName;
		case TOKEN_UNKNOWNALPHANAME: return TokenString_UnknownAlphaName;
		case TOKEN_UNKNOWNPUNCTNAME: return TokenString_UnknownPunctName;

		default:
			Smile_Abort_FatalError(
				String_ToC(String_Format("Cannot convert TokenKind %d to string; unknown token. [Internal error; please report this.]",
					tokenKind)));
	}
}
