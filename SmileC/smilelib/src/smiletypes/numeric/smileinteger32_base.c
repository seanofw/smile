//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/smilefunction.h>

#define Setup(__name__, __value__) \
	(SmileUserObject_QuickSet(base, (__name__), (__value__)))

#define SetupFunction(__name__, __function__, __param__, __argNames__, __argCheckFlags__, __minArgs__, __maxArgs__, __argTypeChecks__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction(__function__, __param__, \
		(__name__), (__argNames__), (__argCheckFlags__), (__minArgs__), (__maxArgs__), (__argTypeChecks__))))

#define SetupSimpleFunction(__name__, __function__, __argNames__, __numArgs__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction(__function__, __param__, \
		(__name__), (__argNames__), ARG_CHECK_EXACT, (__numArgs__), (__numArgs__), NULL)))

static Byte _integer32Values[] = {
	SMILE_KIND_INTEGER32, SMILE_KIND_INTEGER32, SMILE_KIND_INTEGER32, SMILE_KIND_INTEGER32
};

STATIC_STRING(_divideByZero, "Divide by zero error");

STATIC_STRING(_invalidTypeError, "All arguments to 'Integer32.%s' must be of type 'Integer32'.");

static SmileObject Add(int argc, SmileObject *argv, void *param)
{
	Int32 x, i;

	UNUSED(param);

	switch (argc) {
		case 0:
			return (SmileObject)Smile_KnownObjects.ZeroInt32;

		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger32)argv[0])->value;
			x += ((SmileInteger32)argv[1])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		case 3:
			x = ((SmileInteger32)argv[0])->value;
			x += ((SmileInteger32)argv[1])->value;
			x += ((SmileInteger32)argv[2])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			x += ((SmileInteger32)argv[1])->value;
			x += ((SmileInteger32)argv[2])->value;
			x += ((SmileInteger32)argv[3])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if (SMILE_KIND(argv[i]) != SMILE_KIND_INTEGER32)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_invalidTypeError, "+"));
				x += ((SmileInteger32)argv[i])->value;
			}
			return (SmileObject)SmileInteger32_Create(x);
	}
}

static SmileObject Sub(int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create(x - y);
}

static SmileObject Mul(int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create(x * y);
}

static SmileObject Div(int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger32_Create(x / y);
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Int32 MathematiciansModulus(Int32 x, Int32 y)
{
	Int32 rem;

	if (x < 0) {
		if (y < 0)
			return -(-x % -y);
		else {
			rem = -x % y;
			return rem != 0 ? y - rem : 0;
		}
	}
	else if (y < 0) {
		rem = x % -y;
		return rem != 0 ? y + rem : 0;
	}
	else
		return x % y;
}

/// <summary>
/// Perform remainder, in which the result has the same sign as the dividend (x).
/// </summary>
Inline Int32 MathematiciansRemainder(Int32 x, Int32 y)
{
	Int32 rem;

	if (x < 0) {
		if (y < 0) {
			rem = -x % -y;
			return rem != 0 ? rem + y : 0;
		}
		else
			return -(-x % y);
	}
	else if (y < 0)
		return x % -y;
	else {
		rem = x % y;
		return rem != 0 ? rem - y : 0;
	}
}

static SmileObject Mod(int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger32_Create(MathematiciansModulus(x, y));
}

static SmileObject Rem(int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger32_Create(MathematiciansRemainder(x, y));
}

void SmileInteger32_Setup(SmileUserObject base)
{
	SetupFunction("+", Add, NULL, "augend addend", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, _integer32Values);
	SetupFunction("-", Sub, NULL, "minuend subtrahend", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, _integer32Values);
	SetupFunction("*", Mul, NULL, "multiplier multiplicand", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, _integer32Values);
	SetupFunction("/", Div, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, _integer32Values);
	SetupFunction("mod", Mod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, _integer32Values);
	SetupFunction("rem", Rem, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, _integer32Values);
}
