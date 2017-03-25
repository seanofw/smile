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
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/parsing/internal/parsesyntax.h>

//-------------------------------------------------------------------------------------------------
// Applying custom syntax rules to the incoming token stream.

/// <summary>
/// Look up the current syntax class object that contains the tree of syntax rules rooted under
/// the given class symbol.
/// </summary>
/// <param name="parser">The parser that describes the current parsing state.</param>
/// <param name="syntaxClassSymbol">The nonterminal symbol of the class to locate.</param>
/// <returns>The class, if a matching class is defined, or NULL if no such class exists in the
/// current scope.</returns>
/// <remarks>
/// This is highly-optimized for the nine special syntax classes; for those, execution time is
/// guaranteed to be O(1) (with a small constant factor, and no searching).  For all other syntax
/// classes, this code will still work, but is only amortized O(1), and may involve searching.
/// </remarks>
static ParserSyntaxClass GetSyntaxClass(Parser parser, Symbol syntaxClassSymbol)
{
	ParserSyntaxTable syntaxTable = parser->currentScope->syntaxTable;
	ParserSyntaxClass syntaxClass;

	switch (syntaxClassSymbol) {
	
		case SMILE_SPECIAL_SYMBOL_STMT:
			return syntaxTable->stmtClass;
		case SMILE_SPECIAL_SYMBOL_EXPR:
			return syntaxTable->exprClass;
		case SMILE_SPECIAL_SYMBOL_CMPEXPR:
			return syntaxTable->cmpExprClass;
		case SMILE_SPECIAL_SYMBOL_ADDEXPR:
			return syntaxTable->addExprClass;
		case SMILE_SPECIAL_SYMBOL_MULEXPR:
			return syntaxTable->mulExprClass;
		case SMILE_SPECIAL_SYMBOL_BINARYEXPR:
			return syntaxTable->binaryExprClass;
		case SMILE_SPECIAL_SYMBOL_PREFIXEXPR:
			return syntaxTable->prefixExprClass;
		case SMILE_SPECIAL_SYMBOL_POSTFIXEXPR:
			return syntaxTable->postfixExprClass;
		case SMILE_SPECIAL_SYMBOL_TERM:
			return syntaxTable->termClass;
		
		default:
			if (!Int32Dict_TryGetValue(syntaxTable->syntaxClasses, syntaxClassSymbol, &syntaxClass))
				return NULL;
			return syntaxClass;
	}
}

static ParserSyntaxNode Parser_MatchCustomTerminal(Token token, Int32Dict nextDict)
{
	ParserSyntaxNode nextNode;
	Symbol tokenSymbol;

	// Can't match a next state if there is no next state.
	if (nextDict == NULL) return NULL;

	// Match the next token in the input to the names in the nextDict.
	switch (token->kind) {

	case TOKEN_LEFTBRACE:
		tokenSymbol = Smile_KnownSymbols.left_brace;
		goto haveSymbolToken;
	case TOKEN_RIGHTBRACE:
		tokenSymbol = Smile_KnownSymbols.right_brace;
		goto haveSymbolToken;
	case TOKEN_LEFTPARENTHESIS:
		tokenSymbol = Smile_KnownSymbols.left_parenthesis;
		goto haveSymbolToken;
	case TOKEN_RIGHTPARENTHESIS:
		tokenSymbol = Smile_KnownSymbols.right_parenthesis;
		goto haveSymbolToken;
	case TOKEN_COMMA:
		tokenSymbol = Smile_KnownSymbols.comma;
		goto haveSymbolToken;
	case TOKEN_SEMICOLON:
		tokenSymbol = Smile_KnownSymbols.semicolon;
		goto haveSymbolToken;
	case TOKEN_COLON:
		tokenSymbol = Smile_KnownSymbols.colon;
		goto haveSymbolToken;

	case TOKEN_UNKNOWNALPHANAME:
	case TOKEN_ALPHANAME:
	case TOKEN_UNKNOWNPUNCTNAME:
	case TOKEN_PUNCTNAME:
		tokenSymbol = SymbolTable_GetSymbol(Smile_SymbolTable, token->text);

	haveSymbolToken:
		// We have a symbol of some kind from the input stream.  So check the current
		// tree node (dictionary) to see if this symbol appears as a terminal in it.
		return Int32Dict_TryGetValue(nextDict, tokenSymbol, &nextNode) ? nextNode : NULL;

	default:
		// Not a symbol, so not matchable.
		return NULL;
	}
}

