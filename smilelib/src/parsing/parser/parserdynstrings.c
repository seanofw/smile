//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2017 Sean Werkema
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

#include <smile/types.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/internal/staticstring.h>

STATIC_STRING(UnmatchedCurlyBraceError, "Unmatched } in dynamic string");
STATIC_STRING(DisallowedCurlyBraceError, "Disallowed { inside parsed content in dynamic string");
STATIC_STRING(MissingCurlyBraceError, "Missing final } for expression in dynamic string");
STATIC_STRING(BadEscapeCodeError, "Bad backslash-escape code in dynamic string");

//-------------------------------------------------------------------------------------------------
//  DynamicStringPiece objects.

typedef struct DynamicStringPieceStruct {
	LexerPosition firstLine;
	String text;
	Bool isExpression;
} *DynamicStringPiece;

//-------------------------------------------------------------------------------------------------
//  Static prototypes.

static ParseError Parser_SplitDynamicString(Lexer lexer, DynamicStringPiece **dynamicStringPieces, Int *numDynamicStringPieces);

//-------------------------------------------------------------------------------------------------
//  DynamicStringPiece implementation.

static DynamicStringPiece DynamicStringPiece_Create(String text, LexerPosition firstLine, Bool isExpression)
{
	DynamicStringPiece piece = GC_MALLOC_STRUCT(struct DynamicStringPieceStruct);
	piece->text = text;
	piece->firstLine = firstLine;
	piece->isExpression = isExpression;
	return piece;
}

//-------------------------------------------------------------------------------------------------
//  Dynamic string parsing.

ParseError Parser_ParseDynamicString(Parser parser, SmileObject *expr, String text, LexerPosition startPosition)
{
	Lexer stringLexer;
	DynamicStringPiece *dynamicStringPieces, piece;
	Int numDynamicStringPieces;
	ParseError parseError;
	SmileList head, tail;
	SmileObject parsedContent;
	Int i;

	if (String_IsNullOrEmpty(text)) {
		*expr = (SmileObject)text;
		return NULL;
	}

	stringLexer = Lexer_Create(text, 0, String_Length(text), startPosition->filename, startPosition->line, startPosition->column);

	parseError = Parser_SplitDynamicString(stringLexer, &dynamicStringPieces, &numDynamicStringPieces);
	if (parseError != NULL) {
		*expr = NULL;
		return parseError;
	}

	// If there's no parsed content, just return the string verbatim.
	if (numDynamicStringPieces < 2 && !dynamicStringPieces[0]->isExpression) {
		*expr = (SmileObject)dynamicStringPieces[0]->text;
		return NULL;
	}

	// Construct a [[List.of a b c ...].join] form.
	LIST_INIT(head, tail);
	LIST_APPEND_WITH_SOURCE(head, tail, SmileList_CreateDotWithSource(Smile_KnownObjects.ListSymbol, Smile_KnownObjects.ofSymbol, startPosition), startPosition);

	// Spin through the pieces and attach them to the list, parsing the expressions as we find them.
	for (i = 0; i < numDynamicStringPieces; i++) {
		piece = dynamicStringPieces[i];
		if (!piece->isExpression) {
			LIST_APPEND_WITH_SOURCE(head, tail, piece->text, piece->firstLine);
		}
		else {
			parseError = Parser_ParseOneExpressionFromText(parser, &parsedContent, piece->text, piece->firstLine);
			if (parseError != NULL) {
				*expr = NULL;
				return parseError;
			}
			LIST_APPEND_WITH_SOURCE(head, tail, parsedContent, piece->firstLine);
		}
	}

	*expr = (SmileObject)SmileList_CreateOneWithSource(
		(SmileObject)SmileList_CreateDotWithSource(head, Smile_KnownObjects.joinSymbol, startPosition),
		startPosition
	);

	return NULL;
}

#define MAKE_POSITION() \
	(position = GC_MALLOC_STRUCT(struct LexerPositionStruct), \
	 position->filename = lexer->filename, \
	 position->line = (Int32)lexer->line, \
	 position->lineStart = (Int32)(lexer->lineStart - lexer->input), \
	 position->column = (Int32)(src - lexer->lineStart), \
	 position->length = 0)

