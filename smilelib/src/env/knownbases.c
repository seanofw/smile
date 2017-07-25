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

#include <smile/env/env.h>
#include <smile/env/knownbases.h>
#include <smile/env/knownobjects.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilenull.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>

struct SmileUserObjectInt String_BaseObjectStruct = { 0 };

static void SetupNumericTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->Number = SmileUserObject_Create((SmileObject)knownBases->Object);

	knownBases->IntegerBase = SmileUserObject_Create((SmileObject)knownBases->Number);

	knownBases->Byte = SmileUserObject_Create((SmileObject)knownBases->IntegerBase);
	knownBases->Integer16 = SmileUserObject_Create((SmileObject)knownBases->IntegerBase);
	knownBases->Integer32 = SmileUserObject_Create((SmileObject)knownBases->IntegerBase);
	knownBases->Integer64 = SmileUserObject_Create((SmileObject)knownBases->IntegerBase);
	knownBases->Integer128 = SmileUserObject_Create((SmileObject)knownBases->IntegerBase);

	knownBases->RealBase = SmileUserObject_Create((SmileObject)knownBases->Number);

	knownBases->Real32 = SmileUserObject_Create((SmileObject)knownBases->RealBase);
	knownBases->Real64 = SmileUserObject_Create((SmileObject)knownBases->RealBase);
	knownBases->Real128 = SmileUserObject_Create((SmileObject)knownBases->RealBase);

	knownBases->FloatBase = SmileUserObject_Create((SmileObject)knownBases->Number);

	knownBases->Float32 = SmileUserObject_Create((SmileObject)knownBases->FloatBase);
	knownBases->Float64 = SmileUserObject_Create((SmileObject)knownBases->FloatBase);
	knownBases->Float128 = SmileUserObject_Create((SmileObject)knownBases->FloatBase);
}

static void SetupNumericRangeTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->NumericRange = SmileUserObject_Create((SmileObject)knownBases->Range);

	knownBases->IntegerRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRange);

	knownBases->ByteRange = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase);
	knownBases->Integer16Range = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase);
	knownBases->Integer32Range = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase);
	knownBases->Integer64Range = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase);
	knownBases->Integer128Range = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase);

	knownBases->RealRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRange);

	knownBases->Real32Range = SmileUserObject_Create((SmileObject)knownBases->RealRangeBase);
	knownBases->Real64Range = SmileUserObject_Create((SmileObject)knownBases->RealRangeBase);
	knownBases->Real128Range = SmileUserObject_Create((SmileObject)knownBases->RealRangeBase);

	knownBases->FloatRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRange);

	knownBases->Float32Range = SmileUserObject_Create((SmileObject)knownBases->FloatRangeBase);
	knownBases->Float64Range = SmileUserObject_Create((SmileObject)knownBases->FloatRangeBase);
	knownBases->Float128Range = SmileUserObject_Create((SmileObject)knownBases->FloatRangeBase);
}

static void SetupNumericArrayTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->NumericArray = SmileUserObject_Create((SmileObject)knownBases->ArrayBase);

	knownBases->IntegerArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArray);

	knownBases->ByteArray = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase);
	knownBases->Integer16Array = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase);
	knownBases->Integer32Array = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase);
	knownBases->Integer64Array = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase);
	knownBases->Integer128Array = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase);

	knownBases->RealArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArray);

	knownBases->Real32Array = SmileUserObject_Create((SmileObject)knownBases->RealArrayBase);
	knownBases->Real64Array = SmileUserObject_Create((SmileObject)knownBases->RealArrayBase);
	knownBases->Real128Array = SmileUserObject_Create((SmileObject)knownBases->RealArrayBase);

	knownBases->FloatArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArray);

	knownBases->Float32Array = SmileUserObject_Create((SmileObject)knownBases->FloatArrayBase);
	knownBases->Float64Array = SmileUserObject_Create((SmileObject)knownBases->FloatArrayBase);
	knownBases->Float128Array = SmileUserObject_Create((SmileObject)knownBases->FloatArrayBase);
}

