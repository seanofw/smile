#ifndef __SMILE_DICT_SHARED_H__
#define __SMILE_DICT_SHARED_H__

/// <summary>
/// Perform a search through a dictionary (hash table with internal linked lists) construct
/// for the node associated with a known key.
/// </summary>
/// <param name="__dicttype__">The internal struct type of the dictionary container.</param>
/// <param name="__nodetype__">The internal struct type of the dictionary's nodes.</param>
/// <param name="__indextype__">The integer index type to use for the nodes.</param>
/// <param name="__dict__">A pointer to the dictionary struct itself (uncast).</param>
/// <param name="__keyHash__">Expression: How to compute the hash code for the key value to compare against.</param>
/// <param name="__compare__">Expression: How to compare node->key against the current key.</param>
/// <param name="__iffound__">Code block: What to do if we find the key in the dictionary.</param>
/// <param name="__else__">Code block: What to do if we don't find the key in the dictionary.</param>
#define SMILE_DICT_SEARCH(__dicttype__, __nodetype__, __indextype__, __dict__, __keyHash__, __compare__, __iffound__, __else__) \
	{ \
		__dicttype__ *self = (__dicttype__ *)(__dict__); \
		__nodetype__ *heap, *node; \
		__indextype__ nodeIndex; \
		Int32 keyHash; \
		\
		keyHash = (__keyHash__); \
		\
		nodeIndex = self->buckets[keyHash & self->mask]; \
		heap = self->heap; \
		while (nodeIndex >= 0) { \
			node = heap + nodeIndex; \
			if (__compare__) __iffound__ \
			nodeIndex = node->next; \
		} \
		__else__ \
	}

#endif
