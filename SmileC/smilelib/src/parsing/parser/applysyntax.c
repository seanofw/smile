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
		case SMILE_SPECIAL_SYMBOL_CMP:
			return syntaxTable->cmpClass;
		case SMILE_SPECIAL_SYMBOL_ADDSUB:
			return syntaxTable->addSubClass;
		case SMILE_SPECIAL_SYMBOL_MULDIV:
			return syntaxTable->mulDivClass;
		case SMILE_SPECIAL_SYMBOL_BINARY:
			return syntaxTable->binaryClass;
		case SMILE_SPECIAL_SYMBOL_UNARY:
			return syntaxTable->unaryClass;
		case SMILE_SPECIAL_SYMBOL_POSTFIX:
			return syntaxTable->postfixClass;
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

CustomSyntaxResult Parser_ApplyCustomSyntax(Parser parser, SmileObject *expr, Int modeFlags, Symbol syntaxClassSymbol, ParseError *parseError)
{
	ParserSyntaxClass syntaxClass;
	ParserSyntaxNode node, nextNode;
	LexerPosition position;
	SmileObject localExpr;
	SmileList localHead, localTail;
	Bool isFirst;

	// Get the class that contains all the rules under the provided nonterminal symbol.
	syntaxClass = GetSyntaxClass(parser, syntaxClassSymbol);
	if (syntaxClass == NULL)
		return CustomSyntaxResult_NotMatchedAndNoTokensConsumed;

	// Begin walking the tree nodes of the class, consuming input tokens where they match.
	node = (ParserSyntaxNode)syntaxClass;
	isFirst = True;

	// This is the list where we'll be collecting the nonterminal matches as we find them.
	localTail = localHead = NullList;

	do {
		// Try to match the next item (terminal or nonterminal) in the pattern.

		if (node->isNonterminal) {
			// The next node is a nonterminal, so recursively invoke it immediately.

			// Collect the current position, for error-reporting.
			Lexer_Peek(parser->lexer);
			position = Token_GetPosition(parser->lexer->token);

			// Get the nonterminal node.
			nextNode = (ParserSyntaxNode)(Int32Dict_GetFirst(node->nextDict).value);
		
			// Recursively traverse the syntax tree and handle the result.
			switch (Parser_ApplyCustomSyntax(parser, &localExpr, modeFlags, nextNode->name, parseError)) {
				case CustomSyntaxResult_NotMatchedAndNoTokensConsumed:
					Parser_AddError(parser, position, "Syntax error: Missing a %S in %S",
						SymbolTable_GetName(Smile_SymbolTable, nextNode->name),
						SymbolTable_GetName(Smile_SymbolTable, syntaxClassSymbol));
					return CustomSyntaxResult_PartialApplicationWithError;

				case CustomSyntaxResult_PartialApplicationWithError:
					return CustomSyntaxResult_PartialApplicationWithError;
			}

			// Append localExpr to the list of expressions generated by this syntax pattern.
			LIST_APPEND(localHead, localTail, localExpr);
		}
		else {
			// The next node contains a set of possible terminals, so match one of them to the next incoming token.
			Lexer_Next(parser->lexer);
			if ((nextNode = Parser_MatchCustomTerminal(parser->lexer->token, node->nextDict)) != NULL) {
			terminalMatched:
				// Match, so move forward.
				node = nextNode;
			}
			else if (isFirst) {
				// First position, and nothing matched, so we didn't even start to consume this rule.
				Lexer_Unget(parser->lexer);
				return CustomSyntaxResult_NotMatchedAndNoTokensConsumed;
			}
			else {
				// No match, and we've traversed at least one node of the tree.  So the
				// input is an error, and we begin error recovery.
				Parser_AddError(parser, Token_GetPosition(parser->lexer->token), "Syntax error in %S",
					SymbolTable_GetName(Smile_SymbolTable, syntaxClassSymbol));
				return CustomSyntaxResult_PartialApplicationWithError;
			}
		}

		isFirst = False;

		// Continue until we reach an accept node or bail due to a syntax error.
	} while (node->replacement == NullObject);

	// When we reach an accept node, see if a subsequent terminal node *could* match the next
	// token, and if so, we consume that token and keep going; otherwise, we accept.  This
	// rule effectively resolves all shift/reduce conflicts in favor of shift.
	Lexer_Next(parser->lexer);
	if ((nextNode = Parser_MatchCustomTerminal(parser->lexer->token, node->nextDict)) != NULL) {
		// Hey, we got a match, so back up and keep going!
		goto terminalMatched;
	}
	Lexer_Unget(parser->lexer);

	// We're done, and have valid expressions for each of the nonterminals.  We now need
	// to use the expressions and the replacement form and generate the parsed output.
	*expr = Parser_Accept(node->replacement, node->replacementVariables, node->numReplacementVariables, localHead);

	return CustomSyntaxResult_SuccessfullyParsed;
}