static void SetupNumericMapTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->NumericMap = SmileUserObject_Create((SmileObject)knownBases->MapBase);

	knownBases->IntegerMapBase = SmileUserObject_Create((SmileObject)knownBases->NumericMap);

	knownBases->ByteMap = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase);
	knownBases->Integer16Map = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase);
	knownBases->Integer32Map = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase);
	knownBases->Integer64Map = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase);
	knownBases->Integer128Map = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase);

	knownBases->RealMapBase = SmileUserObject_Create((SmileObject)knownBases->NumericMap);

	knownBases->Real32Map = SmileUserObject_Create((SmileObject)knownBases->RealMapBase);
	knownBases->Real64Map = SmileUserObject_Create((SmileObject)knownBases->RealMapBase);
	knownBases->Real128Map = SmileUserObject_Create((SmileObject)knownBases->RealMapBase);

	knownBases->FloatMapBase = SmileUserObject_Create((SmileObject)knownBases->NumericMap);

	knownBases->Float32Map = SmileUserObject_Create((SmileObject)knownBases->FloatMapBase);
	knownBases->Float64Map = SmileUserObject_Create((SmileObject)knownBases->FloatMapBase);
	knownBases->Float128Map = SmileUserObject_Create((SmileObject)knownBases->FloatMapBase);
}

static void SetupRangeTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->Range = SmileUserObject_Create((SmileObject)knownBases->Enumerable);

	SetupNumericRangeTypes(knownBases);
}

static void SetupArrayTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->ArrayBase = SmileUserObject_Create((SmileObject)knownBases->Enumerable);

	knownBases->Array = SmileUserObject_Create((SmileObject)knownBases->ArrayBase);

	knownBases->BoolArray = SmileUserObject_Create((SmileObject)knownBases->ArrayBase);
	knownBases->StringArray = SmileUserObject_Create((SmileObject)knownBases->ArrayBase);
	knownBases->SymbolArray = SmileUserObject_Create((SmileObject)knownBases->ArrayBase);

	SetupNumericArrayTypes(knownBases);
}

static void SetupMapTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->MapBase = SmileUserObject_Create((SmileObject)knownBases->Enumerable);

	knownBases->Map = SmileUserObject_Create((SmileObject)knownBases->MapBase);
	knownBases->StringMap = SmileUserObject_Create((SmileObject)knownBases->MapBase);
	knownBases->SymbolMap = SmileUserObject_Create((SmileObject)knownBases->MapBase);

	SetupNumericMapTypes(knownBases);
}

static void SetupEnumerableTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->Enumerable = SmileUserObject_Create((SmileObject)knownBases->Object);

	SetupRangeTypes(knownBases);
	SetupArrayTypes(knownBases);
	SetupMapTypes(knownBases);

	knownBases->List = SmileUserObject_Create((SmileObject)knownBases->Enumerable);

	knownBases->String = &String_BaseObjectStruct;
	SmileUserObject_Init(&String_BaseObjectStruct, (SmileObject)knownBases->Enumerable);
}

static void SetupMiscTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->Pair = SmileUserObject_Create((SmileObject)knownBases->Object);
	knownBases->Function = SmileUserObject_Create((SmileObject)knownBases->Object);
	knownBases->Bool = SmileUserObject_Create((SmileObject)knownBases->Object);
	knownBases->Symbol = SmileUserObject_Create((SmileObject)knownBases->Object);
	knownBases->Exception = SmileUserObject_Create((SmileObject)knownBases->Object);
	knownBases->Handle = SmileUserObject_Create((SmileObject)knownBases->Object);
}

void KnownBases_Preload(struct KnownBasesStruct *knownBases)
{
	knownBases->Primitive = SmileObject_Create();

	knownBases->Object = SmileUserObject_Create((SmileObject)knownBases->Primitive);
	
	SetupNumericTypes(knownBases);
	SetupEnumerableTypes(knownBases);
	SetupMiscTypes(knownBases);
}

//-------------------------------------------------------------------------------------------------

extern void SmileByte_Setup(SmileUserObject base);
extern void SmileByteArray_Setup(SmileUserObject base);
extern void SmileInteger16_Setup(SmileUserObject base);
extern void SmileInteger32_Setup(SmileUserObject base);
extern void SmileInteger64_Setup(SmileUserObject base);
extern void SmileList_Setup(SmileUserObject base);
extern void SmilePair_Setup(SmileUserObject base);
extern void String_Setup(SmileUserObject base);

void KnownBases_Setup(struct KnownBasesStruct *knownBases)
{
	SmileByte_Setup(knownBases->Byte);
	SmileByteArray_Setup(knownBases->ByteArray);
	SmileInteger16_Setup(knownBases->Integer16);
	SmileInteger32_Setup(knownBases->Integer32);
	SmileInteger64_Setup(knownBases->Integer64);
	SmileList_Setup(knownBases->List);
	SmilePair_Setup(knownBases->Pair);
	String_Setup(knownBases->String);

	SmileUnboxedBool_Instance->base = (SmileObject)knownBases->Bool;
	SmileUnboxedSymbol_Instance->base = (SmileObject)knownBases->Symbol;
}
