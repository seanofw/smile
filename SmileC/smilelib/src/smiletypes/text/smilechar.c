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

#include <smile/numeric/real64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilechar.h>

static struct StringInt *_charStringTable[256];

SmileChar SmileChar_CreateInternal(Byte value)
{
	SmileChar smileChar = GC_MALLOC_STRUCT(struct SmileCharInt);
	if (smileChar == NULL) Smile_Abort_OutOfMemory();
	smileChar->base = (SmileObject)Smile_KnownBases.Char;
	smileChar->kind = SMILE_KIND_CHAR;
	smileChar->vtable = SmileChar_VTable;
	smileChar->value = value;
	return smileChar;
}

Bool SmileChar_CompareEqual(SmileChar self, SmileObject other)
{
	SmileChar otherInt;

	if (SMILE_KIND(other) != SMILE_KIND_BYTE) return False;
	otherInt = (SmileChar)other;

	return self->value == otherInt->value;
}

UInt32 SmileChar_Hash(SmileChar self)
{
	return self->value;
}

void SmileChar_SetSecurityKey(SmileChar self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileChar_SetSecurity(SmileChar self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileChar_GetSecurity(SmileChar self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileChar_GetProperty(SmileChar self, Symbol propertyName)
{
	return self->base->vtable->getProperty(self->base, propertyName);
}

void SmileChar_SetProperty(SmileChar self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on an integer, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileChar_HasProperty(SmileChar self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileChar_GetPropertyNames(SmileChar self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileChar_ToBool(SmileChar self)
{
	return self->value != 0;
}

Int32 SmileChar_ToInteger32(SmileChar self)
{
	return self->value;
}

Float64 SmileChar_ToFloat64(SmileChar self)
{
	return (Float64)self->value;
}

Real64 SmileChar_ToReal64(SmileChar self)
{
	return Real64_FromInt32(self->value);
}

String SmileChar_ToString(SmileChar self)
{
	return (String)_charStringTable[self->value];
}

Bool SmileChar_Call(SmileChar self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError);

	return True;
}

SMILE_VTABLE(SmileChar_VTable, SmileChar)
{
	SmileChar_CompareEqual,
	SmileChar_Hash,

	SmileChar_SetSecurityKey,
	SmileChar_SetSecurity,
	SmileChar_GetSecurity,

	SmileChar_GetProperty,
	SmileChar_SetProperty,
	SmileChar_HasProperty,
	SmileChar_GetPropertyNames,

	SmileChar_ToBool,
	SmileChar_ToInteger32,
	SmileChar_ToFloat64,
	SmileChar_ToReal64,
	SmileChar_ToString,

	SmileChar_Call,
};

STATIC_STRING(Char00, "\\x00");
STATIC_STRING(Char01, "\\x01");
STATIC_STRING(Char02, "\\x02");
STATIC_STRING(Char03, "\\x03");
STATIC_STRING(Char04, "\\x04");
STATIC_STRING(Char05, "\\x05");
STATIC_STRING(Char06, "\\x06");
STATIC_STRING(Char07, "\\a");
STATIC_STRING(Char08, "\\b");
STATIC_STRING(Char09, "\\t");
STATIC_STRING(Char0A, "\\n");
STATIC_STRING(Char0B, "\\v");
STATIC_STRING(Char0C, "\\f");
STATIC_STRING(Char0D, "\\r");
STATIC_STRING(Char0E, "\\x14");
STATIC_STRING(Char0F, "\\x15");

STATIC_STRING(Char10, "\\x16");
STATIC_STRING(Char11, "\\x17");
STATIC_STRING(Char12, "\\x18");
STATIC_STRING(Char13, "\\x19");
STATIC_STRING(Char14, "\\x20");
STATIC_STRING(Char15, "\\x21");
STATIC_STRING(Char16, "\\x22");
STATIC_STRING(Char17, "\\x23");
STATIC_STRING(Char18, "\\x24");
STATIC_STRING(Char19, "\\x25");
STATIC_STRING(Char1A, "\\x26");
STATIC_STRING(Char1B, "\\e");
STATIC_STRING(Char1C, "\\x28");
STATIC_STRING(Char1D, "\\x29");
STATIC_STRING(Char1E, "\\x30");
STATIC_STRING(Char1F, "\\x31");

STATIC_STRING(Char20, " ");
STATIC_STRING(Char21, "!");
STATIC_STRING(Char22, "\"");
STATIC_STRING(Char23, "#");
STATIC_STRING(Char24, "$");
STATIC_STRING(Char25, "%");
STATIC_STRING(Char26, "&");
STATIC_STRING(Char27, "\\'");
STATIC_STRING(Char28, "(");
STATIC_STRING(Char29, ")");
STATIC_STRING(Char2A, "*");
STATIC_STRING(Char2B, "+");
STATIC_STRING(Char2C, ",");
STATIC_STRING(Char2D, "-");
STATIC_STRING(Char2E, ".");
STATIC_STRING(Char2F, "/");

STATIC_STRING(Char30, "0");
STATIC_STRING(Char31, "1");
STATIC_STRING(Char32, "2");
STATIC_STRING(Char33, "3");
STATIC_STRING(Char34, "4");
STATIC_STRING(Char35, "5");
STATIC_STRING(Char36, "6");
STATIC_STRING(Char37, "7");
STATIC_STRING(Char38, "8");
STATIC_STRING(Char39, "9");
STATIC_STRING(Char3A, ":");
STATIC_STRING(Char3B, ";");
STATIC_STRING(Char3C, "<");
STATIC_STRING(Char3D, "=");
STATIC_STRING(Char3E, ">");
STATIC_STRING(Char3F, "?");

STATIC_STRING(Char40, "@");
STATIC_STRING(Char41, "A");
STATIC_STRING(Char42, "B");
STATIC_STRING(Char43, "C");
STATIC_STRING(Char44, "D");
STATIC_STRING(Char45, "E");
STATIC_STRING(Char46, "F");
STATIC_STRING(Char47, "G");
STATIC_STRING(Char48, "H");
STATIC_STRING(Char49, "I");
STATIC_STRING(Char4A, "J");
STATIC_STRING(Char4B, "K");
STATIC_STRING(Char4C, "L");
STATIC_STRING(Char4D, "M");
STATIC_STRING(Char4E, "N");
STATIC_STRING(Char4F, "O");

STATIC_STRING(Char50, "P");
STATIC_STRING(Char51, "Q");
STATIC_STRING(Char52, "R");
STATIC_STRING(Char53, "S");
STATIC_STRING(Char54, "T");
STATIC_STRING(Char55, "U");
STATIC_STRING(Char56, "V");
STATIC_STRING(Char57, "W");
STATIC_STRING(Char58, "X");
STATIC_STRING(Char59, "Y");
STATIC_STRING(Char5A, "Z");
STATIC_STRING(Char5B, "[");
STATIC_STRING(Char5C, "\\");
STATIC_STRING(Char5D, "]");
STATIC_STRING(Char5E, "^");
STATIC_STRING(Char5F, "_");

STATIC_STRING(Char60, "`");
STATIC_STRING(Char61, "a");
STATIC_STRING(Char62, "b");
STATIC_STRING(Char63, "c");
STATIC_STRING(Char64, "d");
STATIC_STRING(Char65, "e");
STATIC_STRING(Char66, "f");
STATIC_STRING(Char67, "g");
STATIC_STRING(Char68, "h");
STATIC_STRING(Char69, "i");
STATIC_STRING(Char6A, "j");
STATIC_STRING(Char6B, "k");
STATIC_STRING(Char6C, "l");
STATIC_STRING(Char6D, "m");
STATIC_STRING(Char6E, "n");
STATIC_STRING(Char6F, "o");

STATIC_STRING(Char70, "p");
STATIC_STRING(Char71, "q");
STATIC_STRING(Char72, "r");
STATIC_STRING(Char73, "s");
STATIC_STRING(Char74, "t");
STATIC_STRING(Char75, "u");
STATIC_STRING(Char76, "v");
STATIC_STRING(Char77, "w");
STATIC_STRING(Char78, "x");
STATIC_STRING(Char79, "y");
STATIC_STRING(Char7A, "z");
STATIC_STRING(Char7B, "{");
STATIC_STRING(Char7C, "|");
STATIC_STRING(Char7D, "}");
STATIC_STRING(Char7E, "~");
STATIC_STRING(Char7F, "\\x7F");

STATIC_STRING(Char80, "\\x80");
STATIC_STRING(Char81, "\\x81");
STATIC_STRING(Char82, "\\x82");
STATIC_STRING(Char83, "\\x83");
STATIC_STRING(Char84, "\\x84");
STATIC_STRING(Char85, "\\x85");
STATIC_STRING(Char86, "\\x86");
STATIC_STRING(Char87, "\\x87");
STATIC_STRING(Char88, "\\x88");
STATIC_STRING(Char89, "\\x89");
STATIC_STRING(Char8A, "\\x8A");
STATIC_STRING(Char8B, "\\x8B");
STATIC_STRING(Char8C, "\\x8C");
STATIC_STRING(Char8D, "\\x8D");
STATIC_STRING(Char8E, "\\x8E");
STATIC_STRING(Char8F, "\\x8F");

STATIC_STRING(Char90, "\\x90");
STATIC_STRING(Char91, "\\x91");
STATIC_STRING(Char92, "\\x92");
STATIC_STRING(Char93, "\\x93");
STATIC_STRING(Char94, "\\x94");
STATIC_STRING(Char95, "\\x95");
STATIC_STRING(Char96, "\\x96");
STATIC_STRING(Char97, "\\x97");
STATIC_STRING(Char98, "\\x98");
STATIC_STRING(Char99, "\\x99");
STATIC_STRING(Char9A, "\\x9A");
STATIC_STRING(Char9B, "\\x9B");
STATIC_STRING(Char9C, "\\x9C");
STATIC_STRING(Char9D, "\\x9D");
STATIC_STRING(Char9E, "\\x9E");
STATIC_STRING(Char9F, "\\x9F");

STATIC_STRING(CharA0, "\\xA0");
STATIC_STRING(CharA1, "\\xA1");
STATIC_STRING(CharA2, "\\xA2");
STATIC_STRING(CharA3, "\\xA3");
STATIC_STRING(CharA4, "\\xA4");
STATIC_STRING(CharA5, "\\xA5");
STATIC_STRING(CharA6, "\\xA6");
STATIC_STRING(CharA7, "\\xA7");
STATIC_STRING(CharA8, "\\xA8");
STATIC_STRING(CharA9, "\\xA9");
STATIC_STRING(CharAA, "\\xAA");
STATIC_STRING(CharAB, "\\xAB");
STATIC_STRING(CharAC, "\\xAC");
STATIC_STRING(CharAD, "\\xAD");
STATIC_STRING(CharAE, "\\xAE");
STATIC_STRING(CharAF, "\\xAF");

STATIC_STRING(CharB0, "\\xB0");
STATIC_STRING(CharB1, "\\xB1");
STATIC_STRING(CharB2, "\\xB2");
STATIC_STRING(CharB3, "\\xB3");
STATIC_STRING(CharB4, "\\xB4");
STATIC_STRING(CharB5, "\\xB5");
STATIC_STRING(CharB6, "\\xB6");
STATIC_STRING(CharB7, "\\xB7");
STATIC_STRING(CharB8, "\\xB8");
STATIC_STRING(CharB9, "\\xB9");
STATIC_STRING(CharBA, "\\xBA");
STATIC_STRING(CharBB, "\\xBB");
STATIC_STRING(CharBC, "\\xBC");
STATIC_STRING(CharBD, "\\xBD");
STATIC_STRING(CharBE, "\\xBE");
STATIC_STRING(CharBF, "\\xBF");

STATIC_STRING(CharC0, "\\xC0");
STATIC_STRING(CharC1, "\\xC1");
STATIC_STRING(CharC2, "\\xC2");
STATIC_STRING(CharC3, "\\xC3");
STATIC_STRING(CharC4, "\\xC4");
STATIC_STRING(CharC5, "\\xC5");
STATIC_STRING(CharC6, "\\xC6");
STATIC_STRING(CharC7, "\\xC7");
STATIC_STRING(CharC8, "\\xC8");
STATIC_STRING(CharC9, "\\xC9");
STATIC_STRING(CharCA, "\\xCA");
STATIC_STRING(CharCB, "\\xCB");
STATIC_STRING(CharCC, "\\xCC");
STATIC_STRING(CharCD, "\\xCD");
STATIC_STRING(CharCE, "\\xCE");
STATIC_STRING(CharCF, "\\xCF");

STATIC_STRING(CharD0, "\\xD0");
STATIC_STRING(CharD1, "\\xD1");
STATIC_STRING(CharD2, "\\xD2");
STATIC_STRING(CharD3, "\\xD3");
STATIC_STRING(CharD4, "\\xD4");
STATIC_STRING(CharD5, "\\xD5");
STATIC_STRING(CharD6, "\\xD6");
STATIC_STRING(CharD7, "\\xD7");
STATIC_STRING(CharD8, "\\xD8");
STATIC_STRING(CharD9, "\\xD9");
STATIC_STRING(CharDA, "\\xDA");
STATIC_STRING(CharDB, "\\xDB");
STATIC_STRING(CharDC, "\\xDC");
STATIC_STRING(CharDD, "\\xDD");
STATIC_STRING(CharDE, "\\xDE");
STATIC_STRING(CharDF, "\\xDF");

STATIC_STRING(CharE0, "\\xE0");
STATIC_STRING(CharE1, "\\xE1");
STATIC_STRING(CharE2, "\\xE2");
STATIC_STRING(CharE3, "\\xE3");
STATIC_STRING(CharE4, "\\xE4");
STATIC_STRING(CharE5, "\\xE5");
STATIC_STRING(CharE6, "\\xE6");
STATIC_STRING(CharE7, "\\xE7");
STATIC_STRING(CharE8, "\\xE8");
STATIC_STRING(CharE9, "\\xE9");
STATIC_STRING(CharEA, "\\xEA");
STATIC_STRING(CharEB, "\\xEB");
STATIC_STRING(CharEC, "\\xEC");
STATIC_STRING(CharED, "\\xED");
STATIC_STRING(CharEE, "\\xEE");
STATIC_STRING(CharEF, "\\xEF");

STATIC_STRING(CharF0, "\\xF0");
STATIC_STRING(CharF1, "\\xF1");
STATIC_STRING(CharF2, "\\xF2");
STATIC_STRING(CharF3, "\\xF3");
STATIC_STRING(CharF4, "\\xF4");
STATIC_STRING(CharF5, "\\xF5");
STATIC_STRING(CharF6, "\\xF6");
STATIC_STRING(CharF7, "\\xF7");
STATIC_STRING(CharF8, "\\xF8");
STATIC_STRING(CharF9, "\\xF9");
STATIC_STRING(CharFA, "\\xFA");
STATIC_STRING(CharFB, "\\xFB");
STATIC_STRING(CharFC, "\\xFC");
STATIC_STRING(CharFD, "\\xFD");
STATIC_STRING(CharFE, "\\xFE");
STATIC_STRING(CharFF, "\\xFF");

static struct StringInt *_charStringTable[256] = {
	&Char00Struct, &Char01Struct, &Char02Struct, &Char03Struct, &Char04Struct, &Char05Struct, &Char06Struct, &Char07Struct,
	&Char08Struct, &Char09Struct, &Char0AStruct, &Char0BStruct, &Char0CStruct, &Char0DStruct, &Char0EStruct, &Char0FStruct,

	&Char10Struct, &Char11Struct, &Char12Struct, &Char13Struct, &Char14Struct, &Char15Struct, &Char16Struct, &Char17Struct,
	&Char18Struct, &Char19Struct, &Char1AStruct, &Char1BStruct, &Char1CStruct, &Char1DStruct, &Char1EStruct, &Char1FStruct,

	&Char20Struct, &Char21Struct, &Char22Struct, &Char23Struct, &Char24Struct, &Char25Struct, &Char26Struct, &Char27Struct,
	&Char28Struct, &Char29Struct, &Char2AStruct, &Char2BStruct, &Char2CStruct, &Char2DStruct, &Char2EStruct, &Char2FStruct,

	&Char30Struct, &Char31Struct, &Char32Struct, &Char33Struct, &Char34Struct, &Char35Struct, &Char36Struct, &Char37Struct,
	&Char38Struct, &Char39Struct, &Char3AStruct, &Char3BStruct, &Char3CStruct, &Char3DStruct, &Char3EStruct, &Char3FStruct,

	&Char40Struct, &Char41Struct, &Char42Struct, &Char43Struct, &Char44Struct, &Char45Struct, &Char46Struct, &Char47Struct,
	&Char48Struct, &Char49Struct, &Char4AStruct, &Char4BStruct, &Char4CStruct, &Char4DStruct, &Char4EStruct, &Char4FStruct,

	&Char50Struct, &Char51Struct, &Char52Struct, &Char53Struct, &Char54Struct, &Char55Struct, &Char56Struct, &Char57Struct,
	&Char58Struct, &Char59Struct, &Char5AStruct, &Char5BStruct, &Char5CStruct, &Char5DStruct, &Char5EStruct, &Char5FStruct,

	&Char60Struct, &Char61Struct, &Char62Struct, &Char63Struct, &Char64Struct, &Char65Struct, &Char66Struct, &Char67Struct,
	&Char68Struct, &Char69Struct, &Char6AStruct, &Char6BStruct, &Char6CStruct, &Char6DStruct, &Char6EStruct, &Char6FStruct,

	&Char70Struct, &Char71Struct, &Char72Struct, &Char73Struct, &Char74Struct, &Char75Struct, &Char76Struct, &Char77Struct,
	&Char78Struct, &Char79Struct, &Char7AStruct, &Char7BStruct, &Char7CStruct, &Char7DStruct, &Char7EStruct, &Char7FStruct,

	&Char80Struct, &Char81Struct, &Char82Struct, &Char83Struct, &Char84Struct, &Char85Struct, &Char86Struct, &Char87Struct,
	&Char88Struct, &Char89Struct, &Char8AStruct, &Char8BStruct, &Char8CStruct, &Char8DStruct, &Char8EStruct, &Char8FStruct,

	&Char90Struct, &Char91Struct, &Char92Struct, &Char93Struct, &Char94Struct, &Char95Struct, &Char96Struct, &Char97Struct,
	&Char98Struct, &Char99Struct, &Char9AStruct, &Char9BStruct, &Char9CStruct, &Char9DStruct, &Char9EStruct, &Char9FStruct,

	&CharA0Struct, &CharA1Struct, &CharA2Struct, &CharA3Struct, &CharA4Struct, &CharA5Struct, &CharA6Struct, &CharA7Struct,
	&CharA8Struct, &CharA9Struct, &CharAAStruct, &CharABStruct, &CharACStruct, &CharADStruct, &CharAEStruct, &CharAFStruct,

	&CharB0Struct, &CharB1Struct, &CharB2Struct, &CharB3Struct, &CharB4Struct, &CharB5Struct, &CharB6Struct, &CharB7Struct,
	&CharB8Struct, &CharB9Struct, &CharBAStruct, &CharBBStruct, &CharBCStruct, &CharBDStruct, &CharBEStruct, &CharBFStruct,

	&CharC0Struct, &CharC1Struct, &CharC2Struct, &CharC3Struct, &CharC4Struct, &CharC5Struct, &CharC6Struct, &CharC7Struct,
	&CharC8Struct, &CharC9Struct, &CharCAStruct, &CharCBStruct, &CharCCStruct, &CharCDStruct, &CharCEStruct, &CharCFStruct,

	&CharD0Struct, &CharD1Struct, &CharD2Struct, &CharD3Struct, &CharD4Struct, &CharD5Struct, &CharD6Struct, &CharD7Struct,
	&CharD8Struct, &CharD9Struct, &CharDAStruct, &CharDBStruct, &CharDCStruct, &CharDDStruct, &CharDEStruct, &CharDFStruct,

	&CharE0Struct, &CharE1Struct, &CharE2Struct, &CharE3Struct, &CharE4Struct, &CharE5Struct, &CharE6Struct, &CharE7Struct,
	&CharE8Struct, &CharE9Struct, &CharEAStruct, &CharEBStruct, &CharECStruct, &CharEDStruct, &CharEEStruct, &CharEFStruct,

	&CharF0Struct, &CharF1Struct, &CharF2Struct, &CharF3Struct, &CharF4Struct, &CharF5Struct, &CharF6Struct, &CharF7Struct,
	&CharF8Struct, &CharF9Struct, &CharFAStruct, &CharFBStruct, &CharFCStruct, &CharFDStruct, &CharFEStruct, &CharFFStruct,
};