/// <summary>
/// Pair up the given values with the given array of keys, generating a lookup dictionary
/// mapping the keys to values.  The keys must be unique.
/// </summary>
/// <param name="keys">The array of key names, in order.</param>
/// <param name="numKeys">The number of key names in the array.</param>
/// <param name="values">A list of the values to associate with each key, in order.</param>
/// <returns>A dictionary mapping the keys to their values.</returns>
static Int32Dict Parser_MapKeysToValues(Symbol *keys, Int numKeys, SmileList values)
{
	Int32Dict result = Int32Dict_CreateWithSize(64);

	for (; numKeys-- && SMILE_KIND(values) == SMILE_KIND_LIST; values = LIST_REST(values)) {
	
		Int32Dict_Add(result, *keys++, values->a);
	}

	return result;
}

static SmileObject Parser_RecursivelyClone(SmileObject expr)
{
	switch (expr->kind & (SMILE_KIND_MASK | SMILE_FLAG_WITHSOURCE)) {
		case SMILE_KIND_LIST:
		{
			SmileList oldList = (SmileList)expr;
			SmileList newList = SmileList_Cons(
				Parser_RecursivelyClone(oldList->a),
				Parser_RecursivelyClone(oldList->d)
			);
			return (SmileObject)newList;
		}

		case SMILE_KIND_LIST | SMILE_FLAG_WITHSOURCE:
		{
			struct SmileListWithSourceInt *oldList = (struct SmileListWithSourceInt *)expr;
			SmileList newList = SmileList_ConsWithSource(
				Parser_RecursivelyClone(oldList->a),
				Parser_RecursivelyClone(oldList->d),
				oldList->position
			);
			return (SmileObject)newList;
		}

		case SMILE_KIND_PAIR:
		{
			SmilePair oldPair = (SmilePair)expr;
			SmilePair newPair = SmilePair_Create(
				Parser_RecursivelyClone(oldPair->left),
				Parser_RecursivelyClone(oldPair->right)
			);
			return (SmileObject)newPair;
		}

		case SMILE_KIND_PAIR | SMILE_FLAG_WITHSOURCE:
		{
			struct SmilePairWithSourceInt *oldPair = (struct SmilePairWithSourceInt *)expr;
			SmilePair newPair = SmilePair_CreateWithSource(
				Parser_RecursivelyClone(oldPair->left),
				Parser_RecursivelyClone(oldPair->right),
				oldPair->position
			);
			return (SmileObject)newPair;
		}

		default:
			return expr;
	}
}

