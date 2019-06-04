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

#include <smile/types.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static Int _barRecoveryTokens[] = {
	TOKEN_LEFTBRACE, TOKEN_LEFTBRACKET, TOKEN_LEFTPARENTHESIS, TOKEN_RIGHTBRACE, TOKEN_RIGHTBRACKET, TOKEN_RIGHTPARENTHESIS,
	TOKEN_BAR, TOKEN_SEMICOLON,
};

// func ::= BAR . params_opt BAR expr semi_opt
ParseResult Parser_ParseFunc(Parser parser, Int modeFlags)
{
	LexerPosition funcPosition;
	SmileList paramList;
	SmileObject body, expr;
	ParseError parseError;
	ParseResult parseResult;

	funcPosition = Token_GetPosition(parser->lexer->token);

	Parser_BeginScope(parser, PARSESCOPE_FUNCTION);

	parseResult = Parser_ParseParamsOpt(parser);
	paramList = (SmileList)parseResult.expr;

	if (Lexer_Next(parser->lexer) != TOKEN_BAR) {
		Lexer_Unget(parser->lexer);
		parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_Format("Expected |...| to end function parameters starting on line %d", funcPosition->lineStart));
		Parser_EndScope(parser, False);
		return ERROR_RESULT(parseError);
	}

	parseResult = Parser_ParseExpr(parser, modeFlags);
	if (IS_PARSE_ERROR(parseResult)) {
		Parser_EndScope(parser, False);
		RETURN_PARSE_ERROR(parseResult);
	}
	body = parseResult.expr;

	// Allow an optional semicolon to terminate the function, which is very useful for short inline functions in long compound expressions.
	if (Lexer_Next(parser->lexer) != TOKEN_SEMICOLON) {
		Lexer_Unget(parser->lexer);
	}

	Parser_EndScope(parser, False);

	expr =
		(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._fnSymbol,
			(SmileObject)SmileList_ConsWithSource((SmileObject)paramList,
				(SmileObject)SmileList_ConsWithSource(body, NullObject, funcPosition),
			funcPosition),
		funcPosition);

	return EXPR_RESULT(expr);
}

