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

#include <smile/env/env.h>
#include <smile/env/symboltable.h>
#include <smile/env/knownsymbols.h>
#include <smile/parsing/parser.h>
#include <smile/eval/eval.h>

//-------------------------------------------------------------------------------------------------
// Common-global initialization.

#define DeclareCommonGlobal(__name__, __value__) \
	(name = (__name__), \
	varInfo.symbol = name, \
	varInfo.value = (SmileObject)(__value__), \
	VarDict_SetValue(varDict, name, &varInfo))

/// <summary>
/// In the given "global scope," declare all of the common objects that people expect to exist
/// in the "global scope":  That is, declare common objects like 'true' and 'false' and 'null',
/// as well as all of the base types like 'Object' and 'Integer' and 'String'.
/// </summary>
void Smile_InitCommonGlobals(ClosureInfo globalClosureInfo)
{
	struct VarInfoStruct varInfo;
	VarDict varDict = globalClosureInfo->variableDictionary;
	Symbol name;

	varInfo.kind = VAR_KIND_COMMONGLOBAL;
	varInfo.offset = 0;

	DeclareCommonGlobal(Smile_KnownSymbols.Object_,				Smile_KnownBases.Object);

	DeclareCommonGlobal(Smile_KnownSymbols.Number_,				Smile_KnownBases.Number);
	DeclareCommonGlobal(Smile_KnownSymbols.IntegerBase_,		Smile_KnownBases.IntegerBase);
	DeclareCommonGlobal(Smile_KnownSymbols.Byte_,				Smile_KnownBases.Byte);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer16_,			Smile_KnownBases.Integer16);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer32_,			Smile_KnownBases.Integer32);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer64_,			Smile_KnownBases.Integer64);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer_,			Smile_KnownBases.Integer64);

	DeclareCommonGlobal(Smile_KnownSymbols.RealBase_,			Smile_KnownBases.RealBase);
	DeclareCommonGlobal(Smile_KnownSymbols.Real32_,				Smile_KnownBases.Real32);
	DeclareCommonGlobal(Smile_KnownSymbols.Real64_,				Smile_KnownBases.Real64);
	DeclareCommonGlobal(Smile_KnownSymbols.Real_,				Smile_KnownBases.Real64);

	DeclareCommonGlobal(Smile_KnownSymbols.FloatBase_,			Smile_KnownBases.FloatBase);
	DeclareCommonGlobal(Smile_KnownSymbols.Float32_,			Smile_KnownBases.Float32);
	DeclareCommonGlobal(Smile_KnownSymbols.Float64_,			Smile_KnownBases.Float64);
	DeclareCommonGlobal(Smile_KnownSymbols.Float_,				Smile_KnownBases.Float64);

	DeclareCommonGlobal(Smile_KnownSymbols.Enumerable_,			Smile_KnownBases.Enumerable);
	DeclareCommonGlobal(Smile_KnownSymbols.List_,				Smile_KnownBases.List);
	DeclareCommonGlobal(Smile_KnownSymbols.String_,				Smile_KnownBases.String);
	DeclareCommonGlobal(Smile_KnownSymbols.ArrayBase_,			Smile_KnownBases.ArrayBase);
	DeclareCommonGlobal(Smile_KnownSymbols.Array_,				Smile_KnownBases.Array);
	DeclareCommonGlobal(Smile_KnownSymbols.Fn_,					Smile_KnownBases.Fn);
	DeclareCommonGlobal(Smile_KnownSymbols.Bool_,				Smile_KnownBases.Bool);
	DeclareCommonGlobal(Smile_KnownSymbols.Symbol_,				Smile_KnownBases.Symbol);
	DeclareCommonGlobal(Smile_KnownSymbols.Exception_,			Smile_KnownBases.Exception);

	DeclareCommonGlobal(Smile_KnownSymbols.Range_,				Smile_KnownBases.Range);
	DeclareCommonGlobal(Smile_KnownSymbols.CharRange_,			Smile_KnownBases.CharRange);
	DeclareCommonGlobal(Smile_KnownSymbols.UniRange_,			Smile_KnownBases.UniRange);
	DeclareCommonGlobal(Smile_KnownSymbols.NumericRange_,		Smile_KnownBases.NumericRange);
	DeclareCommonGlobal(Smile_KnownSymbols.IntegerRangeBase_,	Smile_KnownBases.IntegerRangeBase);
	DeclareCommonGlobal(Smile_KnownSymbols.ByteRange_,			Smile_KnownBases.ByteRange);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer16Range_,		Smile_KnownBases.Integer16Range);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer32Range_,		Smile_KnownBases.Integer32Range);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer64Range_,		Smile_KnownBases.Integer64Range);
	DeclareCommonGlobal(Smile_KnownSymbols.IntegerRange_,		Smile_KnownBases.Integer64Range);
	DeclareCommonGlobal(Smile_KnownSymbols.Real32Range_,		Smile_KnownBases.Real32Range);
	DeclareCommonGlobal(Smile_KnownSymbols.Real64Range_,		Smile_KnownBases.Real64Range);
	DeclareCommonGlobal(Smile_KnownSymbols.RealRangeBase_,		Smile_KnownBases.RealRangeBase);
	DeclareCommonGlobal(Smile_KnownSymbols.Float32Range_,		Smile_KnownBases.Float32Range);
	DeclareCommonGlobal(Smile_KnownSymbols.Float64Range_,		Smile_KnownBases.Float64Range);
	DeclareCommonGlobal(Smile_KnownSymbols.FloatRangeBase_,		Smile_KnownBases.FloatRangeBase);

	DeclareCommonGlobal(Smile_KnownSymbols.ByteArray_,			Smile_KnownBases.ByteArray);

	DeclareCommonGlobal(Smile_KnownSymbols.true_,				Smile_KnownObjects.TrueObj);
	DeclareCommonGlobal(Smile_KnownSymbols.false_,				Smile_KnownObjects.FalseObj);
	DeclareCommonGlobal(Smile_KnownSymbols.null_,				Smile_KnownObjects.NullInstance);
}

