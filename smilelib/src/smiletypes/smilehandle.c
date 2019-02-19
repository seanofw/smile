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

#include <smile/numeric/real.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilehandle.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/stringbuilder.h>
#include <smile/internal/staticstring.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/env/knownbases.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileHandle);

static struct SmileHandleMethodsStruct SmileHandle_DefaultMethods = { 0 };

//-------------------------------------------------------------------------------------------------
// Construction/destruction

static void SmileHandle_Finalize(SmileHandle handle, void *param)
{
	if (handle->methods->end != NULL) {
		handle->methods->end(handle, False);
	}

	handle->ptr = NULL;
	handle->methods = NULL;
	handle->base = (SmileObject)Smile_KnownBases.Object;
	handle->kind = 0;
}

SmileHandle SmileHandle_Create(SmileObject base, SmileHandleMethods methods, Symbol handleKind, void *ptr)
{
	SmileHandle smileHandle = GC_MALLOC_STRUCT(struct SmileHandleInt);
	if (smileHandle == NULL) Smile_Abort_OutOfMemory();

	smileHandle->base = base;
	smileHandle->kind = SMILE_KIND_HANDLE;
	smileHandle->vtable = SmileHandle_VTable;
	smileHandle->methods = methods != NULL ? methods : &SmileHandle_DefaultMethods;
	smileHandle->handleKind = handleKind;
	smileHandle->ptr = ptr;

	GC_REGISTER_FINALIZER(smileHandle, SmileHandle_Finalize, NULL, NULL, NULL);

	return smileHandle;
}

//-------------------------------------------------------------------------------------------------
// Predefined virtual methods

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileHandle)
SMILE_EASY_OBJECT_NO_CALL(SmileHandle, "A Handle object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileHandle)
SMILE_EASY_OBJECT_NO_UNBOX(SmileHandle)

SMILE_EASY_OBJECT_HASH(SmileHandle, Smile_ApplyHashOracle((PtrInt)obj->ptr))

//-------------------------------------------------------------------------------------------------
// Proxy virtual methods, with default implementations

static Bool SmileHandle_ToBool(SmileHandle obj, SmileUnboxedData unboxedData)
{
	return obj->methods->toBool != NULL
		? obj->methods->toBool(obj, unboxedData)
		: True;
}

static String SmileHandle_ToString(SmileHandle obj, SmileUnboxedData unboxedData)
{
	return obj->methods->toString != NULL
		? obj->methods->toString(obj, unboxedData)
		: SymbolTable_GetName(Smile_SymbolTable, obj->handleKind);
}

static SmileObject SmileHandle_GetProperty(SmileHandle obj, Symbol symbol)
{
	return obj->methods->getProperty != NULL
		? obj->methods->getProperty(obj, symbol)
		: obj->base->vtable->getProperty(obj->base, symbol);
}

static Bool SmileHandle_HasProperty(SmileHandle obj, Symbol symbol)
{
	return obj->methods->hasProperty != NULL
		? obj->methods->hasProperty(obj, symbol)
		: False;
}

static void SmileHandle_SetProperty(SmileHandle obj, Symbol symbol, SmileObject value)
{
	if (obj->methods->setProperty != NULL)
		obj->methods->setProperty(obj, symbol, value);
	else {
		Smile_ThrowException(Smile_KnownSymbols.object_security_error,
			String_Format("Cannot set property \"%S\" on a Handle, which is read-only.",
				SymbolTable_GetName(Smile_SymbolTable, symbol)));
	}
}

static SmileList SmileHandle_GetPropertyNames(SmileHandle obj)
{
	return obj->methods->getPropertyNames != NULL
		? obj->methods->getPropertyNames(obj)
		: NullList;
}

//-------------------------------------------------------------------------------------------------
// Actually-implemented-here virtual methods

static Bool SmileHandle_CompareEqual(SmileHandle a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_HANDLE) {
		return ((SmileHandle)a)->handleKind == ((SmileHandle)b)->handleKind
			&& ((SmileHandle)a)->ptr == ((SmileHandle)b)->ptr;
	}
	else return False;
}

static Bool SmileHandle_DeepEqual(SmileHandle a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_HANDLE) {
		return ((SmileHandle)a)->handleKind == ((SmileHandle)b)->handleKind
			&& ((SmileHandle)a)->ptr == ((SmileHandle)b)->ptr;
	}
	else return False;
}
