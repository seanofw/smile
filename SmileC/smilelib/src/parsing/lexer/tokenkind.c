//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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

STATIC_STRING(TokenString_Error, "<error>");
STATIC_STRING(TokenString_None, "<none>");

STATIC_STRING(TokenString_Eoi, "end-of-file");

STATIC_STRING(TokenString_AlphaName, "alpha name");
STATIC_STRING(TokenString_PunctName, "punct name");

STATIC_STRING(TokenString_Char, "char");
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

STATIC_STRING(TokenString_LeftBrace, "{");
STATIC_STRING(TokenString_RightBrace, "}");
STATIC_STRING(TokenString_LeftParenthesis, "(");
STATIC_STRING(TokenString_RightParenthesis, ")");
STATIC_STRING(TokenString_LeftBracket, "[");
STATIC_STRING(TokenString_RightBracket, "]");

STATIC_STRING(TokenString_Bar, "|");
STATIC_STRING(TokenString_Equal, "=");
STATIC_STRING(TokenString_EqualWithoutWhitespace, "=");
STATIC_STRING(TokenString_Backtick, "`");
STATIC_STRING(TokenString_DoubleHash, "##");
STATIC_STRING(TokenString_Dot, ".");
STATIC_STRING(TokenString_Range, "..");
STATIC_STRING(TokenString_Ellipsis, "...");
STATIC_STRING(TokenString_Colon, ":");
STATIC_STRING(TokenString_Comma, ",");
STATIC_STRING(TokenString_SemiColon, ";");

STATIC_STRING(TokenString_Plus, "+");
STATIC_STRING(TokenString_Minus, "-");
STATIC_STRING(TokenString_Star, "*");
STATIC_STRING(TokenString_Slash, "/");

STATIC_STRING(TokenString_SuperEq, "===");
STATIC_STRING(TokenString_SuperNe, "!==");
STATIC_STRING(TokenString_Eq, "==");
STATIC_STRING(TokenString_Ne, "!=");
STATIC_STRING(TokenString_Lt, "<");
STATIC_STRING(TokenString_Gt, ">");
STATIC_STRING(TokenString_Le, "<=");
STATIC_STRING(TokenString_Ge, ">=");

STATIC_STRING(TokenString_If, "if");
STATIC_STRING(TokenString_Unless, "unless");
STATIC_STRING(TokenString_Then, "then");
STATIC_STRING(TokenString_Else, "else");
STATIC_STRING(TokenString_While, "while");
STATIC_STRING(TokenString_Until, "until");
STATIC_STRING(TokenString_Do, "do");
STATIC_STRING(TokenString_Return, "return");
STATIC_STRING(TokenString_Var, "var");
STATIC_STRING(TokenString_Auto, "auto");
STATIC_STRING(TokenString_Const, "const");
STATIC_STRING(TokenString_Try, "try");
STATIC_STRING(TokenString_Catch, "catch");
STATIC_STRING(TokenString_Till, "till");
STATIC_STRING(TokenString_When, "when");
STATIC_STRING(TokenString_New, "new");

STATIC_STRING(TokenString_And, "and");
STATIC_STRING(TokenString_Or, "or");
STATIC_STRING(TokenString_Not, "not");

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

		case TOKEN_LEFTBRACE: return TokenString_LeftBrace;
		case TOKEN_RIGHTBRACE: return TokenString_RightBrace;
		case TOKEN_LEFTPARENTHESIS: return TokenString_LeftParenthesis;
		case TOKEN_RIGHTPARENTHESIS: return TokenString_RightParenthesis;
		case TOKEN_LEFTBRACKET: return TokenString_LeftBracket;
		case TOKEN_RIGHTBRACKET: return TokenString_RightBracket;

		case TOKEN_BAR: return TokenString_Bar;
		case TOKEN_EQUAL: return TokenString_Equal;
		case TOKEN_EQUALWITHOUTWHITESPACE: return TokenString_EqualWithoutWhitespace;
		case TOKEN_BACKTICK: return TokenString_Backtick;
		case TOKEN_DOUBLEHASH: return TokenString_DoubleHash;
		case TOKEN_DOT: return TokenString_Dot;
		case TOKEN_RANGE: return TokenString_Range;
		case TOKEN_ELLIPSIS: return TokenString_Ellipsis;
		case TOKEN_COLON: return TokenString_Colon;
		case TOKEN_COMMA: return TokenString_Comma;
		case TOKEN_SEMICOLON: return TokenString_SemiColon;

		case TOKEN_PLUS: return TokenString_Plus;
		case TOKEN_MINUS: return TokenString_Minus;
		case TOKEN_STAR: return TokenString_Star;
		case TOKEN_SLASH: return TokenString_Slash;

		case TOKEN_SUPEREQ: return TokenString_SuperEq;
		case TOKEN_SUPERNE: return TokenString_SuperNe;
		case TOKEN_EQ: return TokenString_Eq;
		case TOKEN_NE: return TokenString_Ne;
		case TOKEN_LT: return TokenString_Lt;
		case TOKEN_GT: return TokenString_Gt;
		case TOKEN_LE: return TokenString_Le;
		case TOKEN_GE: return TokenString_Ge;

		case TOKEN_IF: return TokenString_If;
		case TOKEN_UNLESS: return TokenString_Unless;
		case TOKEN_THEN: return TokenString_Then;
		case TOKEN_ELSE: return TokenString_Else;
		case TOKEN_WHILE: return TokenString_While;
		case TOKEN_UNTIL: return TokenString_Until;
		case TOKEN_DO: return TokenString_Do;
		case TOKEN_RETURN: return TokenString_Return;
		case TOKEN_VAR: return TokenString_Var;
		case TOKEN_AUTO: return TokenString_Auto;
		case TOKEN_CONST: return TokenString_Const;
		case TOKEN_TRY: return TokenString_Try;
		case TOKEN_CATCH: return TokenString_Catch;
		case TOKEN_TILL: return TokenString_Till;
		case TOKEN_WHEN: return TokenString_When;
		case TOKEN_NEW: return TokenString_New;

		case TOKEN_AND: return TokenString_And;
		case TOKEN_OR: return TokenString_Or;
		case TOKEN_NOT: return TokenString_Not;

		case TOKEN_IS: return TokenString_Is;
		case TOKEN_TYPEOF: return TokenString_TypeOf;

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
			return NULL;
	}
}