static SmileObject Parser_RecursivelyApplyReplacementVariables(SmileObject expr, Int32Dict replacements, Int32Int32Dict usageDict)
{
	switch (expr->kind & (SMILE_KIND_MASK | SMILE_FLAG_WITHSOURCE)) {
		case SMILE_KIND_LIST:
		{
			SmileList oldList = (SmileList)expr;
			SmileList newList = SmileList_Cons(
				Parser_RecursivelyApplyReplacementVariables(oldList->a, replacements, usageDict),
				Parser_RecursivelyApplyReplacementVariables(oldList->d, replacements, usageDict)
			);
			return (SmileObject)newList;
		}

		case SMILE_KIND_LIST | SMILE_FLAG_WITHSOURCE:
		{
			struct SmileListWithSourceInt *oldList = (struct SmileListWithSourceInt *)expr;
			SmileList newList = SmileList_ConsWithSource(
				Parser_RecursivelyApplyReplacementVariables(oldList->a, replacements, usageDict),
				Parser_RecursivelyApplyReplacementVariables(oldList->d, replacements, usageDict),
				oldList->position
			);
			return (SmileObject)newList;
		}

		case SMILE_KIND_PAIR:
		{
			SmilePair oldPair = (SmilePair)expr;
			SmilePair newPair = SmilePair_Create(
				Parser_RecursivelyApplyReplacementVariables(oldPair->left, replacements, usageDict),
				Parser_RecursivelyApplyReplacementVariables(oldPair->right, replacements, usageDict)
			);
			return (SmileObject)newPair;
		}

		case SMILE_KIND_PAIR | SMILE_FLAG_WITHSOURCE:
		{
			struct SmilePairWithSourceInt *oldPair = (struct SmilePairWithSourceInt *)expr;
			SmilePair newPair = SmilePair_CreateWithSource(
				Parser_RecursivelyApplyReplacementVariables(oldPair->left, replacements, usageDict),
				Parser_RecursivelyApplyReplacementVariables(oldPair->right, replacements, usageDict),
				oldPair->position
			);
			return (SmileObject)newPair;
		}

		case SMILE_KIND_SYMBOL:
		case SMILE_KIND_SYMBOL | SMILE_FLAG_WITHSOURCE:
		{
			SmileSymbol symbol = (SmileSymbol)expr;
			SmileObject newExpr;
			if (!Int32Dict_TryGetValue(replacements, symbol->symbol, &newExpr)) {
				return expr;
			}
			if (!Int32Int32Dict_Add(usageDict, symbol->symbol, 1)) {
				// We already used the original copy of this expression, so this time, add a clone of
				// the expression to ensure that the result of the parse is a tree, not a DAG.
				newExpr = Parser_RecursivelyClone(newExpr);
			}
			return newExpr;
		}
		
		default:
			return expr;
	}
}

static SmileObject Parser_Accept(SmileObject replacement, Symbol *replacementVariables, Int numReplacementVariables, SmileList replacementExpressions)
{
	Int32Dict replacementDict;
	Int32Int32Dict usageDict;
	SmileObject result;

	replacementDict = Parser_MapKeysToValues(replacementVariables, numReplacementVariables, replacementExpressions);

	usageDict = Int32Int32Dict_CreateWithSize(64);

	result = Parser_RecursivelyApplyReplacementVariables(replacement, replacementDict, usageDict);

	return result;
}

static CustomSyntaxResult Parser_RecursivelyApplyCustomSyntax(Parser parser, SmileObject *expr, Int modeFlags, Symbol syntaxClassSymbol, ParseError *parseError)
{
	switch (syntaxClassSymbol) {

		case SMILE_SPECIAL_SYMBOL_STMT:
			*parseError = Parser_ParseStmt(parser, expr, modeFlags);
			break;

		case SMILE_SPECIAL_SYMBOL_EXPR:
			*parseError = Parser_ParseEquals(parser, expr, modeFlags);
			break;

		case SMILE_SPECIAL_SYMBOL_CMPEXPR:
			*parseError = Parser_ParseCmpExpr(parser, expr, modeFlags);
			break;

		case SMILE_SPECIAL_SYMBOL_ADDEXPR:
			*parseError = Parser_ParseAddExpr(parser, expr, modeFlags);
			break;

		case SMILE_SPECIAL_SYMBOL_MULEXPR:
			*parseError = Parser_ParseMulExpr(parser, expr, modeFlags);
			break;

		case SMILE_SPECIAL_SYMBOL_BINARYEXPR:
			*parseError = Parser_ParseBinaryExpr(parser, expr, modeFlags);
			break;

		case SMILE_SPECIAL_SYMBOL_PREFIXEXPR:
			*parseError = Parser_ParsePrefixExpr(parser, expr, modeFlags);
			break;

		case SMILE_SPECIAL_SYMBOL_TERM:
			*parseError = Parser_ParseTerm(parser, expr, modeFlags, NULL);
			break;

		default:
			return Parser_ApplyCustomSyntax(parser, expr, modeFlags, syntaxClassSymbol, SYNTAXROOT_RECURSE, 0, parseError);
	}

	return *parseError != NULL ? CustomSyntaxResult_PartialApplicationWithError : CustomSyntaxResult_SuccessfullyParsed;
}

