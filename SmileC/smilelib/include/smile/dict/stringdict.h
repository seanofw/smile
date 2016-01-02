#ifndef __SMILE_DICT_STRINGDICT_H__
#define __SMILE_DICT_STRINGDICT_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_GC_H__
#include <smile/gc.h>
#endif
#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif
#ifndef __SMILE_DICT_SHARED_H__
#include <smile/dict/shared.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Internal types

/// <summary>
/// A single node (key/value pair) within an StringDict.
/// </summary>
struct StringDictNode {
	Int next;					// A pointer to the next node in this bucket (relative to the StringDict's heap).
	String key;					// The key for this node.
	void *value;				// The value for this node.
	UInt32 keyHash;				// The hash code for the key for this node (cached for efficient rebalancing).
};

/// <summary>
/// The internal implementation of an StringDict.
/// </summary>
struct StringDictInt {
	Int *buckets;				// The buckets contain pointers into the heap, indexed by masked hash code.
	struct StringDictNode *heap;	// The heap, which holds all of the key/value pairs as Node structs.
	Int count;					// The number of allocated nodes in the heap.
	Int firstFree;				// The first free node in the heap (successive free nodes follow the 'next' pointers).
	Int mask;					// The current hash mask.  Always equal to 2^n-1 for some n.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A dictionary of key/value pairs, keyed by Strings, with arbitrary pointers as the values.
/// </summary>
typedef struct StringDictStruct {
	struct StringDictInt _opaque;
} *StringDict;

/// <summary>
/// A key/value pair in an StringDict, as returned by StringDict_GetAll().
/// </summary>
typedef struct {
	String key;					// The key for this pair.
	void *value;				// The value for this pair.
} StringDictKeyValuePair;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC Int StringDictInt_Append(struct StringDictInt *stringDict, String key, Int32 keyHash, const void *value);

SMILE_API_FUNC String *StringDict_GetKeys(StringDict stringDict);
SMILE_API_FUNC void **StringDict_GetValues(StringDict stringDict);
SMILE_API_FUNC StringDictKeyValuePair *StringDict_GetAll(StringDict stringDict);

SMILE_API_FUNC void StringDict_ClearWithSize(StringDict stringDict, Int newSize);
SMILE_API_FUNC Bool StringDict_Remove(StringDict stringDict, String key);

SMILE_API_FUNC DictStats StringDict_ComputeStats(StringDict stringDict);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Construct a new, empty dictionary, and control its allocation behavior.
/// </summary>
/// <param name="newSize">The initial allocation size of the dictionary, which is the number of
/// items the dictionary can hold without it needing to invoke another reallocation.</param>
/// <returns>The new, empty dictionary.</returns>
Inline StringDict StringDict_CreateWithSize(Int newSize)
{
	StringDict stringDict;

	stringDict = (StringDict)GC_MALLOC_STRUCT(struct StringDictInt);
	if (stringDict == NULL) Smile_Abort_OutOfMemory();
	StringDict_ClearWithSize(stringDict, newSize);
	return stringDict;
}

/// <summary>
/// Construct a new, empty dictionary.
/// </summary>
/// <returns>The new, empty dictionary.</returns>
Inline StringDict StringDict_Create(void)
{
	return StringDict_CreateWithSize(16);
}

/// <summary>
/// Delete all key/value pairs in the dictionary, resetting it back to an initial state.
/// </summary>
Inline void StringDict_Clear(StringDict stringDict)
{
	StringDict_ClearWithSize(stringDict, 16);
}

/// <summary>
/// Determine if the given key exists in the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool StringDict_ContainsKey(StringDict stringDict, String key)
{
	SMILE_DICT_SEARCH(struct StringDictInt, struct StringDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			return True;
		},
		{
			return False;
		})
}

/// <summary>
/// Determine if the given key exists in the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="ckey">The key to search for.</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool StringDict_ContainsKeyC(StringDict stringDict, const char *ckey)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	return StringDict_ContainsKey(stringDict, (String)&tempStr);
}

