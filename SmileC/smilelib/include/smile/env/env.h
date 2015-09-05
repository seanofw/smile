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

#ifndef __SMILE_ENV_KNOWNSYMBOLS_H__
#include <smile/env/knownsymbols.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// The Smile runtime environment.
/// </summary>
typedef struct SmileEnvInt {
	SymbolTable symbolTable;					// The symbol table itself.
	struct KnownSymbolsStruct knownSymbols;		// The known (preregistered) symbols.
} *SmileEnv;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (core)

SMILE_API SmileEnv SmileEnv_Create(void);

#endif