//-------------------------------------------------------------------------------------------------
// Custom "global" variable declaration in the common global closure.

static ClosureInfo _globalClosureInfo = NULL;

/// <summary>
/// Get the ClosureInfo object for the current global closure.
/// </summary>
/// <returns>The current global ClosureInfo.</returns>
ClosureInfo Smile_GetGlobalClosureInfo(void)
{
	return _globalClosureInfo;
}

/// <summary>
/// Set the ClosureInfo object for the current global closure.
/// </summary>
/// <param name="globalClosureInfo">The new global ClosureInfo, which must be of kind CLOSURE_KIND_GLOBAL.</param>
void Smile_SetGlobalClosureInfo(ClosureInfo globalClosureInfo)
{
	if (globalClosureInfo->kind != CLOSURE_KIND_GLOBAL)
		return;

	_globalClosureInfo = globalClosureInfo;
}

/// <summary>
/// Assign a variable in the global closure.
/// </summary>
/// <param name="name">The name of the variable to assign, as a symbol.</param>
/// <param name="value">The value for the variable to assign.</param>
void Smile_SetGlobalVariable(Symbol name, SmileObject value)
{
	struct VarInfoStruct varInfo;

	varInfo.kind = VAR_KIND_GLOBAL;
	varInfo.offset = 0;
	varInfo.symbol = name;
	varInfo.value = value;

	VarDict_SetValue(_globalClosureInfo->variableDictionary, name, &varInfo);
}

/// <summary>
/// Delete (remove) a variable and its value from the global closure.
/// </summary>
/// <param name="name">The name of the variable to delete, as a symbol.</param>
void Smile_DeleteGlobalVariable(Symbol name)
{
	VarDict_Remove(_globalClosureInfo->variableDictionary, name);
}

