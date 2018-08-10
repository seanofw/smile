#ifndef __SMILE_DICT_POINTERDICT_H__
#define __SMILE_DICT_POINTERDICT_H__

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
/// A single node (key/value pair) within an PointerDict.
/// </summary>
struct PointerDictNode {
	Int next;					// A pointer to the next node in this bucket (relative to the PointerDict's heap).
	void *key;					// The key for this node.
	void *value;				// The value for this node.
};

/// <summary>
/// The internal implementation of an PointerDict.
/// </summary>
struct PointerDictInt {
	Int *buckets;					// The buckets contain pointers into the heap, indexed by masked hash code.
	struct PointerDictNode *heap;	// The heap, which holds all of the key/value pairs as Node structs.
	Int count;						// The number of allocated nodes in the heap.
	Int firstFree;					// The first free node in the heap (successive free nodes follow the 'next' pointers).
	Int mask;						// The current hash mask.  Always equal to 2^n-1 for some n.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A dictionary of key/value pairs, keyed by pointers, with arbitrary pointers as the values.
/// </summary>
typedef struct PointerDictStruct {
	struct PointerDictInt _opaque;
} *PointerDict;

/// <summary>
/// A key/value pair in a PointerDict, as returned by PointerDict_GetAll().
/// </summary>
typedef struct {
	void *key;					// The key for this pair.
	void *value;				// The value for this pair.
} PointerDictKeyValuePair;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Int PointerDictInt_Append(struct PointerDictInt *pointerDict, const void *key, const void *value);

SMILE_API_FUNC void **PointerDict_GetKeys(PointerDict pointerDict);
SMILE_API_FUNC void **PointerDict_GetValues(PointerDict pointerDict);
SMILE_API_FUNC PointerDictKeyValuePair *PointerDict_GetAll(PointerDict pointerDict);

SMILE_API_FUNC void PointerDict_ClearWithSize(PointerDict pointerDict, Int newSize);
SMILE_API_FUNC Bool PointerDict_Remove(PointerDict pointerDict, const void *key);

SMILE_API_FUNC DictStats PointerDict_ComputeStats(PointerDict pointerDict);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Construct a new, empty dictionary, and control its allocation behavior.
/// </summary>
/// <param name="newSize">The initial allocation size of the dictionary, which is the number of
/// items the dictionary can hold without it needing to invoke another reallocation.</param>
/// <returns>The new, empty dictionary.</returns>
Inline PointerDict PointerDict_CreateWithSize(Int newSize)
{
	PointerDict pointerDict;

	pointerDict = (PointerDict)GC_MALLOC_STRUCT(struct PointerDictInt);
	if (pointerDict == NULL) Smile_Abort_OutOfMemory();
	PointerDict_ClearWithSize(pointerDict, newSize);
	return pointerDict;
}

/// <summary>
/// Construct a new, empty dictionary.
/// </summary>
/// <returns>The new, empty dictionary.</returns>
Inline PointerDict PointerDict_Create(void)
{
	return PointerDict_CreateWithSize(16);
}

/// <summary>
/// Delete all key/value pairs in the dictionary, resetting it back to an initial state.
/// </summary>
Inline void PointerDict_Clear(PointerDict pointerDict)
{
	PointerDict_ClearWithSize(pointerDict, 16);
}

/// <summary>
/// Determine if the given key exists in the dictionary.
/// </summary>
/// <param name="pointerDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool PointerDict_ContainsKey(PointerDict pointerDict, const void *key)
{
	SMILE_DICT_SEARCH(struct PointerDictInt, struct PointerDictNode, Int,
		pointerDict, (PtrInt)key, node->key == key,
		{
			return True;
		},
		{
			return False;
		})
}

/// <summary>
/// Add a new key/value pair to the dictionary.  If the key already exists, this will fail.
/// </summary>
/// <param name="pointerDict">A pointer to the dictionary.</param>
/// <param name="key">The key for the new key/value pair to add.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
/// <returns>True if the pair was successfully added, False if the key already existed.</returns>
Inline Bool PointerDict_Add(PointerDict pointerDict, const void *key, const void *value)
{
	SMILE_DICT_SEARCH(struct PointerDictInt, struct PointerDictNode, Int,
		pointerDict, (PtrInt)key, node->key == key,
		{
			return False;
		},
		{
			PointerDictInt_Append((struct PointerDictInt *)pointerDict, key, value);
			return True;
		})
}

/// <summary>
/// Get the number of key/value pairs in the dictionary.
/// </summary>
/// <param name="pointerDict">A pointer to the dictionary.</param>
/// <returns>The number of key/value pairs in the dictionary.</returns>
Inline Int PointerDict_Count(PointerDict pointerDict)
{
	return ((struct PointerDictInt *)pointerDict)->count;
}

/// <summary>
/// Get a specific value from the dictionary by its key.
/// </summary>
/// <param name="pointerDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>The value for that key (NULL if the key is not found).</returns>
Inline void *PointerDict_GetValue(PointerDict pointerDict, const void *key)
{
	SMILE_DICT_SEARCH(struct PointerDictInt, struct PointerDictNode, Int,
		pointerDict, (PtrInt)key, node->key == key,
		{
			return node->value;
		},
		{
			return NULL;
		})
}

/// <summary>
/// Update a preexisting key/value pair in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key was updated, False if it did not exist.</returns>
Inline Bool PointerDict_ReplaceValue(PointerDict pointerDict, const void *key, const void *value)
{
	SMILE_DICT_SEARCH(struct PointerDictInt, struct PointerDictNode, Int,
		pointerDict, (PtrInt)key, node->key == key,
		{
			node->value = (void *)value;
			return True;
		},
		{
			return False;
		})
}

/// <summary>
/// Create or update a key/value pair in the dictionary.
/// </summary>
/// <param name="pointerDict">A pointer to the dictionary.</param>
/// <param name="key">The key to create or update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key already existed, False if it needed to be added.</returns>
Inline Bool PointerDict_SetValue(PointerDict pointerDict, const void *key, const void *value)
{
	SMILE_DICT_SEARCH(struct PointerDictInt, struct PointerDictNode, Int,
		pointerDict, (PtrInt)key, node->key == key,
		{
			node->value = (void *)value;
			return True;
		},
		{
			PointerDictInt_Append((struct PointerDictInt *)pointerDict, key, value);
			return False;
		})
}

/// <summary>
/// Try to get a specific value from the dictionary by its key; this function,
/// unlike PointerDict_GetValue(), can tell you whether the key was found in addition
/// to returning the value.
/// </summary>
/// <param name="pointerDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <param name="value">This will be set to the value for that key (NULL if the key is not found).</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool PointerDict_TryGetValue(PointerDict pointerDict, const void *key, void **value)
{
	SMILE_DICT_SEARCH(struct PointerDictInt, struct PointerDictNode, Int,
		pointerDict, (PtrInt)key, node->key == key,
		{
			*value = node->value;
			return True;
		},
		{
			*value = NULL;
			return False;
		})
}

#endif