//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

/// <summary>
/// Initialize the Smile runtime.  This must be performed at least once on startup.
/// </summary>
void Smile_Init(void)
{
	if (Smile_IsInitialized) return;

	GC_INIT();

	Smile_HashOracle = (UInt32)GetBaselineEntropy();
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
