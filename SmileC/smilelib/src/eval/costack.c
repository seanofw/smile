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

#include <smile/gc.h>
#include <smile/mem.h>
#include <smile/string.h>
#include <smile/eval/costack.h>

static void CoStack_GrowChunkPointerArray(CoStack costack);

/// <summary>
/// Create a new CoStack.
/// </summary>
/// <returns>A new, empty CoStack, with the first chunk pre-allocated.</returns>
CoStack CoStack_Create(void)
{
	CoStack costack;
	CoStackChunk chunk;

	// Make room for the CoStack object itself.
	costack = GC_MALLOC_STRUCT(struct CoStackStruct);
	if (costack == NULL)
		Smile_Abort_OutOfMemory();

	// Make room for the array of CoStackChunk pointers.
	costack->chunks = GC_MALLOC_STRUCT_ARRAY(CoStackChunk, 16);
	if (costack->chunks == NULL)
		Smile_Abort_OutOfMemory();
	costack->maxChunk = 16;
	costack->topChunkIndex = 0;

	// Allocate the first chunk.
	chunk = GC_MALLOC_STRUCT(struct CoStackChunkStruct);
	if (chunk == NULL)
		Smile_Abort_OutOfMemory();
	chunk->top = 8;

	// And attach the first chunk to the CoStack.
	costack->topChunk = chunk;
	costack->chunks[0] = chunk;

	return costack;
}

/// <summary>
/// Grow the given CoStack to ensure that it fits at least 'size' bytes worth of data.
/// </summary>
/// <param name="costack">The CoStack to grow.</param>
/// <param name="size">The number of bytes needed, which must be less than COSTACK_CHUNK_SIZE-8,
/// and which practically should be less than 1 KB if you want to keep performance high.</param>
void CoStack_Grow(CoStack costack, Int size)
{
	Int topChunkIndex;
	CoStackChunk chunk;

	// If we can use the current chunk, there's no reason to do anything.
	if (costack->topChunk->top + size < COSTACK_CHUNK_SIZE)
		return;

	if (size > COSTACK_CHUNK_SIZE - 8) {
		// Whoops, this isn't going to work at all.
		Smile_Abort_FatalError(String_ToC(String_Format("Cannot allocate more than %d bytes on a costack at once; request was %d.",
			COSTACK_CHUNK_SIZE - 8, (Int32)size)));
	}

	// Not enough room in the current chunk.  Move to the next chunk index.
	topChunkIndex = ++costack->topChunkIndex;
	if (topChunkIndex >= costack->maxChunk) {
	
		// Whoops, ran out of chunks.  Need to reallocate the array of chunk pointers before
		// we go any further.
		CoStack_GrowChunkPointerArray(costack);
	}

	// Get the next chunk, or allocate one if there isn't one.
	if ((chunk = costack->chunks[topChunkIndex]) == NULL) {
	
		// No chunk here yet, so we need to allocate one.
		chunk = GC_MALLOC_STRUCT(struct CoStackChunkStruct);
		if (chunk == NULL)
			Smile_Abort_OutOfMemory();
		chunk->top = 8;
		costack->chunks[topChunkIndex] = chunk;
	}

	// Set this new chunk as the current topmost chunk.
	costack->topChunk = chunk;
}

/// <summary>
/// Shrink the given CoStack to a more reasonable size by freeing chunks it no longer needs.
/// If data in those chunks is still referenced, however, the chunks will remain allocated,
/// since they are actually managed by the garbage collector:  This function merely removes
/// any references to them from the CoStack itself.
/// </summary>
void CoStack_Shrink(CoStack costack)
{
	Int i;

	// Don't bother shrinking unless at least 75% of the costack is empty.
	if (costack->topChunkIndex * 4 >= costack->maxChunk) return;

	// Okay, it's mostly empty.  NULL out half the entries in the chunk pointer array so that the GC
	// can reclaim those chunks.
	for (i = costack->maxChunk / 2; i < costack->maxChunk; i++) {
		costack->chunks[i] = NULL;
	}
}

/// <summary>
/// Double the size of the chunk-pointer array in the given CoStack, without altering anything
/// else about the CoStack.  The new array entries will be NULL.
/// </summary>
/// <param name="costack">The CoStack to grow.</param>
static void CoStack_GrowChunkPointerArray(CoStack costack)
{
	CoStackChunk *newChunks;
	Int newMax;

	newMax = costack->maxChunk * 2;
	newChunks = GC_MALLOC_STRUCT_ARRAY(CoStackChunk, newMax);
	if (newChunks == NULL)
		Smile_Abort_OutOfMemory();

	MemCpy((void *)newChunks, costack->chunks, costack->topChunkIndex);
	MemZero((void *)(newChunks + costack->topChunkIndex), sizeof(CoStackChunk) * (newMax - costack->topChunkIndex));

	costack->chunks = newChunks;
	costack->maxChunk = newMax;
}

