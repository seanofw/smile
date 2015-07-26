#ifndef __SMILE_MEM_H__
#define __SMILE_MEM_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#include <string.h>

/// <summary>
/// A safe wrapper for memcpy().  This should be as optimized as the current platform provides.
/// </summary>
Inline void MemCpy(void *dest, const void *src, Int length)
{
	memcpy(dest, src, length);
}

/// <summary>
/// A safe wrapper for memset().  This should be as optimized as the current platform provides.
/// </summary>
Inline void MemSet(void *dest, Byte b, Int length)
{
	memset(dest, b, length);
}

/// <summary>
/// A safe wrapper for bzero().  This should be as optimized as the current platform provides.
/// </summary>
Inline void MemZero(void *dest, Int length)
{
	memset(dest, 0, length);
}

/// <summary>
/// A safe wrapper for memcmp() (which should perform byte-wise comparison of the given arrays,
/// and return a signed value indicating their relationship).  This should be as optimized as
/// the current platform provides.
/// </summary>
Inline Int MemCmp(const void *a, const void *b, Int length)
{
	return memcmp(a, b, length);
}

/// <summary>
/// A safe wrapper for strlen().  This should be as optimized as the current platform provides.
/// </summary>
Inline Int StrLen(const char *str)
{
	return (Int)strlen(str);
}

#endif