
#ifndef __SMILE_PARSING_INTERNAL_PARSEDECL_H__
#define __SMILE_PARSING_INTERNAL_PARSEDECL_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif
#ifndef __SMILE_PARSING_LEXER_H__
#include <smile/parsing/lexer.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Parse-declaration kinds.

// Nonexistent (and therefore *very* redeclarable).
#define PARSEDECL_UNDECLARED		0		// An "undeclared" variable, declared in an outer scope, but not yet in this one.

// Normally-redeclarable forms.
#define PARSEDECL_PRIMITIVE			1		// One of the eighteen primitive forms, like [quote].
#define PARSEDECL_GLOBAL			2		// A global object, like String or List.
#define PARSEDECL_ARGUMENT			3		// An argument in the current function.
#define PARSEDECL_VARIABLE			4		// An ordinary local variable.

// Fixed forms within their scope.
#define PARSEDECL_CONST				5		// A local variable declared with a single static assignment.
#define PARSEDECL_AUTO				6		// A local variable declared to have auto-cleanup.
#define PARSEDECL_POSTCONDITION		7		// The special variable 'result' in a post: condition.
#define PARSEDECL_TILL				8		// A till-name declared for a till...do loop.

//-------------------------------------------------------------------------------------------------
//  Parse declarations.

/// <summary>
/// This describes the shape of a single name declared in the current scope.
/// </summary>
typedef struct ParseDeclStruct {

	DECLARE_BASE_OBJECT_PROPERTIES;

	// The symbol that has been declared.
	Symbol symbol;

	// What kind of declaration this is (see PARSEDECL_*).
	Int declKind;

	// The index of this declaration in the current scope (order in which it was declared).
	Int scopeIndex;

	// Where this declaration was located in the source code (if known; can be NULL).
	LexerPosition position;

	// The initial assignment of this variable in this scope.
	SmileObject initialAssignment;

} *ParseDecl;

SMILE_API_DATA SmileVTable ParseDecl_VTable;

//-------------------------------------------------------------------------------------------------
//  Functions.

/// <summary>
/// Simple helper function to create ParseDecl structs.
/// </summary>
Inline ParseDecl ParseDecl_Create(Symbol symbol, Int declKind, Int scopeIndex, LexerPosition position, SmileObject initialAssignment)
{
	ParseDecl parseDecl = GC_MALLOC_STRUCT(struct ParseDeclStruct);

	parseDecl->kind = SMILE_KIND_PARSEDECL;
	parseDecl->base = Smile_KnownObjects.Object;
	parseDecl->vtable = ParseDecl_VTable;
	parseDecl->assignedSymbol = 0;

	parseDecl->symbol = symbol;
	parseDecl->declKind = declKind;
	parseDecl->scopeIndex = scopeIndex;
	parseDecl->position = position;
	parseDecl->initialAssignment = initialAssignment;

	return parseDecl;
}

#endif
