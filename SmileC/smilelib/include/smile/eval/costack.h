
#ifndef __SMILE_EVAL_COSTACK_H__
#define __SMILE_EVAL_COSTACK_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

//-------------------------------------------------------------------------------------------------
// CoStack type declarations.

#define COSTACK_CHUNK_SIZE 16384

/// <summary>
/// One chunk of the execution stack.  We maintain it in small pieces so that it's easy
/// to allocate more of it or to free it, if necessary.  All chunks are the same size so
/// that simple offset math can determine locations of items on the overall stack.
/// </summary>
typedef struct CoStackChunkStruct {
	Int32 top;	// Top offset of this chunk.
	Int32 reserved;	// Reserved for padding.
	Byte base[COSTACK_CHUNK_SIZE - 8];	// Base pointer to array of data. (Guaranteed alignment to at least 8 bytes).
} *CoStackChunk;

/// <summary>
/// A CoStack is simply an ordinary execution stack (very much like the implicit C execution
/// stack), but a Smile program can have many stacks at once, with one costack instance for each
/// active cofunction.  These grow and shrink dynamically, just like any other data structure,
/// and they are managed (and freed) by the garbage collector; they are stored as a jagged array
/// so that any pointers to data in them may remain valid even across reallocations.  Only one
/// costack may be the "current" costack within a real kernel thread.
/// </summary>
typedef struct CoStackStruct {
	CoStackChunk *chunks;	// Base pointer to array of costack chunks.
	CoStackChunk topChunk;	// Current top chunk pointer.
	Int32 topChunkIndex;	// The index of the current top chunk.
	Int32 maxChunk;	// The maximum number of costack chunks in the costack chunk array.
} *CoStack;

//-------------------------------------------------------------------------------------------------
// External Implementation.

SMILE_API_FUNC CoStack CoStack_Create(void);
SMILE_API_FUNC void CoStack_Grow(CoStack costack, Int size);
SMILE_API_FUNC void CoStack_Shrink(CoStack costack);

//-------------------------------------------------------------------------------------------------
// Inline operations.

/// <summary>
/// Allocate more space at the top of this costack.
/// </summary>
/// <param name="costack">The costack to allocate space on.</param>
/// <param name="size">The number of bytes to allocate.  Note that performance degrades
/// substantially if you use sizes above COSTACK_CHUNK_SIZE / 16, and you cannot allocate
/// a size greater than COSTACK_CHUNK_SIZE in a single request.</param>
Inline void *CoStack_Alloc(CoStack costack, Int size)
{
	CoStackChunk topChunk = costack->topChunk;
	Int32 top = topChunk->top;

	// First, see if there's enough space in the current chunk.
	if (top + size > COSTACK_CHUNK_SIZE) {
		// Not enough space, so go allocate another stack chunk, and, if necessary,
		// grow the base array.
		CoStack_Grow(costack, size);
		topChunk = costack->topChunk;
		top = topChunk->top;
	}

	// There is enough space in the current chunk, so allocate the space, rounding
	// up to ensure that every allocation is at least 8-byte aligned.
	topChunk->top += (size + 7) & ~7;

	return (void *)(topChunk->base + top);
}

/// <summary>
/// Free space from the top of the costack.
/// </summary>
/// <param name="costack">The costack to free space from.</param>
/// <param name="size">The number of bytes to free, same as were passed to CoStack_Alloc().</param>
Inline void CoStack_Free(CoStack costack, Int32 size)
{
	CoStackChunk topChunk = costack->topChunk;
	Int32 top = (topChunk->top -= size);
	Int32 topChunkIndex;

	// If the topmost chunk is empty, switch chunks to one that isn't empty.
	if (top <= 0 && costack->topChunkIndex > 0) {

		// Don't need the topmost chunk anymore (maybe).  Switch to the previous chunk.
		costack->topChunk = costack->chunks[topChunkIndex = --costack->topChunkIndex];
	
		// If we have way too many chunks (>75% are unused), go get rid of some.
		if ((topChunkIndex << 2) < costack->maxChunk) {
			CoStack_Shrink(costack);
		}
	}
}

/// <summary>
/// Allocate more space at the top of this costack.
/// </summary>
/// <param name="__costack__">The costack to allocate space on.</param>
/// <param name="__type__">The type of data to allocate on the costack.</param>
/// <returns>A pointer (of __type__*) to the new object on the costack.</returns>
#define COSTACK_ALLOC_STRUCT(__costack__, __type__) \
	((__type__ *)CoStack_Alloc((__costack__), sizeof(__type__)))

/// <summary>
/// Allocate many entries at the top of this costack.  Note that you must not
/// allocate more than 16 KB total with this call, or it will fail.
/// </summary>
/// <param name="__costack__">The costack to allocate space on.</param>
/// <param name="__type__">The type of data to allocate on the costack.</param>
/// <param name="__count__">The number of items to allocate.</param>
/// <returns>A pointer (of __type__*) to the new object on the costack.</returns>
#define COSTACK_ALLOC_STRUCT_ARRAY(__costack__, __type__, __count__) \
	((__type__ *)CoStack_Alloc((__costack__), sizeof(__type__) * (__count__)))

/// <summary>
/// Free space from the top of the costack.
/// </summary>
/// <param name="__costack__">The costack to free space from.</param>
/// <param name="__type__">The type of data to free from the top of the costack.</param>
#define COSTACK_FREE_STRUCT(__costack__, __type__) \
	(CoStack_Free((__costack__), sizeof(__type__)))

/// <summary>
/// Free space from the top of the costack.
/// </summary>
/// <param name="__costack__">The costack to free space from.</param>
/// <param name="__type__">The type of data to free from the top of the costack.</param>
/// <param name="__count__">The number of items to free.</param>
#define COSTACK_FREE_STRUCT_ARRAY(__costack__, __type__, __count__) \
	(CoStack_Free((__costack__), sizeof(__type__) * (__count__)))

#endif
