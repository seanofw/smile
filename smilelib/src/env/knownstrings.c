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

#include <smile/string.h>
#include <smile/internal/staticstring.h>
#include <smile/env/knownstrings.h>

EXTERN_STATIC_STRING(String_Object, "Object");
EXTERN_STATIC_STRING(String_True, "true");
EXTERN_STATIC_STRING(String_False, "false");
EXTERN_STATIC_STRING(String_InvalidSecurityKey, "Invalid security key.");
EXTERN_STATIC_STRING(String_InvalidFunctionError, "%s cannot be called because it is not a function.");
