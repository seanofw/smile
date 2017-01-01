
#ifndef __SMILE_EVAL_CLOSURE_H__
#define __SMILE_EVAL_CLOSURE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_DICT_VARDICT_H__
#include <smile/dict/vardict.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif
#ifndef __SMILE_ENV_ENV_H__
#include <smile/env/env.h>
#endif

struct CompiledFunctionStruct;

//-------------------------------------------------------------------------------------------------
// Closure Structures.
//
// Closures are stored in two pieces:  One piece, the Closure object, stores the current instance
// data of the closure and is designed to be as small as possible; and the second piece is a
// reusable ClosureInfoStruct that provides all of the metadata about the information in similarly-
// shaped closures.

// A global closure references its local variables by name (by symbol), and has neither arguments
// nor a temporary stack.
#define CLOSURE_KIND_GLOBAL	0

// A local closure references its local varibles by index (by offset), and has an initial set of
// arguments, and a trailing temporary stack.
#define CLOSURE_KIND_LOCAL	1

/// <summary>
/// A ClosureInfo structure is a reusable object that provides all of the metadata about the
/// information stored in similarly-shaped closures.
/// </summary>
typedef struct ClosureInfoStruct {

	Int16 kind;	// What kind of closure this is (see the CLOSURE_KIND enum).
	Int16 numVars;	// The number of variables in this closure.
	Int32 tempSize;	// The maximum amount of temporary variables required by this closure.
		
	struct ClosureInfoStruct *parent;	// A pointer to the parent info struct, if any.
	struct ClosureInfoStruct *global;	// A pointer to the closest global info struct, if any.
		
	VarDict variableDictionary;	// A dictionary that maps Symbol IDs to VarInfo objects.
		// For local closures, this is used only for debugging, and the values are always null;
		// for global closures, this contains the actual variable values.
		
	Symbol *localNames;	// If this is a local closure, this is an array of the names of the variables in
		// this closure, in stack order (used only for debugging).

} *ClosureInfo;

/// <summary>
/// A Closure object stores the current instance data of a closure, and is designed to be as
/// small as possible.  Anything that doesn't absolutely have to be stored here is stored in the
/// ClosureInfo struct, to which this object points.
/// </summary>
typedef struct ClosureStruct {

	struct ClosureStruct *parent;	// A pointer to the parent closure, if any.
	struct ClosureStruct *global;	// A pointer to the closest global closure, if any.
		
	ClosureInfo closureInfo;	// Shared metadata about this closure.
		
	Int32 stackTop;	// The current offset of the top of the temporary variables (0 for global closures).
	SmileObject vars[1];	// The array of vars (matches the ClosureInfo's localNames; size 0 for global closures).

} *Closure;

//-------------------------------------------------------------------------------------------------
// External Implementation.

SMILE_API_FUNC ClosureInfo ClosureInfo_Create(ClosureInfo parent, Int kind);

SMILE_API_FUNC Closure Closure_CreateGlobal(ClosureInfo info, Closure parent);
SMILE_API_FUNC Closure Closure_CreateLocal(ClosureInfo info, Closure parent);

SMILE_API_FUNC SmileObject Closure_GetGlobalVariable(Closure closure, Symbol name);
SMILE_API_FUNC Bool Closure_HasGlobalVariable(Closure closure, Symbol name);
SMILE_API_FUNC void Closure_SetGlobalVariable(Closure closure, Symbol name, SmileObject value);

SMILE_API_FUNC SmileObject Closure_GetVariableInScope(Closure closure, Int scope, Int index);
SMILE_API_FUNC void Closure_SetVariableInScope(Closure closure, Int scope, Int index, SmileObject value);

//-------------------------------------------------------------------------------------------------
// Inlines and Macro Forms.

#define Closure_PushTemp(__closure__, __value__) \
	((__closure__)->vars[(__closure__)->stackTop++] = (__value__))

#define Closure_PopTemp(__closure__) \
	((__closure__)->vars[--(__closure__)->stackTop]

#define Closure_PopCount(__closure__, __count__) \
	((__closure__)->stackTop -= (__count__))

#define Closure_GetLocalVariable(__closure__, __index__) \
	((__closure__)->vars[(__index__)])

Inline void Closure_SetLocalVariable(Closure closure, Int index, SmileObject value)
{
	closure->vars[index] = value;
	if (!value->assignedSymbol) {
		value->assignedSymbol = closure->closureInfo->localNames[index];
	}
}

#define Closure_GetVariableInScope0(__closure__, __index__) \
	Closure_GetLocalVaraible((__closure__), (__index__))
#define Closure_GetVariableInScope1(__closure__, __index__) \
	Closure_GetLocalVaraible((__closure__)->parent, (__index__))
#define Closure_GetVariableInScope2(__closure__, __index__) \
	Closure_GetLocalVaraible((__closure__)->parent->parent, (__index__))
#define Closure_GetVariableInScope3(__closure__, __index__) \
	Closure_GetLocalVaraible((__closure__)->parent->parent->parent, (__index__))
#define Closure_GetVariableInScope4(__closure__, __index__) \
	Closure_GetLocalVaraible((__closure__)->parent->parent->parent->parent, (__index__))
#define Closure_GetVariableInScope5(__closure__, __index__) \
	Closure_GetLocalVaraible((__closure__)->parent->parent->parent->parent->parent, (__index__))
#define Closure_GetVariableInScope6(__closure__, __index__) \
	Closure_GetLocalVaraible((__closure__)->parent->parent->parent->parent->parent->parent, (__index__))
#define Closure_GetVariableInScope7(__closure__, __index__) \
	Closure_GetLocalVaraible((__closure__)->parent->parent->parent->parent->parent->parent->parent, (__index__))

#define Closure_SetVariableInScope0(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__), (__index__), (__value__)))
#define Closure_SetVariableInScope1(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent, (__index__), (__value__)))
#define Closure_SetVariableInScope2(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent, (__index__), (__value__)))
#define Closure_SetVariableInScope3(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetVariableInScope4(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetVariableInScope5(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetVariableInScope6(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetVariableInScope7(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent->parent->parent->parent->parent, (__index__), (__value__)))

#endif
