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
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/easyobject.h>

SMILE_EASY_OBJECT_VTABLE(SmileInteger32);

SmileInteger32 SmileInteger32_CreateInternal(Int32 value)
{
	SmileInteger32 smileInt = GC_MALLOC_STRUCT(struct SmileInteger32Int);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Integer32;
	smileInt->kind = SMILE_KIND_INTEGER32;
	smileInt->vtable = SmileInteger32_VTable;
	smileInt->value = value;
	return smileInt;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileInteger32)
SMILE_EASY_OBJECT_NO_CALL(SmileInteger32)
SMILE_EASY_OBJECT_NO_SOURCE(SmileInteger32)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileInteger32)

SMILE_EASY_OBJECT_COMPARE(SmileInteger32, SMILE_KIND_INTEGER32, a->value == b->value)
SMILE_EASY_OBJECT_HASH(SmileInteger32, obj->value)
SMILE_EASY_OBJECT_TOBOOL(SmileInteger32, obj->value != 0)
SMILE_EASY_OBJECT_TOINT(SmileInteger32, obj->value)
SMILE_EASY_OBJECT_TOREAL(SmileInteger32, Real64_FromInt32(obj->value))
SMILE_EASY_OBJECT_TOFLOAT(SmileInteger32, (Float64)obj->value)
SMILE_EASY_OBJECT_TOSTRING(SmileInteger32, String_Format("%dt", (Int32)obj->value))
