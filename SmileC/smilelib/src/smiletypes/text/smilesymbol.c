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
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileSymbol);

SmileSymbol SmileSymbol_Create(Symbol symbol)
{
	SmileSymbol smileInt = GC_MALLOC_STRUCT(struct SmileSymbolInt);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Symbol;
	smileInt->kind = SMILE_KIND_SYMBOL;
	smileInt->vtable = SmileSymbol_VTable;
	smileInt->symbol = symbol;
	return smileInt;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileSymbol)
SMILE_EASY_OBJECT_NO_CALL(SmileSymbol)
SMILE_EASY_OBJECT_NO_SOURCE(SmileSymbol)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileSymbol)

SMILE_EASY_OBJECT_HASH(SmileSymbol, obj->symbol)
SMILE_EASY_OBJECT_TOBOOL(SmileSymbol, True)
SMILE_EASY_OBJECT_TOINT(SmileSymbol, 0)
SMILE_EASY_OBJECT_TOREAL(SmileSymbol, Real64_Zero)
SMILE_EASY_OBJECT_TOFLOAT(SmileSymbol, 0.0)
SMILE_EASY_OBJECT_TOSTRING(SmileSymbol, SymbolTable_GetName(Smile_SymbolTable, obj->symbol))

static Bool SmileSymbol_CompareEqual(SmileSymbol a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_SYMBOL) {
		return ((SmileSymbol)a)->symbol == bData.symbol;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_SYMBOL) {
		return ((SmileSymbol)a)->symbol == ((SmileSymbol)b)->symbol;
	}
	else return False;
}

SmileObject SmileSymbol_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileSymbol_Unbox(SmileSymbol smileSymbol)
{
	return SmileUnboxedSymbol_From(smileSymbol->symbol);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedSymbol);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedSymbol)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedSymbol)
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedSymbol)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedSymbol)

SMILE_EASY_OBJECT_HASH(SmileUnboxedSymbol, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedSymbol, True)
SMILE_EASY_OBJECT_TOINT(SmileUnboxedSymbol, 0)
SMILE_EASY_OBJECT_TOREAL(SmileUnboxedSymbol, Real64_Zero)
SMILE_EASY_OBJECT_TOFLOAT(SmileUnboxedSymbol, 0.0)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedSymbol, SymbolTable_GetName(Smile_SymbolTable, unboxedData.symbol))

static Bool SmileUnboxedSymbol_CompareEqual(SmileUnboxedSymbol a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_SYMBOL) {
		return aData.symbol == bData.symbol;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_SYMBOL) {
		return aData.symbol == ((SmileSymbol)b)->symbol;
	}
	else return False;
}

static SmileObject SmileUnboxedSymbol_Box(SmileArg src)
{
	return (SmileObject)SmileSymbol_Create(src.unboxed.symbol);
}

static SmileArg SmileUnboxedSymbol_Unbox(SmileUnboxedSymbol smileUnboxedSymbol)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg){ 0 };
}

static struct SmileUnboxedSymbolInt SmileUnboxedSymbol_Instance_Struct = {
	SMILE_KIND_UNBOXED_SYMBOL,
	(SmileVTable)&SmileUnboxedSymbol_VTableData,
};

extern SmileUnboxedSymbol SmileUnboxedSymbol_Instance = &SmileUnboxedSymbol_Instance_Struct;
