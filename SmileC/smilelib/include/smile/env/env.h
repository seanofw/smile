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
//  The core Smile implementation

#ifdef _MSC_VER
	SMILE_API SymbolTable Smile_SymbolTable;
	SMILE_API struct KnownSymbolsStruct Smile_KnownSymbols;
	SMILE_API struct KnownStringsStruct Smile_KnownStrings;
	SMILE_API struct KnownObjectsStruct Smile_KnownObjects;
	SMILE_API struct KnownBasesStruct Smile_KnownBases;
#else
	SMILE_API extern SymbolTable Smile_SymbolTable;
	SMILE_API extern struct KnownSymbolsStruct Smile_KnownSymbols;
	SMILE_API extern struct KnownStringsStruct Smile_KnownStrings;
	SMILE_API extern struct KnownObjectsStruct Smile_KnownObjects;
	SMILE_API extern struct KnownBasesStruct Smile_KnownBases;
#endif

SMILE_API void Smile_ThrowException(Symbol exceptionKind, String message);

SMILE_API void Smile_ResetEnvironment(void);

#endif
