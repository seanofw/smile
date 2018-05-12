#ifndef __SMILE_DICT_VARDICT_H__
#define __SMILE_DICT_VARDICT_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_GC_H__
#include <smile/gc.h>
#endif
#ifndef __SMILE_DICT_SHARED_H__
#include <smile/dict/shared.h>
#endif
#ifndef __SMILE_ENV_SYMBOLTABLE_H__
#include <smile/env/symboltable.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Variable-specific types.

// The possible kinds of variables in a given closure.
#define VAR_KIND_GLOBAL			0	// A global variable (accessed by name/symbol, not by index).
#define VAR_KIND_COMMONGLOBAL	1	// One of the built-in globals (not a custom global).
#define VAR_KIND_ARG			2	// A function argument.
#define VAR_KIND_VAR			3	// A variable.
#define VAR_KIND_CONST			4	// A constant value.

// This structure describes a single variable in a given closure.
typedef struct VarInfoStruct {
	Symbol symbol;
	Int32 kind;
	Int32 offset;
	struct SmileObjectInt *value;
} *VarInfo;

//-------------------------------------------------------------------------------------------------
//  Internal types

struct SmileObjectInt;

/// <summary>
/// A single node (key/value pair) within an VarDict.
/// </summary>
struct VarDictNode {
	Int32 next;	// A pointer to the next node in this bucket (relative to the VarDict's heap).
	Int32 key;	// The key for this node.
	struct VarInfoStruct varInfo;	// The actual variable info, inline.
};

