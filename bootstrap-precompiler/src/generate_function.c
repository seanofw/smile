
#include "stdafx.h"
#include "output_generator.h"

//-----------------------------------------------------------------------------
// Generation of functions.

static void GenerateByteCodeFixupIfNeeded(OutputData outputData, ByteCode byteCode, String bcIdent, Int i)
{
	String symbolRef;

	switch (byteCode->opcode) {
		case Op_LdSym:
		case Op_LdX:
		case Op_StX:
		case Op_StpX:
		case Op_NullX:
		case Op_LdProp:
		case Op_StProp:
		case Op_StpProp:
		case Op_Met0: case Op_Met1: case Op_Met2: case Op_Met3:
		case Op_Met4: case Op_Met5: case Op_Met6: case Op_Met7:
		case Op_TMet0: case Op_TMet1: case Op_TMet2: case Op_TMet3:
		case Op_TMet4: case Op_TMet5: case Op_TMet6: case Op_TMet7:
			symbolRef = GenerateSymbolId(outputData, byteCode->u.symbol);
			StringBuilder_AppendFormat(outputData->bytecodeFixups,
				"\t%S[%d].u.symbol = %S;\n",
				bcIdent, (Int32)i, symbolRef);
			return;

		case Op_Met:
		case Op_TMet:
			symbolRef = GenerateSymbolId(outputData, byteCode->u.i2.b);
			StringBuilder_AppendFormat(outputData->bytecodeFixups,
				"\t%S[%d].u.i2.b = %S;\n",
				bcIdent, (Int32)i, symbolRef);
			return;
	}
}

static String GetByteCodeSymbolName(ByteCode byteCode)
{
	switch (byteCode->opcode) {
		case Op_LdSym:
		case Op_LdX:
		case Op_StX:
		case Op_StpX:
		case Op_NullX:
		case Op_LdProp:
		case Op_StProp:
		case Op_StpProp:
		case Op_Met0: case Op_Met1: case Op_Met2: case Op_Met3:
		case Op_Met4: case Op_Met5: case Op_Met6: case Op_Met7:
		case Op_TMet0: case Op_TMet1: case Op_TMet2: case Op_TMet3:
		case Op_TMet4: case Op_TMet5: case Op_TMet6: case Op_TMet7:
			return SymbolTable_GetName(Smile_SymbolTable, byteCode->u.symbol);
		case Op_Met:
		case Op_TMet:
			return SymbolTable_GetName(Smile_SymbolTable, byteCode->u.i2.b);
		default:
			return String_Empty;
	}
}

static String GenerateByteCodeOperand(ByteCode byteCode)
{
	switch (byteCode->opcode) {
		case Op_Dup:
		case Op_Pop:
		case Op_Rep:
		case Op_LdStr:
		case Op_LdSym:
		case Op_LdObj:
		case Op_Ld32:
		case Op_LdX:
		case Op_NullLoc0:
		case Op_NullArg0:
		case Op_NewFn:
		case Op_NewObj:
		case Op_NewTill:
		case Op_TillEsc:
		case Op_LdArg0: case Op_LdArg1: case Op_LdArg2: case Op_LdArg3:
		case Op_LdArg4: case Op_LdArg5: case Op_LdArg6: case Op_LdArg7:
		case Op_LdLoc0: case Op_LdLoc1: case Op_LdLoc2: case Op_LdLoc3:
		case Op_LdLoc4: case Op_LdLoc5: case Op_LdLoc6: case Op_LdLoc7:
		case Op_StArg0: case Op_StArg1: case Op_StArg2: case Op_StArg3:
		case Op_StArg4: case Op_StArg5: case Op_StArg6: case Op_StArg7:
		case Op_StLoc0: case Op_StLoc1: case Op_StLoc2: case Op_StLoc3:
		case Op_StLoc4: case Op_StLoc5: case Op_StLoc6: case Op_StLoc7:
		case Op_StpArg0: case Op_StpArg1: case Op_StpArg2: case Op_StpArg3:
		case Op_StpArg4: case Op_StpArg5: case Op_StpArg6: case Op_StpArg7:
		case Op_StpLoc0: case Op_StpLoc1: case Op_StpLoc2: case Op_StpLoc3:
		case Op_StpLoc4: case Op_StpLoc5: case Op_StpLoc6: case Op_StpLoc7:
			return String_Format(".int32 = %hd", byteCode->u.int32);
		case Op_LdBool:
			return String_Format(".boolean = %d", (Int32)byteCode->u.boolean);
		case Op_LdChar:
			return String_Format(".ch = %hd", (Int32)byteCode->u.ch);
		case Op_LdUni:
			return String_Format(".uni = %hd", (Int32)byteCode->u.uni);
		case Op_Ld8:
			return String_Format(".byte = %hd", (Int32)byteCode->u.byte);
		case Op_Ld16:
			return String_Format(".int16 = %hd", (Int32)byteCode->u.int16);
		case Op_Ld64:
			return String_Format(".int64 = %lldLL", (Int64)byteCode->u.int64);
		case Op_Ld128:
		case Op_LdR128:
		case Op_LdF128:
			return String_Format(".index = %lld", (Int64)byteCode->u.index);
		case Op_Jmp:
		case Op_Bt:
		case Op_Bf:
		case Op_Auto:
			return String_Format(".index = %lld", (Int64)byteCode->u.index);
		case Op_Call:
		case Op_TCall:
			return String_Format(".index = %lld", (Int64)byteCode->u.index);
		case Op_LdR16:
		case Op_LdR32:
		case Op_LdF16:
		case Op_LdF32:
			return String_Format(".int32 = 0x%hX", (Int32)byteCode->u.int32);
		case Op_LdR64:
		case Op_LdF64:
			return String_Format(".int64 = 0x%llXLLU", (Int64)byteCode->u.int64);
		case Op_LdLoc:
		case Op_StLoc:
		case Op_StpLoc:
		case Op_LdArg:
		case Op_StArg:
		case Op_StpArg:
		case Op_LdInclude:
		case Op_Met:
		case Op_TMet:
		case Op_Try:
			return String_Format(".i2 = { .a = %hd, .b = %hd }", (Int32)byteCode->u.i2.a, (Int32)byteCode->u.i2.b);
		default:
			return String_FromC("0\t\t\t");
	}
}

