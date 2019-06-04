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

#include <stdlib.h>
#include <smile/types.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/smiletypes/kind.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smileinteger128.h>
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/range/smileinteger64range.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/raw/smilebytearray.h>
#include <smile/smiletypes/smilehandle.h>
#include <smile/internal/staticstring.h>
#include <smile/numeric/float64.h>

static void StringifyRecursive(SmileObject obj, StringBuilder stringBuilder, Int indent, Bool includeSources);

static Bool SmileObject_ContainsNestedList(SmileObject obj)
{
	SmileObject item;
	SmileList list;

	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return False;

	list = (SmileList)obj;

	if (SmileObject_IsCallToSymbol(SMILE_SPECIAL_SYMBOL__DOT, obj)) {
		if (SmileObject_ContainsNestedList(LIST_SECOND(list))) return True;
		if (SmileObject_ContainsNestedList(LIST_THIRD(list))) return True;
		return False;
	}

	for (list = (SmileList)obj; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
		item = list->a;
		if (SMILE_KIND(item) == SMILE_KIND_LIST) {
			if (!SmileObject_IsCallToSymbol(SMILE_SPECIAL_SYMBOL__DOT, item))
				return True;
		}
	}

	return False;
}

String SmileObject_Stringify(SmileObject obj)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 1024);
	INIT_INLINE_STRINGBUILDER(stringBuilder);
	StringifyRecursive(obj, stringBuilder, 0, False);
	return StringBuilder_ToString(stringBuilder);
}

String SmileObject_StringifyWithSource(SmileObject obj)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 1024);
	INIT_INLINE_STRINGBUILDER(stringBuilder);
	StringifyRecursive(obj, stringBuilder, 0, True);
	return StringBuilder_ToString(stringBuilder);
}

const char *SmileObject_StringifyToC(SmileObject obj)
{
	return String_ToC(SmileObject_Stringify(obj));
}

const char *SmileObject_StringifyWithSourceToC(SmileObject obj)
{
	return String_ToC(SmileObject_StringifyWithSource(obj));
}

void SmileObject_StringifyToStringBuilder(StringBuilder stringBuilder, SmileObject obj, Int indent, Bool includeSource)
{
	StringifyRecursive(obj, stringBuilder, indent, includeSource);
}

static int UserObjectKeyComparer(const void *a, const void *b)
{
	const Int32DictKeyValuePair *aPair = (const Int32DictKeyValuePair *)a;
	const Int32DictKeyValuePair *bPair = (const Int32DictKeyValuePair *)b;

	String na = SymbolTable_GetName(Smile_SymbolTable, aPair->key);
	String nb = SymbolTable_GetName(Smile_SymbolTable, bPair->key);

	return (int)String_Compare(na, nb);
}

static void StringifyBadlyFormedList(SmileList list, StringBuilder stringBuilder, Int indent, Bool includeSource)
{
	SmileList tortoise = (SmileList)list, hare;

	if (SMILE_KIND(tortoise) != SMILE_KIND_LIST)
		return;
	StringifyRecursive((SmileObject)tortoise->a, stringBuilder, indent, includeSource);
	StringBuilder_AppendC(stringBuilder, " ## ", 0, 4);
	hare = tortoise = (SmileList)tortoise->d;

	hare = LIST_REST(hare);

	for (;;) {
		if (tortoise == hare) {
			StringBuilder_AppendC(stringBuilder, "->", 0, 2);
			return;
		}
		else if (SMILE_KIND(tortoise) != SMILE_KIND_LIST) {
			StringifyRecursive((SmileObject)list, stringBuilder, indent, includeSource);
			return;
		}
		StringifyRecursive((SmileObject)tortoise->a, stringBuilder, indent, includeSource);
		StringBuilder_AppendC(stringBuilder, " ## ", 0, 4);

		tortoise = (SmileList)tortoise->d;
		hare = LIST_REST(LIST_REST(hare));
	}
}

