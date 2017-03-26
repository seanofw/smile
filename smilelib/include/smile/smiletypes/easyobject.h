#ifndef __SMILE_SMILETYPES_EASYOBJECT_H__
#define __SMILE_SMILETYPES_EASYOBJECT_H__

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_NUMERIC_REAL64_H__
#include <smile/numeric/real64.h>
#endif

#ifndef __SMILE_SMILETYPES_TEXT_SMILESTRING_H__
#include <smile/smiletypes/text/smilestring.h>
#endif

/// <summary>
/// This macro makes declaring consistent Smile virtual tables and associated functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare a virtual table and functions for.</param>
#define SMILE_EASY_OBJECT_VTABLE(__type__) \
	\
	/* These are the functions you'll have to implement for your object type. */ \
	static Bool __type__##_CompareEqual(__type__ self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData); \
	static Bool __type__##_DeepEqual(__type__ self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers); \
	static UInt32 __type__##_Hash(__type__ obj); \
	static void __type__##_SetSecurityKey(__type__ self, SmileObject newSecurityKey, SmileObject oldSecurityKey); \
	static void __type__##_SetSecurity(__type__ self, Int security, SmileObject securityKey); \
	static Int __type__##_GetSecurity(__type__ obj); \
	static SmileObject __type__##_GetProperty(__type__ self, Symbol propertyName); \
	static void __type__##_SetProperty(__type__ self, Symbol propertyName, SmileObject value); \
	static Bool __type__##_HasProperty(__type__ self, Symbol propertyName); \
	static SmileList __type__##_GetPropertyNames(__type__ self); \
	static Bool __type__##_ToBool(__type__ obj, SmileUnboxedData unboxedData); \
	static Int32 __type__##_ToInteger32(__type__ obj, SmileUnboxedData unboxedData); \
	static Float64 __type__##_ToFloat64(__type__ obj, SmileUnboxedData unboxedData); \
	static Real64 __type__##_ToReal64(__type__ obj, SmileUnboxedData unboxedData); \
	static String __type__##_ToString(__type__ obj, SmileUnboxedData unboxedData); \
	static void __type__##_Call(__type__ obj, Int argc); \
	static LexerPosition __type__##_GetSourceLocation(__type__ obj); \
	static SmileArg __type__##_Unbox(__type__ self); \
	static SmileObject __type__##_Box(SmileArg src); \
	\
	/* The virtual table that glues it all together and that's needed by the type system. */ \
	SMILE_VTABLE(__type__##_VTable, __type__) { \
		__type__##_CompareEqual, \
		__type__##_DeepEqual, \
		__type__##_Hash, \
		__type__##_SetSecurityKey, \
		__type__##_SetSecurity, \
		__type__##_GetSecurity, \
		__type__##_GetProperty, \
		__type__##_SetProperty, \
		__type__##_HasProperty, \
		__type__##_GetPropertyNames, \
		__type__##_ToBool, \
		__type__##_ToInteger32, \
		__type__##_ToFloat64, \
		__type__##_ToReal64, \
		__type__##_ToString, \
		__type__##_Call, \
		__type__##_GetSourceLocation, \
		__type__##_Unbox, \
		__type__##_Box, \
	}

/// <summary>
/// This macro makes declaring consistent Smile virtual comparison functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
/// <param name="__kind__">The object-kind (enum) of the object you want to declare virtual functions for.</param>
/// <param name="__expr__">A C expression that determines whether two objects of the given type, named 'a' and 'b', are equal.
/// This should run in constant time.</param>
#define SMILE_EASY_OBJECT_COMPARE(__type__, __kind__, __expr__) \
	static Bool __type__##_CompareEqual(__type__ a, SmileUnboxedData aData, SmileObject other, SmileUnboxedData bData) { \
		__type__ b; \
		UNUSED(a); UNUSED(b); \
		UNUSED(aData); UNUSED(bData); \
		if (other->kind != (__kind__)) return False; \
		b = (__type__)other; \
		return __expr__; \
	}

/// <summary>
/// This macro makes declaring consistent Smile virtual comparison functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
/// <param name="__kind__">The object-kind (enum) of the object you want to declare virtual functions for.</param>
/// <param name="__expr__">A C expression that determines whether two objects of the given type, named 'a' and 'b', are equal,
/// including all their descendants.</param>
#define SMILE_EASY_OBJECT_DEEP_COMPARE(__type__, __kind__, __expr__) \
	static Bool __type__##_DeepEqual(__type__ a, SmileUnboxedData aData, SmileObject other, SmileUnboxedData bData, PointerSet visitedPointers) { \
		__type__ b; \
		UNUSED(a); UNUSED(b); \
		UNUSED(aData); UNUSED(bData); \
		UNUSED(visitedPointers); \
		if (other->kind != (__kind__)) return False; \
		b = (__type__)other; \
		return __expr__; \
	}

/// <summary>
/// This macro makes declaring consistent Smile virtual hash functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
/// <param name="__expr__">A C expression that determines a suitable hash code for an object of the given type named 'obj'.</param>
#define SMILE_EASY_OBJECT_HASH(__type__, __expr__) \
	static UInt32 __type__##_Hash(__type__ obj) { UNUSED(obj); return __expr__; }

/// <summary>
/// This macro makes declaring consistent Smile virtual Boolean-conversion functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
/// <param name="__expr__">A C expression that computes a suitable Boolean equivalent of an object of the given type named 'obj'.</param>
#define SMILE_EASY_OBJECT_TOBOOL(__type__, __expr__) \
	static Bool __type__##_ToBool(__type__ obj, SmileUnboxedData unboxedData) { UNUSED(obj); UNUSED(unboxedData); return __expr__; }

