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
#include <smile/dict/int32dict.h>

//-------------------------------------------------------------------------------------------------
//  Semi-Private interface

/// <summary>
/// Append a new key/value pair to the end of the dictionary.  The key must not already exist
/// as a member of the dictionary.
/// </summary>
/// <param name="self">The casted pointer to the implementation of the dictionary.</param>
/// <param name="key">The key for the new key/value pair to add.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
Int32 Int32DictInt_Append(struct Int32DictInt *self, Int32 key, const void *value)
{
	struct Int32DictNode *heap, *newHeap;
	Int32 nodeIndex, bucketIndex, i, oldmax, newmax;

	if (self->firstFree < 0) {
		oldmax = self->heapLen;
		newmax = oldmax * 2;
		newHeap = GC_MALLOC_STRUCT_ARRAY(struct Int32DictNode, newmax);
		if (newHeap == NULL) Smile_Abort_OutOfMemory();
		MemCpy(newHeap, self->heap, oldmax * sizeof(struct Int32DictNode *));

		for (i = oldmax; i < newmax; i++) {
			newHeap[i].next = i + 1;
			newHeap[i].key = 0;
			newHeap[i].value = NULL;
		}

		newHeap[newmax - 1].next = -1;

		self->heap = newHeap;
		self->firstFree = oldmax;
	}

	heap = self->heap;

	nodeIndex = self->firstFree;
	bucketIndex = key & self->mask;
	self->firstFree = heap[nodeIndex].next;

	heap[nodeIndex].next = self->buckets[bucketIndex];
	heap[nodeIndex].key = key;
	heap[nodeIndex].value = (void *)value;

	self->buckets[bucketIndex] = nodeIndex;
	self->count++;

	return nodeIndex;
}

//-------------------------------------------------------------------------------------------------
//  Public interface

/// <summary>
/// Get all key/value pairs from the dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the key/value pairs in the dictionary, in no specific order.
/// This will be the same length as returned by Int32Dict_GetCount().</returns>
Int32DictKeyValuePair *Int32Dict_GetAll(Int32Dict intDict)
{
	struct Int32DictInt *self;
	Int32 bucket, bucketsLen, nodeIndex;
	Int32 *buckets;
	Int32DictKeyValuePair *pairs, *dest;
	struct Int32DictNode *heap, *node;
	
	self = (struct Int32DictInt *)intDict;

	pairs = GC_MALLOC_STRUCT_ARRAY(Int32DictKeyValuePair, self->count);
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
/// <param name="intDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the keys in the dictionary, in no specific order.
/// This will be the same length as returned by Int32Dict_GetCount().</returns>
Int32 *Int32Dict_GetKeys(Int32Dict intDict)
{
	struct Int32DictInt *self;
	Int32 bucket, bucketsLen, nodeIndex;
	Int32 *buckets;
	Int32 *keys, *dest;
	struct Int32DictNode *heap, *node;

	self = (struct Int32DictInt *)intDict;

	keys = GC_MALLOC_RAW_ARRAY(Int32, self->count);
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
/// <param name="intDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the values in the dictionary, in no specific order.
/// This will be the same length as returned by Int32Dict_GetCount().</returns>
void **Int32Dict_GetValues(Int32Dict intDict)
{
	struct Int32DictInt *self;
	Int32 bucket, bucketsLen, nodeIndex;
	Int32 *buckets;
	void **values, **dest;
	struct Int32DictNode *heap, *node;

	self = (struct Int32DictInt *)intDict;

	values = GC_MALLOC_STRUCT_ARRAY(void *, self->count);
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
void Int32Dict_Clear(Int32Dict intDict)
{
	const Int32 DefaultBucketCount = 16;
	const Int32 DefaultHeapSize = 16;

	struct Int32DictInt *self;
	struct Int32DictNode *heap;
	Int32 *buckets;
	Int32 i;

	self = (struct Int32DictInt *)intDict;

	self->buckets = buckets = GC_MALLOC_RAW_ARRAY(Int32, DefaultBucketCount);
	if (buckets == NULL) Smile_Abort_OutOfMemory();
	self->bucketsLen = DefaultBucketCount;
	self->heap = heap = GC_MALLOC_STRUCT_ARRAY(struct Int32DictNode, DefaultHeapSize);
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
		heap[i].value = NULL;
	}

	heap[DefaultHeapSize - 1].next = -1;
}

/// <summary>
/// Remove a specific key/value pair from the dictionary, by key.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to remove.</param>
/// <returns>True if the key was found (and thus removed), False if the key was not found.</returns>
Bool Int32Dict_Remove(Int32Dict intDict, Int32 key)
{
	struct Int32DictInt *self;
	struct Int32DictNode *heap;
	Int32 *buckets;

	int nodeIndex, prevIndex;

	self = (struct Int32DictInt *)intDict;
	heap = self->heap;
	buckets = self->buckets;

	nodeIndex = buckets[key & self->mask];
	prevIndex = -1;

	while (nodeIndex >= 0) {
		if (heap[nodeIndex].key == key) {
			if (prevIndex >= 0)
				heap[prevIndex].next = heap[nodeIndex].next;
			else
				buckets[key & self->mask] = heap[nodeIndex].next;

			heap[nodeIndex].key = 0;
			heap[nodeIndex].value = NULL;
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
