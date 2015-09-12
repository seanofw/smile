//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2015 Sean Werkema
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

#include "../stdafx.h"

TEST_SUITE(SymbolTableTests)

START_TEST(CanCreateAnEmptySymbolTable)
{
	SymbolTable symbolTable;
	symbolTable = SymbolTable_Create();

	ASSERT(symbolTable != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 1);
	ASSERT(((struct SymbolTableInt *)symbolTable)->max > 100);
	ASSERT(((struct SymbolTableInt *)symbolTable)->symbolLookup != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->symbolNames != NULL);
}
END_TEST

START_TEST(CanAddSymbolsToASymbolTable)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 1);

	a = SymbolTable_GetSymbol(symbolTable, String_FromC("soup"));
	b = SymbolTable_GetSymbol(symbolTable, String_FromC("nuts"));
	c = SymbolTable_GetSymbol(symbolTable, String_FromC("raspberry"));
	d = SymbolTable_GetSymbol(symbolTable, String_FromC("banana"));
	e = SymbolTable_GetSymbol(symbolTable, String_FromC("hamburger"));
	f = SymbolTable_GetSymbol(symbolTable, String_FromC("milkshake"));

	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 7);
	ASSERT(a == 1);
	ASSERT(b == 2);
	ASSERT(c == 3);
	ASSERT(d == 4);
	ASSERT(e == 5);
	ASSERT(f == 6);
}
END_TEST

START_TEST(CanAddSymbolsToASymbolTableC)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 1);

	a = SymbolTable_GetSymbolC(symbolTable, "soup");
	b = SymbolTable_GetSymbolC(symbolTable, "nuts");
	c = SymbolTable_GetSymbolC(symbolTable, "raspberry");
	d = SymbolTable_GetSymbolC(symbolTable, "banana");
	e = SymbolTable_GetSymbolC(symbolTable, "hamburger");
	f = SymbolTable_GetSymbolC(symbolTable, "milkshake");

	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 7);
	ASSERT(a == 1);
	ASSERT(b == 2);
	ASSERT(c == 3);
	ASSERT(d == 4);
	ASSERT(e == 5);
	ASSERT(f == 6);
}
END_TEST

START_TEST(CanAddSymbolsFastToAnEmptySymbolTable)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 1);

	a = SymbolTableInt_AddFast(symbolTable, String_FromC("soup"));
	b = SymbolTableInt_AddFast(symbolTable, String_FromC("nuts"));
	c = SymbolTableInt_AddFast(symbolTable, String_FromC("raspberry"));
	d = SymbolTableInt_AddFast(symbolTable, String_FromC("banana"));
	e = SymbolTableInt_AddFast(symbolTable, String_FromC("hamburger"));
	f = SymbolTableInt_AddFast(symbolTable, String_FromC("milkshake"));

	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 7);
	ASSERT(a == 1);
	ASSERT(b == 2);
	ASSERT(c == 3);
	ASSERT(d == 4);
	ASSERT(e == 5);
	ASSERT(f == 6);

	ASSERT_STRING(SymbolTable_GetName(symbolTable, a), "soup", 4);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, b), "nuts", 4);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, c), "raspberry", 9);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, d), "banana", 6);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, e), "hamburger", 9);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, f), "milkshake", 9);
}
END_TEST

START_TEST(ReAddingSymbolsDoesNotChangeASymbolTable)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f, g;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 1);

	a = SymbolTable_GetSymbol(symbolTable, String_FromC("soup"));
	b = SymbolTable_GetSymbol(symbolTable, String_FromC("nuts"));
	c = SymbolTable_GetSymbol(symbolTable, String_FromC("raspberry"));
	d = SymbolTable_GetSymbol(symbolTable, String_FromC("banana"));
	e = SymbolTable_GetSymbol(symbolTable, String_FromC("hamburger"));
	f = SymbolTable_GetSymbol(symbolTable, String_FromC("milkshake"));

	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 7);
	ASSERT(a == 1);
	ASSERT(b == 2);
	ASSERT(c == 3);
	ASSERT(d == 4);
	ASSERT(e == 5);
	ASSERT(f == 6);

	a = SymbolTable_GetSymbol(symbolTable, String_FromC("soup"));
	b = SymbolTable_GetSymbol(symbolTable, String_FromC("nuts"));
	c = SymbolTable_GetSymbol(symbolTable, String_FromC("raspberry"));
	d = SymbolTable_GetSymbol(symbolTable, String_FromC("banana"));
	e = SymbolTable_GetSymbol(symbolTable, String_FromC("hamburger"));
	f = SymbolTable_GetSymbol(symbolTable, String_FromC("milkshake"));

	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 7);
	ASSERT(a == 1);
	ASSERT(b == 2);
	ASSERT(c == 3);
	ASSERT(d == 4);
	ASSERT(e == 5);
	ASSERT(f == 6);

	g = SymbolTable_GetSymbol(symbolTable, String_FromC("pomegranate"));
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 8);
	ASSERT(g == 7);

	d = SymbolTable_GetSymbol(symbolTable, String_FromC("banana"));
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 8);
	ASSERT(d == 4);
}
END_TEST

