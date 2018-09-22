
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

#define ARG_CHECK_MIN	(1 << 0)
#define ARG_CHECK_MAX	(1 << 1)
#define ARG_CHECK_EXACT	(1 << 2)	
#define ARG_CHECK_TYPES	(1 << 3)
#define ARG_STATE_MACHINE	(1 << 4)	// This is a special "state machine" function.
#define ARG_MODE_RAW	(1 << 5)	// This function accesses the call stack directly, and has no frame. (Use with caution!)

/// <summary>
/// This macro knows how to declare a function that is externally-callable by a Smile program.
/// You should not declare the function prototype directly yourself; let the macro do it for you
/// correctly, as the parameters and arguments may change in newer versions of the Smile runtime.
/// </summary>
#define SMILE_EXTERNAL_FUNCTION(__name__) \
	static SmileArg __name__(Int argc, SmileArg *argv, void *param)
#define SMILE_EXTERNAL_RAW_FUNCTION(__name__) \
	static void __name__(Int argc, SmileArg *argv, void *param)

/// <summary>
/// This is the type of an externally-callable function.
/// </summary>
typedef SmileArg (*ExternalFunction)(Int argc, SmileArg *argv, void *param);
typedef void (*ExternalRawFunction)(Int argc, SmileArg *argv, void *param);

typedef struct ExternalFunctionInfoStruct {

	String name;	// The given full name of this function, like "open".
	String argNames;	// The names of the arguments to this function, space-separated, like "x y z".
		
	ExternalFunction externalFunction;	// The actual external C function to call.
	void *param;	// An optional parameter that will be passed to the external function.
		
	Int16 argCheckFlags;	// How to perform argument checks (min, max, types, etc.)
	Int16 numArgsToTypeCheck;	// How many args are included in the type-check array below.
	Int16 minArgs;	// The minimum number of arguments for calling this C function.
	Int16 maxArgs;	// The maximum number of arguments for calling this C function.
		
	const Byte *argTypeChecks;	// Expected type for each argument (or'ed with 0x80 = can be null; value of 0 = don't care)

} *ExternalFunctionInfo;

//-------------------------------------------------------------------------------------------------
//  Type declarations for user-function support

#define USER_ARG_NORMAL	0	// A normal argument
#define USER_ARG_REST	(1 << 0)	// This is a "rest" argument
#define USER_ARG_TYPECHECK	(1 << 1)	// This argument requires a type check
#define USER_ARG_OPTIONAL	(1 << 2)	// This argument is optional (and has an assigned default value)

typedef struct UserFunctionArgStruct {

	Int32 flags;	// Flags describing what kind of argument this is, from the USER_ARG_* flags above
	Symbol name;	// The name of this argument, as it appears in the body of the function
	Symbol typeName;	// The object this argument must inherit from (which must be a variable name in scope).
	SmileArg defaultValue;	// A default value for this argument, if it was omitted by the caller

} *UserFunctionArg;

typedef struct UserFunctionInfoStruct {

	struct UserFunctionInfoStruct *parent;	// The parent (declaring) user function, if any.
		
	Int16 flags;	// A union (bitwise or) of all the flags for all the arguments.
	Int16 numArgs;	// The number of args in the args array below.
	Int16 minArgs;	// The minimum number of args that must be provided (or an error will be thrown).
	Int16 maxArgs;	// The maximum number of args that may be provided (or an error will be thrown).
	UserFunctionArg args;	// Pointer to an array of UserFunctionArgs describing the arguments.
		
	LexerPosition position;	// The original source location of this function
	SmileList argList;	// The original list of arguments to this function
	SmileObject body;	// The original body of this function
		
	struct ClosureInfoStruct closureInfo;	// The ClosureInfo that describes this function's stack behavior. 
		
	ByteCodeSegment byteCodeSegment;	// The byte-code instructions that describe this function's compiled body.

} *UserFunctionInfo;

//-------------------------------------------------------------------------------------------------
//  Core type declarations

struct SmileFunctionInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	union {
	   struct {
	      Closure declaringClosure;	// The closure in which this user function was instantiated
	      UserFunctionInfo userFunctionInfo;	// Static information about this function
	   } u;	// For custom user functions
	   struct ExternalFunctionInfoStruct externalFunctionInfo;	// For C external functions
	} u;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_FUNC UserFunctionInfo UserFunctionInfo_Create(UserFunctionInfo parent, LexerPosition position, SmileList args, SmileObject body, String *errorMessage);
SMILE_API_FUNC void UserFunctionInfo_Init(UserFunctionInfo userFunctionInfo, UserFunctionInfo parent, LexerPosition position, SmileList args, SmileObject body);
SMILE_API_FUNC Bool UserFunctionInfo_ApplyArgs(UserFunctionInfo userFunctionInfo, SmileList argList, String *errorMessage);
SMILE_API_FUNC void SmileFunction_InitUserFunction(SmileFunction smileFunction, UserFunctionInfo userFunctionInfo, Closure declaringClosure);
SMILE_API_FUNC SmileFunction SmileFunction_CreateUserFunction(UserFunctionInfo userFunctionInfo, Closure declaringClosure);
SMILE_API_FUNC SmileFunction SmileFunction_CreateExternalFunction(ExternalFunction externalFunction, void *param,
	const char *name, const char *argNames, Int argCheckFlags, Int minArgs, Int maxArgs, Int numArgsToTypeCheck, const Byte *argTypeChecks);
SMILE_API_FUNC String UserFunctionInfo_ToString(UserFunctionInfo userFunctionInfo);

//-------------------------------------------------------------------------------------------------
//  Inline functions

Inline Bool SmileFunction_IsBuiltIn(SmileFunction fn)
{
	return (fn->kind & SMILE_FLAG_EXTERNAL_FUNCTION) != 0;
}

Inline void SmileFunction_GetArgCounts(SmileFunction fn, Int *minArgs, Int *maxArgs)
{
	if (SmileFunction_IsBuiltIn(fn)) {
		*minArgs = fn->u.externalFunctionInfo.minArgs;
		*maxArgs = fn->u.externalFunctionInfo.maxArgs;
	}
	else {
		*minArgs = fn->u.u.userFunctionInfo->minArgs;
		*maxArgs = fn->u.u.userFunctionInfo->maxArgs;
	}
}

#endif
