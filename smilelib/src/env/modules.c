//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2018 Sean Werkema
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

#include <smile/env/modules.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/dict/vardict.h>
#include <smile/eval/closure.h>
#include <smile/eval/eval.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/eval/compiler_internal.h>

ModuleInfo *ModuleArray = NULL;
static StringDict _moduleDict;
static UInt32 _moduleId = 1;
static UInt32 _moduleArrayMax = 0;

/// <summary>
/// Register the given module as a valid, loaded module.
/// </summary>
/// <param name="moduleInfo">The module to register.</param>
/// <returns>A new, unique ID for this module.</returns>
UInt32 ModuleInfo_Register(ModuleInfo moduleInfo)
{
	if (_moduleDict == NULL)
		_moduleDict = StringDict_Create();

	if (StringDict_ContainsKey(_moduleDict, moduleInfo->name))
		Smile_Abort_FatalError(String_ToC(String_Format("Cannot register duplicate module \"%S\".", moduleInfo->name)));
	if (moduleInfo->id != 0)
		Smile_Abort_FatalError(String_ToC(String_Format("Module \"%S\" has already been registered.", moduleInfo->name)));

	StringDict_SetValue(_moduleDict, moduleInfo->name, moduleInfo);
	moduleInfo->id = _moduleId++;

	if (ModuleArray == NULL) {
		_moduleArrayMax = 64;
		ModuleArray = GC_MALLOC_STRUCT_ARRAY(ModuleInfo, _moduleArrayMax);
	}
	else if (_moduleId >= _moduleArrayMax) {
		ModuleInfo *newModuleArray;

		newModuleArray = GC_MALLOC_STRUCT_ARRAY(ModuleInfo, _moduleArrayMax * 2);
		MemCpy(newModuleArray, ModuleArray, _moduleArrayMax);
		_moduleArrayMax *= 2;
		ModuleArray = newModuleArray;
	}

	ModuleArray[moduleInfo->id] = moduleInfo;

	return moduleInfo->id;
}

/// <summary>
/// Unregister the given module.
/// </summary>
/// <param name="moduleInfo">The module to unregister.</param>
void ModuleInfo_Unregister(ModuleInfo moduleInfo)
{
	if (_moduleDict == NULL || moduleInfo->id != 0)
		Smile_Abort_FatalError(String_ToC(String_Format("Module \"%S\" is not registered.", moduleInfo->name)));

	ModuleArray[moduleInfo->id] = NULL;
	StringDict_Remove(_moduleDict, moduleInfo->name);
}

/// <summary>
/// Get a module by its unique ID. This runs in O(1) time.
/// </summary>
/// <param name="id">The ID of the module to locate.</param>
/// <returns>The ModuleInfo for that ID, or NULL if the ID matches no known module.</returns>
ModuleInfo ModuleInfo_GetModuleById(UInt32 id)
{
	return (id < _moduleId ? ModuleArray[id] : NULL);
}

/// <summary>
/// Get a module by its name. This runs in O(n) time proportional to the length of the name.
/// </summary>
/// <param name="name">The name of the module to locate.</param>
/// <returns>The ModuleInfo for that name, or NULL if the name matches no known module.</returns>
ModuleInfo ModuleInfo_GetModuleByName(String name)
{
	ModuleInfo moduleInfo;
	if (_moduleDict == NULL || !StringDict_TryGetValue(_moduleDict, name, &moduleInfo))
		return NULL;
	return moduleInfo;
}

/// <summary>
/// Return the complete set of all registered modules.
/// </summary>
/// <param name="modules">A return pointer that will be set to an array containing
/// all registered modules (unordered!).  This can be NULL if you don't want the
/// actual module data but simply want the count.</param>
/// <returns>The count of all registered modules.</returns>
Int ModuleInfo_GetAllModules(ModuleInfo **modules)
{
	Int count;

	if (_moduleDict == NULL) {
		*modules = NULL;
		return 0;
	}

	count = StringDict_Count(_moduleDict);
	if (modules != NULL) {
		*modules = (ModuleInfo *)StringDict_GetValues(_moduleDict);
	}

	return count;
}

