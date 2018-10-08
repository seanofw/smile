
#include "stdafx.h"
#include "output_generator.h"

//-----------------------------------------------------------------------------
// Generation of compiler data structures.

static void GenerateCompiledTableStrings(OutputData outputData, CompiledTables compiledTables)
{
	Int i;

	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"static String _compiledTables_Strings[%d] = {\n",
		(Int32)(compiledTables->numStrings > 0 ? compiledTables->numStrings : 1)
	);
	for (i = 0; i < compiledTables->numStrings; i++) {
		StringBuilder_AppendFormat(outputData->compiledTablesDecls,
			"\t%S,\n",
			GenerateStringConst(outputData, compiledTables->strings[i])
		);
	}
	if (compiledTables->numStrings <= 0)
		StringBuilder_AppendFormat(outputData->compiledTablesDecls, "\tNULL,\n");
	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"};\n"
	);
}

static void GenerateCompiledTableSourceLocations(OutputData outputData, CompiledTables compiledTables)
{
	Int i;
	String nullString = String_FromC("NULL");

	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"static struct CompiledSourceLocationStruct _compiledTables_SrcLocs[%d] = {\n",
		(Int32)(compiledTables->numSourceLocations > 0 ? compiledTables->numSourceLocations : 1)
	);
	for (i = 0; i < compiledTables->numSourceLocations; i++) {
		String filename = compiledTables->sourcelocations[i].filename != NULL
			? GenerateStringConst(outputData, Path_GetFilename(compiledTables->sourcelocations[i].filename))
			: nullString;
		StringBuilder_AppendFormat(outputData->compiledTablesDecls,
			"\t{ %S, %d, %d, 0 },\n",
			filename,
			compiledTables->sourcelocations[i].line,
			compiledTables->sourcelocations[i].column
		);
	}
	if (compiledTables->numSourceLocations <= 0)
		StringBuilder_AppendFormat(outputData->compiledTablesDecls, "\tNULL,\n");
	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"};\n"
	);
}

static void GenerateCompiledTableFns(OutputData outputData, CompiledTables compiledTables)
{
	Int i;

	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"static UserFunctionInfo _compiledTables_Fns[%d] = {\n",
		(Int32)(compiledTables->numUserFunctions > 0 ? compiledTables->numUserFunctions : 1)
	);
	for (i = 0; i < compiledTables->numUserFunctions; i++) {
		String fnReference = GenerateUserFunctionInfo(outputData, compiledTables->userFunctions[i]);
		StringBuilder_AppendFormat(outputData->compiledTablesDecls, "\t%S,\n", fnReference);
	}
	if (compiledTables->numUserFunctions <= 0)
		StringBuilder_AppendFormat(outputData->compiledTablesDecls, "\tNULL,\n");
	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"};\n"
	);
}

static void GenerateCompiledTableObjs(OutputData outputData, CompiledTables compiledTables)
{
	Int i;

	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"static SmileObject _compiledTables_Objs[%d] = {\n",
		(Int32)(compiledTables->numObjects > 0 ? compiledTables->numObjects : 1)
	);
	for (i = 0; i < compiledTables->numObjects; i++) {
		String objReference = GenerateValue(outputData, compiledTables->objects[i]);
		StringBuilder_AppendFormat(outputData->compiledTablesDecls, "\t%S,\n", objReference);
	}
	if (compiledTables->numObjects <= 0)
		StringBuilder_AppendFormat(outputData->compiledTablesDecls, "\tNULL,\n");
	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"};\n"
	);
}

static void GenerateCompiledTableTills(OutputData outputData, CompiledTables compiledTables)
{
	Int i;

	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"static TillContinuationInfo _compiledTables_Tills[%d] = {\n",
		(Int32)(compiledTables->numTillInfos > 0 ? compiledTables->numTillInfos : 1)
	);
	for (i = 0; i < compiledTables->numTillInfos; i++) {
		String tillReference = String_FromC("NULL");
		StringBuilder_AppendFormat(outputData->compiledTablesDecls, "\t%S,\n", tillReference);
	}
	if (compiledTables->numTillInfos <= 0)
		StringBuilder_AppendFormat(outputData->compiledTablesDecls, "\tNULL,\n");
	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"};\n"
	);
}

void GenerateCompiledTables(OutputData outputData, CompiledTables compiledTables)
{
	GenerateCompiledTableStrings(outputData, compiledTables);
	GenerateCompiledTableSourceLocations(outputData, compiledTables);
	GenerateCompiledTableFns(outputData, compiledTables);
	GenerateCompiledTableObjs(outputData, compiledTables);
	GenerateCompiledTableTills(outputData, compiledTables);

	StringBuilder_AppendFormat(outputData->compiledTablesDecls,
		"static struct CompiledTablesStruct _compiledTables = {\n"
		"\tNULL, NULL,\n"
		"\t_compiledTables_Strings, %d, %d, NULL,\n"
		"\t_compiledTables_Fns, %d, %d,\n"
		"\t_compiledTables_Objs, %d, %d,\n"
		"\t_compiledTables_Tills, %d, %d,\n"
		"\t_compiledTables_SrcLocs, %d, %d\n"
		"};\n",
		(Int32)compiledTables->numStrings, (Int32)compiledTables->numStrings,
		(Int32)compiledTables->numUserFunctions, (Int32)compiledTables->numUserFunctions,
		(Int32)compiledTables->numObjects, (Int32)compiledTables->numObjects,
		(Int32)compiledTables->numTillInfos, (Int32)compiledTables->numTillInfos,
		(Int32)compiledTables->numSourceLocations, (Int32)compiledTables->numSourceLocations
	);
}
