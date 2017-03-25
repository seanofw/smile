//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

#include "../../stdafx.h"

#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parsesyntax.h>

#include "testhelpers.h"

TEST_SUITE(ParserSyntaxTableTests)

STATIC_STRING(TestFilename, "test.sm");
STATIC_STRING(SemicolonString, ";");
STATIC_STRING(SpaceString, " ");

//-------------------------------------------------------------------------------------------------
//  Helpers.

static ParserSyntaxClass GetSyntaxClassSafely(ParserSyntaxTable table, const char *nonterminal)
{
	ParserSyntaxClass syntaxClass;

	Symbol nonterminalSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, nonterminal);

	if (table == NULL) return NULL;
	if (table->syntaxClasses == NULL) return NULL;
	if (!Int32Dict_TryGetValue(table->syntaxClasses, nonterminalSymbol, &syntaxClass)) return NULL;
	return syntaxClass;
}

static ParserSyntaxNode WalkSyntaxPattern(ParserSyntaxClass cls, const char *expected)
{
	String *terms, *pieces;
	String term;
	Int numTerms, numPieces, i;
	Symbol symbol;
	const char *text;
	ParserSyntaxNode node, nextNode;
	Int32Dict nextDict;

	numTerms = String_Split(String_FromC(expected), SemicolonString, &terms);

	node = (ParserSyntaxNode)cls;

	for (i = 0; i < numTerms; i++) {

		// Move forward to the next node.
		{
			term = String_Trim(terms[i]);
			numPieces = String_Split(term, SpaceString, &pieces);

			symbol = numPieces > 0 ? SymbolTable_GetSymbol(Smile_SymbolTable, String_Trim(pieces[0])) : 0;

			nextDict = numPieces > 1 ? node->nextNonterminals : node->nextTerminals;
			if (nextDict == NULL) return NULL;

			if (!Int32Dict_TryGetValue(nextDict, symbol, &nextNode)) return NULL;

			node = nextNode;
		}

		// Test all of the values at this node against the expected string.
		{
			if (node->name != symbol) return NULL;

			symbol = numPieces > 1 ? SymbolTable_GetSymbol(Smile_SymbolTable, String_Trim(pieces[1])) : 0;
			if (node->variable != symbol) return NULL;

			text = numPieces > 2 ? String_ToC(String_Trim(pieces[2])) : "";
			if (node->repetitionKind != text[0]) return NULL;

			text = numPieces > 3 ? String_ToC(String_Trim(pieces[3])) : "";
			if (node->repetitionSep != text[0]) return NULL;
		}
	}

	return node;
}

//-------------------------------------------------------------------------------------------------
//  Syntax-table construction tests.

START_TEST(CanMakeAnEmptySyntaxTable)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();

	ASSERT(syntaxTable != NULL);

	ASSERT(syntaxTable->referenceCount == 1);
	ASSERT(syntaxTable->syntaxClasses != NULL);

	ASSERT(syntaxTable->addExprClass == NULL);
	ASSERT(syntaxTable->binaryExprClass == NULL);
	ASSERT(syntaxTable->cmpExprClass == NULL);
	ASSERT(syntaxTable->exprClass == NULL);
	ASSERT(syntaxTable->mulExprClass == NULL);
	ASSERT(syntaxTable->postfixExprClass == NULL);
	ASSERT(syntaxTable->stmtClass == NULL);
	ASSERT(syntaxTable->termClass == NULL);
	ASSERT(syntaxTable->prefixExprClass == NULL);
}
END_TEST

