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
#include <smile/env/knownbases.h>
#include <smile/env/knownobjects.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilenull.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>

void KnownBases_Preload(struct KnownBasesStruct *knownBases, SmileObject baseObject)
{
	knownBases->ActorBase = SmileUserObject_Create(baseObject);

	knownBases->NumberBase = SmileUserObject_Create((SmileObject)knownBases->ActorBase);
	knownBases->IntegerBase = SmileUserObject_Create((SmileObject)knownBases->NumberBase);
	knownBases->RealBase = SmileUserObject_Create((SmileObject)knownBases->NumberBase);
	knownBases->FloatBase = SmileUserObject_Create((SmileObject)knownBases->NumberBase);

	knownBases->EnumerableBase = SmileUserObject_Create((SmileObject)knownBases->ActorBase);

	knownBases->ArrayBase = SmileUserObject_Create((SmileObject)knownBases->EnumerableBase);
	knownBases->NumericArrayBase = SmileUserObject_Create((SmileObject)knownBases->ArrayBase);
	knownBases->IntegerArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArrayBase);
	knownBases->RealArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArrayBase);
	knownBases->FloatArrayBase = SmileUserObject_Create((SmileObject)knownBases->NumericArrayBase);

	knownBases->RangeBase = SmileUserObject_Create((SmileObject)knownBases->EnumerableBase);
	knownBases->NumericRangeBase = SmileUserObject_Create((SmileObject)knownBases->RangeBase);
	knownBases->IntegerRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRangeBase);
	knownBases->RealRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRangeBase);
	knownBases->FloatRangeBase = SmileUserObject_Create((SmileObject)knownBases->NumericRangeBase);
}