static String GenerateByteCodeSegment(OutputData outputData, Int dataId, ByteCodeSegment byteCodeSegment)
{
	String bcIdent;
	String bcsIdent;
	Int i;
	String unknownOpcode = String_FromC("<unknown>");
	CompiledSourceLocation sourceLocation;

	if (outputData->compiledTables == NULL) {
		outputData->compiledTables = byteCodeSegment->compiledTables;
	}
	else if (outputData->compiledTables != byteCodeSegment->compiledTables) {
		StringBuilder_AppendFormat(outputData->bytecodeDecls,
			"\n#error CRITICAL ERROR IN COMPILED TABLES. MULTIPLE COMPILED TABLES NOT SUPPORTED.\n");
	}

	bcIdent = String_Format("_bc_%d", dataId);
	bcsIdent = String_Format("_bcs_%d", dataId);

	sourceLocation = &byteCodeSegment->compiledTables->sourcelocations[byteCodeSegment->byteCodes[0].sourceLocation];
	StringBuilder_AppendFormat(outputData->bytecodeDecls, "\n// %S:%hd\n",
		Path_GetFilename(sourceLocation->filename), sourceLocation->line);
	StringBuilder_AppendFormat(outputData->bytecodeDecls, "static struct ByteCodeStruct %S[] = {\n", bcIdent);

	for (i = 0; i < byteCodeSegment->numByteCodes; i++) {
		ByteCode byteCode = &byteCodeSegment->byteCodes[i];
		String opcodeName = Opcode_Names[byteCode->opcode];
		String symbolName = GetByteCodeSymbolName(byteCode);
		const char *symbolSpace = symbolName != String_Empty ? " " : "";

		StringBuilder_AppendFormat(outputData->bytecodeDecls,
			"\t{ 0x%02X, { 0 }, %d, { %S } },\t\t// %S%s%S\n",
			(UInt32)byteCode->opcode, byteCode->sourceLocation,
			GenerateByteCodeOperand(byteCode),
			opcodeName != NULL ? opcodeName : unknownOpcode,
			symbolSpace, symbolName);

		GenerateByteCodeFixupIfNeeded(outputData, byteCode, bcIdent, i);
	}

	StringBuilder_AppendFormat(outputData->bytecodeDecls,
		"};\n");

	StringBuilder_AppendFormat(outputData->bytecodeDecls,
		"static struct ByteCodeSegmentStruct %S = { &_compiledTables, %S, %d, %d };\n",
		bcsIdent, bcIdent, byteCodeSegment->numByteCodes, byteCodeSegment->numByteCodes);

	return String_Format("((ByteCodeSegment)&%S)", bcsIdent);
}

static void GenerateClosureInfoInline(OutputData outputData, String closureInfoRef, ClosureInfo closureInfo);

static String GenerateClosureInfo(OutputData outputData, ClosureInfo closureInfo)
{
	static Int32 closureInfoId = 0;

	String ident;
	String result;
	void *priorName;
	Int32 dataId;

	if (closureInfo == NULL || closureInfo->parent == NULL || closureInfo->parent->parent == NULL)
		return String_FromC("NULL");

	if ((priorName = PointerDict_GetValue(outputData->dataToNamesDict, closureInfo)) != NULL)
		return (String)priorName;

	if (closureInfo->kind == CLOSURE_KIND_GLOBAL) {
		result = String_FromC("NULL");
	}
	else {
		dataId = closureInfoId++;
		ident = String_Format("_ci_%d", dataId);
		result = String_Format("((ClosureInfo)&%S)", ident);

		StringBuilder_AppendFormat(outputData->dataDecls,
			"static struct ClosureInfoStruct %S;\n",
			ident);

		GenerateClosureInfoInline(outputData, result, closureInfo);
	}

	PointerDict_SetValue(outputData->dataToNamesDict, closureInfo, result);
	return result;
}

