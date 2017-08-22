//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2017 Sean Werkema
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

#include <smile/numeric/real64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilenull.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/internal/staticstring.h>

SMILE_EASY_OBJECT_VTABLE(SmileNull);

SmileNull SmileNull_Create(void)
{
	SmileNull smileNull = GC_MALLOC_STRUCT(struct SmileListInt);
	if (smileNull == NULL) Smile_Abort_OutOfMemory();
	smileNull->base = (SmileObject)Smile_KnownBases.List;
	smileNull->kind = SMILE_KIND_NULL;
	smileNull->vtable = SmileNull_VTable;
	smileNull->a = (SmileObject)smileNull;
	smileNull->d = (SmileObject)smileNull;
	return smileNull;
}

static Bool SmileNull_CompareEqual(SmileNull self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	UNUSED(self);
	UNUSED(selfData);
	UNUSED(otherData);
	return (SMILE_KIND(other) == SMILE_KIND_NULL);
}

static Bool SmileNull_DeepEqual(SmileNull self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	UNUSED(self);
	UNUSED(selfData);
	UNUSED(otherData);
	UNUSED(visitedPointers);
	return (SMILE_KIND(other) == SMILE_KIND_NULL);
}

STATIC_STRING(NullString, "null");

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileNull)
SMILE_EASY_OBJECT_NO_CALL(SmileNull, "Null")
SMILE_EASY_OBJECT_NO_SOURCE(SmileNull)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileNull)
SMILE_EASY_OBJECT_NO_UNBOX(SmileNull)

SMILE_EASY_OBJECT_HASH(SmileNull, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileNull, 0)
SMILE_EASY_OBJECT_TOSTRING(SmileNull, NullString)
