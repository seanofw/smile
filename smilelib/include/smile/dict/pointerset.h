#ifndef __SMILE_DICT_POINTERSET_H__
#define __SMILE_DICT_POINTERSET_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_GC_H__
#include <smile/gc.h>
#endif
#ifndef __SMILE_DICT_SHARED_H__
#include <smile/dict/shared.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Internal types

/// <summary>
/// A single node (key) within a PointerSet.
/// </summary>
struct PointerSetNode {
	Int32 next;					// A pointer to the next node in this bucket (relative to the PointerSet's heap).
	void *key;					// The key for this node.
};

/// <summary>
/// The internal implementation of a PointerSet.
/// </summary>
struct PointerSetInt {
	Int32 *buckets;	// The buckets contain pointers into the heap, indexed by masked hash code.
	struct PointerSetNode *heap;	// The heap, which holds all of the keys as Node structs.
	Int32 mask;	// The current size of both the heap and buckets.  Always equal to 2^n - 1 for some n.
	Int32 firstFree;	// The first free node in the heap (successive free nodes follow the 'next' pointers).
	Int32 count;	// The number of allocated nodes in the heap.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A set of pointers, with constant lookup times.
/// </summary>
typedef struct PointerSetStruct {
	struct PointerSetInt _opaque;
} *PointerSet;

/// <summary>
/// A "cloner" function for pointers in a PointerSet, that is, a function that can make a perfect
/// deep copy of a value in the set.
/// </summary>
/// <param name="key">The key that is to be cloned.</param>
/// <param name="param">A custom user-provided parameter that may help with the cloning.</param>
/// <returns>The cloned value.</returns>
typedef void *(*PointerSet_KeyCloner)(void *key, void *param);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Int32 PointerSetInt_Append(struct PointerSetInt *pointerSet, void *key);

SMILE_API_FUNC void **PointerSet_GetAll(PointerSet pointerSet);
SMILE_API_FUNC void *PointerSet_GetFirst(PointerSet pointerSet);

SMILE_API_FUNC void PointerSet_ClearWithSize(PointerSet pointerSet, Int32 newSize);
SMILE_API_FUNC Bool PointerSet_Remove(PointerSet pointerSet, void *key);
SMILE_API_FUNC PointerSet PointerSet_Clone(PointerSet pointerSet, PointerSet_KeyCloner keyCloner, void *param);

SMILE_API_FUNC DictStats PointerSet_ComputeStats(PointerSet pointerSet);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Construct a new, empty pointer set, and control its allocation behavior.
/// </summary>
/// <param name="newSize">The initial allocation size of the set, which is the number of
/// items the set can hold without it needing to invoke another reallocation.</param>
/// <returns>The new, empty set.</returns>
Inline PointerSet PointerSet_CreateWithSize(Int32 newSize)
{
	PointerSet pointerSet;

	pointerSet = (PointerSet)GC_MALLOC_STRUCT(struct PointerSetInt);
	if (pointerSet == NULL) Smile_Abort_OutOfMemory();
	PointerSet_ClearWithSize(pointerSet, newSize);
	return pointerSet;
}

/// <summary>
/// Construct a new, empty set.
/// </summary>
/// <returns>The new, empty set.</returns>
Inline PointerSet PointerSet_Create(void)
{
	return PointerSet_CreateWithSize(16);
}

/// <summary>
/// Delete all keys in the set, resetting it back to an initial state.
/// </summary>
Inline void PointerSet_Clear(PointerSet pointerSet)
{
	PointerSet_ClearWithSize(pointerSet, 16);
}

/// <summary>
/// Determine if the given key exists in the set.
/// </summary>
/// <param name="pointerSet">A pointer to the set.</param>
/// <param name="key">The key to search for.</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool PointerSet_Contains(PointerSet pointerSet, void *key)
{
	SMILE_DICT_SEARCH(struct PointerSetInt, struct PointerSetNode, Int32,
		pointerSet, ((Int32)((PtrInt)key >> 3)), node->key == key,
		{
			return True;
		},
		{
			return False;
		})
}

/// <summary>
/// Add a new key to the set.  If the key already exists, this will fail.
/// </summary>
/// <param name="pointerSet">A pointer to the set.</param>
/// <param name="key">The key to add.</param>
/// <returns>True if the key was successfully added, False if the key already existed.</returns>
Inline Bool PointerSet_Add(PointerSet pointerSet, void *key)
{
	SMILE_DICT_SEARCH(struct PointerSetInt, struct PointerSetNode, Int32,
		pointerSet, ((Int32)((PtrInt)key >> 3)), node->key == key,
		{
			return False;
		},
		{
			PointerSetInt_Append((struct PointerSetInt *)pointerSet, key);
			return True;
		})
}

/// <summary>
/// Get the number of keys in the set.
/// </summary>
/// <param name="pointerSet">A pointer to the set.</param>
/// <returns>The number of key/value pairs in the set.</returns>
Inline Int32 PointerSet_Count(PointerSet pointerSet)
{
	return ((struct PointerSetInt *)pointerSet)->count;
}

#endif