/// <summary>
/// This macro makes declaring consistent Smile virtual Integer32-conversion functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
/// <param name="__expr__">A C expression that computes a suitable Integer32 equivalent of an object of the given type named 'obj'.</param>
#define SMILE_EASY_OBJECT_TOINT(__type__, __expr__) \
	static Int32 __type__##_ToInteger32(__type__ obj, SmileUnboxedData unboxedData) { UNUSED(obj); UNUSED(unboxedData); return __expr__; }

/// <summary>
/// This macro makes declaring consistent Smile virtual String-conversion functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
/// <param name="__expr__">A C expression that computes a suitable String equivalent of an object of the given type named 'obj'.</param>
#define SMILE_EASY_OBJECT_TOSTRING(__type__, __expr__) \
	static String __type__##_ToString(__type__ obj, SmileUnboxedData unboxedData) { UNUSED(obj); UNUSED(unboxedData); return __expr__; }

/// <summary>
/// This macro makes declaring consistent Smile virtual Float64-conversion functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
/// <param name="__expr__">A C expression that computes a suitable Float64 equivalent of an object of the given type named 'obj'.</param>
#define SMILE_EASY_OBJECT_TOFLOAT(__type__, __expr__) \
	static Float64 __type__##_ToFloat64(__type__ obj, SmileUnboxedData unboxedData) { UNUSED(obj); UNUSED(unboxedData); return __expr__; }

/// <summary>
/// This macro makes declaring consistent Smile virtual Real64-conversion functions easier.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
/// <param name="__expr__">A C expression that computes a suitable Real64 equivalent of an object of the given type named 'obj'.</param>
#define SMILE_EASY_OBJECT_TOREAL(__type__, __expr__) \
	static Real64 __type__##_ToReal64(__type__ obj, SmileUnboxedData unboxedData) { UNUSED(obj); UNUSED(unboxedData); return __expr__; }

/// <summary>
/// This macro can be used to declare virtual functions for a type that can't answer source locations.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
#define SMILE_EASY_OBJECT_NO_SOURCE(__type__) \
	static LexerPosition __type__##_GetSourceLocation(__type__ obj) { UNUSED(obj); return NULL; }

/// <summary>
/// This macro can be used to declare virtual functions for a type that can't be unboxed (which is most of them).
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
#define SMILE_EASY_OBJECT_NO_UNBOX(__type__) \
	static SmileArg __type__##_Unbox(__type__ self) \
		{ SmileArg dest; dest.obj = (SmileObject)self; return dest; } \
	static SmileObject __type__##_Box(SmileArg src) \
		{ return (SmileObject)src.obj; }

/// <summary>
/// This macro can be used to declare virtual functions for a type that has no configurable security (i.e., its
/// properties are always read-only).
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
#define SMILE_EASY_OBJECT_READONLY_SECURITY(__type__) \
	static void __type__##_SetSecurityKey(__type__ self, SmileObject newSecurityKey, SmileObject oldSecurityKey) \
		{ UNUSED(self); UNUSED(newSecurityKey); UNUSED(oldSecurityKey); \
			Smile_ThrowException(Smile_KnownSymbols.object_security_error, SmileString_GetString(Smile_KnownStrings.InvalidSecurityKey)); } \
	static void __type__##_SetSecurity(__type__ self, Int security, SmileObject securityKey) \
		{ UNUSED(self); UNUSED(security); UNUSED(securityKey); \
			Smile_ThrowException(Smile_KnownSymbols.object_security_error, SmileString_GetString(Smile_KnownStrings.InvalidSecurityKey)); } \
	static Int __type__##_GetSecurity(__type__ obj) { UNUSED(obj); return SMILE_SECURITY_READONLY; }

/// <summary>
/// This macro can be used to declare virtual functions for a type that can't be readily converted to
/// Float64 or Real64; with this macro, those conversions will use the Integer32 conversion instead.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
#define SMILE_EASY_OBJECT_NO_REALS(__type__) \
	static Float64 __type__##_ToFloat64(__type__ obj, SmileUnboxedData unboxedData) { return (Float64)__type__##_ToInteger32(obj, unboxedData); } \
	static Real64 __type__##_ToReal64(__type__ obj, SmileUnboxedData unboxedData) { return Real64_FromInt32(__type__##_ToInteger32(obj, unboxedData)); }

/// <summary>
/// This macro can be used to declare virtual functions for a type that can't be readily invoked as a function.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
#define SMILE_EASY_OBJECT_NO_CALL(__type__) \
	static void __type__##_Call(__type__ obj, Int argc) { \
		UNUSED(obj); UNUSED(argc); \
		Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError); }

/// <summary>
/// This macro can be used to declare virtual functions for a type that has no properties.
/// </summary>
/// <param name="__type__">The type of the object you want to declare virtual functions for.</param>
#define SMILE_EASY_OBJECT_NO_PROPERTIES(__type__) \
	static SmileObject __type__##_GetProperty(__type__ obj, Symbol symbol) \
		{ UNUSED(obj); UNUSED(symbol); return obj->base->vtable->getProperty(obj->base, symbol); } \
	static Bool __type__##_HasProperty(__type__ obj, Symbol symbol) \
		{ UNUSED(obj); UNUSED(symbol); return False; } \
	static void __type__##_SetProperty(__type__ obj, Symbol symbol, SmileObject value) \
		{ UNUSED(obj); UNUSED(symbol); UNUSED(value); \
			Smile_ThrowException(Smile_KnownSymbols.object_security_error, \
				String_Format("Cannot set property \"%S\" on a " #__type__ ", which is read-only.", \
				SymbolTable_GetName(Smile_SymbolTable, symbol))); } \
	static SmileList __type__##_GetPropertyNames(__type__ obj) \
		{ UNUSED(obj); return NullList; }

#endif