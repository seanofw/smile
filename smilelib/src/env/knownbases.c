//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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
	knownBases->Number = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Number_);

	knownBases->IntegerBase = SmileUserObject_Create((SmileObject)knownBases->Number, Smile_KnownSymbols.IntegerBase_);

	knownBases->Byte = SmileUserObject_Create((SmileObject)knownBases->IntegerBase, Smile_KnownSymbols.Byte_);
	knownBases->Integer16 = SmileUserObject_Create((SmileObject)knownBases->IntegerBase, Smile_KnownSymbols.Integer16_);
	knownBases->Integer32 = SmileUserObject_Create((SmileObject)knownBases->IntegerBase, Smile_KnownSymbols.Integer32_);
	knownBases->Integer64 = SmileUserObject_Create((SmileObject)knownBases->IntegerBase, Smile_KnownSymbols.Integer64_);
	knownBases->Integer128 = SmileUserObject_Create((SmileObject)knownBases->IntegerBase, Smile_KnownSymbols.Integer128_);

	knownBases->RealBase = SmileUserObject_Create((SmileObject)knownBases->Number, Smile_KnownSymbols.RealBase_);

	knownBases->Real32 = SmileUserObject_Create((SmileObject)knownBases->RealBase, Smile_KnownSymbols.Real32_);
	knownBases->Real64 = SmileUserObject_Create((SmileObject)knownBases->RealBase, Smile_KnownSymbols.Real64_);
	knownBases->Real128 = SmileUserObject_Create((SmileObject)knownBases->RealBase, Smile_KnownSymbols.Real128_);

	knownBases->FloatBase = SmileUserObject_Create((SmileObject)knownBases->Number, Smile_KnownSymbols.FloatBase_);

	knownBases->Float32 = SmileUserObject_Create((SmileObject)knownBases->FloatBase, Smile_KnownSymbols.Float32_);
	knownBases->Float64 = SmileUserObject_Create((SmileObject)knownBases->FloatBase, Smile_KnownSymbols.Float64_);
	knownBases->Float128 = SmileUserObject_Create((SmileObject)knownBases->FloatBase, Smile_KnownSymbols.Float128_);
}

static void SetupNumericRangeTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->NumericRange = SmileUserObject_Create((SmileObject)knownBases->Range, Smile_KnownSymbols.NumericRange_);

	knownBases->IntegerRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRange, Smile_KnownSymbols.IntegerRangeBase_);

	knownBases->ByteRange = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase, Smile_KnownSymbols.ByteRange_);
	knownBases->Integer16Range = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase, Smile_KnownSymbols.Integer16Range_);
	knownBases->Integer32Range = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase, Smile_KnownSymbols.Integer32Range_);
	knownBases->Integer64Range = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase, Smile_KnownSymbols.Integer64Range_);
	knownBases->Integer128Range = SmileUserObject_Create((SmileObject)knownBases->IntegerRangeBase, Smile_KnownSymbols.Integer128Range_);

	knownBases->RealRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRange, Smile_KnownSymbols.RealRangeBase_);

	knownBases->Real32Range = SmileUserObject_Create((SmileObject)knownBases->RealRangeBase, Smile_KnownSymbols.Real32Range_);
	knownBases->Real64Range = SmileUserObject_Create((SmileObject)knownBases->RealRangeBase, Smile_KnownSymbols.Real64Range_);
	knownBases->Real128Range = SmileUserObject_Create((SmileObject)knownBases->RealRangeBase, Smile_KnownSymbols.Real128Range_);

	knownBases->FloatRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRange, Smile_KnownSymbols.FloatRangeBase_);

	knownBases->Float32Range = SmileUserObject_Create((SmileObject)knownBases->FloatRangeBase, Smile_KnownSymbols.Float32Range_);
	knownBases->Float64Range = SmileUserObject_Create((SmileObject)knownBases->FloatRangeBase, Smile_KnownSymbols.Float64Range_);
	knownBases->Float128Range = SmileUserObject_Create((SmileObject)knownBases->FloatRangeBase, Smile_KnownSymbols.Float128Range_);
}

