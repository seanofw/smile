//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

#include <smile/gc.h>
#include <smile/mem.h>
#include <smile/dict/stringintdict.h>

//-------------------------------------------------------------------------------------------------
//  Semi-Private interface

/// <summary>
/// Append a new key/value pair to the end of the dictionary.  The key must not already exist
/// as a member of the dictionary.
/// </summary>
/// <param name="self">The casted pointer to the implementation of the dictionary.</param>
/// <param name="key">The key for the new key/value pair to add.</param>
/// <param name="keyHash">The precomputed hash code for that key.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
Int StringIntDictInt_Append(struct StringIntDictInt *self, String key, Int32 keyHash, Int value)
{
	struct StringIntDictNode *heap, *newHeap;
	Int nodeIndex, bucketIndex, i, oldmax, newmax;

	if (self->firstFree < 0) {
		oldmax = self->heapLen;
		newmax = oldmax * 2;
		newHeap = GC_MALLOC_STRUCT_ARRAY(struct StringIntDictNode, newmax);
		if (newHeap == NULL) Smile_Abort_OutOfMemory();
		MemCpy(newHeap, self->heap, oldmax * sizeof(struct StringIntDictNode *));

		for (i = oldmax; i < newmax; i++) {
			newHeap[i].next = i + 1;
			newHeap[i].key = 0;
			newHeap[i].value = 0;
		}

		newHeap[newmax - 1].next = -1;

		self->heap = newHeap;
		self->firstFree = oldmax;
	}

	heap = self->heap;

	nodeIndex = self->firstFree;
	bucketIndex = keyHash & self->mask;
	self->firstFree = heap[nodeIndex].next;

	heap[nodeIndex].next = self->buckets[bucketIndex];
	heap[nodeIndex].key = key;
	heap[nodeIndex].value = value;

	self->buckets[bucketIndex] = nodeIndex;
	self->count++;

	return nodeIndex;
}

//-------------------------------------------------------------------------------------------------
//  Public interface

/// <summary>
/// Get all key/value pairs from the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the key/value pairs in the dictionary, in no specific order.
/// This will be the same length as returned by StringIntDict_GetCount().</returns>
StringIntDictKeyValuePair *StringIntDict_GetAll(StringIntDict stringDict)
{
	struct StringIntDictInt *self;
	Int bucket, bucketsLen, nodeIndex;
	Int *buckets;
	StringIntDictKeyValuePair *pairs, *dest;
	struct StringIntDictNode *heap, *node;
	
	self = (struct StringIntDictInt *)stringDict;

	pairs = GC_MALLOC_STRUCT_ARRAY(StringIntDictKeyValuePair, self->count);
	if (pairs == NULL) Smile_Abort_OutOfMemory();

	bucketsLen = self->bucketsLen;
	buckets = self->buckets;
	dest = pairs;
	heap = self->heap;

	for (bucket = 0; bucket < self->bucketsLen; bucket++) {
		nodeIndex = buckets[bucket];
		while (nodeIndex >= 0) {
			node = heap + nodeIndex;
			dest->key = node->key;
			dest->value = node->value;
			dest++;
			nodeIndex = node->next;
		}
	}

	return pairs;
}

/// <summary>
/// Get all keys from the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the keys in the dictionary, in no specific order.
/// This will be the same length as returned by StringIntDict_GetCount().</returns>
String *StringIntDict_GetKeys(StringIntDict stringDict)
{
	struct StringIntDictInt *self;
	Int bucket, bucketsLen, nodeIndex;
	Int *buckets;
	String *keys, *dest;
	struct StringIntDictNode *heap, *node;

	self = (struct StringIntDictInt *)stringDict;

	keys = GC_MALLOC_STRUCT_ARRAY(String, self->count);
	if (keys == NULL) Smile_Abort_OutOfMemory();

	bucketsLen = self->bucketsLen;
	buckets = self->buckets;
	dest = keys;
	heap = self->heap;

	for (bucket = 0; bucket < self->bucketsLen; bucket++) {
		nodeIndex = buckets[bucket];
		while (nodeIndex >= 0) {
			node = heap + nodeIndex;
			*dest++ = node->key;
			nodeIndex = node->next;
		}
	}

	return keys;
}