/// <summary>
/// Create a ModuleInfo object, given a parsed chunk of source code.
/// </summary>
/// <param name="name">The name of the module.  For built-in "standard" modules, this
/// is simply the "standard" name.  For user modules, this is the full, canonical
/// filesystem path to the source file.</param>
/// <param name="loadedSuccessfully">Whether this module was loaded successfully, or whether
/// it had load/parse errors.  Used to avoid reloading "failed" modules.</param>
/// <param name="expr">An expression that fully describes the (not-yet-executed) source code of this module.</param>
/// <param name="moduleParseScope">A collection of ParseDecls describing this module's exported variable names.</param>
/// <param name="parseMessages">An array of any messages (errors) generated as a result of parsing this
/// module's source code.</param>
/// <param name="numParseMessages">The number of messages generated by parsing this
/// module's source code.</param>
/// <returns>A ModuleInfo object describing the new module.</returns>
ModuleInfo ModuleInfo_Create(String name, Bool loadedSuccessfully,
	SmileObject expr, ParseScope moduleParseScope,
	ParseMessage *parseMessages, Int numParseMessages)
{
	ModuleInfo moduleInfo = GC_MALLOC_STRUCT(struct ModuleInfoStruct);

	moduleInfo->id = 0;
	moduleInfo->name = name;
	moduleInfo->loadedSuccessfully = loadedSuccessfully;
	moduleInfo->expr = expr;
	moduleInfo->closure = NULL;
	moduleInfo->evalResult = NULL;
	moduleInfo->exportDict = NULL;
	moduleInfo->parseScope = moduleParseScope;
	moduleInfo->parseMessages = parseMessages;
	moduleInfo->numParseMessages = numParseMessages;

	return moduleInfo;
}

/// <summary>
/// Create a ModuleInfo object, given a failed load/parse of a module.
/// </summary>
/// <param name="name">The name of the module.  For built-in "standard" modules, this
/// is simply the "standard" name.  For user modules, this is the full, canonical
/// filesystem path to the source file.</param>
/// <param name="parseMessage">The generated error.</param>
/// <returns>A ModuleInfo object describing the failed module.</returns>
ModuleInfo ModuleInfo_CreateFromError(String name, ParseMessage parseMessage)
{
	ModuleInfo moduleInfo = GC_MALLOC_STRUCT(struct ModuleInfoStruct);

	moduleInfo->id = 0;
	moduleInfo->name = name;
	moduleInfo->loadedSuccessfully = False;
	moduleInfo->expr = NullObject;
	moduleInfo->evalResult = NULL;
	moduleInfo->closure = NULL;
	moduleInfo->parseScope = NULL;

	moduleInfo->parseMessages = GC_MALLOC_STRUCT_ARRAY(ParseMessage, 1);
	moduleInfo->parseMessages[0] = parseMessage;
	moduleInfo->numParseMessages = 1;

	return moduleInfo;
}

/// <summary>
/// Return a complete collection of all symbols exposed by this module.
/// </summary>
/// <param name="moduleInfo">The module to examine.</param>
/// <param name="symbols">Return value: An array of all symbols exposed by this module (undefined
/// if no symbols are exposed).</param>
/// <returns>The number of symbols exposed by this module, which may be zero if it does not expose any.</returns>
Int ModuleInfo_GetExposedSymbols(ModuleInfo moduleInfo, Symbol **symbols)
{
	Int numDecls;
	ParseDecl *decls;
	Int numExposedSymbols;
	Int i, dest;
	ParseDecl decl;
	Symbol *exposedSymbols;

	// Get all of the declarations from the scope.
	numDecls = moduleInfo->parseScope->numDecls;
	decls = moduleInfo->parseScope->decls;

	// Count up how many of them are exposable declarations.
	numExposedSymbols = 0;
	for (i = 0; i < numDecls; i++) {
		decl = decls[i];
		if (decl->declKind == PARSEDECL_VARIABLE || decl->declKind == PARSEDECL_CONST)
			numExposedSymbols++;
	}

	// If nothing was exposed, return nothing.
	if (numExposedSymbols == 0) {
		*symbols = NULL;
		return 0;
	}

	// Make an array in which to put the exposed symbol collection, and then copy in the exposed symbols.
	exposedSymbols = GC_MALLOC_RAW_ARRAY(Symbol, numExposedSymbols);
	dest = 0;
	for (i = 0; i < numDecls; i++) {
		decl = decls[i];
		if (decl->declKind == PARSEDECL_VARIABLE || decl->declKind == PARSEDECL_CONST) {
			exposedSymbols[dest++] = decl->symbol;
		}
	}

	// Return it.
	*symbols = exposedSymbols;
	return numExposedSymbols;
}