static void SetupNumericArrayTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->NumericArray = SmileUserObject_Create((SmileObject)knownBases->ArrayBase, Smile_KnownSymbols.NumericArray_);

	knownBases->IntegerArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArray, Smile_KnownSymbols.IntegerArrayBase_);

	knownBases->ByteArray = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase, Smile_KnownSymbols.ByteArray_);
	knownBases->Integer16Array = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase, Smile_KnownSymbols.Integer16Array_);
	knownBases->Integer32Array = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase, Smile_KnownSymbols.Integer32Array_);
	knownBases->Integer64Array = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase, Smile_KnownSymbols.Integer64Array_);
	knownBases->Integer128Array = SmileUserObject_Create((SmileObject)knownBases->IntegerArrayBase, Smile_KnownSymbols.Integer128Array_);

	knownBases->RealArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArray, Smile_KnownSymbols.RealArrayBase_);

	knownBases->Real32Array = SmileUserObject_Create((SmileObject)knownBases->RealArrayBase, Smile_KnownSymbols.Real32Array_);
	knownBases->Real64Array = SmileUserObject_Create((SmileObject)knownBases->RealArrayBase, Smile_KnownSymbols.Real64Array_);
	knownBases->Real128Array = SmileUserObject_Create((SmileObject)knownBases->RealArrayBase, Smile_KnownSymbols.Real128Array_);

	knownBases->FloatArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArray, Smile_KnownSymbols.FloatArrayBase_);

	knownBases->Float32Array = SmileUserObject_Create((SmileObject)knownBases->FloatArrayBase, Smile_KnownSymbols.Float32Array_);
	knownBases->Float64Array = SmileUserObject_Create((SmileObject)knownBases->FloatArrayBase, Smile_KnownSymbols.Float64Array_);
	knownBases->Float128Array = SmileUserObject_Create((SmileObject)knownBases->FloatArrayBase, Smile_KnownSymbols.Float128Array_);
}

static void SetupNumericMapTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->NumericMap = SmileUserObject_Create((SmileObject)knownBases->MapBase, Smile_KnownSymbols.NumericMap_);

	knownBases->IntegerMapBase = SmileUserObject_Create((SmileObject)knownBases->NumericMap, Smile_KnownSymbols.IntegerMapBase_);

	knownBases->ByteMap = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase, Smile_KnownSymbols.ByteMap_);
	knownBases->Integer16Map = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase, Smile_KnownSymbols.Integer16Map_);
	knownBases->Integer32Map = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase, Smile_KnownSymbols.Integer32Map_);
	knownBases->Integer64Map = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase, Smile_KnownSymbols.Integer64Map_);
	knownBases->Integer128Map = SmileUserObject_Create((SmileObject)knownBases->IntegerMapBase, Smile_KnownSymbols.Integer128Map_);

	knownBases->RealMapBase = SmileUserObject_Create((SmileObject)knownBases->NumericMap, Smile_KnownSymbols.RealMapBase_);

	knownBases->Real32Map = SmileUserObject_Create((SmileObject)knownBases->RealMapBase, Smile_KnownSymbols.Real32Map_);
	knownBases->Real64Map = SmileUserObject_Create((SmileObject)knownBases->RealMapBase, Smile_KnownSymbols.Real64Map_);
	knownBases->Real128Map = SmileUserObject_Create((SmileObject)knownBases->RealMapBase, Smile_KnownSymbols.Real128Map_);

	knownBases->FloatMapBase = SmileUserObject_Create((SmileObject)knownBases->NumericMap, Smile_KnownSymbols.FloatMapBase_);

	knownBases->Float32Map = SmileUserObject_Create((SmileObject)knownBases->FloatMapBase, Smile_KnownSymbols.Float32Map_);
	knownBases->Float64Map = SmileUserObject_Create((SmileObject)knownBases->FloatMapBase, Smile_KnownSymbols.Float64Map_);
	knownBases->Float128Map = SmileUserObject_Create((SmileObject)knownBases->FloatMapBase, Smile_KnownSymbols.Float128Map_);
}

static void SetupRangeTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->Range = SmileUserObject_Create((SmileObject)knownBases->Enumerable, Smile_KnownSymbols.Range_);

	knownBases->UniRange = SmileUserObject_Create((SmileObject)knownBases->Range, Smile_KnownSymbols.UniRange_);
	knownBases->CharRange = SmileUserObject_Create((SmileObject)knownBases->Range, Smile_KnownSymbols.CharRange_);

	SetupNumericRangeTypes(knownBases);
}

static void SetupArrayTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->ArrayBase = SmileUserObject_Create((SmileObject)knownBases->Enumerable, Smile_KnownSymbols.ArrayBase_);

	knownBases->Array = SmileUserObject_Create((SmileObject)knownBases->ArrayBase, Smile_KnownSymbols.Array_);

	knownBases->BoolArray = SmileUserObject_Create((SmileObject)knownBases->ArrayBase, Smile_KnownSymbols.BoolArray_);
	knownBases->StringArray = SmileUserObject_Create((SmileObject)knownBases->ArrayBase, Smile_KnownSymbols.StringArray_);
	knownBases->SymbolArray = SmileUserObject_Create((SmileObject)knownBases->ArrayBase, Smile_KnownSymbols.SymbolArray_);

	SetupNumericArrayTypes(knownBases);
}

static void SetupMapTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->MapBase = SmileUserObject_Create((SmileObject)knownBases->Enumerable, Smile_KnownSymbols.MapBase_);

	knownBases->Map = SmileUserObject_Create((SmileObject)knownBases->MapBase, Smile_KnownSymbols.Map_);
	knownBases->StringMap = SmileUserObject_Create((SmileObject)knownBases->MapBase, Smile_KnownSymbols.StringMap_);
	knownBases->SymbolMap = SmileUserObject_Create((SmileObject)knownBases->MapBase, Smile_KnownSymbols.SymbolMap_);

	SetupNumericMapTypes(knownBases);
}

