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
#include <smile/smiletypes/smileloanword.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/stringbuilder.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/identkind.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/internal/staticstring.h>
#include <smile/regex.h>

static void Parser_DeclareCaptures(Parser parser, Regex regex, ParseScope scope, LexerPosition position);

STATIC_STRING(MissingLoanwordName, "Expected a name after #loanword.");
STATIC_STRING(MissingLoanwordColon, "Expected a ':' after the loanword name.");
STATIC_STRING(MissingLoanwordRegex, "Expected regex pattern after the loanword name.");
STATIC_STRING(MissingLoanwordImpliesSymbol, "Expected a '=>' (implies symbol) to separate the loanword pattern from its substitution.");

/// <summary>
/// Tokens that are used during common recovery scenarios to determine the
/// likely end of the current error:  {  }  [  ]  (  )
/// </summary>
static Int _loanwordRecover[] = {
	TOKEN_RIGHTBRACKET,
	TOKEN_RIGHTBRACE,
	TOKEN_RIGHTPARENTHESIS,
	TOKEN_LEFTBRACKET,
	TOKEN_LEFTBRACE,
	TOKEN_LEFTPARENTHESIS,
};
static Int _loanwordRecoverCount = sizeof(_loanwordRecover) / sizeof(Int);

// loanword_expr :: = . anyname COLON LOANWORD_REGEX IMPLIES raw_list_term
ParseResult Parser_ParseLoanword(Parser parser, Int modeFlags)
{
	Token token;
	Symbol name;
	SmileObject replacement;
	LexerPosition rulePosition, impliesPosition;
	Int templateKind;
	SmileLoanword loanword;
	Regex regex;
	TemplateResult templateResult;

	// First, read the loanword's name.
	token = Parser_NextToken(parser);
	rulePosition = Token_GetPosition(token);
	if (token->kind != TOKEN_ALPHANAME && token->kind != TOKEN_UNKNOWNALPHANAME) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, rulePosition, MissingLoanwordName));
	}
	name = token->data.symbol;

	// There must be a colon next.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_COLON) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingLoanwordColon));
	}

	// There must be a regex pattern that follows.
	token = Parser_NextToken(parser);
	if (token->kind != TOKEN_LOANWORD_REGEX) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingLoanwordRegex));
	}
	regex = (Regex)token->data.ptr;

	// Now, ensure that the special '=>' (implies) symbol exists.
	token = Parser_NextToken(parser);
	if (!(token->kind == TOKEN_PUNCTNAME || token->kind == TOKEN_UNKNOWNPUNCTNAME)
		|| token->data.symbol != Smile_KnownSymbols.implies) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), MissingLoanwordImpliesSymbol));
	}
	impliesPosition = Token_GetPosition(token);

	// Create a new scope for the loanword's substitution expression.
	Parser_BeginScope(parser, PARSESCOPE_SYNTAX);

	// Make sure the subsitution expression can "see" the regex capture names in scope.
	Parser_DeclareCaptures(parser, regex, parser->currentScope, rulePosition);

	// Parse the substitution expression in the syntax rule's scope.
	templateResult = Parser_ParseRawListTerm(parser, modeFlags);
	Parser_EndScope(parser, False);
	if (IS_PARSE_ERROR(templateResult.parseResult))
		RETURN_PARSE_ERROR(templateResult.parseResult);
	replacement = templateResult.parseResult.expr;
	templateKind = templateResult.templateKind;

	// Make sure the template is an evaluable expression form, not just a raw term.
	replacement = Parser_ConvertItemToTemplateIfNeeded(replacement, templateKind, impliesPosition);

	// Now make sure that the template is evaluable at parse-time.  The set of supported
	// parse-time template-evaluation forms is intentionally limited:
	//
	//   - n (where n is any nonterminal symbol)
	//   - [] (i.e., null)
	//   - [$quote x] (for any x)
	//   - [List.of x y z ...]
	//   - [List.join x y z ...]
	//   - [List.cons x y]
	//
	// All other computation is expressly prohibited and must be implemented using macros.
	Parser_VerifySyntaxTemplateIsEvaluableAtParseTime(parser, replacement);

	// Everything passes muster, so create the new loanword object.
	loanword = SmileLoanword_Create(name, regex, replacement, rulePosition);

	// Everything is all set up, so return the finished loanword object.
	return EXPR_RESULT(loanword);
}

Inline String GetDollarNameString(Int index)
{
	Byte dollarName[3];

	if (index < 10) {
		// Fast path for the common single-digit forms.
		dollarName[0] = '$';
		dollarName[1] = '0' + (Byte)index;
		return String_Create(dollarName, 2);
	}
	else if (index < 100) {
		// Fast-ish path for the less-common double-digit forms.
		dollarName[0] = '$';
		dollarName[1] = '0' + (Byte)(index % 10);
		dollarName[2] = '0' + (Byte)(index / 10);
		return String_Create(dollarName, 3);
	}
	else {
		// Slow path for the general case.
		return String_Format("$%u", (UInt32)index);
	}
}

