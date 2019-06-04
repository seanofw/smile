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

#include <smile/gc.h>
#include <smile/mem.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/internal/types.h>
#include <smile/internal/staticstring.h>

// The special one-and-only empty string.
EXTERN_STATIC_STRING(String_Empty, "");

// Common ASCII formatting/control codes, as single-character strings.
EXTERN_STATIC_STRING(String_Nul, "\x00");
EXTERN_STATIC_STRING(String_Bell, "\x07");
EXTERN_STATIC_STRING(String_Backspace, "\x08");
EXTERN_STATIC_STRING(String_Tab, "\x09");
EXTERN_STATIC_STRING(String_Newline, "\x0A");
EXTERN_STATIC_STRING(String_VerticalTab, "\x0B");
EXTERN_STATIC_STRING(String_FormFeed, "\x0C");
EXTERN_STATIC_STRING(String_CarriageReturn, "\x0D");
EXTERN_STATIC_STRING(String_Escape, "\x1B");
EXTERN_STATIC_STRING(String_Space, " ");

// ASCII punctuation, as single-character strings.
EXTERN_STATIC_STRING(String_ExclamationPoint, "!");
EXTERN_STATIC_STRING(String_QuotationMark, "\"");
EXTERN_STATIC_STRING(String_PoundSign, "#");
EXTERN_STATIC_STRING(String_Dollar, "$");
EXTERN_STATIC_STRING(String_Percent, "%");
EXTERN_STATIC_STRING(String_Ampersand, "&");
EXTERN_STATIC_STRING(String_Apostrophe, "'");
EXTERN_STATIC_STRING(String_LeftParenthesis, "(");
EXTERN_STATIC_STRING(String_RightParenthesis, ")");
EXTERN_STATIC_STRING(String_Star, "*");
EXTERN_STATIC_STRING(String_Plus, "+");
EXTERN_STATIC_STRING(String_Comma, ",");
EXTERN_STATIC_STRING(String_Hyphen, "-");
EXTERN_STATIC_STRING(String_Period, ".");
EXTERN_STATIC_STRING(String_Slash, "/");
EXTERN_STATIC_STRING(String_Colon, ":");
EXTERN_STATIC_STRING(String_Semicolon, ";");
EXTERN_STATIC_STRING(String_LessThan, "<");
EXTERN_STATIC_STRING(String_Equal, "=");
EXTERN_STATIC_STRING(String_GreaterThan, ">");
EXTERN_STATIC_STRING(String_QuestionMark, "?");
EXTERN_STATIC_STRING(String_AtSign, "@");
EXTERN_STATIC_STRING(String_LeftBracket, "[");
EXTERN_STATIC_STRING(String_Backslash, "\\");
EXTERN_STATIC_STRING(String_RightBracket, "]");
EXTERN_STATIC_STRING(String_Caret, "^");
EXTERN_STATIC_STRING(String_Underscore, "_");
EXTERN_STATIC_STRING(String_Backtick, "`");
EXTERN_STATIC_STRING(String_LeftBrace, "{");
EXTERN_STATIC_STRING(String_VerticalBar, "|");
EXTERN_STATIC_STRING(String_RightBrace, "}");
EXTERN_STATIC_STRING(String_Tilde, "~");

// ASCII numbers, as single-character strings.
EXTERN_STATIC_STRING(String_Zero, "0");
EXTERN_STATIC_STRING(String_One, "1");
EXTERN_STATIC_STRING(String_Two, "2");
EXTERN_STATIC_STRING(String_Three, "3");
EXTERN_STATIC_STRING(String_Four, "4");
EXTERN_STATIC_STRING(String_Five, "5");
EXTERN_STATIC_STRING(String_Six, "6");
EXTERN_STATIC_STRING(String_Seven, "7");
EXTERN_STATIC_STRING(String_Eight, "8");
EXTERN_STATIC_STRING(String_Nine, "9");

// An array of the ASCII numbers, as strings.
const String String_Number[10] = {
	(const String)&String_ZeroStruct,
	(const String)&String_OneStruct,
	(const String)&String_TwoStruct,
	(const String)&String_ThreeStruct,
	(const String)&String_FourStruct,
	(const String)&String_FiveStruct,
	(const String)&String_SixStruct,
	(const String)&String_SevenStruct,
	(const String)&String_EightStruct,
	(const String)&String_NineStruct,
};
