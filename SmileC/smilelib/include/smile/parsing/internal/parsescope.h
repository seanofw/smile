
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
#ifndef __SMILE_PARSING_PARSETYPES_H__
#include <smile/parsing/parsetypes.h>
#endif
#ifndef __SMILE_PARSING_INTERNAL_PARSEDECL_H__
#include <smile/parsing/internal/parsedecl.h>
#endif
#ifndef __SMILE_PARSING_INTERNAL_PARSESYNTAX_H__
#include <smile/parsing/internal/parsesyntax.h>
#endif
#ifndef __SMILE_PARSING_PARSEMESSAGE_H__
#include <smile/parsing/parsemessage.h>
#endif
#ifndef __SMILE_ENV_CLOSURE_H__
#include <smile/env/closure.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Parsing scope kinds.

#define PARSESCOPE_OUTERMOST		0		// The one-and-only outermost scope.
#define PARSESCOPE_FUNCTION			1		// A function scope (a pseudo-scope for its arguments).
#define PARSESCOPE_SCOPEDECL		2		// An explicitly-declared scope using {braces}.
#define PARSESCOPE_POSTCONDITION	3		// A post: or pre:condition mini-scope.
#define PARSESCOPE_TILLDO			4		// A till...do body in which till-names apply.
#define PARSESCOPE_SYNTAX			5		// A syntax rule's body, in which the syntax names apply.

//-------------------------------------------------------------------------------------------------
//  Parse scopes.

/// <summary>
/// Each brace-delimited (function-delimited, statement-delimited, etc.) scope during
/// parsing looks like this.  At runtime, they will be represented as full lexical closures
/// (or equivalent), but during parsing, there's just this much data associated with them.
/// </summary>
struct ParseScopeStruct {

	// Every scope (except for the global scope) has a lexical parent.
	struct ParseScopeStruct *parentScope;

	// What kind of scope this is, of the PARSESCOPE_* values.
	Int kind;

	// This is the collection of symbols that were in some way declared within this scope.
	// Each assignment within this closure is a mapping to a ParseDecl object, not to a real
	// SmileObject.  Because there's only one set of variables per scope, we're a member of
	// the Lisp-1 family.
	Closure closure;

	// The syntax table for this scope, which describes the current effective set of syntax rules.
	ParserSyntaxTable syntaxTable;

};

//-------------------------------------------------------------------------------------------------
//  External implementation.

SMILE_API_FUNC ParseScope ParseScope_CreateRoot(void);
SMILE_API_FUNC ParseScope ParseScope_CreateChild(ParseScope parentScope, Int kind);
SMILE_API_FUNC ParseError ParseScope_DeclareHere(ParseScope scope, Symbol symbol, Int kind, LexerPosition position, ParseDecl *decl);
SMILE_API_FUNC void ParseScope_Finish(ParseScope currentScope);

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
	Int index = Closure_GetNameIndex(scope->closure, symbol);
	return index >= 0 ? (ParseDecl)scope->closure->variables[index] : NULL;
}

/// <summary>
/// Return whether the given symbol has been declared in the given scope or any ancestor scope of it.
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <param name="symbol">The symbol to check for in that scope or any ancestor of it.</param>
/// <returns>True if that symbol has been declared in this scope chain, false if it has not.</returns>
Inline Bool ParseScope_IsDeclared(ParseScope scope, Symbol symbol)
{
	return Closure_Has(scope->closure, symbol);
}

/// <summary>
/// Return whether the given symbol has been declared in the given scope.
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <param name="symbol">The symbol to check for in that scope.</param>
/// <returns>True if that symbol has been declared in this scope, false if it has not.</returns>
Inline Bool ParseScope_IsDeclaredHere(ParseScope scope, Symbol symbol)
{
	return Closure_HasHere(scope->closure, symbol);
}

/// <summary>
/// Return how many names have been declared in the given scope (just *this* scope,
/// not including its ancestor declarations).
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <returns>The number of names declared within that scope.</returns>
Inline Int ParseScope_GetDeclarationCount(ParseScope scope)
{
	return scope->closure->closureInfo->numVariables;
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

/// <summary>
/// Determine whether the given symbol was declared within the given scope
/// or in any parent scopes, and return its declaration.
/// </summary>
/// <param name="scope">The scope to check for the given symbol.</param>
/// <param name="symbol">The symbol to check for.</param>
/// <returns>The declaration if the symbol was declared in this scope or any parent scope, NULL if it was not.</returns>
Inline ParseDecl ParseScope_FindDeclaration(ParseScope scope, Symbol symbol)
{
	ParseDecl decl;
	return Closure_TryGet(scope->closure, symbol, (SmileObject *)&decl) ? decl : NULL;
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
/// <param name="decl">This will be filled in with the new ParseDecl object.</param>
/// <returns>Any errors generated by the declaration, or NULL on success.</returns>
Inline ParseError ParseScope_Declare(ParseScope scope, Symbol symbol, Int kind, LexerPosition position, ParseDecl *decl)
{
	while (ParseScope_IsPseudoScope(scope)) {
		scope = scope->parentScope;
	}
	return ParseScope_DeclareHere(scope, symbol, kind, position, decl);
}

#endif