/// <summary>
/// Determine whether the given named variable exists in the global closure.
/// </summary>
/// <param name="name">The name of the variable to check for, as a symbol.</param>
/// <returns>True if the variable exists, false if it does not.</returns>
Bool Smile_HasGlobalVariable(Symbol name)
{
	return VarDict_ContainsKey(_globalClosureInfo->variableDictionary, name);
}

/// <summary>
/// Get the value of a variable in the global closure.
/// </summary>
/// <param name="name">The name of the global variable, as a symbol.</param>
/// <returns>The value for the global variable, if any; or the NullObject if there is no such variable.</param>
SmileObject Smile_GetGlobalVariable(Symbol name)
{
	VarInfo varInfo = VarDict_GetValue(_globalClosureInfo->variableDictionary, name);
	return (varInfo != NULL ? varInfo->value : NullObject);
}

/// <summary>
/// Get the value of a variable in the global closure.
/// </summary>
/// <param name="name">The name of the global variable, as a nul-terminated C-style string.</param>
/// <returns>The value for the global variable, if any; or the NullObject if there is no such
/// variable.  Note that if the variable does not exist, a name for it will *not* be added to
/// the symbol table by this function:  This function has no side-effects.</param>
SmileObject Smile_GetGlobalVariableC(const char *name)
{
	VarInfo varInfo;
	Symbol nameSymbol;

	nameSymbol = SymbolTable_GetSymbolNoCreateC(Smile_SymbolTable, name);
	if (!nameSymbol) return NullObject;

	varInfo = VarDict_GetValue(_globalClosureInfo->variableDictionary, nameSymbol);
	return (varInfo != NULL ? varInfo->value : NullObject);
}

//-------------------------------------------------------------------------------------------------
// Parsing and running code in the common global closure.

/// <summary>
/// Parse the given source code (text) into a complete object structure with all of its syntax resolved.
/// This performs the parse inside the global scope, with the given external variables declared in the
/// code's local scope.
/// <summary>
/// <param name="text">The actual Smile source code to parse.</param>
/// <param name="filename">The name of the file that source code came from (for error-reporting).</param>
/// <param name="vars">Any external variables that must be imported for this code.</param>
/// <param name="numVars">The number of external variables to import.</param>
/// <param name="parseMessages">If not NULL, this will be set to an array of any errors or warnings that were generated while parsing.</param>
/// <param name="numParseMessages">If not NULL, this will be set to the number of entries in the parseMessages array.</param>
/// <param name="moduleScope">If not NULL, this will be set to the scope-declaration collection for the source code.</param>
/// <returns>The fully-parsed source code, or NullObject if there was a parsing error.</returns>
SmileObject Smile_ParseInScope(String text, String filename,
	ExternalVar *vars, Int numVars,
	ParseMessage **parseMessages, Int *numParseMessages, ParseScope *moduleScope)
{
	SmileObject result;
	Lexer lexer;
	Parser parser;
	ParseScope outerScope;
	struct ParseMessageStruct *parseMessage, **destMessage;
	Int numMessages;

	// Construct the parsing environment.
	lexer = Lexer_Create(text, 0, String_Length(text), filename, 1, 0);
	Lexer_SetSymbolTable(lexer, Smile_SymbolTable);
	parser = Parser_Create();
	outerScope = ParseScope_CreateRoot();

	parser->externalVars = vars;
	parser->numExternalVars = numVars;

	// Declare the names of all of the variables in the global scope.
	parseMessage = ParseScope_DeclareVariablesFromClosureInfo(outerScope, Smile_GetGlobalClosureInfo());

	// Deal with any errors resulting from bad global-variable declarations.
	if (parseMessage != NULL) {
		*parseMessages = GC_MALLOC_STRUCT(struct ParseMessageStruct *);
		if (*parseMessages == NULL)
			Smile_Abort_OutOfMemory();
		if (parseMessages != NULL)
			**parseMessages = parseMessage;
		*numParseMessages = 1;
		return NullObject;
	}

	// Now parse the source code.
	result = Parser_ParseWithDetails(parser, lexer, outerScope, moduleScope);

	// Deal with any errors resulting from the user's source code.
	if (SMILE_KIND(parser->firstMessage) != SMILE_KIND_NULL) {
		numMessages = Parser_GetErrorOrWarningCount(parser);
		if (numParseMessages != NULL)
			*numParseMessages = numMessages;
		if (parseMessages != NULL) {
			SmileList list;

			*parseMessages = destMessage = GC_MALLOC_STRUCT_ARRAY(struct ParseMessageStruct *, numMessages);
			if (*parseMessages == NULL)
				Smile_Abort_OutOfMemory();

			for (list = parser->firstMessage; SMILE_KIND(list) != SMILE_KIND_NULL; list = LIST_REST(list)) {
				if (((ParseMessage)list->a)->messageKind == PARSEMESSAGE_WARNING
					|| ((ParseMessage)list->a)->messageKind == PARSEMESSAGE_ERROR
					|| ((ParseMessage)list->a)->messageKind == PARSEMESSAGE_FATAL) {
					*destMessage++ = (ParseMessage)list->a;
				}
			}
		}
		return NullObject;
	}

	// Done!
	if (parseMessages != NULL)
		*parseMessages = NULL;
	if (numParseMessages != NULL)
		*numParseMessages = 0;
	return result;
}

