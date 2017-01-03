//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include <smile/internal/types.h>
#include <smile/gc.h>
#include <smile/env/env.h>
#include <smile/env/symboltable.h>
#include <smile/env/knownsymbols.h>
#include <smile/env/knownobjects.h>
#include <smile/env/knownbases.h>

extern void Smile_InitTicks(void);

/// <summary>
/// Whether or not the Smile runtime has been initialized yet.
/// </summary>
static Bool Smile_IsInitialized = False;

/// <summary>
/// The global hash oracle, which ensures that successive runs of the same program yield
/// different hash codes.  This ensures that people do not attempt to make assumptions about
/// the order or values of their hash codes.
/// </summary>
UInt32 Smile_HashOracle;

// The environment.
SymbolTable Smile_SymbolTable;
struct KnownSymbolsStruct Smile_KnownSymbols;
struct KnownStringsStruct Smile_KnownStrings;
struct KnownObjectsStruct Smile_KnownObjects;
struct KnownBasesStruct Smile_KnownBases;

/// <summary>
/// Initialize the Smile runtime.  This must be performed at least once on startup.
/// </summary>
void Smile_Init(void)
{
	if (Smile_IsInitialized) return;

	Smile_InitTicks();

	GC_INIT();

	Smile_HashOracle = (UInt32)GetBaselineEntropy();

	Smile_ResetEnvironment();
}

void Smile_ResetEnvironment(void)
{
	// Make a symbol table for this environment.
	Smile_SymbolTable = SymbolTable_Create();

	// Preload the known symbols into this environment.
	KnownSymbols_PreloadSymbolTable(Smile_SymbolTable, &Smile_KnownSymbols);

	// Preload the known bases into this environment.  This must come first, or we can't
	// correctly instantiate any other Smile types.
	KnownBases_Preload(&Smile_KnownBases);

	// Preload the known strings into this environment.
	KnownStrings_Setup(&Smile_KnownStrings);

	// Preload the known objects into this environment.
	KnownObjects_Setup(&Smile_KnownObjects, &Smile_KnownSymbols);

	// Now that we have enough of a usable environment to make real objects, it's time to populate
	// all of the methods on the base objects, and to set up any other shared data they may need.
	KnownBases_Setup(&Smile_KnownBases);
}

/// <summary>
/// Shut down the Smile runtime.  This must be performed at least once on program shutdown.
/// </summary>
void Smile_End(void)
{
}

/// <summary>
/// Abort execution of the program because the runtime has run out of memory.
/// </summary>
void Smile_Abort_OutOfMemory(void)
{
	Smile_Abort_FatalError("Out of memory!");
}

/// <summary>
/// Abort execution of the program because the runtime has run out of memory.
/// </summary>
void Smile_Abort_FatalError(const char *message)
{
	fprintf(stderr, "Fatal error:  %s  Aborting program.", message);
	exit(-1);
}

void Smile_ThrowException(Symbol exceptionKind, String message)
{
	UNUSED(exceptionKind);
	UNUSED(message);

	Smile_Abort_FatalError(String_ToC(message));
}