/// <summary>
/// Skim through the given regex, find all its capture names, and declare them as variables
/// in the given scope, along with the special names $0 through $n for the regex's captures.
/// </summary>
/// <param name="regex">The regex to extract names from.</param>
/// <param name="scope">The parse scope in which the new names will be declared.</param>
/// <param name="position">The lexical position where the regex was found, so that the new
/// variables will be considered to have been declared *there*.</param>
static void Parser_DeclareCaptures(Parser parser, Regex regex, ParseScope scope, LexerPosition position)
{
	String *names;
	Int numNames;
	Int numCaptures;
	ParseDecl decl;
	Int i;
	String dollarNameString;
	String name;

	// Query the regex for its capture names and numbers.
	numNames = Regex_GetCaptureNames(regex, &names);
	numCaptures = Regex_GetCaptureCount(regex);

	// Declare $0 through $n.
	for (i = 0; i < numCaptures; i++) {
		dollarNameString = GetDollarNameString(i);
		ParseScope_Declare(scope, SymbolTable_GetSymbol(Smile_SymbolTable, dollarNameString), PARSEDECL_VARIABLE, position, &decl);
	}

	// Declare any named captures, which must not start with '$'.
	for (i = 0; i < numNames; i++) {
		name = names[i];
		if (String_StartsWithC(name, "$")) {
			Parser_AddError(parser, position, "Named captures in a loanword must not start with the reserved '$' character.");
		}
		else {
			ParseScope_Declare(scope, SymbolTable_GetSymbol(Smile_SymbolTable, name), PARSEDECL_VARIABLE, position, &decl);
		}
	}
}

typedef struct ReplacementInfoStruct {
	Int32Dict replacements;
	RegexMatch match;
} *ReplacementInfo;

static Bool MakeReplacementValue(String key, Int index, void *param)
{
	ReplacementInfo replacementInfo = (ReplacementInfo)param;

	Symbol symbol = SymbolTable_GetSymbol(Smile_SymbolTable, key);
	Int32Dict_Add(replacementInfo->replacements, symbol, RegexMatch_GetCapture(replacementInfo->match, index));

	return True;
}

static Int32Dict Parser_CreateLoanwordReplacementValuesFromMatch(RegexMatch match)
{
	Int32Dict replacements;
	Int i;
	struct ReplacementInfoStruct replacementInfo;
	String dollarNameString;
	Symbol symbol;

	replacements = Int32Dict_CreateWithSize((Int32)(match->numIndexedCaptures + StringIntDict_Count(&match->namedCaptures)));

	for (i = 0; i < match->numIndexedCaptures; i++) {
		dollarNameString = GetDollarNameString(i);
		symbol = SymbolTable_GetSymbol(Smile_SymbolTable, dollarNameString);
		Int32Dict_Add(replacements, symbol, RegexMatch_GetCapture(match, i));
	}

	replacementInfo.match = match;
	replacementInfo.replacements = replacements;
	StringIntDict_ForEach(&match->namedCaptures, MakeReplacementValue, &replacementInfo);

	return replacements;
}

ParseResult Parser_ApplyCustomLoanword(Parser parser, Token token)
{
	SmileLoanword loanword;
	Symbol symbol;
	RegexMatch match;
	Int32Dict replacements;
	LexerPosition position;

	position = Token_GetPosition(token);

	// First, look up the loanword's definition (if there even exists a loanword that matches).
	symbol = SymbolTable_GetSymbolNoCreate(Smile_SymbolTable, token->text);
	if (!symbol
		|| !Int32Dict_TryGetValue(parser->currentScope->loanwordTable->definitions, symbol, (void **)&loanword)) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			String_Format("Unknown loanword '#%S'.", token->text)));
	}

	// We have a valid loanword, and we're going to lex it using its regex.
	// But first, if there is any following whitespace at all, consume whitespace characters
	// up to and including the next newline.  This will result in simpler loanword rules, at the
	// possible cost of losing the ability to treat initial whitespace as meaningful content.
	Lexer_ConsumeWhitespaceOnThisLine(parser->lexer);
	match = Lexer_ConsumeRegex(parser->lexer, loanword->regex);

	// If the regex match failed, this loanword was parsed as an error.
	if (match == NULL) {
		return ERROR_RESULT(ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token),
			String_Format("After loanword '#%S', the source code does not match the loanword's regex pattern.", loanword->name)));
	}

	// The regex match succeeded, and we have its data stored in the 'match' dictionary and array.
	// Now we have to transform those into an appropriate replacement-symbol lookup table.
	replacements = Parser_CreateLoanwordReplacementValuesFromMatch(match);

	// Now use the captures against the template to generate the actual result.
	return EXPR_RESULT(Parser_RecursivelyApplyTemplate(parser, loanword->replacement, replacements, position));
}
