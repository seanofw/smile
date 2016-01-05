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

#include <smile/string.h>
#include <smile/env/knownstrings.h>
#include <smile/smiletypes/text/smilestring.h>

STATIC_STRING(ObjectString, "Object");
STATIC_STRING(true_, "true");
STATIC_STRING(false_, "false");
STATIC_STRING(InvalidSecurityKey, "Invalid security key.");

void KnownStrings_Preload(struct KnownStringsStruct *knownStrings)
{
	knownStrings->Object = SmileString_Create(ObjectString);
	knownStrings->true_ = SmileString_Create(true_);
	knownStrings->false_ = SmileString_Create(false_);
	knownStrings->InvalidSecurityKey = SmileString_Create(InvalidSecurityKey);
}
