
//---------------------------------------------------------------------------------------
//  Windows + MSVC + (x86 or x64)

#pragma warning(push)

#pragma warning(disable: 4255)
#pragma warning(disable: 4668)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma warning(pop)

//---------------------------------------------------------------------------------------
//  Int32 functions

Int32 Atomic_LoadInt32(const Int32 *src)
{
	return InterlockedAdd((volatile Int32 *)src, 0);
}

void Atomic_StoreInt32(Int32 *dest, Int32 value)
{
	InterlockedExchange((volatile Int32 *)dest, value);
}

Int32 Atomic_AddInt32(Int32 *value, Int32 delta)
{
	return InterlockedAdd((volatile Int32 *)value, delta);
}

Bool Atomic_CompareAndSwapInt32(Int32 *value, Int32 comparator, Int32 replacement)
{
	return InterlockedCompareExchange((volatile Int32 *)value, replacement, comparator) == comparator;
}

Int32 Atomic_SwapInt32(Int32 *value, Int32 replacement)
{
	return InterlockedExchange((volatile Int32 *)value, replacement);
}

//---------------------------------------------------------------------------------------
//  Int64 functions

Int64 Atomic_LoadInt64(const Int64 *src)
{
	return InterlockedAdd64((volatile Int64 *)src, 0);
}

void Atomic_StoreInt64(Int64 *dest, Int64 value)
{
	InterlockedExchange64((volatile Int64 *)dest, value);
}

Int64 Atomic_AddInt64(Int64 *value, Int64 delta)
{
	return InterlockedAdd64((volatile Int64 *)value, delta);
}

Bool Atomic_CompareAndSwapInt64(Int64 *value, Int64 comparator, Int64 replacement)
{
	return InterlockedCompareExchange64((volatile Int64 *)value, replacement, comparator) == comparator;
}

Int64 Atomic_SwapInt64(Int64 *value, Int64 replacement)
{
	return InterlockedExchange64((volatile Int64 *)value, replacement);
}

//---------------------------------------------------------------------------------------
//  Pointer functions

#if SMILE_CPU == SMILE_CPU_X86

void *Atomic_LoadPointer(const void **src)
{
	return (void *)InterlockedAdd((volatile Int32 *)src, 0);
}

void Atomic_StorePointer(void **dest, const void *value)
{
	InterlockedExchange((volatile Int32 *)dest, (Int32)value);
}

void *Atomic_AddPointer(void **value, Int delta)
{
	return (void *)InterlockedAdd((volatile Int32 *)value, (Int32)delta);
}

Bool Atomic_CompareAndSwapPointer(void **value, const void *comparator, const void *replacement)
{
	return InterlockedCompareExchange((volatile Int32 *)value, (Int32)replacement, (Int32)comparator) == (Int32)comparator;
}

void *Atomic_SwapPointer(void **value, const void *replacement)
{
	return (void *)InterlockedExchange((volatile Int32 *)value, (Int32)replacement);
}

#elif SMILE_CPU == SMILE_CPU_X64

void *Atomic_LoadPointer(const void **src)
{
	return (void *)InterlockedAdd64((volatile Int64 *)src, 0);
}

void Atomic_StorePointer(void **dest, const void *value)
{
	InterlockedExchange64((volatile Int64 *)dest, (Int64)value);
}

void *Atomic_AddPointer(void **value, Int delta)
{
	return (void *)InterlockedAdd64((volatile Int64 *)value, (Int64)delta);
}

Bool Atomic_CompareAndSwapPointer(void **value, const void *comparator, const void *replacement)
{
	return InterlockedCompareExchange64((volatile Int64 *)value, (Int64)replacement, (Int64)comparator) == (Int64)comparator;
}

void *Atomic_SwapPointer(void **value, const void *replacement)
{
	return (void *)InterlockedExchange64((volatile Int64 *)value, (Int64)replacement);
}

#else
#error No support for atomic operations: CPU is unknown.
#endif
