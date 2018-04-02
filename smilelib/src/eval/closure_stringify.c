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

#include <smile/eval/closure.h>
#include <smile/stringbuilder.h>

const char *Closure_StringifyToC(Closure closure, Bool includeAncestors)
{
	String str = Closure_Stringify(closure, includeAncestors);
	return String_ToC(str);
}

String Closure_Stringify(Closure closure, Bool includeAncestors)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	INIT_INLINE_STRINGBUILDER(stringBuilder);

	Closure_StringifyToStringBuilder(stringBuilder, closure, includeAncestors, 0);

	return StringBuilder_ToString(stringBuilder);
}

void Closure_StringifyToStringBuilder(StringBuilder stringBuilder, Closure closure, Bool includeAncestors, Int indent)
{
	ClosureInfo closureInfo;
	Symbol *variableNames;
	Int numVariableNames;
	Int i;
	Symbol variableName;
	String name;
	SmileObject obj;

	closureInfo = closure->closureInfo;

	if (closureInfo->kind == CLOSURE_KIND_GLOBAL) {
		StringBuilder_AppendFormat(stringBuilder, "global: ");
	}
	else {
		StringBuilder_AppendFormat(stringBuilder, "local: ");
	}
	StringBuilder_AppendFormat(stringBuilder, "%d args, %d vars, %d temp\n",
		closureInfo->numArgs, closureInfo->numVariables - closureInfo->numArgs, closureInfo->tempSize);

	if (closureInfo->kind == CLOSURE_KIND_LOCAL) {
		variableNames = closureInfo->variableNames;
		numVariableNames = closureInfo->numVariables;

		for (i = 0; i < numVariableNames; i++) {
			variableName = variableNames[i];
			StringBuilder_AppendRepeat(stringBuilder, ' ', indent * 4);
			name = SymbolTable_GetName(Smile_SymbolTable, variableName);
			StringBuilder_AppendString(stringBuilder, name != NULL ? name : String_Format("<%d>", variableName));
			StringBuilder_Append(stringBuilder, (const Byte *)": ", 0, 2);
			obj = SmileArg_Box(closure->variables[i]);
			SmileObject_StringifyToStringBuilder(stringBuilder, obj, indent + 1, False);
			StringBuilder_Append(stringBuilder, (const Byte *)"\n", 0, 1);
		}
	}

	if (includeAncestors && closure->parent != NULL) {
		StringBuilder_AppendRepeat(stringBuilder, ' ', indent * 4);
		StringBuilder_AppendFormat(stringBuilder, "> parent: ");
		Closure_StringifyToStringBuilder(stringBuilder, closure->parent, includeAncestors, indent + 1);
	}
}
