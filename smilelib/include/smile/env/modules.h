#ifndef __SMILE_ENV_MODULES_H__
#define __SMILE_ENV_MODULES_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_ENV_SYMBOLTABLE_H__
#include <smile/env/symboltable.h>
#endif

#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif

#ifndef __SMILE_DICT_VARDICT_H__
#include <smile/dict/vardict.h>
#endif

#ifndef __SMILE_PARSING_PARSEMESSAGE_H__
#include <smile/parsing/parsemessage.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Public type declarations

struct ModuleInfoStruct {
	UInt32 id;
	Bool loadedSuccessfully;

	String name;

	SmileObject expr;
	ParseScope parseScope;

	Closure closure;
	EvalResult evalResult;
	VarDict exportDict;

	ParseMessage *parseMessages;
	Int numParseMessages;
};

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC UInt32 ModuleInfo_Register(ModuleInfo moduleInfo);
SMILE_API_FUNC void ModuleInfo_Unregister(ModuleInfo moduleInfo);
SMILE_API_FUNC ModuleInfo ModuleInfo_GetModuleByName(String name);
SMILE_API_FUNC ModuleInfo ModuleInfo_GetModuleById(UInt32 id);
SMILE_API_FUNC Int ModuleInfo_GetAllModules(ModuleInfo **modules);

SMILE_API_FUNC EvalResult ModuleInfo_InitForReal(ModuleInfo moduleInfo);

SMILE_API_FUNC ModuleInfo ModuleInfo_Create(String name, Bool loadedSuccessfully,
	SmileObject expr, ParseScope moduleParseScope,
	ParseMessage *parseMessages, Int numParseMessages);
SMILE_API_FUNC ModuleInfo ModuleInfo_CreateFromError(String name, ParseMessage parseMessage);
SMILE_API_FUNC Int ModuleInfo_GetExposedSymbols(ModuleInfo moduleInfo, Symbol **symbols);
SMILE_API_FUNC Bool ModuleInfo_IsExposedSymbol(ModuleInfo moduleInfo, Symbol symbol);
SMILE_API_FUNC SmileArg ModuleInfo_GetExposedValue(ModuleInfo moduleInfo, Symbol symbol);
SMILE_API_FUNC Int ModuleInfo_GetExposedValueClosureOffset(ModuleInfo moduleInfo, Symbol symbol);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Initialize the given module, if it hasn't already been initialized.  If it hasn't
/// been run, this evals its code, and returns its EvalResult.  Any changes in its "global"
/// state will be updated in its moduleInfo->closureInfo dictionary.
/// </summary>
Inline EvalResult ModuleInfo_Init(ModuleInfo moduleInfo)
{
	if (moduleInfo->evalResult == NULL)
		moduleInfo->evalResult = ModuleInfo_InitForReal(moduleInfo);
	return moduleInfo->evalResult;
}

/// <summary>
/// Return True if this module has been initialized (run at least once); or False if it
/// has not yet been initialized.
/// </summary>
Inline Bool ModuleInfo_IsInitialized(ModuleInfo moduleInfo)
{
	return moduleInfo->evalResult != NULL;
}

#endif