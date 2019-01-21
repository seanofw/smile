
#ifndef __SMILE_ATOMIC_H__
#define __SMILE_ATOMIC_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

//------------------------------------------------------------------------------------------------
//  Core interface (external functions)

SMILE_API_FUNC Int32 Atomic_LoadInt32(const Int32 *src);
SMILE_API_FUNC void Atomic_StoreInt32(Int32 *dest, Int32 value);
SMILE_API_FUNC Int32 Atomic_AddInt32(Int32 *value, Int32 delta);
SMILE_API_FUNC Bool Atomic_CompareAndSwapInt32(Int32 *value, Int32 comparator, Int32 replacement);
SMILE_API_FUNC Int32 Atomic_SwapInt32(Int32 *value, Int32 replacement);

SMILE_API_FUNC Int64 Atomic_LoadInt64(const Int64 *src);
SMILE_API_FUNC void Atomic_StoreInt64(Int64 *dest, Int64 value);
SMILE_API_FUNC Int64 Atomic_AddInt64(Int64 *value, Int64 delta);
SMILE_API_FUNC Bool Atomic_CompareAndSwapInt64(Int64 *value, Int64 comparator, Int64 replacement);
SMILE_API_FUNC Int64 Atomic_SwapInt64(Int64 *value, Int64 replacement);

SMILE_API_FUNC void *Atomic_LoadPointer(const void **src);
SMILE_API_FUNC void Atomic_StorePointer(void **dest, const void *value);
SMILE_API_FUNC void *Atomic_AddPointer(void **value, Int delta);
SMILE_API_FUNC Bool Atomic_CompareAndSwapPointer(void **value, const void *comparator, const void *replacement);
SMILE_API_FUNC void *Atomic_SwapPointer(void **value, const void *replacement);

//------------------------------------------------------------------------------------------------
//  Inline helper functions for common operations.

Inline Int32 Atomic_SubtractInt32(Int32 *value, Int32 delta)
{
	return Atomic_AddInt32(value, -delta);
}

Inline Int32 Atomic_IncrementInt32(Int32 *value)
{
	return Atomic_AddInt32(value, 1);
}

Inline Int32 Atomic_DecrementInt32(Int32 *value)
{
	return Atomic_AddInt32(value, -1);
}

Inline Int64 Atomic_SubtractInt64(Int64 *value, Int64 delta)
{
	return Atomic_AddInt64(value, -delta);
}

Inline Int64 Atomic_IncrementInt64(Int64 *value)
{
	return Atomic_AddInt64(value, 1);
}

Inline Int64 Atomic_DecrementInt64(Int64 *value)
{
	return Atomic_AddInt64(value, -1);
}

Inline void *Atomic_SubtractPointer(void **value, Int delta)
{
	return Atomic_AddPointer(value, -delta);
}

Inline void *Atomic_IncrementPointer(void **value)
{
	return Atomic_AddPointer(value, 1);
}

Inline void *Atomic_DecrementPointer(void **value)
{
	return Atomic_AddPointer(value, -1);
}

#endif
