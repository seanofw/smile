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

#include <smile/env/env.h>
#include <smile/env/knownobjects.h>
#include <smile/env/knownsymbols.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilenull.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuchar.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>

void KnownObjects_Preload(struct KnownObjectsStruct *knownObjects, struct KnownSymbolsStruct *knownSymbols)
{
	Int32 i;

	knownObjects->Object = SmileObject_Create();
	knownObjects->NullInstance = SmileNull_Create();

	for (i = 0; i <= 255; i++) {
		knownObjects->Bytes[i] = SmileByte_CreateInternal((Byte)i);
		knownObjects->Chars[i] = SmileChar_CreateInternal((Byte)i);
		knownObjects->UChars[i] = SmileUChar_CreateInternal(i);
	}

	knownObjects->ZeroByte = knownObjects->Bytes[0];
	knownObjects->OneByte = knownObjects->Bytes[1];

	for (i = -100; i <= 100; i++) {
		knownObjects->SmallInt16s[i + 100] = SmileInteger16_CreateInternal((Int16)i);
		knownObjects->SmallInt32s[i + 100] = SmileInteger32_CreateInternal(i);
		knownObjects->SmallInt64s[i + 100] = SmileInteger64_CreateInternal((Int64)i);
	}

	knownObjects->ZeroInt16 = knownObjects->SmallInt16s[0 + 100];
	knownObjects->ZeroInt32 = knownObjects->SmallInt32s[0 + 100];
	knownObjects->ZeroInt64 = knownObjects->SmallInt64s[0 + 100];

	knownObjects->OneInt16 = knownObjects->SmallInt16s[1 + 100];
	knownObjects->OneInt32 = knownObjects->SmallInt32s[1 + 100];
	knownObjects->OneInt64 = knownObjects->SmallInt64s[1 + 100];

	knownObjects->TrueObj = SmileBool_Create(True);
	knownObjects->FalseObj = SmileBool_Create(False);

	knownObjects->ObjectSymbol = SmileSymbol_Create(knownSymbols->Object_);
	knownObjects->ListSymbol = SmileSymbol_Create(knownSymbols->List_);
	knownObjects->PairSymbol = SmileSymbol_Create(knownSymbols->Pair_);
	knownObjects->RangeSymbol = SmileSymbol_Create(knownSymbols->Range_);

	knownObjects->fnSymbol = SmileSymbol_Create(knownSymbols->fn_);
	knownObjects->quoteSymbol = SmileSymbol_Create(knownSymbols->quote_);
	knownObjects->joinSymbol = SmileSymbol_Create(knownSymbols->join);
	knownObjects->ofSymbol = SmileSymbol_Create(knownSymbols->of);
	knownObjects->prognSymbol = SmileSymbol_Create(knownSymbols->progn_);
	knownObjects->scopeSymbol = SmileSymbol_Create(knownSymbols->scope_);
	knownObjects->opEqualsSymbol = SmileSymbol_Create(knownSymbols->op_equals_);
	knownObjects->equalsSymbol = SmileSymbol_Create(knownSymbols->equals_);
	knownObjects->typeSymbol = SmileSymbol_Create(knownSymbols->type);
	knownObjects->newSymbol = SmileSymbol_Create(knownSymbols->new_);

	knownObjects->orSymbol = SmileSymbol_Create(knownSymbols->or_);
	knownObjects->andSymbol = SmileSymbol_Create(knownSymbols->and_);
	knownObjects->notSymbol = SmileSymbol_Create(knownSymbols->not_);

	knownObjects->eqSymbol = SmileSymbol_Create(knownSymbols->eq);
	knownObjects->neSymbol = SmileSymbol_Create(knownSymbols->ne);
	knownObjects->ltSymbol = SmileSymbol_Create(knownSymbols->lt);
	knownObjects->gtSymbol = SmileSymbol_Create(knownSymbols->gt);
	knownObjects->leSymbol = SmileSymbol_Create(knownSymbols->le);
	knownObjects->geSymbol = SmileSymbol_Create(knownSymbols->ge);
	knownObjects->supereqSymbol = SmileSymbol_Create(knownSymbols->supereq_);
	knownObjects->superneSymbol = SmileSymbol_Create(knownSymbols->superne_);
	knownObjects->isSymbol = SmileSymbol_Create(knownSymbols->is_);

	knownObjects->plusSymbol = SmileSymbol_Create(knownSymbols->plus);
	knownObjects->minusSymbol = SmileSymbol_Create(knownSymbols->minus);
	knownObjects->starSymbol = SmileSymbol_Create(knownSymbols->star);
	knownObjects->slashSymbol = SmileSymbol_Create(knownSymbols->slash);

	knownObjects->typeofSymbol = SmileSymbol_Create(knownSymbols->typeof_);

	knownObjects->getMemberSymbol = SmileSymbol_Create(knownSymbols->get_member);
	knownObjects->setMemberSymbol = SmileSymbol_Create(knownSymbols->set_member);
}
