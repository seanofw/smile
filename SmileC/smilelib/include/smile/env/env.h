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

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// The Smile runtime environment.
/// </summary>
struct SmileEnvInt {
	SymbolTable symbolTable;					// The symbol table itself.
	struct KnownSymbolsStruct knownSymbols;		// The known (preregistered) symbols.
	struct KnownStringsStruct knownStrings;		// The known (preconstructed) string instances.
	struct KnownObjectsStruct knownObjects;		// The known (preconstructed) object instances.
};

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (core)

SMILE_API SmileEnv SmileEnv_Create(void);
SMILE_API void SmileEnv_ThrowException(SmileEnv env, Symbol exceptionKind, String message);

#endif
