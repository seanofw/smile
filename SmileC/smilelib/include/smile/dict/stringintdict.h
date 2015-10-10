#ifndef __SMILE_DICT_STRINGINTDICT_H__
#define __SMILE_DICT_STRINGINTDICT_H__

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
/// A single node (key/value pair) within an StringIntDict.
/// </summary>
struct StringIntDictNode {
	Int next;					// A pointer to the next node in this bucket (relative to the StringIntDict's heap).
	String key;					// The key for this node.
	Int value;					// The value for this node.
	UInt32 keyHash;				// The hash code for the key for this node (cached for efficient rebalancing).
};

/// <summary>
/// The internal implementation of an StringIntDict.
/// </summary>
struct StringIntDictInt {
	Int *buckets;				// The buckets contain pointers into the heap, indexed by masked hash code.
	struct StringIntDictNode *heap;	// The heap, which holds all of the key/value pairs as Node structs.
	Int count;					// The number of allocated nodes in the heap.
	Int firstFree;				// The first free node in the heap (successive free nodes follow the 'next' pointers).
	Int mask;					// The current hash mask.  Always equal to 2^n-1 for some n.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A dictionary of key/value pairs, keyed by Strings, with arbitrary pointers as the values.
/// </summary>
typedef struct {
	struct StringIntDictInt _opaque;
} *StringIntDict;

/// <summary>
/// A key/value pair in an StringIntDict, as returned by StringIntDict_GetAll().
/// </summary>
typedef struct {
	String key;					// The key for this pair.
	Int value;					// The value for this pair.
} StringIntDictKeyValuePair;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API Int StringIntDictInt_Append(struct StringIntDictInt *stringDict, String key, Int32 keyHash, Int value);

SMILE_API String *StringIntDict_GetKeys(StringIntDict stringDict);
SMILE_API Int *StringIntDict_GetValues(StringIntDict stringDict);
SMILE_API StringIntDictKeyValuePair *StringIntDict_GetAll(StringIntDict stringDict);

SMILE_API void StringIntDict_ClearWithSize(StringIntDict stringDict, Int newSize);
SMILE_API Bool StringIntDict_Remove(StringIntDict stringDict, String key);

SMILE_API DictStats StringIntDict_ComputeStats(StringIntDict stringDict);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Construct a new, empty dictionary, and control its allocation behavior.
/// </summary>
/// <param name="newSize">The initial allocation size of the dictionary, which is the number of
/// items the dictionary can hold without it needing to invoke another reallocation.</param>
/// <returns>The new, empty dictionary.</returns>
Inline StringIntDict StringIntDict_CreateWithSize(Int newSize)
{
	StringIntDict stringDict;

	stringDict = (StringIntDict)GC_MALLOC_STRUCT(struct StringIntDictInt);
	if (stringDict == NULL) Smile_Abort_OutOfMemory();
	StringIntDict_ClearWithSize(stringDict, newSize);
	return stringDict;
}

/// <summary>
/// Construct a new, empty dictionary.
/// </summary>
/// <returns>The new, empty dictionary.</returns>
Inline StringIntDict StringIntDict_Create(void)
{
	return StringIntDict_CreateWithSize(16);
}

/// <summary>
/// Delete all key/value pairs in the dictionary, resetting it back to an initial state.
/// </summary>
Inline void StringIntDict_Clear(StringIntDict stringDict)
{
	StringIntDict_ClearWithSize(stringDict, 16);
}

/// <summary>
/// Determine if the given key exists in the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool StringIntDict_ContainsKey(StringIntDict stringDict, String key)
{
	SMILE_DICT_SEARCH(struct StringIntDictInt, struct StringIntDictNode, Int,
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
Inline Bool StringIntDict_ContainsKeyC(StringIntDict stringDict, const char *ckey)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	return StringIntDict_ContainsKey(stringDict, (String)&tempStr);
}

/// <summary>
/// Add a new key/value pair to the dictionary.  If the key already exists, this will fail.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key for the new key/value pair to add.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
/// <returns>True if the pair was successfully added, False if the key already existed.</returns>
Inline Bool StringIntDict_Add(StringIntDict stringDict, String key, Int value)
{
	SMILE_DICT_SEARCH(struct StringIntDictInt, struct StringIntDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			return False;
		},
		{
			StringIntDictInt_Append((struct StringIntDictInt *)stringDict, key, keyHash, value);
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
Inline Bool StringIntDict_AddC(StringIntDict stringDict, const char *ckey, Int value)
{
	return StringIntDict_Add(stringDict, String_FromC(ckey), value);
}

/// <summary>
/// Get the number of key/value pairs in the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <returns>The number of key/value pairs in the dictionary.</returns>
Inline Int StringIntDict_Count(StringIntDict stringDict)
{
	return ((struct StringIntDictInt *)stringDict)->count;
}

/// <summary>
/// Get a specific value from the dictionary by its key.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <returns>The value for that key (0 if the key is not found).</returns>
Inline Int StringIntDict_GetValue(StringIntDict stringDict, String key)
{
	SMILE_DICT_SEARCH(struct StringIntDictInt, struct StringIntDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			return node->value;
		},
		{
			return 0;
		})
}

/// <summary>
/// Get a specific value from the dictionary by its key.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="ckey">The key to search for.</param>
/// <returns>The value for that key (0 if the key is not found).</returns>
Inline Int StringIntDict_GetValueC(StringIntDict stringDict, const char *ckey)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	return StringIntDict_GetValue(stringDict, (String)&tempStr);
}

/// <summary>
/// Update a preexisting key/value pair in the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key was updated, False if it did not exist.</returns>
Inline Bool StringIntDict_ReplaceValue(StringIntDict stringDict, String key, Int value)
{
	SMILE_DICT_SEARCH(struct StringIntDictInt, struct StringIntDictNode, Int,
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
Inline Int StringIntDict_ReplaceValueC(StringIntDict stringDict, const char *ckey, Int value)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	return StringIntDict_ReplaceValue(stringDict, (String)&tempStr, value);
}

/// <summary>
/// Create or update a key/value pair in the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to create or update.</param>
/// <param name="value">The new value for that key.</param>
/// <returns>True if the key already existed, False if it needed to be added.</returns>
Inline Bool StringIntDict_SetValue(StringIntDict stringDict, String key, Int value)
{
	SMILE_DICT_SEARCH(struct StringIntDictInt, struct StringIntDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
		{
			node->value = value;
			return True;
		},
		{
			StringIntDictInt_Append((struct StringIntDictInt *)stringDict, key, keyHash, value);
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
Inline Bool StringIntDict_SetValueC(StringIntDict stringDict, const char *ckey, Int value)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	StringIntDict_SetValue(stringDict, (String)&tempStr, value);
}

/// <summary>
/// Try to get a specific value from the dictionary by its key; this function,
/// unlike StringIntDict_GetValue(), can tell you whether the key was found in addition
/// to returning the value.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to search for.</param>
/// <param name="value">This will be set to the value for that key (0 if the key is not found).</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool StringIntDict_TryGetValue(StringIntDict stringDict, String key, Int *value)
{
	SMILE_DICT_SEARCH(struct StringIntDictInt, struct StringIntDictNode, Int,
		stringDict, String_Hash(key), String_Equals(node->key, key),
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
/// Try to get a specific value from the dictionary by its key; this function,
/// unlike StringIntDict_GetValueC(), can tell you whether the key was found in addition
/// to returning the value.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="ckey">The key to search for.</param>
/// <param name="value">This will be set to the value for that key (0 if the key is not found).</param>
/// <returns>True if the key was found, False if the key was not found.</returns>
Inline Bool StringIntDict_TryGetValueC(StringIntDict stringDict, const char *ckey, Int *value)
{
	struct StringInt tempStr;
	tempStr.text = (Byte *)ckey;
	tempStr.length = StrLen(ckey);
	return StringIntDict_TryGetValue(stringDict, (String)&tempStr, value);
}

#endif