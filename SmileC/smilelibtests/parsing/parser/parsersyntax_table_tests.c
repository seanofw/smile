//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

static ParserSyntaxNode GetRootNodeSafely(ParserSyntaxClass cls, const char *term)
{
	ParserSyntaxNode syntaxNode;

	Symbol termSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, term);

	if (cls == NULL) return NULL;
	if (cls->nextDict == NULL) return NULL;
	if (!Int32Dict_TryGetValue(cls->nextDict, termSymbol, &syntaxNode)) return NULL;
	return syntaxNode;
}

static ParserSyntaxNode WalkSyntaxPattern(ParserSyntaxNode node, const char *expected)
{
	String *terms, *pieces;
	String term;
	Int numTerms, numPieces, i;
	Symbol symbol;
	const char *text;
	ParserSyntaxNode nextNode;

	numTerms = String_Split(String_FromC(expected), SemicolonString, &terms);

	term = String_Trim(terms[i = 0]);
	numPieces = String_Split(term, SpaceString, &pieces);

	symbol = numPieces > 0 ? SymbolTable_GetSymbol(Smile_SymbolTable, String_Trim(pieces[0])) : 0;

	while (True) {
	
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
	
		// If this is the last node, then we're done.
		if (i + 1 >= numTerms) break;

		// Move forward to the next node.
		{
			term = String_Trim(terms[i + 1]);
			numPieces = String_Split(term, SpaceString, &pieces);

			symbol = numPieces > 0 ? SymbolTable_GetSymbol(Smile_SymbolTable, String_Trim(pieces[0])) : 0;

			if (node->nextDict == NULL) return NULL;

			if (!Int32Dict_TryGetValue(node->nextDict, symbol, &nextNode)) return NULL;

			node = nextNode;
			i++;
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

	ASSERT(syntaxTable->addSubClass == NULL);
	ASSERT(syntaxTable->binaryClass == NULL);
	ASSERT(syntaxTable->cmpClass == NULL);
	ASSERT(syntaxTable->exprClass == NULL);
	ASSERT(syntaxTable->mulDivClass == NULL);
	ASSERT(syntaxTable->postfixClass == NULL);
	ASSERT(syntaxTable->stmtClass == NULL);
	ASSERT(syntaxTable->termClass == NULL);
	ASSERT(syntaxTable->unaryClass == NULL);
}
END_TEST

START_TEST(CanAddASimpleRuleToASyntaxTable)
{
	ParserSyntaxNode node;
	ParserSyntaxClass cls;
	SmileObject obj;

	SmileList parsedCode = FullParse("#syntax FOO: [foo] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
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
	ASSERT(cls->isNonterminal == False);
	ASSERT(cls->nextDict != NULL);
	ASSERT(Int32Dict_Count(cls->nextDict) == 1);

	node = (ParserSyntaxNode)(Int32Dict_GetFirst(cls->nextDict).value);
	ASSERT(node != NULL);
	ASSERT(node->referenceCount == 1);
	ASSERT(node->name == SymbolTable_GetSymbolNoCreateC(Smile_SymbolTable, "foo"));
	ASSERT(node->variable == 0);
	ASSERT(node->repetitionKind == 0);
	ASSERT(node->repetitionSep == 0);
	ASSERT(node->isNonterminal == False);
	ASSERT(node->nextDict == NULL);

	obj = SimpleParse("123");
	ASSERT(RecursiveEquals(obj, node->replacement));
}
END_TEST

START_TEST(CanAddAComplexRuleToASyntaxTable)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	SmileList parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => [\\if x y]");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Parser parser = Parser_Create();

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	node = GetRootNodeSafely(cls, "if");

	ASSERT(node != NULL);

	finalNode = WalkSyntaxPattern(node, "if; EXPR x; then; STMT y");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("[if x y]"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddOverlappingRulesToASyntaxTable)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	SmileList parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => [\\if x y]");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Parser parser = Parser_Create();

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [\\if x y z]");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	node = GetRootNodeSafely(cls, "if");

	ASSERT(node != NULL);

	finalNode = WalkSyntaxPattern(node, "if; EXPR x; then; STMT y");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("[if x y]"), finalNode->replacement));

	finalNode = WalkSyntaxPattern(node, "if; EXPR x; then; STMT y; else; STMT z");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("[if x y z]"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithTerminalForksToASyntaxTable)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	SmileList parsedCode = FullParse("#syntax STMT: [do the first [EXPR x] with [EXPR y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Parser parser = Parser_Create();

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	parsedCode = FullParse("#syntax STMT: [do the last [EXPR x] then stop] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	parsedCode = FullParse("#syntax STMT: [do the last [EXPR x] then repeat] => 789");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);

	ASSERT(result == True);
	ASSERT(resultSyntaxTable == syntaxTable);
	ASSERT(Int32Dict_Count(syntaxTable->syntaxClasses) == 1);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	node = GetRootNodeSafely(cls, "do");

	ASSERT(node != NULL);

	finalNode = WalkSyntaxPattern(node, "do; the; first; EXPR x; with; EXPR y");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));

	finalNode = WalkSyntaxPattern(node, "do; the; last; EXPR x; then; stop");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("456"), finalNode->replacement));

	finalNode = WalkSyntaxPattern(node, "do; the; last; EXPR x; then; repeat");

	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);

	ASSERT(RecursiveEquals(SimpleParse("789"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithInitialNonterminals)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileList parsedCode = FullParse("#syntax MYADD: [[EXPR x] + [EXPR y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "MYADD");
	node = GetRootNodeSafely(cls, "EXPR");

	finalNode = WalkSyntaxPattern(node, "EXPR x; +; EXPR y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithRepeatingNonterminals)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileList parsedCode = FullParse("#syntax STMT: [till [NAME+ x] do [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	node = GetRootNodeSafely(cls, "till");

	finalNode = WalkSyntaxPattern(node, "till; NAME x +; do; STMT y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithRepeatingNonterminalsAndSeparators)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileList parsedCode = FullParse("#syntax STMT: [till [NAME+ x,] do [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	node = GetRootNodeSafely(cls, "till");

	finalNode = WalkSyntaxPattern(node, "till; NAME x + ,; do; STMT y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithRepeatingZeroOrMoreNonterminals)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileList parsedCode = FullParse("#syntax STMT: [till [NAME* x] do [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	node = GetRootNodeSafely(cls, "till");

	finalNode = WalkSyntaxPattern(node, "till; NAME x *; do; STMT y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

START_TEST(CanAddRulesWithRepeatingZeroOrOneNonterminals)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;

	SmileList parsedCode = FullParse("#syntax STMT: [till [NAME? x] do [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Parser parser = Parser_Create();

	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "STMT");
	node = GetRootNodeSafely(cls, "till");

	finalNode = WalkSyntaxPattern(node, "till; NAME x ?; do; STMT y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));
}
END_TEST

// This should go without saying, but since it's a common real-world scenario, we test it.
START_TEST(CanAddRulesThatAreRightRecursive)
{
	ParserSyntaxNode node, finalNode;
	ParserSyntaxClass cls;

	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileList parsedCode = FullParse("#syntax MYADD: [[MYMUL x] + [MYADD y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax MYADD: [[MYMUL x]] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax MYMUL: [[TERM x] + [MYMUL y]] => 789");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax MYMUL: [[TERM x]] => \"abc\"");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	cls = GetSyntaxClassSafely(syntaxTable, "MYADD");
	node = GetRootNodeSafely(cls, "MYMUL");

	finalNode = WalkSyntaxPattern(node, "MYMUL x");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("456"), finalNode->replacement));

	finalNode = WalkSyntaxPattern(finalNode, "MYMUL x; +; MYADD y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("123"), finalNode->replacement));

	cls = GetSyntaxClassSafely(syntaxTable, "MYMUL");
	node = GetRootNodeSafely(cls, "TERM");

	finalNode = WalkSyntaxPattern(node, "TERM x");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("\"abc\""), finalNode->replacement));

	finalNode = WalkSyntaxPattern(finalNode, "TERM x; +; MYMUL y");
	ASSERT(finalNode != NULL);
	ASSERT(finalNode->replacement != NullObject);
	ASSERT(RecursiveEquals(SimpleParse("789"), finalNode->replacement));
}
END_TEST

START_TEST(CannotAddRulesWithInitialNonterminalForks)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileList parsedCode = FullParse("#syntax FOO: [[EXPR x] + [EXPR y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax FOO: [[STMT x] and [STMT y]] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesWithMiddleNonterminalForks)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileList parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax STMT: [if [NAME z] abort] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesWithFinalNonterminalForks)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileList parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [NAME z]] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesWithNullReplacements)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileList parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => []");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
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

	SmileList parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
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

	SmileList parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax STMT: [if [EXPR x] then [STMT y]] => 123");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
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

	SmileList parsedCode = FullParse("#syntax MY_LIST: [[ITEM+ x]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax YOUR_LIST: [[ITEM* x]] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
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

	SmileList parsedCode = FullParse("#syntax MY_LIST: [[ITEM x]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax YOUR_LIST: [[ITEM? x]] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesWhoseInitialNonterminalsAreCrossReferential)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileList parsedCode = FullParse("#syntax ADD: [[MUL x] + [TERM y]] => 123");
	SmileSyntax rule = (SmileSyntax)((SmileList)parsedCode)->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	parsedCode = FullParse("#syntax MUL: [[ADD x] * [TERM y]] => 456");
	rule = (SmileSyntax)((SmileList)parsedCode)->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotAddRulesWhoseInitialNonterminalsAreIndirectlyCrossReferential)
{
	ParserSyntaxTable syntaxTable = ParserSyntaxTable_CreateNew();
	ParserSyntaxTable resultSyntaxTable = syntaxTable;
	Parser parser = Parser_Create();

	SmileSyntax rule = (SmileSyntax)((SmileList)FullParse("#syntax ADD: [[MUL x] + [TERM y]] => 123"))->a;
	Bool result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	rule = (SmileSyntax)((SmileList)FullParse("#syntax TERM: [[EXPR x]] => 789"))->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	rule = (SmileSyntax)((SmileList)FullParse("#syntax MUL: [[TERM x] + [TERM y]] => 456"))->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == True);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	rule = (SmileSyntax)((SmileList)FullParse("#syntax EXPR: [[ADD x]] => 555"))->a;
	result = ParserSyntaxTable_AddRule(parser, &resultSyntaxTable, rule);
	ASSERT(result == False);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

#include "parsersyntax_table_tests.generated.inc"
