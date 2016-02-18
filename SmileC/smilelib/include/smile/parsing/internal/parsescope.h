
#ifndef __SMILE_PARSING_INTERNAL_PARSESCOPE_H__
#define __SMILE_PARSING_INTERNAL_PARSESCOPE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILELIST_H__
#include <smile/smiletypes/smilelist.h>
#endif
#ifndef __SMILE_SMILETYPES_TEXT_SMILESYMBOL_H__
#include <smile/smiletypes/text/smilesymbol.h>
#endif
#ifndef __SMILE_PARSING_INTERNAL_PARSEDECL_H__
#include <smile/parsing/internal/parsedecl.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Parsing scope kinds.

#define PARSESCOPE_OUTERMOST		0		// The one-and-only outermost scope.
#define PARSESCOPE_FUNCTION			1		// A function scope (a pseudo-scope for its arguments).
#define PARSESCOPE_SCOPEDECL		2		// An explicitly-declared scope using {braces}.
#define PARSESCOPE_POSTCONDITION	3		// A post: or pre:condition mini-scope.
#define PARSESCOPE_TILLDO			4		// A till...do body in which till-names apply.

//-------------------------------------------------------------------------------------------------
//  Parse scopes.

/// <summary>
/// Each brace-delimited (function-delimited, statement-delimited, etc.) scope during
/// parsing looks like this.  At runtime, they will be represented as full lexical closures
/// (or equivalent), but during parsing, there's just this much data associated with them.
/// </summary>
typedef struct ParseScopeStruct {

	// Every scope (except for the global scope) has a lexical parent.
	struct ParseScopeStruct *parentScope;

	// This is the collection of symbols that were in some way declared within this scope.
	// It is a dictionary mapping of Symbol IDs --> ParseDecl objects.
	Int32Dict symbols;

	// What kind of scope this is, of the PARSESCOPE_* values.
	Int kind;

	// A list of all the declarations in this scope, in declaration order
	// (but just a sequence of SmileSymbol objects are stored in this list).
	SmileList firstDeclaration;
	SmileList lastDeclaration;

} *ParseScope;

//-------------------------------------------------------------------------------------------------
//  External implementation.

ParseScope ParseScope_CreateRoot(void);
ParseScope ParseScope_CreateChild(ParseScope parentScope, Int kind);
ParseDecl ParseScope_FindDeclaration(ParseScope scope, Symbol symbol);
ParseDecl ParseScope_Declare(ParseScope scope, Symbol symbol, Int kind);
ParseDecl ParseScope_DeclareHere(ParseScope scope, Symbol symbol, Int kind);

//-------------------------------------------------------------------------------------------------
//  Inline functions.

/// <summary>
/// Determine whether the given symbol was declared within the given scope
/// (ignoring possible declarations in parent scopes), and return its declaration
/// if it was declared here.
/// </summary>
/// <param name="scope">The scope to check for the given symbol.</param>
/// <param name="symbol">The symbol to check for.</param>
/// <returns>The declaration if the symbol was explicitly declared in this scope, NULL if it was not.</returns>
Inline ParseDecl ParseScope_FindDeclarationHere(ParseScope scope, Symbol symbol)
{
	void *value;
	return Int32Dict_TryGetValue(scope->symbols, symbol, &value) ? (ParseDecl)value : NULL;
}

/// <summary>
/// Return whether the given symbol has been declared in the given scope or any ancestor scope of it.
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <param name="symbol">The symbol to check for in that scope or any ancestor of it.</param>
/// <returns>True if that symbol has been declared in this scope chain, false if it has not.</returns>
Inline Bool ParseScope_IsDeclared(ParseScope scope, Symbol symbol)
{
	return ParseScope_FindDeclaration(scope, symbol) != NULL;
}

/// <summary>
/// Return whether the given symbol has been declared in the given scope.
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <param name="symbol">The symbol to check for in that scope.</param>
/// <returns>True if that symbol has been declared in this scope, false if it has not.</returns>
Inline Bool ParseScope_IsDeclaredHere(ParseScope scope, Symbol symbol)
{
	return ParseScope_FindDeclarationHere(scope, symbol) != NULL;
}

/// <summary>
/// Return how many names have been declared in the given scope (just *this* scope,
/// not including its ancestor declarations).
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <returns>The number of names declared within that scope.</returns>
Inline Int ParseScope_GetDeclarationCount(ParseScope scope)
{
	return Int32Dict_Count(scope->symbols);
}

/// <summary>
/// Determine whether this is a pseudo-scope, that is, something (like a scope resulting
/// from a |fn|) that does not necessarily introduce a new variable-assignment context the 
/// same way {braces} and real scopes do.
/// </summary>
Inline Bool ParseScope_IsPseudoScope(ParseScope parseScope)
{
	return (parseScope->kind == PARSESCOPE_FUNCTION
		|| parseScope->kind == PARSESCOPE_POSTCONDITION
		|| parseScope->kind == PARSESCOPE_TILLDO);
}

#endif