static void GenerateClosureInfoInline(OutputData outputData, String closureInfoRef, ClosureInfo closureInfo)
{
	String parentRef;
	String globalRef;

	parentRef = GenerateClosureInfo(outputData, closureInfo->parent);
	globalRef = GenerateClosureInfo(outputData, closureInfo->global);

	StringBuilder_AppendFormat(outputData->dataInits,
		"\t%S->parent = %S;\n"
		"\t%S->global = %S;\n"
		"\t%S->kind = %d;\n"
		"\t%S->numVariables = %d;\n"
		"\t%S->numArgs = %d;\n"
		"\t%S->tempSize = %d;\n"
		"\t%S->variableDictionary = NULL;\n"
		"\t%S->variableNames = NULL;\n",
		closureInfoRef, parentRef,
		closureInfoRef, globalRef,
		closureInfoRef, (Int32)closureInfo->kind,
		closureInfoRef, (Int32)closureInfo->numVariables,
		closureInfoRef, (Int32)closureInfo->numArgs,
		closureInfoRef, (Int32)closureInfo->tempSize,
		closureInfoRef,
		closureInfoRef);
}

String GenerateUserFunctionInfo(OutputData outputData, UserFunctionInfo userFunctionInfo)
{
	static Int32 userFunctionId = 0;

	Int32 dataId;
	String argList;
	String byteCodeSegment;
	String result;
	String ident;
	String parentRef;
	void *priorName;

	if (userFunctionInfo == NULL || userFunctionInfo->parent == NULL)
		return String_FromC("NULL");

	if ((priorName = PointerDict_GetValue(outputData->dataToNamesDict, userFunctionInfo)) != NULL)
		return (String)priorName;

	dataId = userFunctionId++;
	ident = String_Format("_ufn_%d", dataId);
	result = String_Format("((UserFunctionInfo)&%S)", ident);

	if (userFunctionInfo->parent != NULL)
		parentRef = GenerateUserFunctionInfo(outputData, userFunctionInfo->parent);
	else
		parentRef = String_FromC("NULL");

	argList = GenerateValue(outputData, (SmileObject)userFunctionInfo->argList);
	if (String_EqualsC(argList, "NullObject"))
		argList = String_FromC("NullList");
	byteCodeSegment = GenerateByteCodeSegment(outputData, dataId, userFunctionInfo->byteCodeSegment);

	StringBuilder_AppendFormat(outputData->userFunctions, "static struct UserFunctionInfoStruct %S;\n", ident);

	StringBuilder_AppendFormat(outputData->dataInits,
		"\t%S.parent = %S;\n"
		"\t%S.argList = %S;\n"
		"\t%S.position = NULL;\n"
		"\t%S.body = NullObject;\n",
		ident, parentRef, ident, argList, ident, ident);
	StringBuilder_AppendFormat(outputData->dataInits,
		"\t%S.byteCodeSegment = %S;\n",
		ident, byteCodeSegment);
	StringBuilder_AppendFormat(outputData->dataInits,
		"\tci = (ClosureInfo)&%S.closureInfo;\n",
		ident);
	GenerateClosureInfoInline(outputData, String_FromC("ci"), &userFunctionInfo->closureInfo);
	StringBuilder_AppendFormat(outputData->dataInits, "\tUserFunctionInfo_ApplyArgs(%S, %S, USER_ARG_BOOTSTRAP, NULL);\n",
		result, argList);

	StringBuilder_AppendFormat(outputData->dataInits, "\n");

	PointerDict_SetValue(outputData->dataToNamesDict, userFunctionInfo, result);
	return result;
}

String GenerateFunction(OutputData outputData, SmileFunction smileFunction)
{
	static Int32 functionId = 0;

	Int32 dataId = functionId++;
	String ufn = GenerateUserFunctionInfo(outputData, smileFunction->u.u.userFunctionInfo);

	String ident = String_Format("_fn_%d", dataId);
	String result = String_Format("((SmileFunction)&%S)", ident);

	StringBuilder_AppendFormat(outputData->dataDecls,
		"static struct SmileFunctionInt %S;\n",
		ident);

	StringBuilder_AppendFormat(outputData->dataInits,
		"\tSmileFunction_InitUserFunction(%S, %S, globalClosure);\n\n",
		result, ufn);

	return result;
}
