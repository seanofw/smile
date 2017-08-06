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
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/eval/eval.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(pair, "Pair");

	return SmileArg_From((SmileObject)pair);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	return SmileUnboxedInteger64_From((PtrInt)argv[0].obj ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Construction functions.

SMILE_EXTERNAL_FUNCTION(Of)
{
	SmileObject left, right;
	SmilePair pair;
	SmileUserObject base = (SmileUserObject)param;
	Int i;
	STATIC_STRING(argumentError, "Pair.of requires exactly two arguments, a left and a right object.");

	i = 0;
	if (argv[i].obj == (SmileObject)base)
		i++;

	if (i >= argc)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, argumentError);
	left = SmileArg_Box(argv[i++]);

	if (i >= argc)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, argumentError);
	right = SmileArg_Box(argv[i++]);

	pair = SmilePair_Create(left, right);

	return SmileArg_From((SmileObject)pair);
}

//-------------------------------------------------------------------------------------------------

void SmilePair_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "pair", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "pair", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "pair", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "pair", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "left, right", 0, 0, 0, 0, NULL);
}
