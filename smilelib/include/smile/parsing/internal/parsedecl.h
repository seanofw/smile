
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
#define PARSEDECL_UNDECLARED	0	// An "undeclared" variable, declared in an outer scope, but not yet in this one.

// Normally-redeclarable forms.
#define PARSEDECL_PRIMITIVE		1	// One of the eighteen primitive forms, like [quote].
#define PARSEDECL_GLOBAL		2	// A global object, like String or List.
#define PARSEDECL_ARGUMENT		3	// An argument in the current function.
#define PARSEDECL_VARIABLE		4	// An ordinary local variable.

// Fixed forms within their scope.
#define PARSEDECL_CONST			5	// A local variable whose value cannot be changed.
#define PARSEDECL_SETONCECONST	6	// A local variable whose value can be assigned exactly once (and then it becomes const).
#define PARSEDECL_AUTO			7	// A local variable declared to have auto-cleanup.
#define PARSEDECL_SETONCEAUTO	8	// A local variable whose value can be assigned exactly once (and then it becomes auto).
#define PARSEDECL_KEYWORD		9	// A name that cannot be used as an implicit variable or unary/binary operator.
#define PARSEDECL_POSTCONDITION	10	// The special variable 'result' in a post: condition.
#define PARSEDECL_TILL			11	// A till-name declared for a till...do loop.
#define PARSEDECL_INCLUDE		12	// A single static assignment of a variable included from another module.

//-------------------------------------------------------------------------------------------------
//  Parse declarations.

/// <summary>
/// This describes the shape of a single name declared in the current scope.
/// </summary>
struct ParseDeclStruct {

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

};

//-------------------------------------------------------------------------------------------------
//  Functions.

/// <summary>
/// Simple helper function to create ParseDecl structs.
/// </summary>
Inline ParseDecl ParseDecl_Create(Symbol symbol, Int declKind, Int scopeIndex, LexerPosition position, SmileObject initialAssignment)
{
	ParseDecl parseDecl = GC_MALLOC_STRUCT(struct ParseDeclStruct);

	parseDecl->symbol = symbol;
	parseDecl->declKind = declKind;
	parseDecl->scopeIndex = scopeIndex;
	parseDecl->position = position;
	parseDecl->initialAssignment = initialAssignment;

	return parseDecl;
}

#endif
