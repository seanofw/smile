//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2017 Sean Werkema
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
#include <smile/dict/vardict.h>
#include <smile/smiletypes/smilenull.h>

//-------------------------------------------------------------------------------------------------
//  Private functions

static void VarDictInt_Resize(struct VarDictInt *self, Int32 newLen)
{
	struct VarDictNode *newHeap, *oldHeap;
	Int32 *newBuckets, *oldBuckets;
	Int32 i, oldLen, newMask, oldBucketIndex, newBucketIndex, oldNodeIndex;

	// Get the old heap info.
	oldLen = self->mask + 1;
	oldHeap = self->heap;
	oldBuckets = self->buckets;

	// Construct a new heap and buckets twice as large as the old ones.
	if (newLen >= IntMax || newLen >= Int32Max / sizeof(struct VarDictNode)) Smile_Abort_OutOfMemory();
	newBuckets = GC_MALLOC_RAW_ARRAY(Int32, newLen);
	if (newBuckets == NULL) Smile_Abort_OutOfMemory();
	newHeap = GC_MALLOC_STRUCT_ARRAY(struct VarDictNode, newLen);
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
			newHeap[i].varInfo = oldHeap[oldNodeIndex].varInfo;
			newBuckets[newBucketIndex] = i++;

			oldHeap[oldNodeIndex].varInfo.value = NullObject;		// Help the GC out by breaking references.
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
		newHeap[i].varInfo.kind = 0;
		newHeap[i].varInfo.offset = 0;
		newHeap[i].varInfo.symbol = 0;
		newHeap[i].varInfo.value = NULL;
	}
	newHeap[i].next = -1;
	newHeap[i].key = 0;
	newHeap[i].varInfo.kind = 0;
	newHeap[i].varInfo.offset = 0;
	newHeap[i].varInfo.symbol = 0;
	newHeap[i].varInfo.value = NULL;
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
Int32 VarDictInt_Append(struct VarDictInt *self, Symbol key, const VarInfo value)
{
	struct VarDictNode *heap;
	Int32 nodeIndex, bucketIndex;

	if (self->firstFree < 0) {
		VarDictInt_Resize(self, (self->mask + 1) * 2);
	}

	heap = self->heap;

	nodeIndex = self->firstFree;
	bucketIndex = key & self->mask;
	self->firstFree = heap[nodeIndex].next;

	heap[nodeIndex].next = self->buckets[bucketIndex];
	heap[nodeIndex].key = key;
	heap[nodeIndex].varInfo = *value;

	self->buckets[bucketIndex] = nodeIndex;
	self->count++;

	return nodeIndex;
}

//-------------------------------------------------------------------------------------------------
//  Public interface

/// <summary>
/// Make a perfect clone of this dictionary.
/// </summary>
/// <param name="varDict">The dictionary to make a clone of.</param>
/// <returns>The cloned dictionary.</returns>
VarDict VarDict_Clone(VarDict varDict)
{
	struct VarDictInt *newIntDict;
	struct VarDictInt *oldIntDict = (struct VarDictInt *)varDict;
	Int32 newSize, bucket, nodeIndex, nextNodeIndex;
	Int32 *buckets;
	struct VarDictNode *oldHeap, *newHeap;

	newIntDict = GC_MALLOC_STRUCT(struct VarDictInt);
	if (newIntDict == NULL) Smile_Abort_OutOfMemory();

	newSize = oldIntDict->mask + 1;

	newIntDict->count = oldIntDict->count;
	newIntDict->firstFree = oldIntDict->firstFree;
	newIntDict->mask = oldIntDict->mask;

	newIntDict->buckets = buckets = (Int32 *)GC_MALLOC_ATOMIC(sizeof(Int32) * newSize);
	if (buckets == NULL) Smile_Abort_OutOfMemory();

	MemCpy(buckets, oldIntDict->buckets, sizeof(Int32) * newSize);

	newIntDict->heap = newHeap = GC_MALLOC_STRUCT_ARRAY(struct VarDictNode, newSize);
	if (newHeap == NULL) Smile_Abort_OutOfMemory();

	oldHeap = oldIntDict->heap;

	for (bucket = 0; bucket <= oldIntDict->mask; bucket++) {
		nodeIndex = buckets[bucket];
		while (nodeIndex >= 0) {
			newHeap[nodeIndex].key = oldHeap[nodeIndex].key;
			newHeap[nodeIndex].varInfo = oldHeap[nodeIndex].varInfo;
			newHeap[nodeIndex].next = nextNodeIndex = oldHeap[nodeIndex].next;
			nodeIndex = nextNodeIndex;
		}
	}

	nodeIndex = oldIntDict->firstFree;

	while (nodeIndex >= 0) {
		newHeap[nodeIndex].key = oldHeap[nodeIndex].key;
		newHeap[nodeIndex].varInfo = oldHeap[nodeIndex].varInfo;
		newHeap[nodeIndex].next = nextNodeIndex = oldHeap[nodeIndex].next;
		nodeIndex = nextNodeIndex;
	}

	return (VarDict)newIntDict;
}

