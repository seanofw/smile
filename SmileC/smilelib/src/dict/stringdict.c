
#include <smile/gc.h>
#include <smile/mem.h>
#include <smile/dict/stringdict.h>

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
Int StringDictInt_Append(struct StringDictInt *self, String key, Int32 keyHash, const void *value)
{
	struct StringDictNode *heap, *newHeap;
	Int nodeIndex, bucketIndex, i, oldmax, newmax;

	if (self->firstFree < 0) {
		oldmax = self->heapLen;
		newmax = oldmax * 2;
		newHeap = GC_MALLOC_STRUCT_ARRAY(struct StringDictNode, newmax);
		if (newHeap == NULL) Smile_Abort_OutOfMemory();
		MemCpy(newHeap, self->heap, oldmax * sizeof(struct StringDictNode *));

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
	bucketIndex = keyHash & self->mask;
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
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <returns>An array containing all of the key/value pairs in the dictionary, in no specific order.
/// This will be the same length as returned by StringDict_GetCount().</returns>
StringDictKeyValuePair *StringDict_GetAll(StringDict stringDict)
{
	struct StringDictInt *self;
	Int bucket, bucketsLen, nodeIndex;
	Int *buckets;
	StringDictKeyValuePair *pairs, *dest;
	struct StringDictNode *heap, *node;
	
	self = (struct StringDictInt *)stringDict;

	pairs = GC_MALLOC_STRUCT_ARRAY(StringDictKeyValuePair, self->count);
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
/// This will be the same length as returned by StringDict_GetCount().</returns>
String *StringDict_GetKeys(StringDict stringDict)
{
	struct StringDictInt *self;
	Int bucket, bucketsLen, nodeIndex;
	Int *buckets;
	String *keys, *dest;
	struct StringDictNode *heap, *node;

	self = (struct StringDictInt *)stringDict;

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
/// This will be the same length as returned by StringDict_GetCount().</returns>
void **StringDict_GetValues(StringDict stringDict)
{
	struct StringDictInt *self;
	Int bucket, bucketsLen, nodeIndex;
	Int *buckets;
	void **values, **dest;
	struct StringDictNode *heap, *node;

	self = (struct StringDictInt *)stringDict;

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
void StringDict_Clear(StringDict stringDict)
{
	const Int DefaultBucketCount = 16;
	const Int DefaultHeapSize = 16;

	struct StringDictInt *self;
	struct StringDictNode *heap;
	Int *buckets;
	Int i;

	self = (struct StringDictInt *)stringDict;

	self->buckets = buckets = GC_MALLOC_RAW_ARRAY(Int, DefaultBucketCount);
	if (buckets == NULL) Smile_Abort_OutOfMemory();
	self->bucketsLen = DefaultBucketCount;
	self->heap = heap = GC_MALLOC_STRUCT_ARRAY(struct StringDictNode, DefaultHeapSize);
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
/// <param name="stringDict">A pointer to the dictionary.</param>
/// <param name="key">The key to remove.</param>
/// <returns>True if the key was found (and thus removed), False if the key was not found.</returns>
Bool StringDict_Remove(StringDict stringDict, String key)
{
	struct StringDictInt *self;
	struct StringDictNode *heap;
	Int *buckets;
	Int32 keyHash;

	Int nodeIndex, prevIndex;

	self = (struct StringDictInt *)stringDict;
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
