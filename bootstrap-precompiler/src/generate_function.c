
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

static String GenerateByteCodeSegment(OutputData outputData, Int dataId, ByteCodeSegment byteCodeSegment)
{
	String bcIdent;
	String bcsIdent;
	Int i;

	if (outputData->compiledTables == NULL) {
		outputData->compiledTables = byteCodeSegment->compiledTables;
	}
	else if (outputData->compiledTables != byteCodeSegment->compiledTables) {
		StringBuilder_AppendFormat(outputData->bytecodeDecls,
			"\n#error CRITICAL ERROR IN COMPILED TABLES. MULTIPLE COMPILED TABLES NOT SUPPORTED.\n");
	}

	bcIdent = String_Format("_bc_%d", dataId);
	bcsIdent = String_Format("_bcs_%d", dataId);

	StringBuilder_AppendFormat(outputData->bytecodeDecls, "static struct ByteCodeStruct %S[] = {\n", bcIdent);

	for (i = 0; i < byteCodeSegment->numByteCodes; i++) {
		ByteCode byteCode = &byteCodeSegment->byteCodes[i];
		StringBuilder_AppendFormat(outputData->bytecodeDecls,
			"\t{ 0x%02X, { 0 }, 0, { 0x%lX%s } },\n",
			(UInt32)byteCode->opcode, byteCode->u.int64, (byteCode->u.int64 & ~(UInt64)Int32Max) ? "LLU" : "");

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
