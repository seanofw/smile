// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL64)
		return SmileUnboxedBool_From(!Real64_IsZero(argv[0].unboxed.r64));

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL64)
		return SmileUnboxedInteger64_From(Real64_ToInt64(argv[0].unboxed.r64));

	return SmileUnboxedReal64_From(Real64_Zero);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(real64, "Real64");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL64) {
		return SmileArg_From((SmileObject)(Real64_ToStringEx(argv[0].unboxed.r64, 0, 0, False)));
	}

	return SmileArg_From((SmileObject)real64);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileReal64 obj = (SmileReal64)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_REAL64)
		return SmileUnboxedInteger64_From((UInt32)(*(UInt64 *)&obj->value ^ (*(UInt64 *)&obj->value >> 32)));

	return SmileUnboxedInteger64_From((UInt32)((PtrInt)obj ^ Smile_HashOracle));
}

//-------------------------------------------------------------------------------------------------

void SmileReal64_Setup(SmileUserObject base)
{
	SmileUnboxedReal64_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
}
