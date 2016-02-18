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

#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static void ParseScope_DeclareGlobalOperators(ParseScope parseScope);

/// <summary>
/// Create a root scope that contains only the eighteen or so global forms.
/// </summary>
/// <returns>The newly-created root scope.</returns>
ParseScope ParseScope_CreateRoot(void)
{
	ParseScope parseScope = GC_MALLOC_STRUCT(struct ParseScopeStruct);

	parseScope->kind = PARSESCOPE_OUTERMOST;
	parseScope->parentScope = NULL;
	parseScope->symbols = Int32Dict_Create();
	parseScope->firstDeclaration = Smile_KnownObjects.Null;
	parseScope->lastDeclaration = Smile_KnownObjects.Null;

	ParseScope_DeclareGlobalOperators(parseScope);

	return parseScope;
}

/// <summary>
/// Declare the eighteen-or-so global operators in the given (presumably root-level) scope.
/// </summary>
/// <param name="parseScope">The scope in which to declare the root-level operators.</param>
static void ParseScope_DeclareGlobalOperators(ParseScope parseScope)
{
	ParseScope_Declare(parseScope, Smile_KnownSymbols.equals_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.op_equals_, PARSEDECL_PRIMITIVE);

	ParseScope_Declare(parseScope, Smile_KnownSymbols.if_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.while_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.var_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.till_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.catch_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.fn_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.scope_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.progn_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.quote_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.new_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.is_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.typeof_, PARSEDECL_PRIMITIVE);
}

/// <summary>
/// Create a child scope under some given parent scope.
/// </summary>
/// <param name="parentScope">The parent scope under which the new child should be created.</param>
/// <param name="kind">What kind of scope to create (a PARSESCOPE_* value).</param>
/// <returns>The newly-created child scope.</returns>
ParseScope ParseScope_CreateChild(ParseScope parentScope, Int kind)
{
	ParseScope parseScope = GC_MALLOC_STRUCT(struct ParseScopeStruct);

	parseScope->kind = kind;
	parseScope->parentScope = parentScope;
	parseScope->symbols = Int32Dict_Create();
	parseScope->firstDeclaration = Smile_KnownObjects.Null;
	parseScope->lastDeclaration = Smile_KnownObjects.Null;

	return parseScope;
}

/// <summary>
/// Determine whether the given symbol was declared within the given scope
/// or in any parent scopes, and return its declaration.
/// </summary>
/// <param name="scope">The scope to check for the given symbol.</param>
/// <param name="symbol">The symbol to check for.</param>
/// <returns>The declaration if the symbol was declared in this scope or any parent scope, NULL if it was not.</returns>
ParseDecl ParseScope_FindDeclaration(ParseScope scope, Symbol symbol)
{
	void *value;
	for (; scope != NULL; scope = scope->parentScope) {
		if (Int32Dict_TryGetValue(scope->symbols, symbol, &value))
			return (ParseDecl)value;
	}
	return NULL;
}

/// <summary>
/// Declare or redeclare a name in the most appropriate current scope for declaring names.
/// </summary>
/// <param name="scope">The current scope.  This isn't necessarily where the declaration
/// will occur; it's simply where to start searching for the right place to put the
/// declaration.</param>
/// <param name="symbol">The name to declare.</param>
/// <param name="kind">What kind of thing to declare that name as (one of the PARSEDECL_* values,
/// like ARGUMENT or VARIABLE or CONST).</param>
/// <returns>The new declaration, if a declaration was created.  If this declaration attempt
/// resulted in a conflict with an existing declaration, this will return NULL.</returns>
ParseDecl ParseScope_Declare(ParseScope scope, Symbol symbol, Int kind)
{
	while (ParseScope_IsPseudoScope(scope)) {
		scope = scope->parentScope;
	}
	return ParseScope_DeclareHere(scope, symbol, kind);
}

/// <summary>
/// Declare or redeclare a name in the given scope.
/// </summary>
/// <param name="scope">The scope where the declaration will be created.</param>
/// <param name="symbol">The name to declare.</param>
/// <param name="kind">What kind of thing to declare that name as (one of the PARSEDECL_* values,
/// like ARGUMENT or VARIABLE or CONST).</param>
/// <returns>The new declaration, if a declaration was created.  If this declaration attempt
/// resulted in a conflict with an existing declaration, this will return NULL.</returns>
ParseDecl ParseScope_DeclareHere(ParseScope scope, Symbol symbol, Int kind)
{
	SmileList declList;
	Int symbolIndex;
	ParseDecl parseDecl;

	// Get the unique index of this symbol within the scope.
	symbolIndex = Int32Dict_Count(scope->symbols);

	// Construct the base declaration object for this name.
	parseDecl = ParseDecl_Create(symbol, kind, symbolIndex, NULL);

	// Add it to the set of declared names, if it doesn't already exist there.
	if (!Int32Dict_Add(scope->symbols, symbol, parseDecl)) {

		// Already declared.  This isn't necessarily an error, but it depends on what
		// kind of thing is being declared and how it relates to what was already declared.

		// TODO: Check kinds and make sure this is a valid redeclaration.
		return NULL;
	}

	// It's a valid new declaration, so add the symbol to the list of declared symbols.
	declList = SmileList_Cons((SmileObject)SmileSymbol_Create(symbol), (SmileObject)Smile_KnownObjects.Null);
	if (scope->firstDeclaration == Smile_KnownObjects.Null) {
		scope->firstDeclaration = declList;
	}
	else {
		scope->lastDeclaration->d = (SmileObject)declList;
	}
	scope->lastDeclaration = declList;

	// Finally, return the declaration object itself.
	return parseDecl;
}