static void SetupEnumerableTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->Enumerable = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Enumerable_);

	SetupRangeTypes(knownBases);
	SetupArrayTypes(knownBases);
	SetupMapTypes(knownBases);

	knownBases->List = SmileUserObject_Create((SmileObject)knownBases->Enumerable, Smile_KnownSymbols.List_);

	knownBases->String = &String_BaseObjectStruct;
	SmileUserObject_Init(&String_BaseObjectStruct, (SmileObject)knownBases->Enumerable, Smile_KnownSymbols.String_);
}

static void SetupMiscTypes(struct KnownBasesStruct *knownBases)
{
	knownBases->Fn = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Fn_);
	knownBases->Bool = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Bool_);
	knownBases->Char = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Char_);
	knownBases->Uni = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Uni_);
	knownBases->Symbol = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Symbol_);
	knownBases->Exception = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Exception_);
	knownBases->Handle = SmileUserObject_Create((SmileObject)knownBases->Object, Smile_KnownSymbols.Handle_);

	knownBases->Regex = SmileUserObject_Create((SmileObject)knownBases->Handle, Smile_KnownSymbols.Regex_);
	knownBases->RegexMatch = SmileUserObject_Create((SmileObject)knownBases->Handle, Smile_KnownSymbols.RegexMatch_);
}

void KnownBases_Preload(struct KnownBasesStruct *knownBases)
{
	knownBases->Primitive = SmileObject_Create();

	knownBases->Object = SmileUserObject_Create((SmileObject)knownBases->Primitive, Smile_KnownSymbols.Object_);
	
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
extern void SmileReal32_Setup(SmileUserObject base);
extern void SmileReal64_Setup(SmileUserObject base);
extern void SmileFloat32_Setup(SmileUserObject base);
extern void SmileFloat64_Setup(SmileUserObject base);

extern void SmileCharRange_Setup(SmileUserObject base);
extern void SmileUniRange_Setup(SmileUserObject base);
extern void SmileByteRange_Setup(SmileUserObject base);
extern void SmileInteger16Range_Setup(SmileUserObject base);
extern void SmileInteger32Range_Setup(SmileUserObject base);
extern void SmileInteger64Range_Setup(SmileUserObject base);
extern void SmileReal32Range_Setup(SmileUserObject base);
extern void SmileReal64Range_Setup(SmileUserObject base);
extern void SmileFloat32Range_Setup(SmileUserObject base);
extern void SmileFloat64Range_Setup(SmileUserObject base);

extern void SmileFunction_Setup(SmileUserObject base);
extern void SmileList_Setup(SmileUserObject base);
extern void SmileObject_Setup(SmileUserObject base);
extern void String_Setup(SmileUserObject base);

extern void SmileChar_Setup(SmileUserObject base);
extern void SmileUni_Setup(SmileUserObject base);
extern void SmileSymbol_Setup(SmileUserObject base);

extern void SmileRegex_Setup(SmileUserObject base);
extern void SmileRegexMatch_Setup(SmileUserObject base);

void KnownBases_Setup(struct KnownBasesStruct *knownBases)
{
	SmileByte_Setup(knownBases->Byte);
	SmileByteArray_Setup(knownBases->ByteArray);
	SmileInteger16_Setup(knownBases->Integer16);
	SmileInteger32_Setup(knownBases->Integer32);
	SmileInteger64_Setup(knownBases->Integer64);
	SmileReal32_Setup(knownBases->Real32);
	SmileReal64_Setup(knownBases->Real64);
	SmileFloat32_Setup(knownBases->Float32);
	SmileFloat64_Setup(knownBases->Float64);

	SmileFunction_Setup(knownBases->Fn);
	SmileList_Setup(knownBases->List);
	SmileObject_Setup(knownBases->Object);
	String_Setup(knownBases->String);

	SmileCharRange_Setup(knownBases->CharRange);
	SmileUniRange_Setup(knownBases->UniRange);
	SmileByteRange_Setup(knownBases->ByteRange);
	SmileInteger16Range_Setup(knownBases->Integer16Range);
	SmileInteger32Range_Setup(knownBases->Integer32Range);
	SmileInteger64Range_Setup(knownBases->Integer64Range);
	SmileReal32Range_Setup(knownBases->Real32Range);
	SmileReal64Range_Setup(knownBases->Real64Range);
	SmileFloat32Range_Setup(knownBases->Float32Range);
	SmileFloat64Range_Setup(knownBases->Float64Range);

	SmileChar_Setup(knownBases->Char);
	SmileUni_Setup(knownBases->Uni);
	SmileSymbol_Setup(knownBases->Symbol);

	SmileUnboxedBool_Instance->base = (SmileObject)knownBases->Bool;
	SmileUnboxedSymbol_Instance->base = (SmileObject)knownBases->Symbol;

	SmileRegex_Setup(knownBases->Regex);
	SmileRegexMatch_Setup(knownBases->RegexMatch);
}
