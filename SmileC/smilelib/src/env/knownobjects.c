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
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilenull.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>

void KnownObjects_Preload(struct KnownObjectsStruct *knownObjects)
{
	Int32 i;

	knownObjects->Object = SmileObject_Create();
	knownObjects->NullInstance = SmileNull_Create();

	for (i = -100; i <= 100; i++) {
		knownObjects->SmallInt32s[i + 100] = SmileInteger32_CreateInternal(i);
	}
	knownObjects->ZeroInt32 = knownObjects->SmallInt32s[0 + 100];
	knownObjects->OneInt32 = knownObjects->SmallInt32s[1 + 100];

	for (i = -100; i <= 100; i++) {
		knownObjects->SmallInt64s[i + 100] = SmileInteger64_CreateInternal(i);
	}
	knownObjects->ZeroInt64 = knownObjects->SmallInt64s[0 + 100];
	knownObjects->OneInt64 = knownObjects->SmallInt64s[1 + 100];

	knownObjects->TrueObj = SmileBool_Create(True);
	knownObjects->FalseObj = SmileBool_Create(False);
}
