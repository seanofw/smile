//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2018 Sean Werkema
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

#include <smile/numeric/real.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smiletillcontinuation.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/stringbuilder.h>
#include <smile/internal/staticstring.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileTillContinuation);

SmileTillContinuation SmileTillContinuation_Create(SmileObject base, Closure closure,
	struct CompiledTablesStruct *compiledTables, ByteCodeSegment segment,
	Int32 *branchTargetAddresses, Int numBranchTargetAddresses)
{
	SmileTillContinuation smileTillContinuation = GC_MALLOC_STRUCT(struct SmileTillContinuationInt);
	if (smileTillContinuation == NULL) Smile_Abort_OutOfMemory();

	smileTillContinuation->base = base;
	smileTillContinuation->kind = SMILE_KIND_TILL_CONTINUATION;
	smileTillContinuation->vtable = SmileTillContinuation_VTable;
	smileTillContinuation->closure = closure;
	smileTillContinuation->compiledTables = compiledTables;
	smileTillContinuation->segment = segment;
	smileTillContinuation->branchTargetAddresses = branchTargetAddresses;
	smileTillContinuation->numBranchTargetAddresses = numBranchTargetAddresses;

	return smileTillContinuation;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileTillContinuation)
SMILE_EASY_OBJECT_NO_CALL(SmileTillContinuation, "A TillContinuation object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileTillContinuation)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileTillContinuation)
SMILE_EASY_OBJECT_NO_UNBOX(SmileTillContinuation)

SMILE_EASY_OBJECT_HASH(SmileTillContinuation, (UInt32)(PtrInt)obj ^ Smile_HashOracle)
SMILE_EASY_OBJECT_TOBOOL(SmileTillContinuation, True)
SMILE_EASY_OBJECT_TOSTRING(SmileTillContinuation, String_FromC("till-continuation"))

static Bool SmileTillContinuation_CompareEqual(SmileTillContinuation a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	return (SmileObject)a == b;
}

static Bool SmileTillContinuation_DeepEqual(SmileTillContinuation a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	return (SmileObject)a == b;
}
