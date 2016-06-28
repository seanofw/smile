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

#include <smile/numeric/real64.h>

static ParseError ParseScope_DeclareGlobalOperators(ParseScope parseScope);

static const char *ParseDecl_Names[] = {
	"an undeclared variable",
	"a primitive",
	"a global variable",
	"a function argument",
	"a local variable",
	"a const value",
	"an auto variable",
	"a postcondition result",
	"a till-loop name",
};

/// <summary>
/// Create a root scope that contains only the eighteen or so global forms.
/// </summary>
/// <returns>The newly-created root scope.</returns>
ParseScope ParseScope_CreateRoot(void)
{
	ParseError error;

	ParseScope parseScope = GC_MALLOC_STRUCT(struct ParseScopeStruct);

	parseScope->kind = PARSESCOPE_OUTERMOST;
	parseScope->parentScope = NULL;
	parseScope->closure = Closure_CreateDynamic(NULL, ClosureInfo_Create(NULL));

	if ((error = ParseScope_DeclareGlobalOperators(parseScope)) != NULL) {
		String errorMessage = String_Format("Unable to declare global operators; this is most likely due to a corrupt memory space. Reported error was: %S", error->message);
		Smile_Abort_FatalError(String_ToC(errorMessage));
	}

	return parseScope;
}

/// <summary>
/// Declare the eighteen-or-so global operators in the given (presumably root-level) scope.
/// This also declares 'null', 'true', and 'false'; even though they're not operators,
/// everybody expects them to exist in the earliest stages of the universe.
/// </summary>
/// <param name="parseScope">The scope in which to declare the root-level operators.</param>
/// <returns>True if all were declared successfully, False if any failed to be declared.</returns>
static ParseError ParseScope_DeclareGlobalOperators(ParseScope parseScope)
{
	ParseError error;

	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.equals_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.op_equals_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;

	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.if_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.while_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.var_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.till_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.catch_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.fn_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.scope_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.progn_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.quote_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.new_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.is_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.typeof_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.and_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.or_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.not_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;

	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.null_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.true_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.false_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;

	return NULL;
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
	parseScope->closure = Closure_CreateDynamic(parentScope->closure, ClosureInfo_Create(parentScope->closure->closureInfo));

	return parseScope;
}

/// <summary>
/// Declare or redeclare a name in the given scope.
/// </summary>
/// <param name="scope">The scope where the declaration will be created.</param>
/// <param name="symbol">The name to declare.</param>
/// <param name="kind">What kind of thing to declare that name as (one of the PARSEDECL_* values,
/// like ARGUMENT or VARIABLE or CONST).</param>
/// <param name="position">The position in the source code, if known, of this declaration.</param>
/// <param name="decl">This will be filled in with the new ParseDecl object.</param>
/// <returns>Any errors generated by the declaration, or NULL on success.</returns>
ParseError ParseScope_DeclareHere(ParseScope scope, Symbol symbol, Int kind, LexerPosition position, ParseDecl *decl)
{
	ParseDecl parseDecl;
	ParseDecl previousDecl;
	ParseError error;

	// Construct the base declaration object for this name.
	parseDecl = ParseDecl_Create(symbol, kind, scope->closure->closureInfo->numVariables, position, NULL);

	if ((previousDecl = (ParseDecl)Closure_GetHereByName(scope->closure, symbol))->kind != SMILE_KIND_NULL) {

		// Already declared.  This isn't necessarily an error, but it depends on what
		// kind of thing is being declared and how it relates to what was already declared.
		if ((previousDecl->declKind != kind && previousDecl->declKind >= PARSEDECL_UNDECLARED)
			|| previousDecl->declKind >= PARSEDECL_CONST) {

			if (decl != NULL)
				*decl = NULL;
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, position,
				String_Format("Cannot redeclare \"%S\" as %s; it is already declared as %s, on line \"%d\".",
				SymbolTable_GetName(Smile_SymbolTable, symbol),
				ParseDecl_Names[kind],
				ParseDecl_Names[previousDecl->declKind],
				previousDecl->position != NULL ? previousDecl->position->line : 0));
			return error;
		}
	}

	Closure_Let(scope->closure, symbol, (SmileObject)parseDecl);

	// Last, return the declaration object itself.
	if (decl != NULL)
		*decl = parseDecl;
	return NULL;
}
