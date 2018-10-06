#include "stdafx.h"
#include "output_generator.h"
#include "string_helpers.h"
#include "eval.h"
#include "error.h"

//-----------------------------------------------------------------------------
// Top-level object-construction functions.

Inline void GetVarAndProp(SmileObject target, Symbol *var, Symbol *prop)
{
	if (SMILE_KIND(target) == SMILE_KIND_LIST) {
		SmileList targetList = (SmileList)target;
		*var = ((SmileSymbol)LIST_SECOND(targetList))->symbol;
		*prop = ((SmileSymbol)LIST_THIRD(targetList))->symbol;
	}
	else {
		*var = ((SmileSymbol)target)->symbol;
		*prop = 0;
	}
}

static Int SetPairsComparer(SmileObject a, SmileObject b, void *param)
{
	SmileObject aTarget, bTarget;
	Symbol aVar, bVar, aProp, bProp;

	aTarget = LIST_FIRST((SmileList)a);
	bTarget = LIST_FIRST((SmileList)b);

	// Decode what we're writing to.
	GetVarAndProp(aTarget, &aVar, &aProp);
	GetVarAndProp(bTarget, &bVar, &bProp);

	if (aVar != bVar) {
		// If they're different variables, then sort by top-level symbol (ASCII-betically, case-sensitive).
		return String_Compare(SymbolTable_GetName(Smile_SymbolTable, aVar), SymbolTable_GetName(Smile_SymbolTable, bVar));
	}
	else if (aProp != bProp) {
		// If they're different properties, then sort by property symbol (ASCII-betically, case-sensitive).
		return String_Compare(SymbolTable_GetName(Smile_SymbolTable, aProp), SymbolTable_GetName(Smile_SymbolTable, bProp));
	}
	else {
		// If they're the same, that's an error.
		if (aProp) {
			Error(NULL, 0, "Cannot repeat assignment to \"%S.%S\".",
				SymbolTable_GetName(Smile_SymbolTable, aVar), SymbolTable_GetName(Smile_SymbolTable, aProp));
		}
		else {
			Error(NULL, 0, "Cannot repeat assignment to \"%S\".",
				SymbolTable_GetName(Smile_SymbolTable, aVar));
		}
		return 0;
	}
}

static void CompilePropAssignment(Compiler compiler, OutputData outputData, Symbol var, Symbol prop, SmileObject valueExpr, ClosureInfo closureInfo)
{
	String symbolRef = GenerateSymbolId(outputData, prop);

	SmileObject value = EvalExpr(compiler, valueExpr, closureInfo, String_FromC("<no file>"));
	SmileObject target;

	StringBuilder_AppendFormat(outputData->functions,
		"\tSMILE_VCALL2(target, setProperty, %S, (SmileObject)(%S));\n",
		symbolRef, GenerateValue(outputData, value));

	target = ClosureInfo_GetGlobalVariable(closureInfo, var);
	SMILE_VCALL2(target, setProperty, prop, value);
}

static void CompileVarBegin(OutputData outputData, Symbol var, ClosureInfo closureInfo)
{
	String varName = SymbolToCIdentifier(var);
	String symbolRef = GenerateSymbolId(outputData, var);

	StringBuilder_AppendFormat(outputData->functions,
		"static void Setup_%S(ClosureInfo dest)\n"
		"{\n"
		"\tSymbol targetSymbol = %S;\n"
		"\n",
		varName, symbolRef);
}

static void CompileLoadTarget(OutputData outputData, Symbol var, ClosureInfo closureInfo)
{
	StringBuilder_AppendFormat(outputData->functions,
		"\tSmileObject target = ClosureInfo_GetGlobalVariable(dest, targetSymbol);\n");
}

static void CompileVarEnd(OutputData outputData, Symbol var, ClosureInfo closureInfo)
{
	StringBuilder_AppendFormat(outputData->functions,
		"}\n"
		"\n");
}

static void CompileVarAssignment(Compiler compiler, OutputData outputData, Symbol var, SmileObject valueExpr, ClosureInfo closureInfo)
{
	SmileObject value = EvalExpr(compiler, valueExpr, closureInfo, String_FromC("<no file>"));

	StringBuilder_AppendFormat(outputData->functions,
		"\tSmileObject target = (SmileObject)(%S);\n",
		GenerateValue(outputData, value));

	StringBuilder_AppendFormat(outputData->functions,
		"\tClosureInfo_SetGlobalVariable(dest, targetSymbol, target);\n");
	
	ClosureInfo_SetGlobalVariable(closureInfo, var, value);
}

//-----------------------------------------------------------------------------
// Top-level generator.

OutputData GenerateOutput(SmileList setPairs, ClosureInfo closureInfo)
{
	OutputData outputData;
	SmileList sortedPairs;
	Compiler compiler;

	outputData = GC_MALLOC_STRUCT(struct OutputDataStruct);

	outputData->dataDecls = StringBuilder_Create();
	outputData->stringDecls = StringBuilder_Create();
	outputData->symbolDecls = StringBuilder_Create();
	outputData->smileSymbolDecls = StringBuilder_Create();
	outputData->compiledTablesDecls = StringBuilder_Create();
	outputData->bytecodeDecls = StringBuilder_Create();
	outputData->bytecodeFixups = StringBuilder_Create();
	outputData->symbolInits = StringBuilder_Create();
	outputData->smileSymbolInits = StringBuilder_Create();
	outputData->dataInits = StringBuilder_Create();
	outputData->functions = StringBuilder_Create();
	outputData->userFunctions = StringBuilder_Create();
	outputData->topmostCalls = StringBuilder_Create();

	outputData->dataToNamesDict = PointerDict_Create();
	outputData->stringToStaticsDict = StringDict_Create();
	outputData->symbolIdsDict = Int32Dict_Create();
	outputData->symbolToStaticsDict = Int32Dict_Create();

	sortedPairs = SmileList_Sort(setPairs, SetPairsComparer, NULL);

	compiler = Compiler_Create();

	for (; SMILE_KIND(sortedPairs) == SMILE_KIND_LIST;) {
		Symbol lastVar;
		Symbol var, prop;
		SmileList temp;
		Bool needLoad;
		SmileList setExpr;
		
		setExpr = (SmileList)sortedPairs->a;

		GetVarAndProp(LIST_FIRST(setExpr), &var, &prop);
		lastVar = var;

		StringBuilder_AppendFormat(outputData->topmostCalls,
			"\tSetup_%S(dest);\n",
			SymbolToCIdentifier(var));

		CompileVarBegin(outputData, var, closureInfo);

		needLoad = True;

		for (temp = sortedPairs; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = LIST_REST(temp)) {
			setExpr = (SmileList)temp->a;
			GetVarAndProp(LIST_FIRST(setExpr), &var, &prop);
			if (var != lastVar) break;

			if (prop) {
				if (needLoad) {
					CompileLoadTarget(outputData, var, closureInfo);
					needLoad = False;
				}
				CompilePropAssignment(compiler, outputData, var, prop, LIST_SECOND(setExpr), closureInfo);
			}
			else {
				CompileVarAssignment(compiler, outputData, var, LIST_SECOND(setExpr), closureInfo);
				needLoad = False;
			}
		}

		CompileVarEnd(outputData, var, closureInfo);

		sortedPairs = temp;
	}

	if (outputData->compiledTables != NULL) {
		GenerateCompiledTables(outputData, outputData->compiledTables);
	}

	return outputData;
}
