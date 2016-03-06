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

#include <math.h>

#include <smile/gc.h>
#include <smile/mem.h>
#include <smile/bittwiddling.h>
#include <smile/dict/int32int32dict.h>

//-------------------------------------------------------------------------------------------------
//  Private functions

static void Int32Int32DictInt_Resize(struct Int32Int32DictInt *self, Int32 newLen)
{
	struct Int32Int32DictNode *newHeap, *oldHeap;
	Int32 *newBuckets, *oldBuckets;
	Int32 i, oldLen, newMask, oldBucketIndex, newBucketIndex, oldNodeIndex;

	// Get the old heap info.
	oldLen = self->mask + 1;
	oldHeap = self->heap;
	oldBuckets = self->buckets;

	// Construct a new heap and buckets twice as large as the old ones.
	if (newLen >= IntMax || newLen >= Int32Max / sizeof(struct Int32Int32DictNode)) Smile_Abort_OutOfMemory();
	newBuckets = GC_MALLOC_RAW_ARRAY(Int32, newLen);
	if (newBuckets == NULL) Smile_Abort_OutOfMemory();
	newHeap = GC_MALLOC_STRUCT_ARRAY(struct Int32Int32DictNode, newLen);
	if (newHeap == NULL) Smile_Abort_OutOfMemory();

	// The new buckets start out empty.  This runs in O(n) time.
	for (i = 0; i < newLen; i++) {
		newBuckets[i] = -1;
	}

	// Spin over the nodes in the old heap and insert them into a new set of buckets twice as large
	// as the old set of buckets, which keeps the average bucket size <= 1 node, guaranteeing amortized
	// O(1) read time.
	newMask = newLen - 1;
	i = 0;

	// The outer loop spins over O(n) buckets.
	for (oldBucketIndex = 0; oldBucketIndex < oldLen; oldBucketIndex++) {

		// This inner loop runs in O(1) amortized time, assuming reasonable random distribution of the keys.
		for (oldNodeIndex = oldBuckets[oldBucketIndex]; oldNodeIndex >= 0; oldNodeIndex = oldHeap[oldNodeIndex].next) {
			newBucketIndex = oldHeap[oldNodeIndex].key & newMask;

			newHeap[i].next = newBuckets[newBucketIndex];
			newHeap[i].key = oldHeap[oldNodeIndex].key;
			newHeap[i].value = oldHeap[oldNodeIndex].value;
			newBuckets[newBucketIndex] = i++;

			oldHeap[oldNodeIndex].value = 0;
		}
	}

	// The new buckets and new heap are now the canonical data, so replace this dictionary's content
	// with them.
	self->heap = newHeap;
	self->buckets = newBuckets;
	self->mask = newMask;

	// Everything has been inserted into the new buckets and new heap, but the new heap still needs
	// the rest of its free nodes to be created, if there's any unused space at the end.  This runs
	// in O(n) time, since the callers guarantee the number of free nodes is always proportional to
	// the number of allocated nodes.
	self->firstFree = (i < newLen ? i : -1);
	for (; i < newLen - 1; i++) {
		newHeap[i].next = i + 1;
		newHeap[i].key = 0;
		newHeap[i].value = 0;
	}
	newHeap[i].next = -1;
	newHeap[i].key = 0;
	newHeap[i].value = 0;
}

//-------------------------------------------------------------------------------------------------
//  Semi-Private interface

