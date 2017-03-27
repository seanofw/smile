
#ifndef __SMILE_INTERNAL_STATICSTRING_H__
#define __SMILE_INTERNAL_STATICSTRING_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

/// <summary>
/// Declare a static string, preallocated in const (readonly) memory, rather than on the heap.
/// </summary>
/// <param name="__name__">The name of the static string instance to declare.</param>
/// <param name="__text__">A C-style string that contains the static text.</param>
/// <param name="__textLength__">The number of bytes in the C-style string, not including the terminating nul character.</param>
#define EXTERN_STATIC_STRING(__name__, __text__) \
	static struct StringStruct __name__##Struct = { \
		SMILE_KIND_STRING, (SmileVTable)&String_VTableData, (SmileObject)&String_BaseObjectStruct, \
		{ (sizeof(__text__) - 1), (__text__) } \
	}; \
	String __name__ = (String)(&__name__##Struct)

/// <summary>
/// Declare a private (i.e., not accessible outside this source file) static string,
/// preallocated in const (readonly) memory, rather than on the heap.
/// </summary>
/// <param name="__name__">The name of the static string instance to declare.</param>
/// <param name="__text__">A C-style string that contains the static text.</param>
#define STATIC_STRING(__name__, __text__) \
	static struct StringStruct __name__##Struct = { \
		SMILE_KIND_STRING, (SmileVTable)&String_VTableData, (SmileObject)&String_BaseObjectStruct, \
		{ (sizeof(__text__) - 1), (__text__) } \
	}; \
	static String __name__ = (String)(&__name__##Struct)

#endif