/// <summary>
/// Get all values from the dictionary.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the values in the dictionary, in no specific order.
/// This will be the same length as returned by StringIntDict_GetCount().</returns>
Int *StringIntDict_GetValues(StringIntDict stringDict)
{
	struct StringIntDictInt *self;
	Int bucket, bucketsLen, nodeIndex;
	Int *buckets;
	Int *values, *dest;
	struct StringIntDictNode *heap, *node;

	self = (struct StringIntDictInt *)stringDict;

	values = GC_MALLOC_RAW_ARRAY(Int, self->count);
	if (values == NULL) Smile_Abort_OutOfMemory();

	bucketsLen = self->bucketsLen;
	buckets = self->buckets;
	dest = values;
	heap = self->heap;

	for (bucket = 0; bucket < self->bucketsLen; bucket++) {
		nodeIndex = buckets[bucket];
		while (nodeIndex >= 0) {
			node = heap + nodeIndex;
			*dest++ = node->value;
			nodeIndex = node->next;
		}
	}

	return values;
}

/// <summary>
/// Delete all key/value pairs in the dictionary, resetting it back to its initial state.
/// </summary>
void StringIntDict_Clear(StringIntDict stringDict)
{
	const Int DefaultBucketCount = 16;
	const Int DefaultHeapSize = 16;

	struct StringIntDictInt *self;
	struct StringIntDictNode *heap;
	Int *buckets;
	Int i;

	self = (struct StringIntDictInt *)stringDict;

	self->buckets = buckets = GC_MALLOC_RAW_ARRAY(Int, DefaultBucketCount);
	if (buckets == NULL) Smile_Abort_OutOfMemory();
	self->bucketsLen = DefaultBucketCount;
	self->heap = heap = GC_MALLOC_STRUCT_ARRAY(struct StringIntDictNode, DefaultHeapSize);
	if (heap == NULL) Smile_Abort_OutOfMemory();
	self->heapLen = DefaultHeapSize;
	self->firstFree = 0;
	self->count = 0;
	self->mask = DefaultHeapSize - 1;

	for (i = 0; i < DefaultBucketCount; i++) {
		buckets[i] = -1;
	}

	for (i = 0; i < (DefaultHeapSize - 1); i++) {
		heap[i].next = i + 1;
		heap[i].key = 0;
		heap[i].value = 0;
	}

	heap[DefaultHeapSize - 1].next = -1;
}

/// <summary>
/// Remove a specific key/value pair from the dictionary, by key.
/// </summary>
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to remove.</param>
/// <returns>True if the key was found (and thus removed), False if the key was not found.</returns>
Bool StringIntDict_Remove(StringIntDict stringDict, String key)
{
	struct StringIntDictInt *self;
	struct StringIntDictNode *heap;
	Int *buckets;
	Int32 keyHash;

	Int nodeIndex, prevIndex;

	self = (struct StringIntDictInt *)stringDict;
	heap = self->heap;
	buckets = self->buckets;
	
	keyHash = String_GetHashCode(key);

	nodeIndex = buckets[keyHash & self->mask];
	prevIndex = -1;

	while (nodeIndex >= 0) {
		if (String_Equals(heap[nodeIndex].key, key)) {
			if (prevIndex >= 0)
				heap[prevIndex].next = heap[nodeIndex].next;
			else
				buckets[keyHash & self->mask] = heap[nodeIndex].next;

			heap[nodeIndex].key = NULL;
			heap[nodeIndex].value = 0;
			heap[nodeIndex].next = self->firstFree;

			self->firstFree = nodeIndex;
			self->count--;

			return True;
		}

		prevIndex = nodeIndex;
		nodeIndex = heap[nodeIndex].next;
	}

	return False;
}
