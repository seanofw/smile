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

#include <smile/types.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

//-------------------------------------------------------------------------------------------------
// Recovery modes

Int Parser_BracesBracketsParenthesesBar_Recovery[] = {
	TOKEN_LEFTBRACE, TOKEN_LEFTBRACKET, TOKEN_LEFTPARENTHESIS,
	TOKEN_RIGHTBRACE, TOKEN_RIGHTBRACKET, TOKEN_RIGHTPARENTHESIS,
	TOKEN_BAR
};
Int Parser_BracesBracketsParenthesesBar_Count = sizeof(Parser_BracesBracketsParenthesesBar_Recovery) / sizeof(Int);

Int Parser_RightBracesBracketsParentheses_Recovery[] = {
	TOKEN_RIGHTBRACE, TOKEN_RIGHTBRACKET, TOKEN_RIGHTPARENTHESIS
};
Int Parser_RightBracesBracketsParentheses_Count = sizeof(Parser_RightBracesBracketsParentheses_Recovery) / sizeof(Int);

//-------------------------------------------------------------------------------------------------
// Helper methods

/// <summary>
/// When an error occurs, skip through the input until one of the given targets is found.
/// Does not consume the target token, and returns it.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <param name="tokenKinds">The tokens to search for.</param>
/// <param name="numTokenKinds">The number of tokens in the set of tokens to search for.</param>
Token Parser_Recover(Parser parser, Int *tokenKinds, Int numTokenKinds)
{
	Token token;

	while ((token = Parser_NextToken(parser))->kind != TOKEN_EOI
		&& !IntArrayContains(token->kind, tokenKinds, numTokenKinds));

	Lexer_Unget(parser->lexer);

	return token;
}

/// <summary>
/// Determine if the next token in the input is one of the two forms of '=', without consuming the input.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>True if the next token is one of the two forms of '=', False if it's
// anything else or nonexistent.</returns>
Bool Parser_HasEqualLookahead(Parser parser)
{
	Token token = Parser_NextToken(parser);
	Lexer_Unget(parser->lexer);
	return (token->kind == TOKEN_EQUAL || token->kind == TOKEN_EQUALWITHOUTWHITESPACE);
}

/// <summary>
/// Determine if the next token in the input is one of the two forms of '=' or a ':', without consuming the input.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>True if the next token is one of the two forms of '=' or a ':', False if it's
// anything else or nonexistent.</returns>
Bool Parser_HasEqualOrColonLookahead(Parser parser)
{
	Token token = Parser_NextToken(parser);
	Lexer_Unget(parser->lexer);
	return (token->kind == TOKEN_EQUAL || token->kind == TOKEN_EQUALWITHOUTWHITESPACE || token->kind == TOKEN_COLON);
}

/// <summary>
/// Determine if the next token in the input is the given token kind, without consuming the input.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>True if the next token is the named token kind, False if it's anything else or nonexistent.</returns>
Bool Parser_HasLookahead(Parser parser, Int tokenKind)
{
	Token token = Parser_NextToken(parser);
	Lexer_Unget(parser->lexer);
	return (token->kind == tokenKind);
}

/// <summary>
/// Determine if the next two token in the input are the given token kinds, without consuming the input.
/// </summary>
/// <param name="parser">The parser instance.</param>
/// <returns>True if the next two tokens are the named token kinds, False if they're anything else or nonexistent.</returns>
Bool Parser_Has2Lookahead(Parser parser, Int tokenKind1, Int tokenKind2)
{
	Token token1 = Parser_NextToken(parser);
	Token token2 = Parser_NextToken(parser);
	Lexer_Unget(parser->lexer);
	Lexer_Unget(parser->lexer);
	return (token1->kind == tokenKind1 && token2->kind == tokenKind2);
}