/// <summary>
/// Determine if this module exports the given named symbol.  This runs in amortized O(1) time.
/// </summary>
/// <param name="libraryInfo">The library to test.</param>
/// <param name="symbol">The symbol to search for in that library.</param>
/// <returns>True if the symbol is exported by the library, False if it is an unknown symbol or not exported.</returns>
Bool ModuleInfo_IsExposedSymbol(ModuleInfo moduleInfo, Symbol symbol)
{
	Int32 index;
	ParseDecl decl;

	if (!Int32Int32Dict_TryGetValue(moduleInfo->parseScope->symbolDict, symbol, &index))
		return False;

	decl = moduleInfo->parseScope->decls[index];
	return (decl->declKind == PARSEDECL_VARIABLE || decl->declKind == PARSEDECL_CONST);
}

/// <summary>
/// Get the value for a symbol exposed by this module.  This runs in amortized O(1) time.
/// </summary>
/// <param name="moduleInfo">The module to extract a value from.</param>
/// <param name="symbol">The symbol to search for in that module.</param>
/// <returns>The value for that symbol, as exposed by this module, or NullObject if that symbol
/// does not exist or is not exposed by this module.</returns>
SmileArg ModuleInfo_GetExposedValue(ModuleInfo moduleInfo, Symbol symbol)
{
	VarInfo varInfo;

	// If we haven't yet compiled any part of this library, quickly precompute its exports.
	if (moduleInfo->exportDict == NULL)
		moduleInfo->exportDict = Compiler_PrecomputeModuleClosureLayout(moduleInfo->expr, moduleInfo->parseScope);

	// First, make sure this library has heard of this symbol.
	if (!VarDict_TryGetValue(moduleInfo->exportDict, symbol, &varInfo))
		return SmileArg_From(NullObject);

	// Next, make sure it's a symbol the library intended to share.
	if (!(varInfo->kind == VAR_KIND_VAR || varInfo->kind == VAR_KIND_CONST))
		return SmileArg_From(NullObject);

	// Now, since we know it's supposed to be exposed, go get its value.
	return moduleInfo->closure->variables[varInfo->offset];
}

/// <summary>
/// Get the offset within this module's closure for one of its exposed symbol's values.
/// This runs in amortized O(1) time.
/// </summary>
/// <param name="moduleInfo">The module to extract a symbol offset from.</param>
/// <param name="symbol">The symbol to search for in that module.</param>
/// <returns>The offset for that symbol, as exposed by this module, or -1 if that symbol
/// does not exist or is not exposed by this module.</returns>
Int ModuleInfo_GetExposedValueClosureOffset(ModuleInfo moduleInfo, Symbol symbol)
{
	VarInfo varInfo;

	// If we haven't yet compiled any part of this library, quickly precompute its exports.
	if (moduleInfo->exportDict == NULL)
		moduleInfo->exportDict = Compiler_PrecomputeModuleClosureLayout(moduleInfo->expr, moduleInfo->parseScope);

	// First, make sure this library has heard of this symbol.
	if (!VarDict_TryGetValue(moduleInfo->exportDict, symbol, &varInfo))
		return -1;

	// Next, make sure it's a symbol the library intended to share.
	if (!(varInfo->kind == VAR_KIND_VAR || varInfo->kind == VAR_KIND_CONST))
		return -1;

	// Now, since we know it's supposed to be exposed, go get its offset.
	return varInfo->offset;
}

/// <summary>
/// Given a module known not to have been initialized yet, run its code.
/// </summary>
/// <param name="moduleInfo">The module that has yet to be run.</param>
/// <returns>A copy of the EvalResult for the run of this module.</returns>
EvalResult ModuleInfo_InitForReal(ModuleInfo moduleInfo)
{
	// Did we already initialize this?
	if (moduleInfo->evalResult != NULL)
		return moduleInfo->evalResult;

	// Did it fail its parse?  If so, the result is a failure result.
	if (moduleInfo->numParseMessages != 0) {
		ParseMessage *m, *end;

		for (m = moduleInfo->parseMessages, end = moduleInfo->parseMessages + moduleInfo->numParseMessages; m < end; m++) {
			if ((*m)->kind == PARSEMESSAGE_ERROR || (*m)->kind == PARSEMESSAGE_FATAL) {

				// The parser bailed, so we abort with an error before trying to eval.
				EvalResult result = EvalResult_Create(EVAL_RESULT_PARSEERRORS);
				result->parseMessages = moduleInfo->parseMessages;
				result->numMessages = moduleInfo->numParseMessages;
				return moduleInfo->evalResult = result;
			}
		}
	}

	// Go compile and eval it for real.
	moduleInfo->evalResult = Smile_EvalInScope(Smile_GetGlobalClosureInfo(), moduleInfo->expr);

	// Return the result.
	moduleInfo->closure = moduleInfo->evalResult->closure;
	return moduleInfo->evalResult;
}
