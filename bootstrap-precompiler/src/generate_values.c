
#include "stdafx.h"
#include "output_generator.h"
#include "string_helpers.h"

//-----------------------------------------------------------------------------
// Generation of common data objects.

String GenerateInt64Const(OutputData outputData, SmileInteger64 int64)
{
	String ident = String_Format("_int64_%s%ld",
		int64 < 0 ? "m" : "",
		int64 < 0 ? -int64->value : int64->value);
	String result = String_Format("((SmileInteger64)&%S)", ident);

	StringBuilder_AppendFormat(outputData->dataDecls, "static struct SmileInteger64Int %S;\n", ident);
	StringBuilder_AppendFormat(outputData->dataInits, "\tSmileInteger64_Init(&%S, %ld);\n\n", ident, int64->value);

	return result;
}

String GenerateInt32Const(OutputData outputData, SmileInteger32 int32)
{
	String ident = String_Format("_int32_%s%d",
		int32 < 0 ? "m" : "",
		int32 < 0 ? -int32->value : int32->value);
	String result = String_Format("((SmileInteger32)&%S)", ident);

	StringBuilder_AppendFormat(outputData->dataDecls, "static struct SmileInteger32Int %S;\n", ident);
	StringBuilder_AppendFormat(outputData->dataInits, "\tSmileInteger32_Init(&%S, %d);\n\n", ident, int32->value);

	return result;
}

String GenerateStringConst(OutputData outputData, String str)
{
	String result;
	String ident;

	if (StringDict_TryGetValue(outputData->stringToStaticsDict, str, (void **)&result))
		return result;

	ident = StringToCIdentifier(str);
	result = String_Format("((String)&_str_%SStruct)", ident);

	StringBuilder_AppendFormat(outputData->stringDecls,
		"STATIC_STRING(_str_%S, \"%S\");\n",
		ident,
		String_AddCSlashes(str));

	StringDict_SetValue(outputData->stringToStaticsDict, str, result);
	return result;
}

String GenerateSymbolId(OutputData outputData, Symbol symbol)
{
	String symbolId;
	String result;
	String stringRef;
	String symbolText;

	if (Int32Dict_TryGetValue(outputData->symbolIdsDict, symbol, (void **)&result))
		return result;

	symbolText = SymbolTable_GetName(Smile_SymbolTable, symbol);
	stringRef = GenerateStringConst(outputData, symbolText);

	symbolId = SymbolToCIdentifier(symbol);
	StringBuilder_AppendFormat(outputData->symbolDecls,
		"static Int32 _sym_%S;\n",
		symbolId);

	result = String_Format("_sym_%S", symbolId);
	StringBuilder_AppendFormat(outputData->symbolInits,
		"\t%S = SymbolTable_GetSymbol(symbolTable, %S);\n",
		result, stringRef);

	Int32Dict_SetValue(outputData->symbolIdsDict, symbol, result);

	return result;
}

String GenerateSymbolConst(OutputData outputData, Symbol symbol)
{
	String symbolId;
	String result;
	String stringRef;

	if (Int32Dict_TryGetValue(outputData->symbolToStaticsDict, symbol, (void **)&result))
		return result;

	stringRef = GenerateSymbolId(outputData, symbol);

	symbolId = SymbolToCIdentifier(symbol);
	StringBuilder_AppendFormat(outputData->smileSymbolDecls,
		"static struct SmileSymbolInt _smileSym_%S;\n",
		symbolId);

	result = String_Format("((SmileSymbol)&_smileSym_%S)", symbolId);
	StringBuilder_AppendFormat(outputData->smileSymbolInits,
		"\tSmileSymbol_Init(%S, %S);\n",
		result, stringRef);

	Int32Dict_SetValue(outputData->symbolToStaticsDict, symbol, result);

	return result;
}

String GenerateList(OutputData outputData, SmileList smileList)
{
	static Int32 listId = 0;

	Int32 dataId = listId++;
	String a = GenerateValue(outputData, smileList->a);
	String d = GenerateValue(outputData, smileList->d);
	String result;

	StringBuilder_AppendFormat(outputData->dataDecls,
		"static struct SmileListInt _list_%d;\n",
		dataId);
	result = String_Format("((SmileList)&_list_%d)", dataId);
	StringBuilder_AppendFormat(outputData->dataInits,
		"\tSmileList_Init(%S, (SmileObject)%S, (SmileObject)%S);\n\n",
		result, a, d);

	return result;
}

String GenerateValue(OutputData outputData, SmileObject obj)
{
	String result;
	void *priorName;

	if ((priorName = PointerDict_GetValue(outputData->dataToNamesDict, obj)) != NULL)
		return (String)priorName;

	switch (SMILE_KIND(obj)) {
	case SMILE_KIND_INTEGER32:
		result = GenerateInt32Const(outputData, (SmileInteger32)obj);
		break;
	case SMILE_KIND_INTEGER64:
		result = GenerateInt64Const(outputData, (SmileInteger64)obj);
		break;
	case SMILE_KIND_STRING:
		result = GenerateStringConst(outputData, (String)obj);
		break;
	case SMILE_KIND_SYMBOL:
		result = GenerateSymbolConst(outputData, ((SmileSymbol)obj)->symbol);
		break;
	case SMILE_KIND_FUNCTION:
		result = GenerateFunction(outputData, (SmileFunction)obj);
		break;
	case SMILE_KIND_LIST:
		result = GenerateList(outputData, (SmileList)obj);
		break;
	case SMILE_KIND_NULL:
		result = String_FromC("NullObject");
		break;

	default:
		result = String_Format("/* %S */", SmileObject_Stringify(obj));
		break;
	}

	PointerDict_SetValue(outputData->dataToNamesDict, obj, result);

	return result;
}
