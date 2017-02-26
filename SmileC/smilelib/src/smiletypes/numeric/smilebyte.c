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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/easyobject.h>

SMILE_EASY_OBJECT_VTABLE(SmileByte);

SmileByte SmileByte_CreateInternal(Byte value)
{
	SmileByte smileByte = GC_MALLOC_STRUCT(struct SmileByteInt);
	if (smileByte == NULL) Smile_Abort_OutOfMemory();
	smileByte->base = (SmileObject)Smile_KnownBases.Byte;
	smileByte->kind = SMILE_KIND_BYTE;
	smileByte->vtable = SmileByte_VTable;
	smileByte->value = value;
	return smileByte;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileByte)
SMILE_EASY_OBJECT_NO_CALL(SmileByte)
SMILE_EASY_OBJECT_NO_SOURCE(SmileByte)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileByte)

SMILE_EASY_OBJECT_COMPARE(SmileByte, SMILE_KIND_BYTE, a->value == b->value)
SMILE_EASY_OBJECT_HASH(SmileByte, obj->value)
SMILE_EASY_OBJECT_TOBOOL(SmileByte, obj->value != 0)
SMILE_EASY_OBJECT_TOINT(SmileByte, obj->value)
SMILE_EASY_OBJECT_TOREAL(SmileByte, Real64_FromInt32(obj->value))
SMILE_EASY_OBJECT_TOFLOAT(SmileByte, (Float64)obj->value)
SMILE_EASY_OBJECT_TOSTRING(SmileByte, String_Format("%ux", (UInt32)obj->value))