/// <summary>
/// Get the "first" key/value pair from the dictionary.  As dictionaries are UNORDERED,
/// this function is really only useful if you have a dictionary that you know contains
/// one item.  This will run in AMORTIZED O(1) time, and does not allocate heap memory.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <returns>The "first" key/value pairs in the dictionary.  If the dictionary contains
/// more than one key/value pair, this will be a randomly-chosen entry.  If the dictionary
/// is empty, this will contain "0" and "NULL" for the key and value, respectively.</returns>
VarDictKeyValuePair VarDict_GetFirst(VarDict varDict)
{
	struct VarDictInt *self;
	Int32 bucket, nodeIndex;
	Int32 *buckets;
	VarDictKeyValuePair result;
	struct VarDictNode *heap, *node;

	self = (struct VarDictInt *)varDict;

	buckets = self->buckets;
	heap = self->heap;

	for (bucket = 0; bucket <= self->mask; bucket++) {
		nodeIndex = buckets[bucket];
		while (nodeIndex >= 0) {
			node = heap + nodeIndex;

			result.key = node->key;
			result.value = &node->varInfo;
			return result;
		}
	}

	result.key = 0;
	result.value = NULL;
	return result;
}

/// <summary>
/// Get all key/value pairs from the dictionary.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the key/value pairs in the dictionary, in no specific order.
/// This will be the same length as returned by VarDict_GetCount().</returns>
VarDictKeyValuePair *VarDict_GetAll(VarDict varDict)
{
	struct VarDictInt *self;
	Int32 bucket, nodeIndex;
	Int32 *buckets;
	VarDictKeyValuePair *pairs, *dest;
	struct VarDictNode *heap, *node;

	self = (struct VarDictInt *)varDict;

	pairs = GC_MALLOC_STRUCT_ARRAY(VarDictKeyValuePair, self->count);
	if (pairs == NULL) Smile_Abort_OutOfMemory();

	buckets = self->buckets;
	dest = pairs;
	heap = self->heap;

	for (bucket = 0; bucket <= self->mask; bucket++) {
		nodeIndex = buckets[bucket];
		while (nodeIndex >= 0) {
			node = heap + nodeIndex;
			dest->key = node->key;
			dest->value = &node->varInfo;
			dest++;
			nodeIndex = node->next;
		}
	}

	return pairs;
}