// params_opt :: = params param_rest | param_rest |
// param_rest :: = param_name ELLIPSIS
// params :: = params param | params ',' param | param
ParseResult Parser_ParseParamsOpt(Parser parser)
{
	SmileList head = NullList, tail = NullList;
	SmileObject param = NullObject;
	Int tokenKind;
	Bool isFirst;
	LexerPosition paramPosition;
	SmileList paramList;
	Symbol paramMetaSymbol;
	Symbol paramName;
	ParseResult parseResult;

	isFirst = True;
	for (;;) {

		tokenKind = Lexer_Next(parser->lexer);
		if (tokenKind == TOKEN_BAR) {
			// End of arguments.
			Lexer_Unget(parser->lexer);
			break;
		}
		if (tokenKind == TOKEN_COMMA) {
			if (isFirst) {
				Parser_AddMessage(parser,
					ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
						String_Format("Illegal comma found in function parameter list")));
				// Recover by simply absorbing the comma.
			}
		}
		else {
			Lexer_Unget(parser->lexer);
		}

		// Parse the next argument.
		parseResult = Parser_ParseParam(parser, &paramPosition);
		if (IS_PARSE_ERROR(parseResult)) {
			HANDLE_PARSE_ERROR(parser, parseResult);
			Parser_Recover(parser, _barRecoveryTokens, sizeof(_barRecoveryTokens));
			break;
		}
		param = parseResult.expr;
	
		// Is this the trailing "rest" parameter?
		if (Lexer_Next(parser->lexer) == TOKEN_DOTDOTDOT) {
			
			// It is the rest parameter.  Make sure it doesn't have a default value assigned, because
			// such things are nonsensical for the "rest" parameter.
			if (SMILE_KIND(param) == SMILE_KIND_LIST) {
				paramName = ((SmileSymbol)((SmileList)param)->a)->symbol;
				for (paramList = (SmileList)((SmileList)param)->d; SMILE_KIND(paramList) == SMILE_KIND_LIST; paramList = LIST_REST(LIST_REST(paramList))) {
					paramMetaSymbol = ((SmileSymbol)paramList->a)->symbol;
					if (paramMetaSymbol == Smile_KnownSymbols.default_) {
						Parser_AddMessage(parser,
							ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
								String_Format("Rest argument '%S' cannot have a default value assigned to it.",
									SymbolTable_GetName(Smile_SymbolTable, paramName))));
						// Recover by simply continuing.
					}
				}
			}
			else {
				paramName = ((SmileSymbol)param)->symbol;
			}
		
			// If the next token isn't a bar, we have an error.
			if (Lexer_Peek(parser->lexer) != TOKEN_BAR) {
				Parser_AddMessage(parser,
					ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
						String_Format("Rest argument '%S' must be the last argument in the function declaration.",
							SymbolTable_GetName(Smile_SymbolTable, paramName))));
				// Recover by simply continuing.
			}
		
			// This is an annotated argument, so if it was previously a plain symbol, turn it into a list.
			if (SMILE_KIND(param) != SMILE_KIND_LIST) {
				param = (SmileObject)SmileList_ConsWithSource(
					param,
					NullObject,
					paramPosition
				);
			}
	
			// Now append [... rest] to the argument's annotation list.
			for (paramList = (SmileList)param; SMILE_KIND(paramList->d) == SMILE_KIND_LIST; paramList = LIST_REST(paramList)) ;
			paramList->d =
				(SmileObject)SmileList_ConsWithSource(
					(SmileObject)Smile_KnownObjects.restSymbol,
					NullObject,
					paramPosition
				);
		}
		else {
			// Not an ellipsis, so don't eat it.
			Lexer_Unget(parser->lexer);
		}
	
		LIST_APPEND_WITH_SOURCE(head, tail, param, paramPosition);

		isFirst = False;
	}

	// This function always succeeds, but may add to the parser's error list as a side effect.
	return EXPR_RESULT(head);
}

