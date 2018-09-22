#ifndef __BPC_OUTPUT_GENERATOR_H__
#define __BPC_OUTPUT_GENERATOR_H__

#ifndef __STDAFX_H__
#include "../stdafx.h"
#endif

typedef struct OutputDataStruct {
	StringBuilder stringDecls;
	StringBuilder symbolDecls;
	StringBuilder smileSymbolDecls;
	StringBuilder dataDecls;
	StringBuilder bytecodeDecls;
	StringBuilder compiledTablesDecls;
	StringBuilder bytecodeFixups;

	StringBuilder symbolInits;
	StringBuilder smileSymbolInits;
	StringBuilder dataInits;

	StringBuilder functions;
	StringBuilder userFunctions;
	StringBuilder topmostCalls;

	PointerDict dataToNamesDict;
	StringDict stringToStaticsDict;
	Int32Dict symbolToStaticsDict;
	Int32Dict symbolIdsDict;

	CompiledTables compiledTables;
} *OutputData;

OutputData GenerateOutput(SmileList setPairs, ClosureInfo closureInfo);

void GenerateCompiledTables(OutputData outputData, CompiledTables compiledTables);

String GenerateUserFunctionInfo(OutputData outputData, UserFunctionInfo userFunctionInfo);
String GenerateFunction(OutputData outputData, SmileFunction smileFunction);

String GenerateInt64Const(OutputData outputData, SmileInteger64 int64);
String GenerateInt32Const(OutputData outputData, SmileInteger32 int32);
String GenerateStringConst(OutputData outputData, String str);
String GenerateSymbolId(OutputData outputData, Symbol symbol);
String GenerateSymbolConst(OutputData outputData, Symbol symbol);

String GenerateList(OutputData outputData, SmileList smileList);

String GenerateValue(OutputData outputData, SmileObject obj);

#endif