static ParseError Parser_SplitDynamicString(Lexer lexer, DynamicStringPiece **dynamicStringPieces, Int *numDynamicStringPieces)
{
	Array pieces;
	StringBuilder builder;
	Bool inParsedContent;
	Byte ch;
	const Byte *src, *end;
	Int decoded;
	DynamicStringPiece *dest;
	ParseError parseError;
	LexerPosition position;

	pieces = Array_Create(sizeof(DynamicStringPiece), 16, False);

	builder = StringBuilder_Create();

	inParsedContent = False;

	src = lexer->src;
	end = lexer->end;

	while (src < end) {
		ch = *src;
		if (ch == '\n') {
			src++;
			StringBuilder_AppendByte(builder, ch);
			if (*src == '\r') {
				src++;
				StringBuilder_AppendByte(builder, ch);
			}
			lexer->line++;
			lexer->lineStart = src;
		}
		else if (ch == '\r') {
			src++;
			StringBuilder_AppendByte(builder, ch);
			if (*src == '\n') {
				src++;
				StringBuilder_AppendByte(builder, ch);
			}
			lexer->line++;
			lexer->lineStart = src;
		}
		else if (!inParsedContent) {
			// In the plain-text side until we reach a curly brace.
			if (ch == '{') {
				src++;
				if (*src == '{') {
					src++;
					StringBuilder_AppendByte(builder, ch);
				}
				else {
					inParsedContent = True;
					MAKE_POSITION();
					if (StringBuilder_GetLength(builder) > 0) {
						dest = (DynamicStringPiece *)Array_Push(pieces);
						*dest = DynamicStringPiece_Create(StringBuilder_ToString(builder), position, False);
						StringBuilder_SetLength(builder, 0);
					}
				}
			}
			else if (ch == '}') {
				src++;
				if (*src == '}') {
					src++;
					StringBuilder_AppendByte(builder, ch);
				}
				else {
					MAKE_POSITION();
					parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, UnmatchedCurlyBraceError);
					*dynamicStringPieces = NULL;
					*numDynamicStringPieces = 0;
					return parseError;
				}
			}
			else if (ch == '\\') {
				src++;
				if ((ch = *src) == '{' || ch == '}') {
					// Backslash can be used to escape curly braces in a dynamic string.
					StringBuilder_AppendByte(builder, ch);
				}
				else {
					decoded = Lexer_DecodeEscapeCode(&src, end, False);
					if (decoded < 0) {
						MAKE_POSITION();
						parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, BadEscapeCodeError);
						*dynamicStringPieces = NULL;
						*numDynamicStringPieces = 0;
						return parseError;
					}
					StringBuilder_AppendUnicode(builder, (UInt32)(Int32)decoded);
				}
			}
			else {
				src++;
				StringBuilder_AppendByte(builder, ch);
			}
		}
		else {
			// Inside executable code until we reach a closing curly brace.
			if (ch == '{') {
				MAKE_POSITION();
				parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, DisallowedCurlyBraceError);
				*dynamicStringPieces = NULL;
				*numDynamicStringPieces = 0;
				return parseError;
			}
			else if (ch == '}') {
				src++;
				inParsedContent = False;
				dest = (DynamicStringPiece *)Array_Push(pieces);
				MAKE_POSITION();
				*dest = DynamicStringPiece_Create(StringBuilder_ToString(builder), position, True);
				StringBuilder_SetLength(builder, 0);
			}
			else {
				src++;
				StringBuilder_AppendByte(builder, ch);
			}
		}
	}

	if (inParsedContent) {
		MAKE_POSITION();
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position, MissingCurlyBraceError);
		*dynamicStringPieces = NULL;
		*numDynamicStringPieces = 0;
		return parseError;
	}

	if (StringBuilder_GetLength(builder) > 0) {
		MAKE_POSITION();
		dest = (DynamicStringPiece *)Array_Push(pieces);
		*dest = DynamicStringPiece_Create(StringBuilder_ToString(builder), position, False);
	}

	*dynamicStringPieces = pieces->data;
	*numDynamicStringPieces = pieces->length;

	return NULL;
}
