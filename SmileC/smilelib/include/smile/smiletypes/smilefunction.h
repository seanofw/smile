
#ifndef __SMILE_SMILETYPES_SMILEFUNCTION_H__
#define __SMILE_SMILETYPES_SMILEFUNCTION_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_EVAL_CLOSURE_H__
#include <smile/eval/closure.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations for external-function support

#define ARG_CHECK_NOCOUNT	(0 << 0)
#define ARG_CHECK_MIN	(1 << 0)
#define ARG_CHECK_MAX	(2 << 0)
#define ARG_CHECK_EXACT	(3 << 0)
#define ARG_CHECK_COUNT_MASK	(3 << 0)
	
#define ARG_CHECK_TYPES	(1 << 2)

typedef SmileObject (*ExternalFunction)(Int argc, SmileObject *argv, void *param);

typedef struct ExternalFunctionInfoStruct {

	String name;	// The given full name of this function, like "File.open".
		
	ExternalFunction externalFunction;	// The actual external C function to call.
	void *param;	// An optional parameter that will be passed to the external function.
		
	Int16 argCheckFlags;	// How to perform argument checks (min, max, types, etc.)
	Int16 numArgsToTypeCheck;	// How many args are included in the type-check array below.
	Int16 minArgs;	// The minimum number of arguments for calling this C function.
	Int16 maxArgs;	// The maximum number of arguments for calling this C function.
		
	const Byte *argTypeChecks;	// Expected type for each argument (or'ed with 0x80 = can be null; value of 0 = don't care)

} *ExternalFunctionInfo;

struct ExternalFunctionInfoStruct;

//-------------------------------------------------------------------------------------------------
//  Core type declarations

struct SmileFunctionInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	SmileList args;	// A list of argument names for this function
	SmileObject body;	// The body of this function (NullObject for a C function)
		
	union {	
	   ClosureInfo closureInfo;	// For Smile user functions
	   ExternalFunctionInfo externalFunctionInfo;	// For C external functions
	} u;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileUserFunction_VTable;
SMILE_API_DATA SmileVTable SmileExternalFunction_VTable;

SMILE_API_FUNC SmileFunction SmileFunction_CreateUserFunction(SmileList args, SmileObject body, struct ClosureInfoStruct *closureInfo);
SMILE_API_FUNC SmileFunction SmileFunction_CreateExternalFunction(ExternalFunction externalFunction, void *param,
	const char *name, const char *argNames, Int argCheckFlags, Int minArgs, Int maxArgs, const Byte *argTypeChecks);

//-------------------------------------------------------------------------------------------------
//  Inline functions

Inline Bool SmileFunction_IsBuiltIn(SmileFunction fn)
{
	return (fn->vtable == SmileExternalFunction_VTable);
}

#endif
