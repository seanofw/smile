
#ifndef __SMILE_ENV_CLOSURE_H__
#define __SMILE_ENV_CLOSURE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_DICT_INT32INT32DICT_H__
#include <smile/dict/int32int32dict.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif
#ifndef __SMILE_ENV_ENV_H__
#include <smile/env/env.h>
#endif
#ifndef __SMILE_EVAL_COSTACK_H__
#include <smile/eval/costack.h>
#endif

struct CompiledFunctionStruct;

//-------------------------------------------------------------------------------------------------
// Closure Structures.
//
// Closures are stored in two pieces:  One piece, the Closure object, stores the current instance
// data of the closure and is designed to be as small as possible; and the second piece is a
// reusable ClosureInfoStruct that provides all of the metadata about the information in similarly-
// shaped closures.

/// <summary>
/// A ClosureInfo structure is a reusable object that provides all of the metadata about the
/// information stored in similarly-shaped closures.
/// </summary>
typedef struct ClosureInfoStruct {

	struct ClosureInfoStruct *parent;	// A pointer to the parent info struct, if any.
		
	Int32Int32Dict argsDictionary;	// A shared dictionary that maps Symbol IDs to args before the frame.
	Int32Int32Dict localsDictionary;	// A shared dictionary that maps Symbol IDs to indices in the frame.
		
	Symbol *locals;	// The names of the variables in the 'locals' array.
	Int32 numlocals;	// The number of locals in this closure's 'locals' array.
	Int32 maxlocals;	// The current maximum number of locals in this closure's 'locals' array, if it can grow.
		
	Symbol *args;	// The names of the variables in the 'args' array.
	Int32 numArgs;	// The number of args in this closure's 'args' array.
		
	Int32 maxTemps;	// The maximum amount of temporary stack space required by this closure.

} *ClosureInfo;

/// <summary>
/// A Closure object stores the current instance data of a closure, and is designed to be as
/// small as possible.  Anything that doesn't absolutely have to be stored here is stored in the
/// ClosureInfo struct, to which this object points.  These objects are allocated directly
/// on the current costack.
/// </summary>
typedef struct ClosureStruct {

	ClosureInfo closureInfo;	// Shared metadata about this closure.
		
	CoStack costack;	// A back-pointer to the costack that contains this closure.
		
	Int32 parentClosureOffset;	// The offset of the parent (lexically-containing) closure on the stack, if any.
	Int32 callingClosureOffset;	// The offset of the calling closure on the stack, if any.
		
	Int32 retAddr;	// The return address within the caller's byte-code.
	Int32 catchAddr;	// The exception-catch address within the caller's byte-code.
		
	Int32 args;	// Stack offset, the start of arguments for this closure.
	Int32 frame;	// Stack offset, the start of this closure's frame (locals and temps).

} *Closure;

//-------------------------------------------------------------------------------------------------
// External Implementation.

#if 0

SMILE_API_FUNC ClosureInfo ClosureInfo_Create(ClosureInfo parent);

SMILE_API_FUNC Bool Closure_Let(Closure closure, Symbol name, SmileObject value);

SMILE_API_FUNC SmileObject Closure_Get(Closure closure, Symbol name);
SMILE_API_FUNC Bool Closure_TryGet(Closure closure, Symbol name, SmileObject *value);
SMILE_API_FUNC Closure Closure_Set(Closure closure, Symbol name, SmileObject value);
SMILE_API_FUNC Bool Closure_Has(Closure closure, Symbol name);

//-------------------------------------------------------------------------------------------------
// Inline Functions.

/// <summary>
/// Create a dynamic-size closure.  The variables associated with this will be allocated
/// in a separate array.  Note that the variables will *not* be initialized to any specific
/// value; in particular, if you want them set to Null, you'll have to set them yourself.
/// </summary>
/// <param name="parent">The parent closure, if any.</param>
/// <param name="closureInfo">The ClosureInfo that describes how this closure will be created.</param>
/// <returns>The new Closure object.</returns>
Inline Closure Closure_CreateDynamic(Closure parent, ClosureInfo closureInfo)
{
	Closure closure = GC_MALLOC_STRUCT(struct ClosureStruct);
	closure->parent = parent;
	closure->closureInfo = closureInfo;
	closure->variables = GC_MALLOC_STRUCT_ARRAY(SmileObject, closureInfo->maxVariables);
	return closure;
}

