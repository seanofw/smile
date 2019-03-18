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
#include <smile/env/knownobjects.h>
#include <smile/env/knownsymbols.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilenull.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smileinteger128.h>

static struct SmileByteInt _bytes[256];
static struct SmileCharInt _chars[256];
static struct SmileUniInt _unis[1024];

void KnownObjects_Setup(struct KnownObjectsStruct *knownObjects, struct KnownSymbolsStruct *knownSymbols)
{
	Int32 i;
	Int128 i128;

	knownObjects->NullInstance = SmileNull_Create();

	for (i = 0; i < 256; i++) {
		knownObjects->Chars[i] = SmileChar_Init(&_chars[i], (Byte)i);
	}

	for (i = 0; i < 1024; i++) {
		knownObjects->Unis[i] = SmileUni_Init(&_unis[i], (UInt32)i);
	}

	for (i = 0; i < 256; i++) {
		knownObjects->Bytes[i] = SmileByte_Init(&_bytes[i], (Byte)i);
	}

	knownObjects->ZeroByte = knownObjects->Bytes[0];
	knownObjects->OneByte = knownObjects->Bytes[1];
	knownObjects->NegOneByte = knownObjects->Bytes[255];

	for (i = -100; i <= 100; i++) {
		knownObjects->SmallInt16s[i + 100] = SmileInteger16_CreateInternal((Int16)i);
		knownObjects->SmallInt32s[i + 100] = SmileInteger32_CreateInternal(i);
		knownObjects->SmallInt64s[i + 100] = SmileInteger64_CreateInternal((Int64)i);
		i128.hi = (Int64)i >> 31;
		i128.lo = (Int64)i;
		knownObjects->SmallInt128s[i + 100] = SmileInteger128_CreateInternal(i128);
	}

	knownObjects->ZeroInt16 = knownObjects->SmallInt16s[0 + 100];
	knownObjects->ZeroInt32 = knownObjects->SmallInt32s[0 + 100];
	knownObjects->ZeroInt64 = knownObjects->SmallInt64s[0 + 100];
	knownObjects->ZeroInt128 = knownObjects->SmallInt128s[0 + 100];

	knownObjects->OneInt16 = knownObjects->SmallInt16s[1 + 100];
	knownObjects->OneInt32 = knownObjects->SmallInt32s[1 + 100];
	knownObjects->OneInt64 = knownObjects->SmallInt64s[1 + 100];
	knownObjects->OneInt128 = knownObjects->SmallInt128s[1 + 100];

	knownObjects->NegOneInt16 = knownObjects->SmallInt16s[-1 + 100];
	knownObjects->NegOneInt32 = knownObjects->SmallInt32s[-1 + 100];
	knownObjects->NegOneInt64 = knownObjects->SmallInt64s[-1 + 100];
	knownObjects->NegOneInt128 = knownObjects->SmallInt128s[-1 + 100];

	knownObjects->TrueObj = SmileBool_Create(True);
	knownObjects->FalseObj = SmileBool_Create(False);
	knownObjects->BooleanObjs[0] = knownObjects->FalseObj;
	knownObjects->BooleanObjs[1] = knownObjects->TrueObj;

	knownObjects->ObjectSymbol = SmileSymbol_Create(knownSymbols->Object_);
	knownObjects->ListSymbol = SmileSymbol_Create(knownSymbols->List_);
	knownObjects->RegexSymbol = SmileSymbol_Create(knownSymbols->Regex_);

	knownObjects->_setSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__SET);
	knownObjects->_opsetSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__OPSET);
	knownObjects->_includeSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__INCLUDE);
	knownObjects->_ifSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__IF);
	knownObjects->_whileSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__WHILE);
	knownObjects->_tillSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__TILL);
	knownObjects->_fnSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__FN);
	knownObjects->_quoteSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__QUOTE);
	knownObjects->_scopeSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__SCOPE);
	knownObjects->_prog1Symbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__PROG1);
	knownObjects->_prognSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__PROGN);
	knownObjects->_returnSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__RETURN);
	knownObjects->_catchSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__CATCH);
	knownObjects->_notSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__NOT);
	knownObjects->_orSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__OR);
	knownObjects->_andSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__AND);
	knownObjects->_eqSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__EQ);
	knownObjects->_neSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__NE);
	knownObjects->_newSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__NEW);
	knownObjects->_dotSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__DOT);
	knownObjects->_indexSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__INDEX);
	knownObjects->_isSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__IS);
	knownObjects->_typeofSymbol = SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__TYPEOF);

	knownObjects->getMemberSymbol = SmileSymbol_Create(knownSymbols->get_member);
	knownObjects->setMemberSymbol = SmileSymbol_Create(knownSymbols->set_member);
	knownObjects->ofSymbol = SmileSymbol_Create(knownSymbols->of);
	knownObjects->combineSymbol = SmileSymbol_Create(knownSymbols->combine);
	knownObjects->consSymbol = SmileSymbol_Create(knownSymbols->cons);
	knownObjects->typeSymbol = SmileSymbol_Create(knownSymbols->type);
	knownObjects->defaultSymbol = SmileSymbol_Create(knownSymbols->default_);
	knownObjects->restSymbol = SmileSymbol_Create(knownSymbols->rest);
	knownObjects->joinSymbol = SmileSymbol_Create(knownSymbols->join);
	knownObjects->rangeToSymbol = SmileSymbol_Create(knownSymbols->range_to);

	knownObjects->eqSymbol = SmileSymbol_Create(knownSymbols->eq);
	knownObjects->neSymbol = SmileSymbol_Create(knownSymbols->ne);
	knownObjects->ltSymbol = SmileSymbol_Create(knownSymbols->lt);
	knownObjects->gtSymbol = SmileSymbol_Create(knownSymbols->gt);
	knownObjects->leSymbol = SmileSymbol_Create(knownSymbols->le);
	knownObjects->geSymbol = SmileSymbol_Create(knownSymbols->ge);
	knownObjects->plusSymbol = SmileSymbol_Create(knownSymbols->plus);
	knownObjects->minusSymbol = SmileSymbol_Create(knownSymbols->minus);
	knownObjects->starSymbol = SmileSymbol_Create(knownSymbols->star);
	knownObjects->slashSymbol = SmileSymbol_Create(knownSymbols->slash);
}
