#ifndef __SMILE_ENV_SYMBOLTABLE_H__
#define __SMILE_ENV_SYMBOLTABLE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_MEM_H__
#include <smile/mem.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_DICT_STRINGINTDICT_H__
#include <smile/dict/stringintdict.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Internal types

/// <summary>
/// The internal implementation of the Smile shared symbol table.
/// </summary>
struct SymbolTableInt {
	String *symbolNames;			// This is the set of known symbols, in registration order.
	Int count;						// The number of registered symbols.
	Int max;						// The current maximum size of the symbol arrays.
	StringIntDict symbolLookup;		// A lookup table for finding symbol IDs by their names, quickly.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

// A symbol is (currently) a magic integer that is not zero (zero is an invalid symbol ID).
typedef Int Symbol;

typedef struct SymbolTableStruct {
	struct SymbolTableInt _opaque;
} *SymbolTable;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation.

SMILE_API_FUNC SymbolTable SymbolTable_Create(void);
SMILE_API_FUNC Symbol SymbolTable_GetSymbol(SymbolTable symbolTable, String name);
SMILE_API_FUNC Symbol SymbolTable_GetSymbolC(SymbolTable symbolTable, const char *name);
SMILE_API_FUNC Symbol SymbolTable_GetSymbolNoCreate(SymbolTable symbolTable, String name);
SMILE_API_FUNC Symbol SymbolTable_GetSymbolNoCreateC(SymbolTable symbolTable, const char *name);
SMILE_API_FUNC String SymbolTable_GetName(SymbolTable symbolTable, Symbol symbol);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation.

/// <summary>
/// Add a known-new symbol to a known-large-enough symbol table.  This is unsafe in the general
/// case, but safe during the initialization of the symbol table initially.
/// </summary>
/// <param name="symbolTable">The symbol table to which the new symbol should be added.</param>
/// <param name="name">The name of the new symbol, which must not already exist in the symbol table.</param>
/// <returns>The new symbol.</returns>
Inline Symbol SymbolTableInt_AddFast(SymbolTable symbolTable, String name)
{
	struct SymbolTableInt *table = (struct SymbolTableInt *)symbolTable;
	Symbol symbol;

	// Add the symbol to the table, and return it.  This skips all the usual preexistence checks.
	table->symbolNames[symbol = table->count++] = name;
	StringIntDictInt_Append((struct StringIntDictInt *)(table->symbolLookup), name, String_Hash(name), symbol);

	return symbol;
}

#endif