/// <summary>
/// Add a new key/value pair to the dictionary.  If the key already exists, this will fail.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key for the new key/value pair to add.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
/// <returns>True if the pair was successfully added, False if the key already existed.</returns>
Inline Bool StringDict_Add(StringDict stringDict, String key, const void *value)
{
	SMILE_DICT_SEARCH(struct StringDictInt, struct StringDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			return False;
		},
		{
			StringDictInt_Append((struct StringDictInt *)stringDict, key, keyHash, value);
			return True;
		})
}

/// <summary>
/// Add a new key/value pair to the dictionary.  If the key already exists, this will fail.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="ckey">The key for the new key/value pair to add.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
/// <returns>True if the pair was successfully added, False if the key already existed.</returns>
Inline Bool StringDict_AddC(StringDict stringDict, const char *ckey, const void *value)
{
	return StringDict_Add(stringDict, String_FromC(ckey), value);
}

/// <summary>
/// Get the number of key/value pairs in the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <returns>The number of key/value pairs in the dictionary.</returns>
Inline Int StringDict_Count(StringDict stringDict)
{
	return ((struct StringDictInt *)stringDict)->count;
}

/// <summary>
/// Get a specific value from the dictionary by its key.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>The value for that key (NULL if the key is not found).</returns>
Inline void *StringDict_GetValue(StringDict stringDict, String key)
{
	SMILE_DICT_SEARCH(struct StringDictInt, struct StringDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			return node->value;
		},
		{
			return NULL;
		})
}

/// <summary>
/// Get a specific value from the dictionary by its key.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="ckey">The key to search for.</param>
/// <returns>The value for that key (NULL if the key is not found).</returns>
Inline void *StringDict_GetValueC(StringDict stringDict, const char *ckey)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	return StringDict_GetValue(stringDict, (String)&tempStr);
}

/// <summary>
/// Update a preexisting key/value pair in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key was updated, False if it did not exist.</returns>
Inline Bool StringDict_ReplaceValue(StringDict stringDict, String key, void *value)
{
	SMILE_DICT_SEARCH(struct StringDictInt, struct StringDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			node->value = value;
			return True;
		},
		{
			return False;
		})
}

/// <summary>
/// Update a preexisting key/value pair in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key was updated, False if it did not exist.</returns>
Inline Int StringDict_ReplaceValueC(StringDict stringDict, const char *ckey, void *value)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	return StringDict_ReplaceValue(stringDict, (String)&tempStr, value);
}

/// <summary>
/// Create or update a key/value pair in the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to create or update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key already existed, False if it needed to be added.</returns>
Inline Bool StringDict_SetValue(StringDict stringDict, String key, void *value)
{
	SMILE_DICT_SEARCH(struct StringDictInt, struct StringDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			node->value = value;
			return True;
		},
		{
			StringDictInt_Append((struct StringDictInt *)stringDict, key, keyHash, value);
			return False;
		})
}

/// <summary>
/// Create or update a key/value pair in the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="ckey">The key to create or update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key already existed, False if it needed to be added.</returns>
Inline Bool StringDict_SetValueC(StringDict stringDict, const char *ckey, void *value)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	StringDict_SetValue(stringDict, (String)&tempStr, value);
}

/// <summary>
/// Try to get a specific value from the dictionary by its key; this function,
/// unlike StringDict_GetValue(), can tell you whether the key was found in addition
/// to returning the value.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <param name="value">This will be set to the value for that key (NULL if the key is not found).</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool StringDict_TryGetValue(StringDict stringDict, String key, void **value)
{
	SMILE_DICT_SEARCH(struct StringDictInt, struct StringDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			*value = node->value;
			return True;
		},
		{
			*value = NULL;
			return False;
		})
}

/// <summary>
/// Try to get a specific value from the dictionary by its key; this function,
/// unlike StringDict_GetValueC(), can tell you whether the key was found in addition
/// to returning the value.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="ckey">The key to search for.</param>
/// <param name="value">This will be set to the value for that key (NULL if the key is not found).</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool StringDict_TryGetValueC(StringDict stringDict, const char *ckey, void **value)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	return StringDict_TryGetValue(stringDict, (String)&tempStr, value);
}

#endif