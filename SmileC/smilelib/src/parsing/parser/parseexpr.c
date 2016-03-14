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
// Base expression parsing

//  nonbreak_expr ::= . expr    // Explicitly in a nonbreak_expr, binary operators cannot be matched
//									if they are the first non-whitespace on a line.  This behavior is
//									disabled at the end of the nonbreak_expr, whenever any [], (), or {}
//									grouping is entered, or whenever we are parsing inside the first
//									expr/arith of an if_then or a do-while.
//
//  expr ::= . base_expr
ParseError Parser_ParseExpr(Parser parser, SmileObject *expr, Int binaryLineBreaks)
{
	return Parser_ParseTerm(parser, expr, binaryLineBreaks, NULL);
}

//  base_expr ::= . arith
//         | . if_then
//         | . while_do
//         | . do_while
//         | . var_decl
//         | . till_do
//         | . try_catch
//         | . scope
//         | . return
//         | . INCLUDE string
//         | . INSERT_BRK base_expr
//         | . INSERT_SYNTAX syntax_expr
//         | . INSERT_MACRO macro_expr
//         | . INSERT_UNDEFINE any_name
ParseError Parser_ParseBaseExpr(Parser parser, SmileObject *expr, Int binaryLineBreaks)
{
	Token token;

	switch ((token = Parser_NextToken(parser))->kind) {
	/*
		case TokenKind.If:
		case TokenKind.Unless:
			return ParseIf(binaryLineBreaks, tokenKind == TokenKind.If);

		case TokenKind.While:
		case TokenKind.Until:
			return ParseWhileDo(binaryLineBreaks, tokenKind == TokenKind.While);

		case TokenKind.Do:
			return ParseDoWhile(binaryLineBreaks);

		case TokenKind.Till:
			return ParseTillDo(binaryLineBreaks);

		case TokenKind.Try:
			return ParseTryCatch(binaryLineBreaks);
	*/
		case TOKEN_VAR:
			return Parser_ParseVarDecls(parser, expr, binaryLineBreaks, PARSEDECL_VARIABLE);

		case TOKEN_CONST:
			return Parser_ParseVarDecls(parser, expr, binaryLineBreaks, PARSEDECL_CONST);

		case TOKEN_AUTO:
			return Parser_ParseVarDecls(parser, expr, binaryLineBreaks, PARSEDECL_AUTO);
	/*
		case TokenKind.LeftBrace:
			_lexer.Unget(_lexer.Token);
			return ParseScope(binaryLineBreaks);

		case TokenKind.Return:
			return ParseReturn(binaryLineBreaks);

		case TokenKind.Insert_Include:
			return ParseInclude(binaryLineBreaks);

		case TokenKind.Insert_Brk:
			return Env.List(_lexer.Token.Position, Env.BrkSymbol, ParseBaseExpr(binaryLineBreaks));

		case TokenKind.Insert_Syntax:
			return ParseSyntax(binaryLineBreaks);

		case TokenKind.Insert_Macro:
			return ParseMacro(binaryLineBreaks);

		case TokenKind.Insert_Undefine:
			return ParseUndefine(binaryLineBreaks);
	*/
		default:
			Lexer_Unget(parser->lexer);
			return Parser_ParseOpEquals(parser, expr, binaryLineBreaks, COMMAMODE_NORMAL);
	}
}

ParseError Parser_ParseOr(Parser parser, SmileObject *expr, Int binaryLineBreaks, Int commaMode)
{
	UNUSED(commaMode);

	return Parser_ParseTerm(parser, expr, binaryLineBreaks, NULL);
}