/// <summary>
/// Get all keys from the dictionary.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the keys in the dictionary, in no specific order.
/// This will be the same length as returned by VarDict_GetCount().</returns>
Symbol *VarDict_GetKeys(VarDict varDict)
{
	struct VarDictInt *self;
	Int32 bucket, nodeIndex;
	Int32 *buckets;
	Symbol *keys, *dest;
	struct VarDictNode *heap, *node;

	self = (struct VarDictInt *)varDict;

	keys = GC_MALLOC_RAW_ARRAY(Symbol, self->count);
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
/// <param name="varDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the values in the dictionary, in no specific order.
/// This will be the same length as returned by VarDict_GetCount().</returns>
VarInfo *VarDict_GetValues(VarDict varDict)
{
	struct VarDictInt *self;
	Int32 bucket, nodeIndex;
	Int32 *buckets;
	VarInfo *values, *dest;
	struct VarDictNode *heap, *node;

	self = (struct VarDictInt *)varDict;

	values = GC_MALLOC_STRUCT_ARRAY(VarInfo, self->count);
	if (values == NULL) Smile_Abort_OutOfMemory();

	buckets = self->buckets;
	dest = values;
	heap = self->heap;

	for (bucket = 0; bucket <= self->mask; bucket++) {
		nodeIndex = buckets[bucket];
		while (nodeIndex >= 0) {
			node = heap + nodeIndex;
			*dest++ = &node->varInfo;
			nodeIndex = node->next;
		}
	}

	return values;
}

Bool VarDict_ForEach(VarDict varDict, Bool (*func)(VarInfo varInfo, void *param), void *param)
{
	struct VarDictInt *self;
	Int32 bucket, nodeIndex;
	Int32 *buckets;
	struct VarDictNode *heap, *node;

	self = (struct VarDictInt *)varDict;

	buckets = self->buckets;
	heap = self->heap;

	for (bucket = 0; bucket <= self->mask; bucket++) {
		nodeIndex = buckets[bucket];
		while (nodeIndex >= 0) {
			node = heap + nodeIndex;
			if (!func(&node->varInfo, param))
				return False;
			nodeIndex = node->next;
		}
	}

	return True;
}

/// <summary>
/// Delete all key/value pairs in the dictionary, resetting it back to its initial state.
/// </summary>
/// <param name="newSize">The new allocation size of the dictionary, which is the number of
/// items the dictionary can hold without it needing to invoke another reallocation.</param>
void VarDict_ClearWithSize(VarDict varDict, Int32 newSize)
{
	struct VarDictInt *self;
	struct VarDictNode *heap;
	Int32 *buckets;
	Int32 i;

	if (newSize < 0x10) newSize = 0x10;
	if (newSize > 0x1000000) newSize = 0x1000000;

	newSize = NextPowerOfTwo32(newSize);

	self = (struct VarDictInt *)varDict;

	self->buckets = buckets = GC_MALLOC_RAW_ARRAY(Int32, newSize);
	if (buckets == NULL) Smile_Abort_OutOfMemory();
	self->heap = heap = GC_MALLOC_STRUCT_ARRAY(struct VarDictNode, newSize);
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
		heap[i].varInfo.kind = 0;
		heap[i].varInfo.offset = 0;
		heap[i].varInfo.symbol = 0;
		heap[i].varInfo.value = NullObject;
	}

	heap[i].next = -1;
	heap[i].key = 0;
	heap[i].varInfo.kind = 0;
	heap[i].varInfo.offset = 0;
	heap[i].varInfo.symbol = 0;
	heap[i].varInfo.value = NullObject;
}

/// <summary>
/// Remove a specific key/value pair from the dictionary, by key.
/// </summary>
/// <param name="varDict">A pointer to the dictionary.</param>
/// <param name="key">The key to remove.</param>
/// <returns>True if the key was found (and thus removed), False if the key was not found.</returns>
Bool VarDict_Remove(VarDict varDict, Symbol key)
{
	struct VarDictInt *self;
	struct VarDictNode *heap;
	Int32 *buckets;
	Int32 nodeIndex, prevIndex;
	Int32 mask;

	self = (struct VarDictInt *)varDict;
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
			heap[nodeIndex].varInfo.kind = 0;
			heap[nodeIndex].varInfo.offset = 0;
			heap[nodeIndex].varInfo.symbol = 0;
			heap[nodeIndex].varInfo.value = NullObject;
			heap[nodeIndex].next = self->firstFree;

			self->firstFree = nodeIndex;
			self->count--;

			if (self->count <= ((mask + 1) >> 2) && (mask + 1) > 16) {
				VarDictInt_Resize(self, (mask + 1) >> 1);
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
/// <param name="varDict">A pointer to the dictionary.</param>
SMILE_API_FUNC DictStats VarDict_ComputeStats(VarDict varDict)
{
	struct VarDictInt *self;
	struct VarDictNode *heap;
	Int32 bucketIndex, nodeIndex, numInThisBucket;
	Int32 *buckets;
	DictStats stats;

	self = (struct VarDictInt *)varDict;
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
