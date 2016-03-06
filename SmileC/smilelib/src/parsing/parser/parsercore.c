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

#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

Parser Parser_Create(void)
{
	Parser parser = GC_MALLOC_STRUCT(struct ParserStruct);
	parser->lexer = NULL;
	parser->currentScope = NULL;
	parser->firstError = parser->lastError = NullList;
	return parser;
}

static Token NextToken(void)
{
	return NULL;
}

static void AddErrorv(LexerPosition position, const char *message, va_list v)
{
	String string = String_FormatV(message, v);
	UNUSED(position);
	UNUSED(string);
}

static void AddError(LexerPosition position, const char *message, ...)
{
	va_list v;
	va_start(v, message);
	AddErrorv(position, message, v);
	va_end(v);
}

static SmileObject ParseExprsOpt(Int binaryLineBreaks);

// program ::= . exprs_opt
SmileObject Parse(Parser parser, Lexer lexer, ParseScope scope)
{
	SmileList head, tail;
	ParseScope parentScope;
	Lexer oldLexer;
	Token token;
	
	head = tail = NullList;
	parentScope = parser->currentScope;
	oldLexer = parser->lexer;
	
	parser->currentScope = scope;
	parser->lexer = lexer;

	ParseExprsOpt(BINARYLINEBREAKS_DISALLOWED);

	if ((token = NextToken())->kind != TOKEN_EOI) {
		AddError(&token->position, "Unexpected content at end of file.");
	}

	return NULL;
}

static SmileObject ParseExprsOpt(Int binaryLineBreaks)
{
	UNUSED(binaryLineBreaks);
	return NULL;
}