START_TEST(ReAddingSymbolsDoesNotChangeASymbolTableC)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f, g;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 1);

	a = SymbolTable_GetSymbolC(symbolTable, "soup");
	b = SymbolTable_GetSymbolC(symbolTable, "nuts");
	c = SymbolTable_GetSymbolC(symbolTable, "raspberry");
	d = SymbolTable_GetSymbolC(symbolTable, "banana");
	e = SymbolTable_GetSymbolC(symbolTable, "hamburger");
	f = SymbolTable_GetSymbolC(symbolTable, "milkshake");

	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 7);
	ASSERT(a == 1);
	ASSERT(b == 2);
	ASSERT(c == 3);
	ASSERT(d == 4);
	ASSERT(e == 5);
	ASSERT(f == 6);

	a = SymbolTable_GetSymbolC(symbolTable, "soup");
	b = SymbolTable_GetSymbolC(symbolTable, "nuts");
	c = SymbolTable_GetSymbolC(symbolTable, "raspberry");
	d = SymbolTable_GetSymbolC(symbolTable, "banana");
	e = SymbolTable_GetSymbolC(symbolTable, "hamburger");
	f = SymbolTable_GetSymbolC(symbolTable, "milkshake");

	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 7);
	ASSERT(a == 1);
	ASSERT(b == 2);
	ASSERT(c == 3);
	ASSERT(d == 4);
	ASSERT(e == 5);
	ASSERT(f == 6);

	g = SymbolTable_GetSymbolC(symbolTable, "pomegranate");
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 8);
	ASSERT(g == 7);

	d = SymbolTable_GetSymbolC(symbolTable, "banana");
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 8);
	ASSERT(d == 4);
}
END_TEST

START_TEST(CanGetNamesFromSymbols)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);

	a = SymbolTable_GetSymbol(symbolTable, String_FromC("soup"));
	b = SymbolTable_GetSymbol(symbolTable, String_FromC("nuts"));
	c = SymbolTable_GetSymbol(symbolTable, String_FromC("raspberry"));
	d = SymbolTable_GetSymbol(symbolTable, String_FromC("banana"));
	e = SymbolTable_GetSymbol(symbolTable, String_FromC("hamburger"));
	f = SymbolTable_GetSymbol(symbolTable, String_FromC("milkshake"));

	ASSERT_STRING(SymbolTable_GetName(symbolTable, a), "soup", 4);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, b), "nuts", 4);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, c), "raspberry", 9);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, d), "banana", 6);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, e), "hamburger", 9);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, f), "milkshake", 9);
}
END_TEST

START_TEST(CanGetNamesFromSymbolsC)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);

	a = SymbolTable_GetSymbolC(symbolTable, "soup");
	b = SymbolTable_GetSymbolC(symbolTable, "nuts");
	c = SymbolTable_GetSymbolC(symbolTable, "raspberry");
	d = SymbolTable_GetSymbolC(symbolTable, "banana");
	e = SymbolTable_GetSymbolC(symbolTable, "hamburger");
	f = SymbolTable_GetSymbolC(symbolTable, "milkshake");

	ASSERT_STRING(SymbolTable_GetName(symbolTable, a), "soup", 4);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, b), "nuts", 4);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, c), "raspberry", 9);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, d), "banana", 6);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, e), "hamburger", 9);
	ASSERT_STRING(SymbolTable_GetName(symbolTable, f), "milkshake", 9);
}
END_TEST

START_TEST(CanExamineASymbolTableWithoutAlteringIt)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 1);

	a = SymbolTable_GetSymbol(symbolTable, String_FromC("soup"));
	b = SymbolTable_GetSymbol(symbolTable, String_FromC("nuts"));
	c = SymbolTable_GetSymbol(symbolTable, String_FromC("raspberry"));
	d = SymbolTable_GetSymbol(symbolTable, String_FromC("banana"));
	e = SymbolTable_GetSymbol(symbolTable, String_FromC("hamburger"));
	f = SymbolTable_GetSymbol(symbolTable, String_FromC("milkshake"));

	ASSERT(SymbolTable_GetSymbolNoCreate(symbolTable, String_FromC("banana")) == d);
	ASSERT(SymbolTable_GetSymbolNoCreate(symbolTable, String_FromC("nuts")) == b);

	ASSERT(SymbolTable_GetSymbolNoCreate(symbolTable, String_FromC("pomegranate")) == 0);
	ASSERT(SymbolTable_GetSymbolNoCreate(symbolTable, String_FromC("banan")) == 0);
}
END_TEST

START_TEST(CanExamineASymbolTableWithoutAlteringItC)
{
	SymbolTable symbolTable;
	Symbol a, b, c, d, e, f;

	symbolTable = SymbolTable_Create();
	ASSERT(symbolTable != NULL);
	ASSERT(((struct SymbolTableInt *)symbolTable)->count == 1);

	a = SymbolTable_GetSymbolC(symbolTable, "soup");
	b = SymbolTable_GetSymbolC(symbolTable, "nuts");
	c = SymbolTable_GetSymbolC(symbolTable, "raspberry");
	d = SymbolTable_GetSymbolC(symbolTable, "banana");
	e = SymbolTable_GetSymbolC(symbolTable, "hamburger");
	f = SymbolTable_GetSymbolC(symbolTable, "milkshake");

	ASSERT(SymbolTable_GetSymbolNoCreateC(symbolTable, "banana") == d);
	ASSERT(SymbolTable_GetSymbolNoCreateC(symbolTable, "nuts") == b);

	ASSERT(SymbolTable_GetSymbolNoCreateC(symbolTable, "pomegranate") == 0);
	ASSERT(SymbolTable_GetSymbolNoCreateC(symbolTable, "banan") == 0);
}
END_TEST

START_TEST(SymbolTablePerformanceTest)
{
	SymbolTable symbolTable;
	struct KnownSymbolsStruct knownSymbols;
	Int counter;

	for (counter = 0; counter < 1000; counter++) {
		symbolTable = SymbolTable_Create();
		KnownSymbols_PreloadSymbolTable(symbolTable, &knownSymbols);
	}
}
END_TEST

#include "symboltable_tests.generated.inc"
