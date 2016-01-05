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

#include <smile/env/symboltable.h>

//-------------------------------------------------------------------------------------------------
//  Private helper functions

/// <summary>
/// Add a known-new symbol to the symbol table.
/// </summary>
/// <param name="symbolTable">The symbol table to which the new symbol should be added.</param>
/// <param name="name">The name of the new symbol, which must not already exist in the symbol table.</param>
/// <returns>The new symbol.</returns>
static Symbol SymbolTableInt_AddSymbol(SymbolTable symbolTable, String name)
{
	struct SymbolTableInt *table = (struct SymbolTableInt *)symbolTable;
	Symbol symbol;
	String *newNames;
	Int newMax;

	// If we've run out of space for symbols, make more.
	if (table->count >= table->max) {
		newMax = table->max * 2;
		newNames = GC_MALLOC_STRUCT_ARRAY(String, newMax);
		MemCpy(newNames, table->symbolNames, sizeof(String) * table->count);
		table->symbolNames = newNames;
		table->max = newMax;
	}

	// Add the symbol to the table, and return it.
	table->symbolNames[symbol = table->count++] = name;
	StringIntDict_Add(table->symbolLookup, name, symbol);

	return symbol;
}

//-------------------------------------------------------------------------------------------------
//  Public API

/// <summary>
/// Create a new, empty symbol table.
/// </summary>
/// <returns>The new, empty symbol table.</returns>
SymbolTable SymbolTable_Create(void)
{
	struct SymbolTableInt *table;

	table = GC_MALLOC_STRUCT(struct SymbolTableInt);
	if (table == NULL) Smile_Abort_OutOfMemory();

	table->symbolNames = GC_MALLOC_STRUCT_ARRAY(String, 1024);
	if (table->symbolNames == NULL) Smile_Abort_OutOfMemory();

	// Symbol zero is always preallocated as the empty string.
	table->symbolNames[0] = String_Empty;

	table->count = 1;
	table->max = 1024;

	table->symbolLookup = StringIntDict_CreateWithSize(1024);

	return (SymbolTable)table;
}

/// <summary>
/// Find or create in the given symbol table a symbol that matches the given name.
/// </summary>
/// <param name="symbolTable">The symbol table in which the symbol will exist.</param>
/// <param name="name">The name of the symbol to find or create.</param>
/// <returns>The symbol, a preexisting value if it already was found in the symbol table,
/// or a new value if it did not already exist in the table.</returns>
Symbol SymbolTable_GetSymbol(SymbolTable symbolTable, String name)
{
	Symbol symbol;
	if ((symbol = (Symbol)StringIntDict_GetValue(((struct SymbolTableInt *)symbolTable)->symbolLookup, name)) == 0)
		return SymbolTableInt_AddSymbol(symbolTable, name);

	return symbol;
}

/// <summary>
/// Find or create in the given symbol table a symbol that matches the given name.
/// </summary>
/// <param name="symbolTable">The symbol table in which the symbol will exist.</param>
/// <param name="name">The name of the symbol to find or create, expressed as a C nul-terminated string.</param>
/// <returns>The symbol, a preexisting value if it already was found in the symbol table,
/// or a new value if it did not already exist in the table.</returns>
Symbol SymbolTable_GetSymbolC(SymbolTable symbolTable, const char *name)
{
	Symbol symbol;
	if ((symbol = (Symbol)StringIntDict_GetValueC(((struct SymbolTableInt *)symbolTable)->symbolLookup, name)) == 0)
		return SymbolTableInt_AddSymbol(symbolTable, String_FromC(name));

	return symbol;
}

/// <summary>
/// Find in the given symbol table a symbol that matches the given name.
/// </summary>
/// <param name="symbolTable">The symbol table in which the symbol will exist.</param>
/// <param name="name">The name of the symbol to find.</param>
/// <returns>The symbol, if it was found in the symbol table, or zero if it was not found.</returns>
Symbol SymbolTable_GetSymbolNoCreate(SymbolTable symbolTable, String name)
{
	return (Symbol)StringIntDict_GetValue(((struct SymbolTableInt *)symbolTable)->symbolLookup, name);
}

/// <summary>
/// Find in the given symbol table a symbol that matches the given name.
/// </summary>
/// <param name="symbolTable">The symbol table in which the symbol will exist.</param>
/// <param name="name">The name of the symbol to find, expressed as a C nul-terminated string.</param>
/// <returns>The symbol, if it was found in the symbol table, or zero if it was not found.</returns>
Symbol SymbolTable_GetSymbolNoCreateC(SymbolTable symbolTable, const char *name)
{
	return (Symbol)StringIntDict_GetValueC(((struct SymbolTableInt *)symbolTable)->symbolLookup, name);
}

/// <summary>
/// Look up the name of the given symbol.
/// </summary>
/// <param name="symbolTable">The symbol table in which the symbol will exist.</param>
/// <param name="symbol">The symbol whose name you would like to retrieve.</param>
/// <returns>The name, if this is a valid symbol in this table, or NULL if this is not a valid symbol
/// or does not exist in this table.</returns>
String SymbolTable_GetName(SymbolTable symbolTable, Symbol symbol)
{
	struct SymbolTableInt *table = (struct SymbolTableInt *)symbolTable;

	if ((Int)symbol <= 0 || (Int)symbol >= table->count)
		return NULL;

	return table->symbolNames[(Int)symbol];
}