// param :: = param_name | param_name '=' raw_list_term
// param_name :: = name | param_type COLON name
ParseResult Parser_ParseParam(Parser parser, LexerPosition *position)
{
	Int tokenKind, nextTokenKind;
	SmileObject param, type, defaultValue;
	LexerPosition paramPosition, typePosition, defaultPosition;
	Token token;
	Symbol nameSymbol;
	ParseDecl decl;
	SmileList tail = NullList;
	ParseResult parseResult;
	TemplateResult templateResult;

	tokenKind = Lexer_Next(parser->lexer);
	token = parser->lexer->token;
	*position = paramPosition = Token_GetPosition(token);

	// Make sure this starts with a valid name for the new function argument.
	if (tokenKind != TOKEN_ALPHANAME && tokenKind != TOKEN_UNKNOWNALPHANAME
		&& tokenKind != TOKEN_PUNCTNAME && tokenKind != TOKEN_UNKNOWNPUNCTNAME) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_Format("Invalid function argument name")));
	}

	// Is it followed by a colon? If so, this isn't the argument's name; it's actually the type.
	nextTokenKind = Lexer_Next(parser->lexer);
	if (nextTokenKind != TOKEN_COLON) {
		Lexer_Unget(parser->lexer);

		// Just a plain function argument.
		nameSymbol = token->data.symbol;
		ParseScope_DeclareHere(parser->currentScope, nameSymbol, PARSEDECL_ARGUMENT, paramPosition, &decl);
		param = (SmileObject)SmileSymbol_Create(nameSymbol);
	}
	else {

		// This looks like a type assertion, so parse it as a type form, and then get the name that follows it.
		Lexer_Unget(parser->lexer);
		Lexer_Unget(parser->lexer);
	
		// First, get the type.
		parseResult = Parser_ParseParamType(parser);
		if (IS_PARSE_ERROR(parseResult))
			RETURN_PARSE_ERROR(parseResult);
		type = parseResult.expr;
		typePosition = Token_GetPosition(parser->lexer->token);

		// Make sure there's a colon after it.
		tokenKind = Lexer_Next(parser->lexer);
		if (tokenKind != TOKEN_COLON)
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Expected a ':' after function argument type")));
	
		// Now get the real function argument name.
		tokenKind = Lexer_Next(parser->lexer);
		token = parser->lexer->token;
		paramPosition = Token_GetPosition(token);
		if (!(tokenKind == TOKEN_ALPHANAME || tokenKind == TOKEN_UNKNOWNALPHANAME
			|| tokenKind == TOKEN_PUNCTNAME || tokenKind == TOKEN_UNKNOWNPUNCTNAME)) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, paramPosition,
				String_Format("Invalid function argument name after function argument type")));
		}
	
		nameSymbol = token->data.symbol;
		ParseScope_DeclareHere(parser->currentScope, nameSymbol, PARSEDECL_ARGUMENT, paramPosition, &decl);

		// This is an annotated argument, so the parameter name needs to be represented as a list.
		param = (SmileObject)SmileList_ConsWithSource(
			(SmileObject)SmileSymbol_Create(nameSymbol),
			NullObject,
			paramPosition
		);
		tail = (SmileList)param;
	
		// Now append [... type foo] to the argument's annotation list.
		tail->d =
			(SmileObject)SmileList_ConsWithSource(
				(SmileObject)Smile_KnownObjects.typeSymbol,
				(SmileObject)SmileList_ConsWithSource(
					type,
					NullObject,
					typePosition
				),
				typePosition
			);
		tail = (SmileList)((SmileList)tail->d)->d;
	}

	// Next, see if this has a default value assigned after it.
	if ((tokenKind = Lexer_Next(parser->lexer)) == TOKEN_EQUAL
		|| tokenKind == TOKEN_EQUALWITHOUTWHITESPACE) {
	
		// Optional parameter, with an assigned default value.
		defaultPosition = Lexer_GetPosition(parser->lexer);
		templateResult = Parser_ParseRawListTerm(parser, 0);
		if (IS_PARSE_ERROR(templateResult.parseResult))
			RETURN_PARSE_ERROR(templateResult.parseResult);
		if (templateResult.templateKind != TemplateKind_None) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Default value for argument '%S' is not a constant value.", SymbolTable_GetName(Smile_SymbolTable, nameSymbol))));
		}
		defaultValue = templateResult.parseResult.expr;
	
		// This is an annotated argument, so if it was previously a plain symbol, turn it into a list.
		if (SMILE_KIND(param) != SMILE_KIND_LIST) {
			param = (SmileObject)SmileList_ConsWithSource(
				(SmileObject)SmileSymbol_Create(nameSymbol),
				NullObject,
				paramPosition
			);
			tail = (SmileList)param;
		}

		// Now append [... default foo] to the argument's annotation list.
		tail->d =
			(SmileObject)SmileList_ConsWithSource(
				(SmileObject)Smile_KnownObjects.defaultSymbol,
				(SmileObject)SmileList_ConsWithSource(
					defaultValue,
					NullObject,
					defaultPosition
				),
				defaultPosition
			);
		tail = (SmileList)((SmileList)tail->d)->d;
	}
	else {
		// No equal sign, so don't eat the next token.
		Lexer_Unget(parser->lexer);
	}

	// Success!
	return EXPR_RESULT(param);
}