Inline Symbol GetSymbolForToken(Token token)
{
	Symbol tokenSymbol;

	switch (token->kind) {

		case TOKEN_ALPHANAME:
		case TOKEN_UNKNOWNALPHANAME:
		case TOKEN_PUNCTNAME:
		case TOKEN_UNKNOWNPUNCTNAME:
			// If this token had escapes in it, we treat it as an unknown generic name.  If it didn't,
			// then we look it up in the symbol table to try to match it as a keyword.
			tokenSymbol = token->hasEscapes ? -1 : SymbolTable_GetSymbolNoCreate(Smile_SymbolTable, token->text);
			break;
		
		case TOKEN_COMMA:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_COMMA;
			break;
		
		case TOKEN_SEMICOLON:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_SEMICOLON;
			break;
		
		case TOKEN_COLON:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_COLON;
			break;
		
		case TOKEN_LEFTPARENTHESIS:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_LEFTPARENTHESIS;
			break;
		
		case TOKEN_RIGHTPARENTHESIS:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_RIGHTPARENTHESIS;
			break;
		
		case TOKEN_LEFTBRACE:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_LEFTBRACE;
			break;
		
		case TOKEN_RIGHTBRACE:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_RIGHTBRACE;
			break;
		
		case TOKEN_LEFTBRACKET:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_LEFTBRACKET;
			break;
		
		case TOKEN_RIGHTBRACKET:
			tokenSymbol = SMILE_SPECIAL_SYMBOL_RIGHTBRACKET;
			break;
		
		default:
			tokenSymbol = 0;
			break;
	}

	return tokenSymbol;
}

