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
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/easyobject.h>

SMILE_EASY_OBJECT_VTABLE(SmileInteger16);

SmileInteger16 SmileInteger16_CreateInternal(Int16 value)
{
	SmileInteger16 smileInt = GC_MALLOC_STRUCT(struct SmileInteger16Int);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Integer16;
	smileInt->kind = SMILE_KIND_INTEGER16;
	smileInt->vtable = SmileInteger16_VTable;
	smileInt->value = value;
	return smileInt;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileInteger16)
SMILE_EASY_OBJECT_NO_CALL(SmileInteger16)
SMILE_EASY_OBJECT_NO_SOURCE(SmileInteger16)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileInteger16)

SMILE_EASY_OBJECT_COMPARE(SmileInteger16, SMILE_KIND_INTEGER16, a->value == b->value)
SMILE_EASY_OBJECT_HASH(SmileInteger16, obj->value)
SMILE_EASY_OBJECT_TOBOOL(SmileInteger16, obj->value != 0)
SMILE_EASY_OBJECT_TOINT(SmileInteger16, obj->value)
SMILE_EASY_OBJECT_TOREAL(SmileInteger16, Real64_FromInt32(obj->value))
SMILE_EASY_OBJECT_TOFLOAT(SmileInteger16, (Float64)obj->value)
SMILE_EASY_OBJECT_TOSTRING(SmileInteger16, String_Format("%ds", (Int32)obj->value))