// Helper function for Smile_EvalInScope().
static Bool DeclareVariableForCompiler(VarInfo varInfo, void *param)
{
	CompileScope_DefineSymbol((CompileScope)param, varInfo->symbol, PARSEDECL_GLOBAL, 0);
	return True;
}

/// <summary>
/// Evaluate the given expression in the given scope.
/// </summary>
/// <param name="globalClosureInfo">The global scope in which to evaluate the given expression.</param>
/// <param name="expression">The expression to evaluate, as a tree of Smile objects.</param>
/// <returns>The result of evaluating the given expression in the given global scope.</returns>
EvalResult Smile_EvalInScope(ClosureInfo globalClosureInfo, SmileObject expression)
{
	Compiler compiler;
	CompileScope compileScope;
	UserFunctionInfo globalFunction;
	EvalResult result;

	// Set up the compiler...
	compiler = Compiler_Create();
	Compiler_SetGlobalClosureInfo(compiler, globalClosureInfo);

	// Now go through the global closure and declare all of its contents in a new global scope.
	// This is so that the compiler can unambiguously tell the difference between whether it should
	// use LdX/StX on a variable name, or whether it should bail for an unknown variable.
	compileScope = Compiler_BeginScope(compiler, PARSESCOPE_OUTERMOST);
	VarDict_ForEach(globalClosureInfo->variableDictionary, DeclareVariableForCompiler, compileScope);
	globalFunction = Compiler_CompileGlobal(compiler, expression);
	Compiler_EndScope(compiler);

	// If the compile failed, stop now.
	if (compiler->firstMessage != NullList) {
		SmileList parseMessage;
		Int index;

		result = EvalResult_Create(EVAL_RESULT_PARSEERRORS);
		result->numMessages = SmileList_Length(compiler->firstMessage);
		result->parseMessages = GC_MALLOC_STRUCT_ARRAY(ParseMessage, result->numMessages);
		if (result->parseMessages == NULL)
			Smile_Abort_OutOfMemory();

		index = 0;
		for (parseMessage = compiler->firstMessage; SMILE_KIND(parseMessage) != SMILE_KIND_NULL; parseMessage = LIST_REST(parseMessage)) {
			result->parseMessages[index++] = (ParseMessage)LIST_FIRST(parseMessage);
		}

		return result;
	}

	// Now run the compiled bytecode!
	result = Eval_Run(globalFunction);

	return result;
}
