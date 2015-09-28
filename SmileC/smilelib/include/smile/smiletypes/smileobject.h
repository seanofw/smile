
#ifndef __SMILE_SMILETYPES_OBJECT_H__
#define __SMILE_SMILETYPES_OBJECT_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_ENV_ENV_H__
#include <smile/env/env.h>
#endif

#ifndef __SMILE_ENV_SYMBOLTABLE_H__
#include <smile/env/symboltable.h>
#endif

#ifndef __SMILE_SMILETYPES_KIND_H__
#include <smile/smiletypes/kind.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#define DECLARE_BASE_OBJECT_PROPERTIES \
	UInt32 kind; \
	Symbol assignedSymbol; \
	SmileVTable vtable; \
	SmileObject base

struct SmileObjectInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

#define SMILE_VTABLE_TYPE(__name__, __type__) \
	__name__ { \
		Bool (*compareEqual)(__type__ self, SmileObject other); \
		UInt32 (*hash)(__type__ self); \
		void (*setSecurity)(__type__ self, Int security); \
		Int (*getSecurity)(__type__ self); \
		\
		SmileObject (*getProperty)(__type__ self, Symbol propertyName); \
		void (*setProperty)(__type__ self, Symbol propertyName, SmileObject value); \
		Bool (*hasProperty)(__type__ self, Symbol propertyName); \
		SmileList (*getPropertyNames)(__type__ self); \
		\
		Bool (*toBool)(__type__ self); \
		Int32 (*toInteger32)(__type__ self); \
		Real64 (*toReal64)(__type__ self); \
		String (*toString)(__type__ self); \
	}

#define SMILE_VTABLE(__name__, __type__) \
	SMILE_VTABLE_TYPE(struct __name__##Int, __type__); \
	static struct __name__##Int __name__##Data; \
	\
	SmileVTable __name__ = (SmileVTable)&__name__##Data; \
	\
	static struct __name__##Int __name__##Data =

SMILE_VTABLE_TYPE(struct SmileVTableInt, SmileObject);

SMILE_API SmileVTable SmileObject_VTable;

SMILE_API Bool SmileObject_CompareEqual(SmileObject self, SmileObject other);
SMILE_API UInt32 SmileObject_Hash(SmileObject self);
SMILE_API void SmileObject_SetSecurity(SmileObject self, Int security);
SMILE_API Int SmileObject_GetSecurity(SmileObject self);
SMILE_API SmileObject SmileObject_GetProperty(SmileObject self, Symbol propertyName);
SMILE_API void SmileObject_SetProperty(SmileObject self, Symbol propertyName, SmileObject value);
SMILE_API Bool SmileObject_HasProperty(SmileObject self, Symbol propertyName);
SMILE_API SmileList SmileObject_GetPropertyNames(SmileObject self);
SMILE_API Bool SmileObject_ToBool(SmileObject self);
SMILE_API Int32 SmileObject_ToInteger32(SmileObject self);
SMILE_API Real64 SmileObject_ToReal64(SmileObject self);
SMILE_API String SmileObject_ToString(SmileObject self);

SMILE_API SmileObject SmileObject_Create(void);

Inline Bool SmileObject_IsList(SmileObject self)
{
	register UInt32 kind = self->kind;
	return kind == SMILE_KIND_LIST || kind == SMILE_KIND_NULL;
}

Inline Bool SmileObject_IsListWithSource(SmileObject self)
{
	return SmileObject_IsList(self) && (self->kind & SMILE_FLAG_LISTWITHSOURCE);
}

#endif