/// <summary>
/// Create a fixed-size closure.  The variables associated with this will be allocated
/// at the end of the closure struct.  Note that the variables will *not* be initialized
/// to any specific value; in particular, if you want them set to Null, you'll have to
/// set them yourself.
/// </summary>
/// <param name="parent">The parent closure, if any.</param>
/// <param name="closureInfo">The ClosureInfo that describes how this closure will be created.</param>
/// <returns>The new Closure object.</returns>
Inline Closure Closure_CreateFixed(Closure parent, ClosureInfo closureInfo)
{
	Closure closure = (Closure)GC_MALLOC(sizeof(struct ClosureStruct) + sizeof(SmileObject) * closureInfo->numVariables);
	closure->parent = parent;
	closure->closureInfo = closureInfo;
	closure->variables = (SmileObject *)((Byte *)closure + sizeof(struct ClosureStruct));
	return closure;
}

/// <summary>
/// Create a closure based on the specifications of the given ClosureInfo object.
/// </summary>
/// <param name="parent">The parent closure, if any.</param>
/// <param name="closureInfo">The ClosureInfo that describes how this closure will be created.</param>
/// <returns>The new Closure object.</returns>
Inline Closure Closure_Create(Closure parent, ClosureInfo closureInfo)
{
	return closureInfo->canGrow
		? Closure_CreateDynamic(parent, closureInfo)
		: Closure_CreateFixed(parent, closureInfo);
}

/// <summary>
/// Assign a variable in the provided closure, by its index.
/// </summary>
/// <param name="closure">The closure in which the variable will be assigned.  The
/// variable will be assigned *here*, and not in any parent closure, even if there are
/// overlapping names.</param>
/// <param name="name">The name of the variable to assign.</param>
/// <param name="value">The value to assign that variable to.</param>
Inline void Closure_SetHereByIndex(Closure closure, Int index, SmileObject value)
{
	closure->variables[index] = value;
}

/// <summary>
/// Assign a variable in the provided closure, by its name.
/// </summary>
/// <param name="closure">The closure in which the variable will be assigned.  The
/// variable will be assigned *here*, and not in any parent closure, even if there are
/// overlapping names.</param>
/// <param name="name">The name of the variable to assign.  If no variable exists within
/// this closure by that name, this function will fail.</param>
/// <param name="value">The value to assign that variable to.</param>
/// <returns>True if the variable could be assigned, False if the variable did not exist
/// within this closure.</returns>
Inline Bool Closure_SetHereByName(Closure closure, Symbol name, SmileObject value)
{
	Int32 index;
	if (!Int32Int32Dict_TryGetValue(closure->closureInfo->symbolDictionary, name, &index))
		return False;
	closure->variables[index] = value;
	return True;
}

/// <summary>
/// Get the value of a variable in the provided closure, by its index.
/// </summary>
/// <param name="closure">The closure in which the variable can be found.  The
/// variable will be retrieved *here*, and not in any parent closure, even if there are
/// overlapping names.</param>
/// <param name="index">The index of the variable to retrieve.</param>
/// <returns>The current value of that variable.</returns>
Inline SmileObject Closure_GetHereByIndex(Closure closure, Int index)
{
	return closure->variables[index];
}

/// <summary>
/// Get the index of a variable in the provided closure, by its name.
/// </summary>
/// <param name="closure">The closure in which the variable can be found.  The
/// variable will be retrieved *here*, and not in any parent closure, even if there are
/// overlapping names.</param>
/// <param name="name">The name of the variable index to retrieve.</param>
/// <returns>The index of that variable, if it exists, or -1 if it does not exist.</returns>
Inline Int Closure_GetNameIndex(Closure closure, Symbol name)
{
	Int32 index;
	if (!Int32Int32Dict_TryGetValue(closure->closureInfo->symbolDictionary, name, &index))
		return -1;
	return index;
}

/// <summary>
/// Get the value of a variable in the provided closure, by its name.
/// </summary>
/// <param name="closure">The closure in which the variable can be found.  The
/// variable will be retrieved *here*, and not in any parent closure, even if there are
/// overlapping names.</param>
/// <param name="name">The name of the variable to retrieve.</param>
/// <returns>The current value of that variable.</returns>
Inline SmileObject Closure_GetHereByName(Closure closure, Symbol name)
{
	Int32 index;
	if (!Int32Int32Dict_TryGetValue(closure->closureInfo->symbolDictionary, name, &index))
		return (SmileObject)Null;
	return closure->variables[index];
}

/// <summary>
/// Determine if a variable exists in the provided closure, by its name.
/// </summary>
/// <param name="closure">The closure in which the variable may be found.  The
/// variable will be tested *here*, and not in any parent closure, even if there are
/// overlapping names.</param>
/// <param name="name">The name of the variable to test.</param>
/// <returns>True if the variable exists in this closure, False if the variable does not.</returns>
Inline Bool Closure_HasHere(Closure closure, Symbol name)
{
	return Int32Int32Dict_ContainsKey(closure->closureInfo->symbolDictionary, name);
}

#endif

#endif
