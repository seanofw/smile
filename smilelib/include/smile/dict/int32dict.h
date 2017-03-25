#ifndef __SMILE_DICT_INT32DICT_H__
#define __SMILE_DICT_INT32DICT_H__

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
/// A single node (key/value pair) within an Int32Dict.
/// </summary>
struct Int32DictNode {
	Int32 next;					// A pointer to the next node in this bucket (relative to the Int32Dict's heap).
	Int32 key;					// The key for this node.
	void *value;				// The value for this node.
};

/// <summary>
/// The internal implementation of an Int32Dict.
/// </summary>
struct Int32DictInt {
	Int32 *buckets;				// The buckets contain pointers into the heap, indexed by masked hash code.
	struct Int32DictNode *heap;	// The heap, which holds all of the key/value pairs as Node structs.
	Int32 mask;					// The current size of both the heap and buckets.  Always equal to 2^n - 1 for some n.
	Int32 firstFree;			// The first free node in the heap (successive free nodes follow the 'next' pointers).
	Int32 count;				// The number of allocated nodes in the heap.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A dictionary of key/value pairs, keyed by Int32 keys, with arbitrary pointers as the values.
/// </summary>
typedef struct Int32DictStruct {
	struct Int32DictInt _opaque;
} *Int32Dict;

/// <summary>
/// A key/value pair in an Int32Dict, as returned by Int32Dict_GetAll().
/// </summary>
typedef struct Int32DictKeyValuePairStruct {
	Int32 key;					// The key for this pair.
	void *value;				// The value for this pair.
} Int32DictKeyValuePair;

/// <summary>
/// A "cloner" function for values in an Int32Dict, that is, a function that can make a perfect
/// deep copy of a value in the dictionary.
/// </summary>
/// <param name="key">The key for the value that is to be cloned.</param>
/// <param name="value">The value that is to be cloned.</param>
/// <param name="param">A custom user-provided parameter that may help with the cloning.</param>
/// <returns>The cloned value.</returns>
typedef void *(*Int32Dict_ValueCloner)(Int32 key, void *value, void *param);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Int32 Int32DictInt_Append(struct Int32DictInt *intDict, Int32 key, const void *value);

SMILE_API_FUNC Int32 *Int32Dict_GetKeys(Int32Dict intDict);
SMILE_API_FUNC void **Int32Dict_GetValues(Int32Dict intDict);
SMILE_API_FUNC Int32DictKeyValuePair *Int32Dict_GetAll(Int32Dict intDict);
SMILE_API_FUNC Int32DictKeyValuePair Int32Dict_GetFirst(Int32Dict intDict);

SMILE_API_FUNC void Int32Dict_ClearWithSize(Int32Dict intDict, Int32 newSize);
SMILE_API_FUNC Bool Int32Dict_Remove(Int32Dict intDict, Int32 key);
SMILE_API_FUNC Int32Dict Int32Dict_Clone(Int32Dict intDict, Int32Dict_ValueCloner valueCloner, void *param);

SMILE_API_FUNC DictStats Int32Dict_ComputeStats(Int32Dict intDict);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Construct a new, empty dictionary, and control its allocation behavior.
/// </summary>
/// <param name="newSize">The initial allocation size of the dictionary, which is the number of
/// items the dictionary can hold without it needing to invoke another reallocation.</param>
/// <returns>The new, empty dictionary.</returns>
Inline Int32Dict Int32Dict_CreateWithSize(Int32 newSize)
{
	Int32Dict intDict;
	
	intDict = (Int32Dict)GC_MALLOC_STRUCT(struct Int32DictInt);
	if (intDict == NULL) Smile_Abort_OutOfMemory();
	Int32Dict_ClearWithSize(intDict, newSize);
	return intDict;
}

/// <summary>
/// Construct a new, empty dictionary.
/// </summary>
/// <returns>The new, empty dictionary.</returns>
Inline Int32Dict Int32Dict_Create(void)
{
	return Int32Dict_CreateWithSize(16);
}

/// <summary>
/// Delete all key/value pairs in the dictionary, resetting it back to an initial state.
/// </summary>
Inline void Int32Dict_Clear(Int32Dict intDict)
{
	Int32Dict_ClearWithSize(intDict, 16);
}

/// <summary>
/// Determine if the given key exists in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool Int32Dict_ContainsKey(Int32Dict intDict, Int32 key)
{
	SMILE_DICT_SEARCH(struct Int32DictInt, struct Int32DictNode, Int32,
		intDict, key, node->key == key,
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
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key for the new key/value pair to add.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
/// <returns>True if the pair was successfully added, False if the key already existed.</returns>
Inline Bool Int32Dict_Add(Int32Dict intDict, Int32 key, const void *value)
{
	SMILE_DICT_SEARCH(struct Int32DictInt, struct Int32DictNode, Int32,
		intDict, key, node->key == key,
		{
			return False;
		},
		{
			Int32DictInt_Append((struct Int32DictInt *)intDict, key, value);
			return True;
		})
}

/// <summary>
/// Get the number of key/value pairs in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <returns>The number of key/value pairs in the dictionary.</returns>
Inline Int32 Int32Dict_Count(Int32Dict intDict)
{
	return ((struct Int32DictInt *)intDict)->count;
}

/// <summary>
/// Get a specific value from the dictionary by its key.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>The value for that key (NULL if the key is not found).</returns>
Inline void *Int32Dict_GetValue(Int32Dict intDict, Int32 key)
{
	SMILE_DICT_SEARCH(struct Int32DictInt, struct Int32DictNode, Int32,
		intDict, key, node->key == key,
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
Inline Bool Int32Dict_ReplaceValue(Int32Dict intDict, Int32 key, void *value)
{
	SMILE_DICT_SEARCH(struct Int32DictInt, struct Int32DictNode, Int32,
		intDict, key, node->key == key,
		{
			node->value = value;
			return True;
		},
		{
			return False;
		})
}

/// <summary>
/// Create or update a key/value pair in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to create or update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key already existed, False if it needed to be added.</returns>
Inline Bool Int32Dict_SetValue(Int32Dict intDict, Int32 key, void *value)
{
	SMILE_DICT_SEARCH(struct Int32DictInt, struct Int32DictNode, Int32,
		intDict, key, node->key == key,
		{
			node->value = value;
			return True;
		},
		{
			Int32DictInt_Append((struct Int32DictInt *)intDict, key, value);
			return False;
		})
}

/// <summary>
/// Try to get a specific value from the dictionary by its key; this function,
/// unlike Int32Dict_GetValue(), can tell you whether the key was found in addition
/// to returning the value.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <param name="value">This will be set to the value for that key (NULL if the key is not found).</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool Int32Dict_TryGetValue(Int32Dict intDict, Int32 key, void **value)
{
	SMILE_DICT_SEARCH(struct Int32DictInt, struct Int32DictNode, Int32,
		intDict, key, node->key == key,
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