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
#include <smile/smiletypes/smileobject.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/internal/staticstring.h>

static ParseError Parser_LoadPackage(Parser parser, String filename, SmileObject *expr, LexerPosition position);

ParseError Parser_ParseInclude(Parser parser, SmileObject *expr)
{
	Int tokenKind;
	String filename;
	LexerPosition position;
	ClosureInfo oldGlobalClosure;
	STATIC_STRING(dotSM, ".sm");
	ParseError error;

	if ((tokenKind = Lexer_Next(parser->lexer)) != TOKEN_DYNSTRING
		&& tokenKind != TOKEN_RAWSTRING) {
		return ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_FromC("#include must be followed by a string naming a source file or a package"));
	}

	position = Token_GetPosition(parser->lexer->token);

	filename = parser->lexer->token->text;

	oldGlobalClosure = Smile_GetGlobalClosureInfo();

	if (String_EndsWith(filename, dotSM)) {
		// This ends with ".sm", so it's a file in a path relative to the current source
		// file, and not a package name.
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Cannot load #include file \"%S\".", filename));
		Smile_SetGlobalClosureInfo(oldGlobalClosure);
		return error;
	}
	else {
		// This is an installed package name, so inhale it.
		error = Parser_LoadPackage(parser, filename, expr, position);
		Smile_SetGlobalClosureInfo(oldGlobalClosure);
		return error;
	}
}

extern SmileObject Stdio_Main(void);

static ParseError Parser_LoadPackage(Parser parser, String filename, SmileObject *expr, LexerPosition position)
{
	UNUSED(parser);

	// Empty is just wrong.
	if (String_IsNullOrEmpty(filename))
		return ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_FromC("Unknown #include package \"\"."));

	// Certain packages are built-in, so let's test those first.
	switch (String_At(filename, 0)) {
		case 's':
			if (String_EqualsC(filename, "stdio")) {
				*expr = Stdio_Main();
			}
			return NULL;

		default:
			return ParseMessage_Create(PARSEMESSAGE_ERROR, position, String_Format("Unknown #include package \"%S\".", filename));
	}
}
