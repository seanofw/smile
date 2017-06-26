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

#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/eval/eval.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/raw/smilebytearray.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_BYTEARRAY)
		return SmileUnboxedBool_From(String_Length((String)argv[0].obj) > 0);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING)
		return SmileUnboxedInteger64_From(String_Length((String)argv[0].obj));

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(byteArrayString, "ByteArray");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_BYTEARRAY) {
		SmileByteArray byteArray = (SmileByteArray)(argv[0].obj);
		String result = String_Create(byteArray->data, byteArray->length);
		return SmileArg_From((SmileObject)result);
	}
	else {
		return SmileArg_From((SmileObject)byteArrayString);
	}
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	return SmileUnboxedInteger64_From(((PtrInt)argv[0].obj) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------

void SmileByteArray_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	/*
		SetupFunction("get-member", GetMember, NULL, "byte-array index", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringNumberChecks);
		SetupFunction("set-member", SetMember, NULL, "byte-array index value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringNumberChecks);

		SetupFunction("each", Each, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
		SetupFunction("map", Map, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
		SetupFunction("where", Where, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
		SetupFunction("count", Count, NULL, "string", ARG_STATE_MACHINE, 0, 0, 0, NULL);
	*/
}
