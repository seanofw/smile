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

	varInfo.kind = VAR_KIND_GLOBAL;
	varInfo.offset = 0;

	DeclareCommonGlobal(Smile_KnownSymbols.Object_,	Smile_KnownBases.Object);
	DeclareCommonGlobal(Smile_KnownSymbols.Number_,	  Smile_KnownBases.Number);
	DeclareCommonGlobal(Smile_KnownSymbols.IntegerBase_,	    Smile_KnownBases.IntegerBase);
	DeclareCommonGlobal(Smile_KnownSymbols.Byte_,	      Smile_KnownBases.Byte);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer16_,	      Smile_KnownBases.Integer16);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer32_,	      Smile_KnownBases.Integer32);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer64_,	      Smile_KnownBases.Integer64);
	DeclareCommonGlobal(Smile_KnownSymbols.Integer_,	      Smile_KnownBases.Integer64);
	DeclareCommonGlobal(Smile_KnownSymbols.Enumerable_,	  Smile_KnownBases.Enumerable);
	DeclareCommonGlobal(Smile_KnownSymbols.List_,	    Smile_KnownBases.List);
	DeclareCommonGlobal(Smile_KnownSymbols.String_,	    Smile_KnownBases.String);
	DeclareCommonGlobal(Smile_KnownSymbols.ArrayBase_,	    Smile_KnownBases.ArrayBase);
	DeclareCommonGlobal(Smile_KnownSymbols.Array_,	      Smile_KnownBases.Array);
	DeclareCommonGlobal(Smile_KnownSymbols.Pair_,	  Smile_KnownBases.Pair);
	DeclareCommonGlobal(Smile_KnownSymbols.Fn_,	  Smile_KnownBases.Function);
	DeclareCommonGlobal(Smile_KnownSymbols.Bool_,	  Smile_KnownBases.Bool);
	DeclareCommonGlobal(Smile_KnownSymbols.Symbol_,	  Smile_KnownBases.Symbol);
	DeclareCommonGlobal(Smile_KnownSymbols.Exception_,	  Smile_KnownBases.Exception);

	DeclareCommonGlobal(Smile_KnownSymbols.true_,	Smile_KnownObjects.TrueObj);
	DeclareCommonGlobal(Smile_KnownSymbols.false_,	Smile_KnownObjects.FalseObj);
	DeclareCommonGlobal(Smile_KnownSymbols.null_,	Smile_KnownObjects.NullInstance);
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
/// <summary>
/// <param name="text">The actual Smile source code to parse.</param>
/// <param name="filename">The name of the file that source code came from (for error-reporting).</param>
/// <param name="parseMessages">This will be set to an array of any errors or warnings that were generated while parsing.</param>
/// <param name="numParseMessages">This will be set to the number of entries in the parseMessages array.</param>
/// <returns>The fully-parsed source code, or NullObject if there was a parsing error.</returns>
SmileObject Smile_Parse(String text, String filename, struct ParseMessageStruct ***parseMessages, Int *numParseMessages)
{
	return Smile_ParseInScope(_globalClosureInfo, text, filename, parseMessages, numParseMessages);
}

