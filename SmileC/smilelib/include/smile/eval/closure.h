
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
struct ClosureInfoBaseStruct {

	struct ClosureInfoStruct *parent;	// A pointer to the parent info struct, if any.
	struct ClosureInfoStruct *global;	// A pointer to the closest global info struct, if any.
		
	Int16 kind;	// What kind of closure this is (see the CLOSURE_KIND enum).
	Int16 reserved1;	// Reserved for use by other closure-info types.
	Int32 reserved2;	// Reserved for use by other closure-info types.

};

/// <summary>
/// A ClosureInfo structure is a reusable object that provides all of the metadata about the
/// information stored in similarly-shaped closures.
/// </summary>
typedef struct ClosureInfoStruct {

	struct ClosureInfoStruct *parent;	// A pointer to the parent info struct, if any.
	struct ClosureInfoStruct *global;	// A pointer to the closest global info struct, if any.
		
	Int16 kind;	// What kind of closure this is (see the CLOSURE_KIND enum).
	Int16 numVariables;	// The number of variables in this closure.
	Int32 tempSize;	// The maximum amount of temporary variables required by this closure.
		
	VarDict variableDictionary;	// A dictionary that maps Symbol IDs to VarInfo objects.
		// For local closures, this is used only for debugging, and the values are always null;
		// for global closures, this contains the actual variable values.
		
	Symbol *variableNames;	// If this is a local closure, this is an array of the names of the variables in
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
		
	SmileObject *frame;	// The offset of the stack frame (- for args, + for local vars).
	SmileObject *stackTop;	// The current address of the top of the temporary variables (NULL for global closures).
	SmileObject variables[1];	// The array of variables (matches the ClosureInfo's numVariables + tempSize; size 0 for global closures).

} *Closure;

//-------------------------------------------------------------------------------------------------
// External Implementation.

SMILE_API_FUNC ClosureInfo ClosureInfo_Create(ClosureInfo parent, Int kind);

SMILE_API_FUNC Closure Closure_CreateGlobal(ClosureInfo info, Closure parent);
SMILE_API_FUNC Closure Closure_CreateLocal(ClosureInfo info, Closure parent);

SMILE_API_FUNC SmileObject Closure_GetGlobalVariable(Closure closure, Symbol name);
SMILE_API_FUNC Bool Closure_HasGlobalVariable(Closure closure, Symbol name);
SMILE_API_FUNC void Closure_SetGlobalVariable(Closure closure, Symbol name, SmileObject value);

SMILE_API_FUNC SmileObject Closure_GetLocalVariableInScope(Closure closure, Int scope, Int index);
SMILE_API_FUNC void Closure_SetLocalVariableInScope(Closure closure, Int scope, Int index, SmileObject value);

SMILE_API_FUNC SmileObject Closure_GetArgumentInScope(Closure closure, Int scope, Int index);
SMILE_API_FUNC void Closure_SetArgumentInScope(Closure closure, Int scope, Int index, SmileObject value);

//-------------------------------------------------------------------------------------------------
// Inlines and Macro Forms.

#define Closure_PushTemp(__closure__, __value__) \
	(*(__closure__)->stackTop++ = (SmileObject)(__value__))

#define Closure_PopTemp(__closure__) \
	(*--(__closure__)->stackTop)

#define Closure_GetTop(__closure__) \
	((__closure__)->stackTop[-1])

#define Closure_SetTop(__closure__, __value__) \
	((__closure__)->stackTop[-1] = (SmileObject)(__value__))

#define Closure_GetTemp(__closure__, __offset__) \
	((__closure__)->stackTop[-1 - (__offset__)])

#define Closure_SetTemp(__closure__, __offset__, __value__) \
	((__closure__)->stackTop[-1 - (__offset__)] = (SmileObject)(__value__))

#define Closure_PopCount(__closure__, __count__) \
	((__closure__)->stackTop -= (__count__))

//-------------------------------------------------------------------------------------------------

#define Closure_GetLocalVariable(__closure__, __index__) \
	((__closure__)->frame[(__index__)])

#define Closure_SetLocalVariable(__closure__, __index__, __value__) \
	(((__closure__)->frame[(__index__)]) = (SmileObject)(__value__))

#define Closure_GetLocalVariableInScope0(__closure__, __index__) \
	Closure_GetLocalVariable((__closure__), (__index__))
#define Closure_GetLocalVariableInScope1(__closure__, __index__) \
	Closure_GetLocalVariable((__closure__)->parent, (__index__))
