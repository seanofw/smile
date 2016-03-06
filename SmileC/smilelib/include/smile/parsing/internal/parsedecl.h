
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

//-------------------------------------------------------------------------------------------------
//  Parse-declaration kinds.

#define PARSEDECL_PRIMITIVE			0		// One of the eighteen primitive forms, like [quote].
#define PARSEDECL_GLOBAL			1		// A global object, like String or List.
#define PARSEDECL_ARGUMENT			2		// An argument in the current function.
#define PARSEDECL_VARIABLE			3		// An ordinary local variable.
#define PARSEDECL_CONST				4		// A local variable declared with a single static assignment.
#define PARSEDECL_AUTO				5		// A local variable declared to have auto-cleanup.
#define PARSEDECL_POSTCONDITION		6		// The special variable 'result' in a post: condition.
#define PARSEDECL_TILL				7		// A till-name declared for a till...do loop.

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

	// The initial assignment of this variable in this scope.
	SmileObject initialAssignment;

} *ParseDecl;

SMILE_API_DATA SmileVTable ParseDecl_VTable;

//-------------------------------------------------------------------------------------------------
//  Functions.

/// <summary>
/// Simple helper function to create ParseDecl structs.
/// </summary>
Inline ParseDecl ParseDecl_Create(Symbol symbol, Int declKind, Int scopeIndex, SmileObject initialAssignment)
{
	ParseDecl parseDecl = GC_MALLOC_STRUCT(struct ParseDeclStruct);

	parseDecl->kind = SMILE_KIND_HANDLE;
	parseDecl->base = Smile_KnownObjects.Object;
	parseDecl->vtable = ParseDecl_VTable;
	parseDecl->assignedSymbol = 0;

	parseDecl->symbol = symbol;
	parseDecl->declKind = declKind;
	parseDecl->scopeIndex = scopeIndex;
	parseDecl->initialAssignment = initialAssignment;

	return parseDecl;
}

#endif