// param_type ::= . ALPHA_NAME | . PUNCT_NAME
ParseResult Parser_ParseParamType(Parser parser)
{
	Token token;
	SmileSymbol smileSymbol;

	token = Parser_NextToken(parser);

	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_PUNCTNAME) {
		if (token->kind == TOKEN_UNKNOWNALPHANAME || token->kind == TOKEN_UNKNOWNPUNCTNAME) {
			return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
				String_Format("The function argument type name \"{0}\" does not exist.",
					SymbolTable_GetName(Smile_SymbolTable, token->data.symbol))));
		}
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			String_Format("Illegal function argument type name")));
	}

	smileSymbol = SmileSymbol_Create(parser->lexer->token->data.symbol);
	Lexer_Unget(parser->lexer);
	return EXPR_RESULT(smileSymbol);
}

static ParseError DeclareClassicFnArg(Parser parser, SmileObject arg, LexerPosition argPosition)
{
	SmileSymbol smileSymbol;
	SmileList smileList;
	ParseDecl decl;

	switch (SMILE_KIND(arg)) {
	
		case SMILE_KIND_SYMBOL:
			smileSymbol = (SmileSymbol)arg;
			return ParseScope_DeclareHere(parser->currentScope, smileSymbol->symbol, PARSEDECL_ARGUMENT, argPosition, &decl);

		case SMILE_KIND_LIST:
			smileList = (SmileList)arg;
			if (SMILE_KIND(smileList->a) != SMILE_KIND_SYMBOL) {
				return ParseMessage_Create(PARSEMESSAGE_ERROR, argPosition,
					String_FromC("Invalid function argument name."));
			}
			smileSymbol = (SmileSymbol)smileList->a;
			return ParseScope_DeclareHere(parser->currentScope, smileSymbol->symbol, PARSEDECL_ARGUMENT, argPosition, &decl);

		default:
			return ParseMessage_Create(PARSEMESSAGE_ERROR, argPosition,
				String_FromC("Invalid function argument name."));
	}
}

// term ::= '[' '$fn' . '[' args ']' body ']'
ParseResult Parser_ParseClassicFn(Parser parser, LexerPosition startPosition)
{
	ParseResult parseResult;
	TemplateResult templateResult;
	SmileList args, temp;
	SmileObject body, expr;
	Int argsTemplateKind;

	Parser_BeginScope(parser, PARSESCOPE_FUNCTION);

	templateResult = Parser_ParseRawListTerm(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (IS_PARSE_ERROR(templateResult.parseResult)) {
		Parser_EndScope(parser, False);
		RETURN_PARSE_ERROR(templateResult.parseResult);
	}
	args = (SmileList)templateResult.parseResult.expr;
	argsTemplateKind = templateResult.templateKind;

	if ((SMILE_KIND(args) != SMILE_KIND_LIST && SMILE_KIND(args) != SMILE_KIND_NULL) || argsTemplateKind != TemplateKind_None) {
		Parser_EndScope(parser, False);
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, startPosition,
			String_Format("First argument to [$fn] must be a list of symbols.", startPosition->line)));
	}

	for (temp = args; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
		ParseError error;
		if ((error = DeclareClassicFnArg(parser, temp->a, SMILE_VCALL(temp, getSourceLocation))) != NULL) {
			Parser_EndScope(parser, False);
			return ERROR_RESULT(error);
		}
	}

	parseResult = Parser_ParseExpr(parser, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);
	if (IS_PARSE_ERROR(parseResult)) {
		Parser_EndScope(parser, False);
		RETURN_PARSE_ERROR(parseResult);
	}
	body = parseResult.expr;

	Parser_EndScope(parser, False);

	parseResult = Parser_ExpectRightBracket(parser, NULL, "[$fn] form", startPosition);
	if (IS_PARSE_ERROR(parseResult))
		RETURN_PARSE_ERROR(parseResult);

	expr =
		(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._fnSymbol,
			(SmileObject)SmileList_ConsWithSource((SmileObject)args,
				(SmileObject)SmileList_ConsWithSource(body, NullObject, startPosition),
			startPosition),
		startPosition);

	return EXPR_RESULT(expr);
}