CustomSyntaxResult Parser_ApplyCustomSyntax(Parser parser, SmileObject *expr, Int modeFlags, Symbol syntaxClassSymbol,
	Int syntaxRootMode, Symbol rootSkipSymbol, ParseError *parseError)
{
	ParserSyntaxClass syntaxClass;
	ParserSyntaxNode node, nextNode;
	LexerPosition position;
	SmileObject localExpr;
	SmileList localHead, localTail;
	Bool isFirst;
	Int32Dict transitionTable;
	Int tokenKind;
	Symbol tokenSymbol;
	Int32Int32Dict oldCustomFollowSet;
	CustomSyntaxResult nestedSyntaxResult;

	// Get the class that contains all the rules under the provided nonterminal symbol.
	syntaxClass = GetSyntaxClass(parser, syntaxClassSymbol);
	if (syntaxClass == NULL) {
		*parseError = NULL;
		return CustomSyntaxResult_NotMatchedAndNoTokensConsumed;
	}

	// Begin walking the tree nodes of the class, consuming input tokens where they match.
	node = (ParserSyntaxNode)syntaxClass;
	isFirst = True;

	// This is the list where we'll be collecting the nonterminal matches as we find them.
	localTail = localHead = NullList;

	// For the special syntax classes, we may need to apply special behaviors for the initial transition.
	if (syntaxRootMode == SYNTAXROOT_KEYWORD) {
		// We can only transition via an initial terminal; we don't need to construct or find a
		// transition table, since the nextTerminals set will be sufficient.
		tokenKind = Lexer_Next(parser->lexer);
		tokenSymbol = GetSymbolForToken(parser->lexer->token);
		if (node->nextTerminals == NULL || !Int32Dict_TryGetValue(node->nextTerminals, tokenSymbol, &nextNode)) {
			Lexer_Unget(parser->lexer);
			return CustomSyntaxResult_NotMatchedAndNoTokensConsumed;
		}
		node = nextNode;
		goto handleTransition;
	}
	else if (syntaxRootMode == SYNTAXROOT_NONTERMINAL) {
		// We need to skip over the given initial nonterminal (which should already have been
		// parsed and sitting in *expr), and then parse everything after it.
		if (node->nextNonterminals == NULL || !Int32Dict_TryGetValue(node->nextNonterminals, rootSkipSymbol, &nextNode))
			return CustomSyntaxResult_NotMatchedAndNoTokensConsumed;
		node = nextNode;
	}

	for (;;) {
		// Try to match the next item (terminal or nonterminal) in the pattern.
	
		// Find or construct a table that describes possible transitions to subsequent states.
		transitionTable = ParserSyntaxTable_GetTransitionTable(parser->currentScope->syntaxTable, node);
		if (transitionTable == NULL) {
			// Couldn't construct a transition table due to a grammar conflict.
			*parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
				String_Format("Grammar error: In rule '%S', the next state after '%S' is ambiguous",
					SymbolTable_GetName(Smile_SymbolTable, syntaxClassSymbol),
					SymbolTable_GetName(Smile_SymbolTable, node->name)));
			return isFirst ? CustomSyntaxResult_NotMatchedAndNoTokensConsumed : CustomSyntaxResult_PartialApplicationWithError;
		}

		// Try to actually transition to the next state based on the next token in the input.
		tokenKind = Lexer_Next(parser->lexer);
		tokenSymbol = GetSymbolForToken(parser->lexer->token);
		if (!Int32Dict_TryGetValue(transitionTable, tokenSymbol, &nextNode)) {
			// Didn't match it exactly.  See if this transition table has an "every symbol" catch-all rule.
			if (!Int32Dict_TryGetValue(transitionTable, -1, &nextNode)) {
				// Nothing in the transition table matches the next incoming token, so we're done with this rule.
				Lexer_Unget(parser->lexer);
				break;
			}
		}
		node = nextNode;
	
	handleTransition:

		// We have a next state for this token in nextNode, so transition into it.
		if (!node->variable) {

			// The next node is a simple terminal (effectively a keyword or piece of punctuation), so consume the
			// token, and transition directly into it.
		}
		else {
			// The next node is a nonterminal, so recursively invoke it.

			// Collect the current position, for error-reporting.
			position = Token_GetPosition(parser->lexer->token);
			Lexer_Unget(parser->lexer);

			// Recursively traverse the syntax tree.  We record the 'follow' set so that the
			// main parser knows what symbols it can and cannot safely consume.
			oldCustomFollowSet = parser->customFollowSet;
			parser->customFollowSet = ParserSyntaxTable_GetFollowSet(parser->currentScope->syntaxTable, node);
			nestedSyntaxResult = Parser_RecursivelyApplyCustomSyntax(parser, &localExpr, modeFlags, node->name, parseError);
			parser->customFollowSet = oldCustomFollowSet;

			// Handle the result.
			switch (nestedSyntaxResult) {
				case CustomSyntaxResult_NotMatchedAndNoTokensConsumed:
					*parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, position,
						String_Format("Syntax error: Missing a '%S' in '%S'",
							SymbolTable_GetName(Smile_SymbolTable, node->name),
							SymbolTable_GetName(Smile_SymbolTable, syntaxClassSymbol)));
					return CustomSyntaxResult_PartialApplicationWithError;

				case CustomSyntaxResult_PartialApplicationWithError:
					return CustomSyntaxResult_PartialApplicationWithError;
			}

			// Append localExpr to the list of expressions generated by this syntax pattern.
			LIST_APPEND(localHead, localTail, localExpr);
		}

		isFirst = False;

		// Continue until we reach an accept node or bail due to a syntax error.
		syntaxRootMode = SYNTAXROOT_ASIS;
	}

	if (node->replacement != NullObject) {

		// We're done, and have valid expressions for each of the nonterminals.  We now need
		// to use the expressions and the replacement form and generate the parsed output.
		*expr = Parser_Accept(node->replacement, node->replacementVariables, node->numReplacementVariables, localHead);
		*parseError = NULL;

		return CustomSyntaxResult_SuccessfullyParsed;
	}
	else if (isFirst) {
		// No match, but we haven't consumed anything, so maybe we don't need to match.
		*parseError = NULL;
		return CustomSyntaxResult_NotMatchedAndNoTokensConsumed;
	}
	else {
		// No match, and we've traversed at least one node of the tree.  So the
		// input is an error, and we begin error recovery.
		*parseError = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(parser->lexer->token),
			String_Format("Syntax error in %S", SymbolTable_GetName(Smile_SymbolTable, syntaxClassSymbol)));
		return CustomSyntaxResult_PartialApplicationWithError;
	}
}
