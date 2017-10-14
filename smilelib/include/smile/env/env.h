#ifndef __SMILE_ENV_ENV_H__
#define __SMILE_ENV_ENV_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_MEM_H__
#include <smile/mem.h>
#endif

#ifndef __SMILE_ARRAY_H__
#include <smile/array.h>
#endif

#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif

#ifndef __SMILE_DICT_INT32INT32DICT_H__
#include <smile/dict/int32int32dict.h>
#endif

#ifndef __SMILE_DICT_STRINGDICT_H__
#include <smile/dict/stringdict.h>
#endif

#ifndef __SMILE_DICT_STRINGINTDICT_H__
#include <smile/dict/stringintdict.h>
#endif

#ifndef __SMILE_DICT_POINTERSET_H__
#include <smile/dict/pointerset.h>
#endif

#ifndef __SMILE_ENV_SYMBOLTABLE_H__
#include <smile/env/symboltable.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_ENV_KNOWNSYMBOLS_H__
#include <smile/env/knownsymbols.h>
#endif

#ifndef __SMILE_ENV_KNOWNSTRINGS_H__
#include <smile/env/knownstrings.h>
#endif

#ifndef __SMILE_ENV_KNOWNOBJECTS_H__
#include <smile/env/knownobjects.h>
#endif

#ifndef __SMILE_ENV_KNOWNBASES_H__
#include <smile/env/knownbases.h>
#endif

//-------------------------------------------------------------------------------------------------
//  The core Smile implementation

SMILE_API_DATA SymbolTable Smile_SymbolTable;
SMILE_API_DATA struct KnownSymbolsStruct Smile_KnownSymbols;
SMILE_API_DATA struct KnownObjectsStruct Smile_KnownObjects;
SMILE_API_DATA struct KnownBasesStruct Smile_KnownBases;

SMILE_API_FUNC void Smile_ResetEnvironment(void);

#define Null (Smile_KnownObjects.NullInstance)
#define NullList ((SmileList)Smile_KnownObjects.NullInstance)
#define NullObject ((SmileObject)Smile_KnownObjects.NullInstance)

//-------------------------------------------------------------------------------------------------
//  Global-variable and global-evaluation helper functions.

SMILE_API_FUNC void Smile_InitCommonGlobals(ClosureInfo globalClosureInfo);

SMILE_API_FUNC ClosureInfo Smile_GetGlobalClosureInfo(void);
SMILE_API_FUNC void Smile_SetGlobalClosureInfo(ClosureInfo globalClosureInfo);
SMILE_API_FUNC void Smile_SetGlobalVariable(Symbol name, SmileObject value);
SMILE_API_FUNC Bool Smile_HasGlobalVariable(Symbol name);
SMILE_API_FUNC void Smile_DeleteGlobalVariable(Symbol name);
SMILE_API_FUNC SmileObject Smile_GetGlobalVariable(Symbol name);
SMILE_API_FUNC SmileObject Smile_GetGlobalVariableC(const char *name);

SMILE_API_FUNC SmileObject Smile_Parse(String text, String filename, struct ParseMessageStruct ***parseMessages, Int *numParseMessages);
SMILE_API_FUNC SmileObject Smile_ParseInScope(ClosureInfo globalClosureInfo, String text, String filename, struct ParseMessageStruct ***parseMessages, Int *numParseMessages);
SMILE_API_FUNC EvalResult Smile_Eval(SmileObject expression);
SMILE_API_FUNC EvalResult Smile_EvalInScope(ClosureInfo globalClosureInfo, SmileObject expression);
SMILE_API_FUNC EvalResult Smile_ParseAndEval(String text, String filename);
SMILE_API_FUNC EvalResult Smile_ParseAndEvalInScope(ClosureInfo globalClosureInfo, String text, String filename);

SMILE_API_DATA Bool Stdio_Invoked;

/// <summary>
/// Assign a variable in the global closure.
/// </summary>
/// <param name="name">The name of the variable to assign, as a symbol.</param>
/// <param name="value">The value for the variable to assign.</param>
Inline void Smile_SetGlobalVariableC(const char *name, SmileObject value)
{
	Smile_SetGlobalVariable(SymbolTable_GetSymbolC(Smile_SymbolTable, name), value);
}

/// <summary>
/// Parse the given source code (text) into a complete object structure with all of its syntax resolved.
/// <summary>
/// <param name="text">The actual Smile source code to parse.</param>
/// <param name="filename">The name of the file that source code came from (for error-reporting).</param>
/// <param name="parseMessages">This will be set to an array of any errors or warnings that were generated while parsing.</param>
/// <param name="numParseMessages">This will be set to the number of entries in the parseMessages array.</param>
/// <returns>The fully-parsed source code, or NullObject if there was a parsing error.</returns>
Inline SmileObject Smile_ParseC(const char *text, const char *filename, struct ParseMessageStruct ***parseMessages, Int *numParseMessages)
{
	return Smile_Parse(String_Create((const Byte *)text, StrLen(text)), String_Create((const Byte *)filename, StrLen(filename)), parseMessages, numParseMessages);
}

/// <summary>
/// Parse and evaluate the given source code in the global scope.
/// </summary>
/// <param name="text">The actual Smile source code to parse.</param>
/// <param name="filename">The name of the file that source code came from (for error-reporting).</param>
/// <returns>The result of parsing, compiling, and evaluating the given expression in the given global scope.</returns>
Inline EvalResult Smile_ParseAndEvalC(const char *text, const char *filename)
{
	return Smile_ParseAndEval(String_Create((const Byte *)text, StrLen(text)), String_Create((const Byte *)filename, StrLen(filename)));
}

//-------------------------------------------------------------------------------------------------
//  Exception support

SMILE_API_FUNC void SMILE_NO_RETURN Smile_Throw(SmileObject object);
SMILE_API_FUNC SmileUserObject Smile_CreateException(Symbol exceptionKind, String message);
SMILE_API_FUNC SmileUserObject Smile_CreateExceptionC(const char *exceptionKind, const char *format, ...);
SMILE_API_FUNC SmileUserObject Smile_CreateExceptionCV(const char *exceptionKind, const char *format, va_list v);
SMILE_API_FUNC void SMILE_NO_RETURN Smile_ThrowException(Symbol exceptionKind, String message);
SMILE_API_FUNC void SMILE_NO_RETURN Smile_ThrowExceptionC(const char *exceptionKind, const char *format, ...);
SMILE_API_FUNC void SMILE_NO_RETURN Smile_ThrowExceptionCV(const char *exceptionKind, const char *format, va_list v);

SMILE_API_FUNC String Smile_Win32_GetErrorString(UInt32 errorCode);
SMILE_API_FUNC String Smile_Unix_GetErrorString(Int32 errorCode);

SMILE_API_FUNC String Smile_FormatStackTrace(SmileList stackTrace);

#endif
