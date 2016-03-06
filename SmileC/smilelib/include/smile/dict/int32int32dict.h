#ifndef __SMILE_DICT_INT32INT32DICT_H__
#define __SMILE_DICT_INT32INT32DICT_H__

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
/// A single node (key/value pair) within an Int32Int32Dict.
/// </summary>
struct Int32Int32DictNode {
	Int32 next;					// A pointer to the next node in this bucket (relative to the Int32Int32Dict's heap).
	Int32 key;					// The key for this node.
	Int32 value;				// The value for this node.
};

/// <summary>
/// The internal implementation of an Int32Int32Dict.
/// </summary>
struct Int32Int32DictInt {
	Int32 *buckets;				// The buckets contain pointers into the heap, indexed by masked hash code.
	struct Int32Int32DictNode *heap;	// The heap, which holds all of the key/value pairs as Node structs.
	Int32 mask;					// The current size of both the heap and buckets.  Always equal to 2^n - 1 for some n.
	Int32 firstFree;			// The first free node in the heap (successive free nodes follow the 'next' pointers).
	Int32 count;				// The number of allocated nodes in the heap.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A dictionary of key/value pairs, keyed by Int32 keys, with arbitrary pointers as the values.
/// </summary>
typedef struct Int32Int32DictStruct {
	struct Int32Int32DictInt _opaque;
} *Int32Int32Dict;

/// <summary>
/// A key/value pair in an Int32Int32Dict, as returned by Int32Int32Dict_GetAll().
/// </summary>
typedef struct Int32Int32DictKeyValuePairStruct {
	Int32 key;					// The key for this pair.
	Int32 value;				// The value for this pair.
} Int32Int32DictKeyValuePair;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Int32 Int32Int32DictInt_Append(struct Int32Int32DictInt *intDict, Int32 key, Int32 value);

SMILE_API_FUNC Int32 *Int32Int32Dict_GetKeys(Int32Int32Dict intDict);
SMILE_API_FUNC Int32 *Int32Int32Dict_GetValues(Int32Int32Dict intDict);
SMILE_API_FUNC Int32Int32DictKeyValuePair *Int32Int32Dict_GetAll(Int32Int32Dict intDict);

SMILE_API_FUNC void Int32Int32Dict_ClearWithSize(Int32Int32Dict intDict, Int32 newSize);
SMILE_API_FUNC Bool Int32Int32Dict_Remove(Int32Int32Dict intDict, Int32 key);

SMILE_API_FUNC DictStats Int32Int32Dict_ComputeStats(Int32Int32Dict intDict);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Construct a new, empty dictionary, and control its allocation behavior.
/// </summary>
/// <param name="newSize">The initial allocation size of the dictionary, which is the number of
/// items the dictionary can hold without it needing to invoke another reallocation.</param>
/// <returns>The new, empty dictionary.</returns>
Inline Int32Int32Dict Int32Int32Dict_CreateWithSize(Int32 newSize)
{
	Int32Int32Dict intDict;
	
	intDict = (Int32Int32Dict)GC_MALLOC_STRUCT(struct Int32Int32DictInt);
	if (intDict == NULL) Smile_Abort_OutOfMemory();
	Int32Int32Dict_ClearWithSize(intDict, newSize);
	return intDict;
}

/// <summary>
/// Construct a new, empty dictionary.
/// </summary>
/// <returns>The new, empty dictionary.</returns>
Inline Int32Int32Dict Int32Int32Dict_Create(void)
{
	return Int32Int32Dict_CreateWithSize(16);
}

/// <summary>
/// Delete all key/value pairs in the dictionary, resetting it back to an initial state.
/// </summary>
Inline void Int32Int32Dict_Clear(Int32Int32Dict intDict)
{
	Int32Int32Dict_ClearWithSize(intDict, 16);
}

/// <summary>
/// Determine if the given key exists in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool Int32Int32Dict_ContainsKey(Int32Int32Dict intDict, Int32 key)
{
	SMILE_DICT_SEARCH(struct Int32Int32DictInt, struct Int32Int32DictNode, Int32,
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
Inline Bool Int32Int32Dict_Add(Int32Int32Dict intDict, Int32 key, Int32 value)
{
	SMILE_DICT_SEARCH(struct Int32Int32DictInt, struct Int32Int32DictNode, Int32,
		intDict, key, node->key == key,
		{
			return False;
		},
		{
			Int32Int32DictInt_Append((struct Int32Int32DictInt *)intDict, key, value);
			return True;
		})
}

/// <summary>
/// Get the number of key/value pairs in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <returns>The number of key/value pairs in the dictionary.</returns>
Inline Int32 Int32Int32Dict_Count(Int32Int32Dict intDict)
{
	return ((struct Int32Int32DictInt *)intDict)->count;
}

/// <summary>
/// Get a specific value from the dictionary by its key.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>The value for that key (NULL if the key is not found).</returns>
Inline Int32 Int32Int32Dict_GetValue(Int32Int32Dict intDict, Int32 key)
{
	SMILE_DICT_SEARCH(struct Int32Int32DictInt, struct Int32Int32DictNode, Int32,
		intDict, key, node->key == key,
		{
			return node->value;
		},
		{
			return 0;
		})
}

/// <summary>
/// Update a preexisting key/value pair in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key was updated, False if it did not exist.</returns>
Inline Bool Int32Int32Dict_ReplaceValue(Int32Int32Dict intDict, Int32 key, Int32 value)
{
	SMILE_DICT_SEARCH(struct Int32Int32DictInt, struct Int32Int32DictNode, Int32,
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
Inline Bool Int32Int32Dict_SetValue(Int32Int32Dict intDict, Int32 key, Int32 value)
{
	SMILE_DICT_SEARCH(struct Int32Int32DictInt, struct Int32Int32DictNode, Int32,
		intDict, key, node->key == key,
		{
			node->value = value;
			return True;
		},
		{
			Int32Int32DictInt_Append((struct Int32Int32DictInt *)intDict, key, value);
			return False;
		})
}

/// <summary>
/// Try to get a specific value from the dictionary by its key; this function,
/// unlike Int32Int32Dict_GetValue(), can tell you whether the key was found in addition
/// to returning the value.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <param name="value">This will be set to the value for that key (NULL if the key is not found).</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool Int32Int32Dict_TryGetValue(Int32Int32Dict intDict, Int32 key, Int32 *value)
{
	SMILE_DICT_SEARCH(struct Int32Int32DictInt, struct Int32Int32DictNode, Int32,
		intDict, key, node->key == key,
		{
			*value = node->value;
			return True;
		},
		{
			*value = 0;
			return False;
		})
}

/// <summary>
/// Try to add a specific value to the dictionary; but if it already exists, don't change
/// the dictionary.  Returns the current value for this key.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <param name="value">If this is a new key, this value will become the value for that key.</param>
/// <param name="result">The resulting value for the key.</param>
/// <returns>True if this key/value pair was newly added, or False if this key already existed and its value was not changed.</returns>
Inline Bool Int32Int32Dict_TryAddValue(Int32Int32Dict intDict, Int32 key, Int32 value, Int32 *result)
{
	SMILE_DICT_SEARCH(struct Int32Int32DictInt, struct Int32Int32DictNode, Int32,
		intDict, key, node->key == key,
		{
			*result = node->value = value;
			return True;
		},
		{
			Int32Int32DictInt_Append((struct Int32Int32DictInt *)intDict, key, *result = value);
			return False;
		})
}

#endif