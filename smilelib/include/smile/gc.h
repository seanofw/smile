
#ifndef __SMILE_GC_H__
#define __SMILE_GC_H__

#ifdef _DEBUG
#	ifndef GC_DEBUG
#		define GC_DEBUG
#	endif
#endif

// The GC is inside a DLL, along with the rest of Smile's interpeter.
#if !defined(GC_DLL) && !defined(GC_NOT_DLL)
	#define GC_DLL
#endif

// The GC warnings aren't especially useful here, since Dictionaries and StringBuilders
// may trigger them during normal operations.
#define GC_IGNORE_WARN

#if defined(_MSC_VER) && _MSC_VER >= 1900
#pragma warning(push)
#pragma warning(disable: 4464)
#endif

#include "../../gc/include/gc.h"

#if defined(_MSC_VER) && _MSC_VER >= 1900
#pragma warning(pop)
#endif

/// <summary>Allocate a raw array of the given type.  Raw arrays must not contain pointers within them,
/// so they are more efficient for garbage collection than structured arrays.</summary>
/// <param name="__t__">The type of the data elements in the array.</param>
/// <param name="__n__">The number of data elements in the array.</param>
/// <returns>A pointer to the start of the new array, or NULL if the allocation failed.</returns>
#define GC_MALLOC_RAW_ARRAY(__t__, __n__) ( (__t__ *) GC_MALLOC_ATOMIC( sizeof(__t__) * (__n__) ) )

/// <summary>Allocate a raw byte array, which must not contain pointers.</summary>
/// <param name="__n__">The number of bytes in the array.</param>
/// <returns>A pointer to the start of the new array, or NULL if the allocation failed.</returns>
#define GC_MALLOC_BYTES(__n__) ( GC_MALLOC_RAW_ARRAY( Byte, (__n__) ) )

/// <summary>Allocate a raw byte array for the purpose of storing text within it.  This intentionally
/// over-allocates by one byte to leave room for a trailing '\0'.</summary>
/// <param name="__n__">The number of characters in the array.</param>
/// <returns>A pointer to the start of the new array, or NULL if the allocation failed.</returns>
#define GC_MALLOC_TEXT(__len__) GC_MALLOC_BYTES( (__len__) + 1 )

/// <summary>Allocate a struct of known type.</summary>
/// <param name="__t__">The type the new struct.</param>
/// <returns>A pointer to the new object, or NULL if the allocation failed.</returns>
#define GC_MALLOC_STRUCT(__t__) ( (__t__ *) GC_MALLOC( sizeof(__t__) ) )

/// <summary>Allocate an array of the given type.  Arrays created by this may contain pointers to other data.</summary>
/// <param name="__t__">The type of the data elements in the array.</param>
/// <param name="__n__">The number of data elements in the array.</param>
/// <returns>A pointer to start of the new array, or NULL if the allocation failed.</returns>
#define GC_MALLOC_STRUCT_ARRAY(__t__, __n__) ((__t__ *) GC_MALLOC( sizeof(__t__) * (__n__) ))

#endif