#define Closure_GetLocalVariableInScope2(__closure__, __index__) \
	Closure_GetLocalVariable((__closure__)->parent->parent, (__index__))
#define Closure_GetLocalVariableInScope3(__closure__, __index__) \
	Closure_GetLocalVariable((__closure__)->parent->parent->parent, (__index__))
#define Closure_GetLocalVariableInScope4(__closure__, __index__) \
	Closure_GetLocalVariable((__closure__)->parent->parent->parent->parent, (__index__))
#define Closure_GetLocalVariableInScope5(__closure__, __index__) \
	Closure_GetLocalVariable((__closure__)->parent->parent->parent->parent->parent, (__index__))
#define Closure_GetLocalVariableInScope6(__closure__, __index__) \
	Closure_GetLocalVariable((__closure__)->parent->parent->parent->parent->parent->parent, (__index__))
#define Closure_GetLocalVariableInScope7(__closure__, __index__) \
	Closure_GetLocalVariable((__closure__)->parent->parent->parent->parent->parent->parent->parent, (__index__))

#define Closure_SetLocalVariableInScope0(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__), (__index__), (__value__)))
#define Closure_SetLocalVariableInScope1(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent, (__index__), (__value__)))
#define Closure_SetLocalVariableInScope2(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent, (__index__), (__value__)))
#define Closure_SetLocalVariableInScope3(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetLocalVariableInScope4(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetLocalVariableInScope5(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetLocalVariableInScope6(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetLocalVariableInScope7(__closure__, __index__, __value__) \
	(Closure_SetLocalVariable((__closure__)->parent->parent->parent->parent->parent->parent->parent, (__index__), (__value__)))

//-------------------------------------------------------------------------------------------------

#define Closure_GetArgument(__closure__, __index__) \
	((__closure__)->variables[(__index__)])

#define Closure_SetArgument(__closure__, __index__, __value__) \
	(((__closure__)->variables[(__index__)]) = (SmileObject)(__value__))

#define Closure_GetArgumentInScope0(__closure__, __index__) \
	Closure_GetArgument((__closure__), (__index__))
#define Closure_GetArgumentInScope1(__closure__, __index__) \
	Closure_GetArgument((__closure__)->parent, (__index__))
#define Closure_GetArgumentInScope2(__closure__, __index__) \
	Closure_GetArgument((__closure__)->parent->parent, (__index__))
#define Closure_GetArgumentInScope3(__closure__, __index__) \
	Closure_GetArgument((__closure__)->parent->parent->parent, (__index__))
#define Closure_GetArgumentInScope4(__closure__, __index__) \
	Closure_GetArgument((__closure__)->parent->parent->parent->parent, (__index__))
#define Closure_GetArgumentInScope5(__closure__, __index__) \
	Closure_GetArgument((__closure__)->parent->parent->parent->parent->parent, (__index__))
#define Closure_GetArgumentInScope6(__closure__, __index__) \
	Closure_GetArgument((__closure__)->parent->parent->parent->parent->parent->parent, (__index__))
#define Closure_GetArgumentInScope7(__closure__, __index__) \
	Closure_GetArgument((__closure__)->parent->parent->parent->parent->parent->parent->parent, (__index__))

#define Closure_SetArgumentInScope0(__closure__, __index__, __value__) \
	(Closure_SetArgument((__closure__), (__index__), (__value__)))
#define Closure_SetArgumentInScope1(__closure__, __index__, __value__) \
	(Closure_SetArgument((__closure__)->parent, (__index__), (__value__)))
#define Closure_SetArgumentInScope2(__closure__, __index__, __value__) \
	(Closure_SetArgument((__closure__)->parent->parent, (__index__), (__value__)))
#define Closure_SetArgumentInScope3(__closure__, __index__, __value__) \
	(Closure_SetArgument((__closure__)->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetArgumentInScope4(__closure__, __index__, __value__) \
	(Closure_SetArgument((__closure__)->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetArgumentInScope5(__closure__, __index__, __value__) \
	(Closure_SetArgument((__closure__)->parent->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetArgumentInScope6(__closure__, __index__, __value__) \
	(Closure_SetArgument((__closure__)->parent->parent->parent->parent->parent->parent, (__index__), (__value__)))
#define Closure_SetArgumentInScope7(__closure__, __index__, __value__) \
	(Closure_SetArgument((__closure__)->parent->parent->parent->parent->parent->parent->parent, (__index__), (__value__)))

//-------------------------------------------------------------------------------------------------

#endif