START_TEST(CanAddASimpleRuleToASyntaxTable)
{
	ParserSyntaxNode node;
	ParserSyntaxClass cls;
	SmileObject obj;

	SmileObject parsedCode = FullParse("#syntax FOO-BAR: [foo] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	cls = (ParserSyntaxClass)(Int32Dict_GetFirst(syntaxTable->syntaxClasses).value);
	ASSERT(cls != NULL);
	ASSERT(cls->referenceCount == 1);
	ASSERT(cls->nextNonterminals == NULL);
	ASSERT(cls->nextTerminals != NULL);
	ASSERT(Int32Dict_Count(cls->nextTerminals) == 1);

	node = (ParserSyntaxNode)(Int32Dict_GetFirst(cls->nextTerminals).value);
	ASSERT(node != NULL);
	ASSERT(node->referenceCount == 1);
	ASSERT(node->name == SymbolTable_GetSymbolNoCreateC(Smile_SymbolTable, "foo"));
	ASSERT(node->variable == 0);
	ASSERT(node->repetitionKind == 0);
	ASSERT(node->repetitionSep == 0);
	ASSERT(node->nextNonterminals == NULL);
	ASSERT(node->nextTerminals == NULL);

	obj = SimpleParse("123");
	ASSERT(RecursiveEquals(obj, node->replacement));
}
END_TEST

START_TEST(CanAddAComplexRuleToASyntaxTable)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	SmileObject parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => [\\if x y]");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	ASSERT(cls != NULL);

	finalNode = WalkSyntaxPattern(cls, "if; EXPR x; then; STMT y");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("[if x y]"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddOverlappingRulesToASyntaxTable)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	SmileObject parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => [\\if x y]");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [\\if x y z]");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	ASSERT(cls != NULL);

	finalNode = WalkSyntaxPattern(cls, "if; EXPR x; then; STMT y");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("[if x y]"), finalNode->replacement));

	finalNode = WalkSyntaxPattern(cls, "if; EXPR x; then; STMT y; else; STMT z");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("[if x y z]"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithTerminalForksToASyntaxTable)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	SmileObject parsedCode = FullParse("#syntax STMT: [do the first [EXPR x] with [EXPR y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	parsedCode = FullParse("#syntax STMT: [do the last [EXPR x] then stop] => 456");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	parsedCode = FullParse("#syntax STMT: [do the last [EXPR x] then repeat] => 789");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	ASSERT(cls != NULL);

	finalNode = WalkSyntaxPattern(cls, "do; the; first; EXPR x; with; EXPR y");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));

	finalNode = WalkSyntaxPattern(cls, "do; the; last; EXPR x; then; stop");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("456"), finalNode->replacement));

	finalNode = WalkSyntaxPattern(cls, "do; the; last; EXPR x; then; repeat");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("789"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithInitialNonterminals)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileObject parsedCode = FullParse("#syntax MY-ADD: [[EXPR x] + [EXPR y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "MY-ADD");

	finalNode = WalkSyntaxPattern(cls, "EXPR x; +; EXPR y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithRepeatingNonterminals)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileObject parsedCode = FullParse("#syntax STMT: [till [NAME+ x] do [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");

	finalNode = WalkSyntaxPattern(cls, "till; NAME x +; do; STMT y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithRepeatingNonterminalsAndSeparators)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileObject parsedCode = FullParse("#syntax STMT: [till [NAME+ x,] do [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");

	finalNode = WalkSyntaxPattern(cls, "till; NAME x + ,; do; STMT y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithRepeatingZeroOrMoreNonterminals)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileObject parsedCode = FullParse("#syntax STMT: [till [NAME* x] do [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");

	finalNode = WalkSyntaxPattern(cls, "till; NAME x *; do; STMT y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithRepeatingZeroOrOneNonterminals)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileObject parsedCode = FullParse("#syntax STMT: [till [NAME? x] do [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");

	finalNode = WalkSyntaxPattern(cls, "till; NAME x ?; do; STMT y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

// This should go without saying, but since it's a common real-world scenario, we test it.
START_TEST(CanAddRulesThatAreRightRecursive)
{
	ParserSyntaxNode finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileObject parsedCode = FullParse("#syntax MY-ADD: [[MY-MUL x] + [MY-ADD y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax MY-ADD: [[MY-MUL x]] => 456");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax MY-MUL: [[TERM x] + [MY-MUL y]] => 789");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax MY-MUL: [[TERM x]] => \"abc\"");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "MY-ADD");

	finalNode = WalkSyntaxPattern(cls, "MY-MUL x");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("456"), finalNode->replacement));

	finalNode = WalkSyntaxPattern((ParserSyntaxClass)finalNode, "+; MY-ADD y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));

	cls = GetSyntaxClassSafely(syntaxTable, "MY-MUL");

	finalNode = WalkSyntaxPattern(cls, "TERM x");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("\"abc\""), finalNode->replacement));

	finalNode = WalkSyntaxPattern((ParserSyntaxClass)finalNode, "+; MY-MUL y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("789"), finalNode->replacement));
}
END_TEST

START_TEST(CannotAddRulesWithNullReplacements)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileObject parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => []");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesThatAreDuplicatesExceptForTheReplacement)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileObject parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 456");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesThatAreDuplicates)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileObject parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 123");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesWhoseInitialNonterminalsRepeatZeroOrMoreTimes)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileObject parsedCode = FullParse("#syntax MY-LIST: [[ITEM+ x]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax YOUR-LIST: [[ITEM* x]] => 456");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesWhoseInitialNonterminalsRepeatZeroOrOneTimes)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileObject parsedCode = FullParse("#syntax MY-LIST: [[ITEM x]] => 123");
	SmileSyntax rule = (SmileSyntax)parsedCode;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax YOUR-LIST: [[ITEM? x]] => 456");
	rule = (SmileSyntax)parsedCode;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

#include "parsersyntax_table_tests.generated.inc"