/// <summary>
/// Parse the given source code (text) into a complete object structure with all of its syntax resolved.
/// This performs the parse inside the given global scope, where some global objects may already have been declared.
/// <summary>
/// <param name="globalClosureInfo">The closure in which the parse takes place.</param>
/// <param name="text">The actual Smile source code to parse.</param>
/// <param name="filename">The name of the file that source code came from (for error-reporting).</param>
/// <param name="parseMessages">This will be set to an array of any errors or warnings that were generated while parsing.</param>
/// <param name="numParseMessages">This will be set to the number of entries in the parseMessages array.</param>
/// <returns>The fully-parsed source code, or NullObject if there was a parsing error.</returns>
SmileObject Smile_ParseInScope(ClosureInfo globalClosureInfo, String text, String filename, struct ParseMessageStruct ***parseMessages, Int *numParseMessages)
{
	SmileObject result;
	Lexer lexer;
	Parser parser;
	ParseScope globalScope;
	struct ParseMessageStruct *parseMessage, **destMessage;
	Int numMessages;

	// Construct the parsing environment.
	lexer = Lexer_Create(text, 0, String_Length(text), filename, 1, 0);
	parser = Parser_Create();
	globalScope = ParseScope_CreateRoot();

	// Declare the names of all of the variables in the global scope.
	parseMessage = ParseScope_DeclareVariablesFromClosureInfo(globalScope, globalClosureInfo);

	// Deal with any errors resulting from bad global-variable declarations.
	if (parseMessage != NULL) {
		*parseMessages = GC_MALLOC_STRUCT(struct ParseMessageStruct *);
		if (*parseMessages == NULL)
			Smile_Abort_OutOfMemory();
		**parseMessages = parseMessage;
		*numParseMessages = 1;
		return NullObject;
	}

	// Now parse the source code.
	result = Parser_Parse(parser, lexer, globalScope);

	// Deal with any errors resulting from the user's source code.
	if (parser->firstMessage != NULL) {
		*numParseMessages = numMessages = Parser_GetErrorOrWarningCount(parser);
		*parseMessages = destMessage = GC_MALLOC_STRUCT_ARRAY(struct ParseMessageStruct *, numMessages);
		if (*parseMessages == NULL)
			Smile_Abort_OutOfMemory();
	
		for (SmileList list = parser->firstMessage; SMILE_KIND(list) != SMILE_KIND_NULL; list = LIST_REST(list)) {
			if (((ParseMessage)list->a)->messageKind == PARSEMESSAGE_WARNING
				|| ((ParseMessage)list->a)->messageKind == PARSEMESSAGE_ERROR
				|| ((ParseMessage)list->a)->messageKind == PARSEMESSAGE_FATAL) {
				*destMessage++ = (ParseMessage)list->a;
			}
		}
		return NullObject;
	}

	// Done!
	return result;
}

/// <summary>
/// Evaluate the given expression in the global scope.
/// </summary>
/// <param name="expression">The expression to evaluate, as a tree of Smile objects.</param>
/// <returns>The result of evaluating the given expression in the global scope.</returns>
EvalResult Smile_Eval(SmileObject expression)
{
	return Smile_EvalInScope(_globalClosureInfo, expression);
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

	// Now run the compiled bytecode!
	result = Eval_Run(compiler->compiledTables, globalFunction);

	return result;
}

/// <summary>
/// Parse and evaluate the given source code in the global scope.
/// </summary>
/// <param name="text">The actual Smile source code to parse.</param>
/// <param name="filename">The name of the file that source code came from (for error-reporting).</param>
/// <returns>The result of parsing, compiling, and evaluating the given expression in the given global scope.</returns>
EvalResult Smile_ParseAndEval(String text, String filename)
{
	return Smile_ParseAndEvalInScope(_globalClosureInfo, text, filename);
}

/// <summary>
/// Parse and evaluate the given source code in the given scope.
/// </summary>
/// <param name="globalClosureInfo">The global scope in which to evaluate the given Smile source code.</param>
/// <param name="text">The actual Smile source code to parse.</param>
/// <param name="filename">The name of the file that source code came from (for error-reporting).</param>
/// <returns>The result of parsing, compiling, and evaluating the given expression in the given global scope.</returns>
EvalResult Smile_ParseAndEvalInScope(ClosureInfo globalClosureInfo, String text, String filename)
{
	ParseMessage *parseMessages;
	Int numParseMessages;
	SmileObject expression;
	EvalResult result;

	expression = Smile_ParseInScope(globalClosureInfo, text, filename, &parseMessages, &numParseMessages);

	if (SMILE_KIND(expression) == SMILE_KIND_NULL) {
		ParseMessage *m;
	
		for (m = parseMessages; m < parseMessages + numParseMessages; m++) {
			if ((*m)->kind == PARSEMESSAGE_ERROR || (*m)->kind == PARSEMESSAGE_FATAL) {

				// The parser bailed, so we abort with an error before trying to eval.
				result = EvalResult_Create(EVAL_RESULT_PARSEERRORS);
				result->parseMessages = parseMessages;
				result->numMessages = numParseMessages;
				return result;
			}
		}
	}

	result = Smile_EvalInScope(globalClosureInfo, expression);

	return result;
}
