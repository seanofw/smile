
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
#ifndef __SMILE_PARSING_INTERNAL_PARSELOANWORD_H__
#include <smile/parsing/internal/parseloanword.h>
#endif
#ifndef __SMILE_PARSING_PARSEMESSAGE_H__
#include <smile/parsing/parsemessage.h>
#endif
#ifndef __SMILE_EVAL_CLOSURE_H__
#include <smile/eval/closure.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Parsing scope kinds.

#define PARSESCOPE_OUTERMOST	0	// The one-and-only outermost scope.
#define PARSESCOPE_FUNCTION	1	// A function scope (a pseudo-scope for its arguments).
#define PARSESCOPE_SCOPEDECL	2	// A declared scope using {braces}.
#define PARSESCOPE_POSTCONDITION	3	// A post: or pre:condition mini-scope.
#define PARSESCOPE_TILLDO	4	// A till...do body in which till-names apply.
#define PARSESCOPE_SYNTAX	5	// A syntax rule's body, in which the syntax names apply.
#define PARSESCOPE_EXPLICIT	6	// An explicitly-declared scope using a Lisp-style [$scope] form.

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
	// Each assignment within this dictionary is a mapping to the index of a ParseDecl object
	// in the declaration array below.  Because there's only one set of variables per scope,
	// we're a member of the Lisp-1 family.
	Int32Int32Dict symbolDict;

	// This is the collection of actual symbol declarations in this scope, in declaration order.
	ParseDecl *decls;
	Int numDecls;
	Int maxDecls;

	// The list of actual syntax declarations in this scope, in declaration order.
	SmileList syntaxListHead, syntaxListTail;

	// The list of included syntax rules in this scope, in declaration order.
	SmileList syntaxIncludeListHead, syntaxIncludeListTail;

	// The syntax table for this scope, which describes the current effective set of syntax rules.
	ParserSyntaxTable syntaxTable;

	// The list of actual loanword declarations in this scope, in declaration order.
	SmileList loanwordListHead, loanwordListTail;

	// The list of included loanword rules in this scope, in declaration order.
	SmileList loanwordIncludeListHead, loanwordIncludeListTail;

	// The loanword table for this scope, which describes the current effective set of loanword rules.
	ParserLoanwordTable loanwordTable;

	Bool reexport;
};

//-------------------------------------------------------------------------------------------------
//  External implementation.

SMILE_API_FUNC ParseScope ParseScope_CreateRoot(void);
SMILE_API_FUNC ParseScope ParseScope_CreateChild(ParseScope parentScope, Int kind);
SMILE_API_FUNC ParseError ParseScope_DeclareVariablesFromClosureInfo(ParseScope scope, ClosureInfo closureInfo);
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
	Int32 index;
	if (!Int32Int32Dict_TryGetValue(scope->symbolDict, (Int32)symbol, &index))
		return NULL;
	return scope->decls[index];
}

/// <summary>
/// Return whether the given symbol has been declared in the given scope or any ancestor scope of it.
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <param name="symbol">The symbol to check for in that scope or any ancestor of it.</param>
/// <returns>True if that symbol has been declared in this scope chain, false if it has not.</returns>
Inline Bool ParseScope_IsDeclared(ParseScope scope, Symbol symbol)
{
	for (; scope != NULL; scope = scope->parentScope) {
		if (Int32Int32Dict_ContainsKey(scope->symbolDict, (Int32)symbol))
			return True;
	}
	return False;
}

/// <summary>
/// Return whether the given symbol has been declared in the given scope.
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <param name="symbol">The symbol to check for in that scope.</param>
/// <returns>True if that symbol has been declared in this scope, false if it has not.</returns>
Inline Bool ParseScope_IsDeclaredHere(ParseScope scope, Symbol symbol)
{
	return Int32Int32Dict_ContainsKey(scope->symbolDict, (Int32)symbol);
}

/// <summary>
/// Return how many names have been declared in the given scope (just *this* scope,
/// not including its ancestor declarations).
/// </summary>
/// <param name="scope">The scope to check.</param>
/// <returns>The number of names declared within that scope.</returns>
Inline Int ParseScope_GetDeclarationCount(ParseScope scope)
{
	return Int32Int32Dict_Count(scope->symbolDict);
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
	Int32 index;
	for (; scope != NULL; scope = scope->parentScope) {
		if (Int32Int32Dict_TryGetValue(scope->symbolDict, (Int32)symbol, &index)) {
			return scope->decls[index];
		}
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
/// <param name="decl">This will be filled in with the new ParseDecl object.</param>
/// <returns>Any errors generated by the declaration, or NULL on success.</returns>
Inline ParseError ParseScope_Declare(ParseScope scope, Symbol symbol, Int kind, LexerPosition position, ParseDecl *decl)
{
	while (ParseScope_IsPseudoScope(scope)) {
		scope = scope->parentScope;
	}
	return ParseScope_DeclareHere(scope, symbol, kind, position, decl);
}

/// <summary>
/// Declare a named variable in the given scope, using a C-style name.
/// </summary>
Inline ParseError ParseScope_DeclareHereC(ParseScope scope, const char *name, Int kind, LexerPosition position, ParseDecl *decl)
{
	Symbol symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, name);
	return ParseScope_DeclareHere(scope, symbol, kind, position, decl);
}

/// <summary>
/// Add a SmileSyntax object to this scope's collection of declared syntax rules.
/// </summary>
Inline void ParseScope_AddSyntax(ParseScope scope, SmileSyntax syntax)
{
	LIST_APPEND(scope->syntaxListHead, scope->syntaxListTail, syntax);
}

/// <summary>
/// Add a SmileSyntax object to this scope's collection of included syntax rules.
/// </summary>
Inline void ParseScope_AddIncludeSyntax(ParseScope scope, SmileSyntax syntax)
{
	LIST_APPEND(scope->syntaxIncludeListHead, scope->syntaxIncludeListTail, syntax);
}

/// <summary>
/// Add a SmileLoanword object to this scope's collection of declared loanword rules.
/// </summary>
Inline void ParseScope_AddLoanword(ParseScope scope, SmileLoanword loanword)
{
	LIST_APPEND(scope->loanwordListHead, scope->loanwordListTail, loanword);
}

/// <summary>
/// Add a SmileLoanword object to this scope's collection of included loanword rules.
/// </summary>
Inline void ParseScope_AddIncludeLoanword(ParseScope scope, SmileLoanword loanword)
{
	LIST_APPEND(scope->loanwordIncludeListHead, scope->loanwordIncludeListTail, loanword);
}

#endif