static void StringifyRecursive(SmileObject obj, StringBuilder stringBuilder, Int indent, Bool includeSource)
{
	SmileList list;
	STATIC_STRING(nullName, "<NULL>");

	if (obj == NULL) {
		StringBuilder_AppendString(stringBuilder, nullName);
		return;
	}

	switch (SMILE_KIND(obj)) {

	case SMILE_KIND_LIST:
		{
			Bool isFirst;

			list = (SmileList)obj;
			if (!SmileList_IsWellFormed(obj)) {
				StringBuilder_AppendByte(stringBuilder, '(');
				StringifyBadlyFormedList(list, stringBuilder, indent + 1, includeSource);
				StringBuilder_AppendByte(stringBuilder, ')');
			}
			else if (SmileObject_ContainsNestedList(obj)) {
				isFirst = True;
				StringBuilder_AppendByte(stringBuilder, '[');
				while (SMILE_KIND(list) == SMILE_KIND_LIST && SMILE_KIND(list->a) == SMILE_KIND_SYMBOL) {
					if (!isFirst) {
						StringBuilder_AppendByte(stringBuilder, ' ');
					}
					StringifyRecursive((SmileObject)list->a, stringBuilder, indent + 1, includeSource);
					list = LIST_REST(list);
					isFirst = False;
				}
				if (includeSource && (list->kind & SMILE_FLAG_WITHSOURCE)) {
					String filename = ((struct SmileListWithSourceInt *)list)->position->filename;
					if (filename != NULL) {
						StringBuilder_AppendFormat(stringBuilder, "\t// %S:%d",
							Path_GetFilename(filename), ((struct SmileListWithSourceInt *)list)->position->line);
					}
					else {
						StringBuilder_AppendFormat(stringBuilder, "\t// line %d",
							((struct SmileListWithSourceInt *)list)->position->line);
					}
				}
				StringBuilder_AppendByte(stringBuilder, '\n');
				while (SMILE_KIND(list) == SMILE_KIND_LIST) {
					StringBuilder_AppendRepeat(stringBuilder, ' ', (indent + 1) * 4);
					StringifyRecursive((SmileObject)list->a, stringBuilder, indent + 1, includeSource);
					if (includeSource && (list->kind & SMILE_FLAG_WITHSOURCE)) {
						String filename = ((struct SmileListWithSourceInt *)list)->position->filename;
						if (filename != NULL) {
							StringBuilder_AppendFormat(stringBuilder, "\t// %S:%d",
								Path_GetFilename(filename), ((struct SmileListWithSourceInt *)list)->position->line);
						}
						else {
							StringBuilder_AppendFormat(stringBuilder, "\t// line %d",
								((struct SmileListWithSourceInt *)list)->position->line);
						}
					}
					StringBuilder_AppendByte(stringBuilder, '\n');
					list = LIST_REST(list);
				}
				StringBuilder_AppendRepeat(stringBuilder, ' ', indent * 4);
				StringBuilder_AppendByte(stringBuilder, ']');
			}
			else {
				isFirst = True;
				StringBuilder_AppendByte(stringBuilder, '[');
				while (SMILE_KIND(list) == SMILE_KIND_LIST) {
					if (!isFirst) {
						StringBuilder_AppendByte(stringBuilder, ' ');
					}
					StringifyRecursive((SmileObject)list->a, stringBuilder, indent + 1, includeSource);
					list = LIST_REST(list);
					isFirst = False;
				}
				StringBuilder_AppendByte(stringBuilder, ']');
			}
		}
		return;

	case SMILE_KIND_PRIMITIVE:
		StringBuilder_AppendC(stringBuilder, "Primitive", 0, 9);
		return;

	case SMILE_KIND_NULL:
		StringBuilder_AppendC(stringBuilder, "null", 0, 4);
		return;

	case SMILE_KIND_BOOL:
		StringBuilder_AppendString(stringBuilder, ((SmileBool)obj)->value ? String_True : String_False);
		return;
	
	case SMILE_KIND_SYMBOL:
		{
			String keyName = SymbolTable_GetName(Smile_SymbolTable, ((SmileSymbol)obj)->symbol);
			StringBuilder_AppendString(stringBuilder, keyName != NULL ? keyName : nullName);
		}
		return;

	case SMILE_KIND_BYTE:
		StringBuilder_AppendFormat(stringBuilder, "%u", (Int32)((SmileByte)obj)->value);
		return;

	case SMILE_KIND_INTEGER16:
		StringBuilder_AppendFormat(stringBuilder, "%d", (Int32)((SmileInteger16)obj)->value);
		return;

	case SMILE_KIND_INTEGER32:
		StringBuilder_AppendFormat(stringBuilder, "%d", ((SmileInteger32)obj)->value);
		return;

	case SMILE_KIND_INTEGER64:
		StringBuilder_AppendFormat(stringBuilder, "%ld", ((SmileInteger64)obj)->value);
		return;

	case SMILE_KIND_REAL32:
		StringBuilder_AppendFormat(stringBuilder, "%S", Real32_ToStringEx(((SmileReal32)obj)->value, 0, 0, False));
		return;

	case SMILE_KIND_REAL64:
		StringBuilder_AppendFormat(stringBuilder, "%S", Real64_ToStringEx(((SmileReal64)obj)->value, 0, 0, False));
		return;

	case SMILE_KIND_FLOAT32:
		StringBuilder_AppendFormat(stringBuilder, "%S", Float64_ToStringEx((double)((SmileFloat32)obj)->value, 0, 6, False));
		return;

	case SMILE_KIND_FLOAT64:
		StringBuilder_AppendFormat(stringBuilder, "%S", Float64_ToStringEx(((SmileFloat64)obj)->value, 0, 15, False));
		return;

	case SMILE_KIND_CHAR:
		StringBuilder_AppendFormat(stringBuilder, "'%S'", String_AddCSlashes(String_CreateRepeat(((SmileChar)obj)->ch, 1)));
		return;

	case SMILE_KIND_UNI:
		StringBuilder_AppendFormat(stringBuilder, "'\\u%X'", ((SmileUni)obj)->code);
		return;

	case SMILE_KIND_STRING:
		StringBuilder_AppendFormat(stringBuilder, "\"%S\"", String_AddCSlashes((String)obj));
		return;

	case SMILE_KIND_BYTEARRAY:
		StringBuilder_AppendFormat(stringBuilder, "(ByteArray of %ld)", (Int64)((SmileByteArray)obj)->length);
		return;

	case SMILE_KIND_USEROBJECT:
		{
			SmileUserObject userObject = (SmileUserObject)obj;
			Int32DictKeyValuePair *pairs = Int32Dict_GetAll((Int32Dict)&userObject->dict);
			Int numPairs = Int32Dict_Count((Int32Dict)&userObject->dict);
			Int i;
			String name;

			name = SymbolTable_GetName(Smile_SymbolTable, userObject->name);
			if (name != NULL) {
				StringBuilder_AppendString(stringBuilder, name);
				StringBuilder_AppendByte(stringBuilder, ' ');
			}
		
			if (numPairs == 0) {
				StringBuilder_Append(stringBuilder, (const Byte *)"{ }", 0, 3);
			}
			else {
				qsort(pairs, numPairs, sizeof(Int32DictKeyValuePair), UserObjectKeyComparer);

				StringBuilder_Append(stringBuilder, (const Byte *)"{\n", 0, 2);

				for (i = 0; i < numPairs; i++) {
					String keyName = SymbolTable_GetName(Smile_SymbolTable, pairs[i].key);
					StringBuilder_AppendRepeat(stringBuilder, ' ', (indent + 1) * 4);
					StringBuilder_AppendString(stringBuilder, keyName != NULL ? keyName : nullName);
					StringBuilder_Append(stringBuilder, (const Byte *)": ", 0, 2);
					StringifyRecursive((SmileObject)pairs[i].value, stringBuilder, indent + 2, includeSource);
					StringBuilder_AppendByte(stringBuilder, '\n');
				}

				StringBuilder_AppendRepeat(stringBuilder, ' ', indent * 4);
				StringBuilder_AppendByte(stringBuilder, '}');
			}
		}
		return;
	
	case SMILE_KIND_NONTERMINAL:
		{
			String str;
			SmileNonterminal nt = (SmileNonterminal)obj;
			Bool isFirst = True;

			StringBuilder_AppendByte(stringBuilder, '[');

			str = SymbolTable_GetName(Smile_SymbolTable, nt->nonterminal);
			if (!String_IsNullOrEmpty(str)) {
				if (!isFirst) StringBuilder_AppendByte(stringBuilder, ' ');
				StringBuilder_AppendString(stringBuilder, str);
				isFirst = False;
			}

			str = SymbolTable_GetName(Smile_SymbolTable, nt->repeat);
			if (!String_IsNullOrEmpty(str)) {
				StringBuilder_AppendString(stringBuilder, str);
				isFirst = False;
			}

			str = SymbolTable_GetName(Smile_SymbolTable, nt->name);
			if (!String_IsNullOrEmpty(str)) {
				if (!isFirst) StringBuilder_AppendByte(stringBuilder, ' ');
				StringBuilder_AppendString(stringBuilder, str);
				isFirst = False;
			}

			str = SymbolTable_GetName(Smile_SymbolTable, nt->separator);
			if (!String_IsNullOrEmpty(str)) {
				if (!isFirst) StringBuilder_AppendByte(stringBuilder, ' ');
				StringBuilder_AppendString(stringBuilder, str);
				isFirst = False;
			}

			StringBuilder_AppendByte(stringBuilder, ']');
		}
		return;

	case SMILE_KIND_SYNTAX:
		StringBuilder_AppendFormat(stringBuilder, "#syntax %S ",
			((SmileSyntax)obj)->nonterminal != 0 ? SymbolTable_GetName(Smile_SymbolTable, ((SmileSyntax)obj)->nonterminal) : String_Empty);
		StringifyRecursive((SmileObject)((SmileSyntax)obj)->pattern, stringBuilder, indent + 1, includeSource);
		StringBuilder_AppendC(stringBuilder, " => ", 0, 4);
		StringifyRecursive(((SmileSyntax)obj)->replacement, stringBuilder, indent + 1, includeSource);
		return;

	case SMILE_KIND_FUNCTION:
		{
			SmileFunction function = (SmileFunction)obj;
			if (obj->kind & SMILE_FLAG_EXTERNAL_FUNCTION) {
				StringBuilder_AppendFormat(stringBuilder, "|%S| <native>", function->u.externalFunctionInfo.argNames);
			}
			else {
				SmileObject body = function->u.u.userFunctionInfo->body;
				SmileList argList = function->u.u.userFunctionInfo->argList;
				Bool isFirst = True;

				StringBuilder_AppendByte(stringBuilder, '|');

				if (SmileList_IsWellFormed((SmileObject)argList)) {
					while (SMILE_KIND(argList) == SMILE_KIND_LIST) {
						if (!isFirst) {
							StringBuilder_AppendByte(stringBuilder, ' ');
						}
						StringifyRecursive((SmileObject)argList->a, stringBuilder, indent + 1, includeSource);
						argList = LIST_REST(argList);
						isFirst = False;
					}
				}
				else StringifyRecursive((SmileObject)argList, stringBuilder, indent + 1, includeSource);

				StringBuilder_AppendByte(stringBuilder, '|');
				StringBuilder_AppendByte(stringBuilder, ' ');

				if (SMILE_KIND(body) == SMILE_KIND_LIST) {
					if (SmileList_IsWellFormed(body)) {
						// Don't need parentheses when we're outputting a list form anyway.
						StringifyRecursive(body, stringBuilder, indent, includeSource);
					}
					else {
						StringBuilder_AppendByte(stringBuilder, '(');
						StringifyRecursive(body, stringBuilder, indent + 1, includeSource);
						StringBuilder_AppendByte(stringBuilder, ')');
					}
				}
				else {
					StringifyRecursive(body, stringBuilder, indent + 1, includeSource);
				}
			}
		}
		return;

	case SMILE_KIND_HANDLE:
		StringBuilder_AppendByte(stringBuilder, '<');
		StringBuilder_AppendString(stringBuilder,
			SymbolTable_GetName(Smile_SymbolTable, ((SmileHandle)obj)->handleKind));
		StringBuilder_AppendByte(stringBuilder, '>');
		return;

	case SMILE_KIND_TILL_CONTINUATION:
		StringBuilder_AppendC(stringBuilder, "TillContinuation", 0, 16);
		return;

	default:
		StringBuilder_AppendString(stringBuilder, SMILE_VCALL1(obj, toString, (SmileUnboxedData) { 0 }));
		return;
	}
}