/// <summary>
/// The internal implementation of a VarDict.
/// </summary>
struct VarDictInt {
	Int32 *buckets;	// The buckets contain pointers into the heap, indexed by masked hash code.
	struct VarDictNode *heap;	// The heap, which holds all of the key/value pairs as Node structs.
	Int32 mask;	// The current size of both the heap and buckets.  Always equal to 2^n - 1 for some n.
	Int32 firstFree;	// The first free node in the heap (successive free nodes follow the 'next' pointers).
	Int32 count;	// The number of allocated nodes in the heap.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A dictionary of key/value pairs, keyed by Int32 keys, with arbitrary pointers as the values.
/// </summary>
typedef struct VarDictStruct {
	struct VarDictInt _opaque;
} *VarDict;

/// <summary>
/// A key/value pair in an VarDict, as returned by VarDict_GetAll().
/// </summary>
typedef struct VarDictKeyValuePairStruct {
	Symbol key;	// The key for this pair.
	VarInfo value;	// The value for this pair.
} VarDictKeyValuePair;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Int32 VarDictInt_Append(struct VarDictInt *varDict, Symbol key, const VarInfo value);

SMILE_API_FUNC Symbol *VarDict_GetKeys(VarDict varDict);
SMILE_API_FUNC VarInfo *VarDict_GetValues(VarDict varDict);
SMILE_API_FUNC VarDictKeyValuePair *VarDict_GetAll(VarDict varDict);
SMILE_API_FUNC VarDictKeyValuePair VarDict_GetFirst(VarDict varDict);

SMILE_API_FUNC void VarDict_ClearWithSize(VarDict varDict, Int32 newSize);
SMILE_API_FUNC Bool VarDict_Remove(VarDict varDict, Symbol key);
SMILE_API_FUNC VarDict VarDict_Clone(VarDict varDict);

SMILE_API_FUNC DictStats VarDict_ComputeStats(VarDict varDict);

SMILE_API_FUNC Bool VarDict_ForEach(VarDict varDict, Bool (*func)(VarInfo varInfo, void *param), void *param);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Construct a new, empty dictionary, and control its allocation behavior.
/// </summary>
/// <param name="newSize">The initial allocation size of the dictionary, which is the number of
/// items the dictionary can hold without it needing to invoke another reallocation.</param>
/// <returns>The new, empty dictionary.</returns>
Inline VarDict VarDict_CreateWithSize(Int32 newSize)
{
	VarDict varDict;

	varDict = (VarDict)GC_MALLOC_STRUCT(struct VarDictInt);
	if (varDict == NULL) Smile_Abort_OutOfMemory();
	VarDict_ClearWithSize(varDict, newSize);
	return varDict;
}

/// <summary>
/// Construct a new, empty dictionary.
/// </summary>
/// <returns>The new, empty dictionary.</returns>
Inline VarDict VarDict_Create(void)
{
	return VarDict_CreateWithSize(16);
}

/// <summary>
/// Delete all key/value pairs in the dictionary, resetting it back to an initial state.
/// </summary>
Inline void VarDict_Clear(VarDict varDict)
{
	VarDict_ClearWithSize(varDict, 16);
}

/// <summary>
/// Determine if the given key exists in the dictionary.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool VarDict_ContainsKey(VarDict varDict, Symbol key)
{
	SMILE_DICT_SEARCH(struct VarDictInt, struct VarDictNode, Int32,
		varDict, key, node->key == key,
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
/// <param name="varDict">A pointer to the dictionary.</param>
/// <param name="key">The key for the new key/value pair to add.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
/// <returns>True if the pair was successfully added, False if the key already existed.</returns>
Inline Bool VarDict_Add(VarDict varDict, Symbol key, const VarInfo value)
{
	SMILE_DICT_SEARCH(struct VarDictInt, struct VarDictNode, Int32,
		varDict, key, node->key == key,
		{
			return False;
		},
		{
			VarDictInt_Append((struct VarDictInt *)varDict, key, value);
			return True;
		})
}

/// <summary>
/// Get the number of key/value pairs in the dictionary.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <returns>The number of key/value pairs in the dictionary.</returns>
Inline Int32 VarDict_Count(VarDict varDict)
{
	return ((struct VarDictInt *)varDict)->count;
}

/// <summary>
/// Get a specific value from the dictionary by its key.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>The value for that key (NULL if the key is not found).</returns>
Inline VarInfo VarDict_GetValue(VarDict varDict, Symbol key)
{
	SMILE_DICT_SEARCH(struct VarDictInt, struct VarDictNode, Int32,
		varDict, key, node->key == key,
		{
			return &node->varInfo;
		},
		{
			return NULL;
		})
}

/// <summary>
/// Update a preexisting key/value pair in the dictionary.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <param name="key">The key to update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key was updated, False if it did not exist.</returns>
Inline Bool VarDict_ReplaceValue(VarDict varDict, Symbol key, VarInfo value)
{
	SMILE_DICT_SEARCH(struct VarDictInt, struct VarDictNode, Int32,
		varDict, key, node->key == key,
		{
			node->varInfo = *value;
			return True;
		},
		{
			return False;
		})
}

/// <summary>
/// Create or update a key/value pair in the dictionary.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <param name="key">The key to create or update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key already existed, False if it needed to be added.</returns>
Inline Bool VarDict_SetValue(VarDict varDict, Int32 key, const VarInfo value)
{
	SMILE_DICT_SEARCH(struct VarDictInt, struct VarDictNode, Int32,
		varDict, key, node->key == key,
		{
			node->varInfo = *value;
			return True;
		},
		{
			VarDictInt_Append((struct VarDictInt *)varDict, key, value);
			return False;
		})
}

/// <summary>
/// Try to get a specific value from the dictionary by its key; this function,
/// unlike VarDict_GetValue(), can tell you whether the key was found in addition
/// to returning the value.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <param name="value">This will be set to the value for that key (NULL if the key is not found).</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool VarDict_TryGetValue(VarDict varDict, Int32 key, VarInfo *value)
{
	SMILE_DICT_SEARCH(struct VarDictInt, struct VarDictNode, Int32,
		varDict, key, node->key == key,
		{
			*value = &node->varInfo;
			return True;
		},
		{
			*value = NULL;
			return False;
		})
}

/// <summary>
/// Walk through every node in a VarDict, performing a specific action for each node (in no particular order).
/// </summary>
/// <param name="__varDict__">A pointer to the dictionary itself.</param>
/// <param name="__action__">Code block: What action to perform for each 'node'.</param>
#define VARDICT_WALK(__varDict__, __action__) \
	SMILE_DICT_WALK(struct VarDictInt, struct VarDictNode, Int, &(__varDict__)->_opaque, __action__)

#endif