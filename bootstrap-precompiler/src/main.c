//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Bootstrap Precompiler)
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

#include "stdafx.h"
#include "string_helpers.h"
#include "output_generator.h"
#include "eval.h"
#include "error.h"
#include "load_and_parse.h"

/// <summary>
/// Make sure that the given expr is for a [$progn] that consists of a series of [$set] operations
/// (or discardable constant values like 'null').  Anything else is a structural error in the bootstrap
/// and is disallowed.
/// </summary>
/// <returns>If valid, the subexpressions, decomposed into a just list of the assignment pairs from
/// each $set expression:  [[dest1 src1] [dest2 src2] [dest3 src3] ... ]  If invalid, returns NULL.</returns>
static Bool CheckExpr(SmileObject expr, String filename, SmileList *head, SmileList *tail)
{
	SmileList list;
	SmileObject childExpr;
	SmileList setExpr;
	SmileObject src, dest;

	if (SMILE_KIND(expr) != SMILE_KIND_LIST
		|| SMILE_KIND((list = (SmileList)expr)->a) != SMILE_KIND_SYMBOL
		|| ((SmileSymbol)list->a)->symbol != SMILE_SPECIAL_SYMBOL__PROGN) {
		Error(String_ToC(filename), SMILE_VCALL(expr, getSourceLocation)->line,
			"Bootstrap script *must* consist of a [$progn] sequence of [$set] to known names or properites of known names;"
			" this script does not start with a [$progn] (did you accidentally create a global variable?).");
		return False;
	}

	for (list = LIST_REST(list); SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
		childExpr = list->a;

		if (SMILE_KIND(childExpr) != SMILE_KIND_LIST
			|| SMILE_KIND((setExpr = (SmileList)childExpr)->a) != SMILE_KIND_SYMBOL
			|| ((SmileSymbol)setExpr->a)->symbol != SMILE_SPECIAL_SYMBOL__SET
			|| SmileList_SafeLength(setExpr) != 3) {
			Error(String_ToC(filename), SMILE_VCALL(setExpr, getSourceLocation)->line,
				"Bootstrap script *must* consist of a [$progn] sequence of [$set] to known names or properties of known names;"
				" this line is not a [$set] expression (are you trying to perform disallowed computation in the bootstrap?)");
			return False;
		}

		dest = LIST_SECOND(setExpr);
		src = LIST_THIRD(setExpr);

		// Source can be anything, but destination must either be 'foo' or 'foo.bar', no exceptions.
		if (SMILE_KIND(dest) == SMILE_KIND_SYMBOL) {
			// Found 'foo' as a valid pattern.
			LIST_APPEND_WITH_SOURCE(*head, *tail, setExpr->d, SMILE_VCALL(setExpr, getSourceLocation));
			continue;
		}

		if (SMILE_KIND(dest) == SMILE_KIND_LIST) {
			// Possibly 'foo.bar'.
			SmileList destList = (SmileList)dest;
			if (SmileList_SafeLength(destList) == 3
				|| SMILE_KIND(LIST_FIRST(destList)) == SMILE_KIND_SYMBOL
				|| SMILE_KIND(LIST_SECOND(destList)) == SMILE_KIND_SYMBOL
				|| SMILE_KIND(LIST_THIRD(destList)) == SMILE_KIND_SYMBOL) {
				// Found [$dot foo bar], which is 'foo.bar', which is a valid pattern.
				LIST_APPEND_WITH_SOURCE(*head, *tail, setExpr->d, SMILE_VCALL(setExpr, getSourceLocation));
				continue;
			}
		}

		// Nope.
		Error(String_ToC(filename), SMILE_VCALL(setExpr, getSourceLocation)->line,
			"Bootstrap [$set] must be to a known name ('foo') or to a property of a known name ('foo.bar');"
			" this line is not a valid bootstrap [$set] expression.");
		return False;
	}

	return True;
}

