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

#include <smile/env/env.h>
#include <smile/env/symboltable.h>
#include <smile/env/knownsymbols.h>

SmileEnv SmileEnv_Create(void)
{
	SmileEnv smileEnv;

	// Create the environment itself.
	smileEnv = GC_MALLOC_STRUCT(struct SmileEnvInt);
	if (smileEnv == NULL) Smile_Abort_OutOfMemory();

	// Make a symbol table for this environment.
	smileEnv->symbolTable = SymbolTable_Create();

	// Preload the known symbols into this environment.
	KnownSymbols_PreloadSymbolTable(smileEnv->symbolTable, &smileEnv->knownSymbols);

	return smileEnv;
}
