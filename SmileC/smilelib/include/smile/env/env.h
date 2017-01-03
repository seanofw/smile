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

#ifndef __SMILE_ENV_KNOWNBASES_H__
#include <smile/env/knownbases.h>
#endif

//-------------------------------------------------------------------------------------------------
//  The core Smile implementation

SMILE_API_DATA SymbolTable Smile_SymbolTable;
SMILE_API_DATA struct KnownSymbolsStruct Smile_KnownSymbols;
SMILE_API_DATA struct KnownStringsStruct Smile_KnownStrings;
SMILE_API_DATA struct KnownObjectsStruct Smile_KnownObjects;
SMILE_API_DATA struct KnownBasesStruct Smile_KnownBases;

SMILE_API_FUNC void Smile_ThrowException(Symbol exceptionKind, String message);

SMILE_API_FUNC void Smile_ResetEnvironment(void);

#define Null (Smile_KnownObjects.NullInstance)
#define NullList ((SmileList)Smile_KnownObjects.NullInstance)
#define NullObject ((SmileObject)Smile_KnownObjects.NullInstance)

#endif