static String EmitCSourceCode(OutputData outputData)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	INIT_INLINE_STRINGBUILDER(stringBuilder);

	StringBuilder_AppendFormat(stringBuilder,
		"//-----------------------------------------------------\n"
		"// THIS FILE IS AUTOMATICALLY GENERATED.  DO NOT EDIT!\n"
		"//\n"
		"// bootstrap.c:  Smile source code, compiled to byte-\n"
		"// code, and embedded within optimized C, designed to\n"
		"// run at startup to add functions and methods to the\n"
		"// \"global\" namespace, all written in Smile itself.\n"
		"//\n"
		"// THIS FILE IS AUTOMATICALLY GENERATED.  DO NOT EDIT!\n"
		"//-----------------------------------------------------\n"
		"\n"
		"#include <smile/smiletypes/smileobject.h>\n"
		"#include <smile/smiletypes/smileuserobject.h>\n"
		"#include <smile/smiletypes/smilelist.h>\n"
		"#include <smile/smiletypes/smilebool.h>\n"
		"#include <smile/smiletypes/text/smilesymbol.h>\n"
		"#include <smile/smiletypes/numeric/smilebyte.h>\n"
		"#include <smile/smiletypes/numeric/smileinteger16.h>\n"
		"#include <smile/smiletypes/numeric/smileinteger32.h>\n"
		"#include <smile/smiletypes/numeric/smileinteger64.h>\n"
		"#include <smile/smiletypes/range/smileinteger64range.h>\n"
		"#include <smile/smiletypes/smilefunction.h>\n"
		"#include <smile/eval/compiler.h>\n"
		"#include <smile/internal/staticstring.h>\n"
		"\n"
		"SMILE_IGNORE_UNUSED_VARIABLES\n"
		"\n"
		"//-----------------------------------------------------------------------------\n"
		"// Data declarations\n\n");

	StringBuilder_AppendFormat(stringBuilder, "//--- String declarations ---\n\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->stringDecls);
	StringBuilder_AppendByte(stringBuilder, '\n');
	StringBuilder_AppendFormat(stringBuilder, "//--- Symbol declarations ---\n\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->symbolDecls);
	StringBuilder_AppendByte(stringBuilder, '\n');
	StringBuilder_AppendFormat(stringBuilder, "//--- SmileSymbol declarations ---\n\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->smileSymbolDecls);
	StringBuilder_AppendByte(stringBuilder, '\n');
	StringBuilder_AppendFormat(stringBuilder, "//--- Other data declarations ---\n\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->dataDecls);
	StringBuilder_AppendByte(stringBuilder, '\n');
	StringBuilder_AppendFormat(stringBuilder,
		"//-----------------------------------------------------------------------------\n"
		"// UserFunction declarations \n"
		"\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->userFunctions);
	StringBuilder_AppendByte(stringBuilder, '\n');

	StringBuilder_AppendFormat(stringBuilder, "//-----------------------------------------------------------------------------\n"
		"// Compiler data tables\n"
		"\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->compiledTablesDecls);
	StringBuilder_AppendByte(stringBuilder, '\n');

	StringBuilder_AppendFormat(stringBuilder, "//-----------------------------------------------------------------------------\n"
		"// ByteCode segments\n"
		"\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->bytecodeDecls);
	StringBuilder_AppendByte(stringBuilder, '\n');


	StringBuilder_AppendFormat(stringBuilder, "//-----------------------------------------------------------------------------\n"
		"// Symbol registrations\n");

	StringBuilder_AppendFormat(stringBuilder, "\n"
		"static void SetupSymbols(SymbolTable symbolTable)\n"
		"{\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->symbolInits);
	StringBuilder_AppendByte(stringBuilder, '\n');
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->smileSymbolInits);
	StringBuilder_AppendFormat(stringBuilder, "}\n"
		"\n");

	StringBuilder_AppendFormat(stringBuilder, "//-----------------------------------------------------------------------------\n"
		"// Bytecode fixups\n");

	StringBuilder_AppendFormat(stringBuilder, "\n"
		"static void FixupBytecode(void)\n"
		"{\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->bytecodeFixups);
	StringBuilder_AppendFormat(stringBuilder, "}\n"
		"\n");

	StringBuilder_AppendFormat(stringBuilder, "//-----------------------------------------------------------------------------\n"
		"// Raw-data bootstrapping\n");

	StringBuilder_AppendFormat(stringBuilder, "\n"
		"static void SetupData(Closure globalClosure)\n"
		"{\n"
		"\tClosureInfo ci;\n"
		"\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->dataInits);
	StringBuilder_AppendFormat(stringBuilder, "}\n"
		"\n");

	StringBuilder_AppendFormat(stringBuilder, "//-----------------------------------------------------------------------------\n"
		"// Members-and-globals instantiations\n"
		"\n");

	StringBuilder_AppendStringBuilder(stringBuilder, outputData->functions);

	StringBuilder_AppendFormat(stringBuilder, "//-----------------------------------------------------------------------------\n"
		"// Bootstrap main\n"
		"\n");

	StringBuilder_AppendFormat(stringBuilder, "void InitBootstrap(SymbolTable symbolTable, ClosureInfo dest)\n"
		"{\n"
		"\tClosure globalClosure = Closure_CreateGlobal(dest, NULL);\n"
		"\n"
		"\tSetupSymbols(symbolTable);\n"
		"\tFixupBytecode();\n"
		"\tSetupData(globalClosure);\n"
		"\n");
	StringBuilder_AppendStringBuilder(stringBuilder, outputData->topmostCalls);
	StringBuilder_AppendFormat(stringBuilder, "}\n"
		"\n");

	return StringBuilder_ToString(stringBuilder);
}

int main(int argc, const char **argv)
{
	int i;
	const char *arg;
	String filename, text;
	SmileObject expr;
	SmileList allPairsHead, allPairsTail;
	ClosureInfo closureInfo;
	OutputData outputData;
	String result;

	Smile_Init();

	closureInfo = ClosureInfo_Create(NULL, CLOSURE_KIND_GLOBAL);

	Smile_SetGlobalClosureInfo(closureInfo);
	Smile_InitCommonGlobals(closureInfo);

	LIST_INIT(allPairsHead, allPairsTail);

	for (i = 1; i < argc; i++) {
		arg = argv[i];
		filename = String_FromC(arg);
		text = LoadFile(filename);
		expr = Parse(text, filename, closureInfo);
		if (!CheckExpr(expr, filename, &allPairsHead, &allPairsTail))
			continue;
	}

	outputData = GenerateOutput(allPairsHead, closureInfo);

	result = EmitCSourceCode(outputData);

	fwrite(String_GetBytes(result), 1, String_Length(result), stdout);
	
	Smile_End();

	return 0;
}