/// <summary>
/// Append a new key/value pair to the end of the dictionary.  The key must not already exist
/// as a member of the dictionary.
/// </summary>
/// <param name="self">The casted pointer to the implementation of the dictionary.</param>
/// <param name="key">The key for the new key/value pair to add.</param>
/// <param name="value">The value for the new key/value pair to add.</param>
Int32 Int32Int32DictInt_Append(struct Int32Int32DictInt *self, Int32 key, Int32 value)
{
	struct Int32Int32DictNode *heap;
	Int32 nodeIndex, bucketIndex;

	if (self->firstFree < 0) {
		Int32Int32DictInt_Resize(self, (self->mask + 1) * 2);
	}

	heap = self->heap;

	nodeIndex = self->firstFree;
	bucketIndex = key & self->mask;
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
/// <param name="intDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the key/value pairs in the dictionary, in no specific order.
/// This will be the same length as returned by Int32Int32Dict_GetCount().</returns>
Int32Int32DictKeyValuePair *Int32Int32Dict_GetAll(Int32Int32Dict intDict)
{
	struct Int32Int32DictInt *self;
	Int32 bucket, nodeIndex;
	Int32 *buckets;
	Int32Int32DictKeyValuePair *pairs, *dest;
	struct Int32Int32DictNode *heap, *node;
	
	self = (struct Int32Int32DictInt *)intDict;

	pairs = GC_MALLOC_STRUCT_ARRAY(Int32Int32DictKeyValuePair, self->count);
	if (pairs == NULL) Smile_Abort_OutOfMemory();

	buckets = self->buckets;
	dest = pairs;
	heap = self->heap;

	for (bucket = 0; bucket <= self->mask; bucket++) {
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
/// This will be the same length as returned by Int32Int32Dict_GetCount().</returns>
Int32 *Int32Int32Dict_GetKeys(Int32Int32Dict intDict)
{
	struct Int32Int32DictInt *self;
	Int32 bucket, nodeIndex;
	Int32 *buckets;
	Int32 *keys, *dest;
	struct Int32Int32DictNode *heap, *node;

	self = (struct Int32Int32DictInt *)intDict;

	keys = GC_MALLOC_RAW_ARRAY(Int32, self->count);
	if (keys == NULL) Smile_Abort_OutOfMemory();

	buckets = self->buckets;
	dest = keys;
	heap = self->heap;

	for (bucket = 0; bucket <= self->mask; bucket++) {
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
/// This will be the same length as returned by Int32Int32Dict_GetCount().</returns>
Int32 *Int32Int32Dict_GetValues(Int32Int32Dict intDict)
{
	struct Int32Int32DictInt *self;
	Int32 bucket, nodeIndex;
	Int32 *buckets;
	Int32 *values, *dest;
	struct Int32Int32DictNode *heap, *node;

	self = (struct Int32Int32DictInt *)intDict;

	values = GC_MALLOC_STRUCT_ARRAY(Int32, self->count);
	if (values == NULL) Smile_Abort_OutOfMemory();

	buckets = self->buckets;
	dest = values;
	heap = self->heap;

	for (bucket = 0; bucket <= self->mask; bucket++) {
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
/// <param name="newSize">The new allocation size of the dictionary, which is the number of
/// items the dictionary can hold without it needing to invoke another reallocation.</param>
void Int32Int32Dict_ClearWithSize(Int32Int32Dict intDict, Int32 newSize)
{
	struct Int32Int32DictInt *self;
	struct Int32Int32DictNode *heap;
	Int32 *buckets;
	Int32 i;

	if (newSize < 0x10) newSize = 0x10;
	if (newSize > 0x1000000) newSize = 0x1000000;

	newSize = NextPowerOfTwo32(newSize);

	self = (struct Int32Int32DictInt *)intDict;

	self->buckets = buckets = GC_MALLOC_RAW_ARRAY(Int32, newSize);
	if (buckets == NULL) Smile_Abort_OutOfMemory();
	self->heap = heap = GC_MALLOC_STRUCT_ARRAY(struct Int32Int32DictNode, newSize);
	if (heap == NULL) Smile_Abort_OutOfMemory();
	self->firstFree = 0;
	self->count = 0;
	self->mask = newSize - 1;

	for (i = 0; i < newSize; i++) {
		buckets[i] = -1;
	}

	for (i = 0; i < newSize - 1; i++) {
		heap[i].next = i + 1;
		heap[i].key = 0;
		heap[i].value = 0;
	}

	heap[i].next = -1;
	heap[i].key = 0;
	heap[i].value = 0;
}

/// <summary>
/// Remove a specific key/value pair from the dictionary, by key.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
/// <param name="key">The key to remove.</param>
/// <returns>True if the key was found (and thus removed), False if the key was not found.</returns>
Bool Int32Int32Dict_Remove(Int32Int32Dict intDict, Int32 key)
{
	struct Int32Int32DictInt *self;
	struct Int32Int32DictNode *heap;
	Int32 *buckets;
	Int32 nodeIndex, prevIndex;
	Int32 mask;

	self = (struct Int32Int32DictInt *)intDict;
	heap = self->heap;
	buckets = self->buckets;

	mask = self->mask;
	nodeIndex = buckets[key & mask];
	prevIndex = -1;

	while (nodeIndex >= 0) {
		if (heap[nodeIndex].key == key) {
			if (prevIndex >= 0)
				heap[prevIndex].next = heap[nodeIndex].next;
			else
				buckets[key & mask] = heap[nodeIndex].next;

			heap[nodeIndex].key = 0;
			heap[nodeIndex].value = 0;
			heap[nodeIndex].next = self->firstFree;

			self->firstFree = nodeIndex;
			self->count--;

			if (self->count <= ((mask + 1) >> 2) && (mask + 1) > 16) {
				Int32Int32DictInt_Resize(self, (mask + 1) >> 1);
			}
			return True;
		}

		prevIndex = nodeIndex;
		nodeIndex = heap[nodeIndex].next;
	}

	return False;
}

/// <summary>
/// Compute statistics on this dictionary.
/// </summary>
/// <param name="intDict">A pointer to the dictionary.</param>
SMILE_API_FUNC DictStats Int32Int32Dict_ComputeStats(Int32Int32Dict intDict)
{
	struct Int32Int32DictInt *self;
	struct Int32Int32DictNode *heap;
	Int32 bucketIndex, nodeIndex, numInThisBucket;
	Int32 *buckets;
	DictStats stats;

	self = (struct Int32Int32DictInt *)intDict;
	heap = self->heap;
	buckets = self->buckets;

	stats = GC_MALLOC_STRUCT(struct DictStatsStruct);

	stats->heapTotal = self->mask + 1;
	stats->heapAlloc = self->count;
	stats->heapFree = self->mask + 1 - self->count;

	stats->bucketStats = SimpleStats_Create();
	stats->keyStats = SimpleStats_Create();

	for (bucketIndex = 0; bucketIndex <= self->mask; bucketIndex++) {
		numInThisBucket = 0;
		for (nodeIndex = buckets[bucketIndex]; nodeIndex >= 0; nodeIndex = heap[nodeIndex].next) {
			numInThisBucket++;
			SimpleStats_Add(stats->keyStats, 1);
		}
		SimpleStats_Add(stats->bucketStats, numInThisBucket);
	}

	return stats;
}
