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

#include <smile/env/closure.h>

/// <summary>
/// Create a new ClosureInfo struct, which contains metadata about a closure.
/// </summary>
/// <param name="parent">The parent closureInfo whose lexical namespace this will be
/// contained within.</param>
/// <returns>The newly-created ClosureInfo object, which will be set to writable,
/// growable, and with no variables yet.</returns>
ClosureInfo ClosureInfo_Create(ClosureInfo parent)
{
	ClosureInfo closureInfo = GC_MALLOC_STRUCT(struct ClosureInfoStruct);

	closureInfo->parent = parent;
	closureInfo->symbolDictionary = Int32Int32Dict_Create();
	closureInfo->numVariables = 0;
	closureInfo->maxVariables = 0;
	closureInfo->isReadOnly = False;
	closureInfo->canGrow = True;

	return closureInfo;
}

/// <summary>
/// "Let" a variable in the provided closure:  Create it, if it doesn't already exist,
/// and assign it the given value.  In fixed-shape closures, this will only assign to
/// preexisting variables defined in the ClosureInfo for this Closure.
/// </summary>
/// <param name="closure">The closure in which the new variable will be assigned.  The
/// variable will be assigned *here*, and not in any parent closure, even if there are
/// overlapping names.</param>
/// <param name="name">The name of the variable to assign.</param>
/// <param name="value">The value to assign that variable to.</param>
/// <returns>True if the variable was assigned, False if it could not be (i.e., this is
/// a fixed-size closure and the named variable did not exist within it.</returns>
Bool Closure_Let(Closure closure, Symbol name, SmileObject value)
{
	ClosureInfo closureInfo = closure->closureInfo;
	Int32 index;
	Int32 maxVariables, newMax;
	SmileObject *variables = closure->variables;

	// If we can grow, we try to optimally extend the set of symbols in this closure.
	if (closureInfo->canGrow) {

		// Try to extend the symbol dictionary.
		if (Int32Int32Dict_TryAddValue(closureInfo->symbolDictionary, name, closureInfo->numVariables, &index)) {

			// We're adding a new symbol, so make sure we have space for it.
			if (index >= (maxVariables = closureInfo->maxVariables)) {

				// No space, so reallocate the closure's storage array.
				newMax = maxVariables * 2;
				variables = GC_MALLOC_STRUCT_ARRAY(SmileObject, newMax);
				MemCpy(variables, closure->variables, sizeof(SmileObject) * maxVariables);
				closure->variables = variables;
				closureInfo->maxVariables = newMax;
			}
		}
	}
	else {
		// Can't grow, so just try to find the index of the symbol.
		if (!Int32Int32Dict_TryGetValue(closure->closureInfo->symbolDictionary, name, &index))
			return False;
	}

	// Assign the variable, by index.
	variables[index] = value;
	return True;
}

/// <summary>
/// Get the value of a variable in the provided closure chain, by its name.
/// This function is roughly equivalent to getting a variable's value in Smile code.
/// </summary>
/// <param name="closure">The closure in which the variable can be found.  The
/// variable will be retrieved from the nearest closure in which it is defined.</param>
/// <param name="name">The name of the variable to retrieve.</param>
/// <returns>The current value of that variable.</returns>
SmileObject Closure_Get(Closure closure, Symbol name)
{
	// Walk up the tree of closures searching for one that contains this name.
	Int32 index;
	for (; closure != NULL; closure = closure->parent) {
		if (Int32Int32Dict_TryGetValue(closure->closureInfo->symbolDictionary, name, &index))
			return closure->variables[index];
	}

	// Didn't find it, so it's treated as a Smile Null.
	return (SmileObject)Smile_KnownObjects.Null;
}

/// <summary>
/// Set the value of a variable in the provided closure chain, by its name.
/// This function is roughly equivalent to setting a variable's value in Smile code.
/// </summary>
/// <param name="closure">The closure in which the variable can be found.  The
/// variable will be assigned in the nearest closure in which it is already defined,
/// or added to the current closure if it is not already defined (if the current
/// closure is not read-only, that is).</param>
/// <param name="name">The name of the variable to assign.</param>
/// <param name="value>The new value of that variable.</param>
/// <returns>The closure in which the variable was assigned, which can be NULL if the
/// variable could not be assigned anywhere.</returns>
/// <remarks>The behavior of this function may differ slightly from real Smile code
/// depending on whether the target closure is read-only, since the Smile optimizer
/// may mark some closures as read-only that would not be considered "read-only" by
/// actual Smile code.  Most of the time, the result will be the same, but in optimized
/// closures, you may find that this function is unable to create new variables.</remarks>
Closure Closure_Set(Closure closure, Symbol name, SmileObject value)
{
	// Search for a closure that already contains this name.
	Int32 index;
	Closure container;
	for (container = closure; container != NULL; container = container->parent) {
		if (Int32Int32Dict_TryGetValue(container->closureInfo->symbolDictionary, name, &index)) {

			// Found a closure in which a variable with this name already existed.
			// But is it read-only?
			if (container->closureInfo->isReadOnly)
				return NULL;

			// Assign the variable in the found closure.
			container->variables[index] = value;
			return container;
		}
	}

	// Didn't find a matching closure, so attempt to perform a Let in the original
	// closure to create it.
	return Closure_Let(closure, name, value) ? closure : NULL;
}
