#ifndef __SMILE_EVAL_COMPILER_H__
#define __SMILE_EVAL_COMPILER_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_GC_H__
#include <smile/gc.h>
#endif

#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif

#ifndef __SMILE_DICT_STRINGINTDICT_H__
#include <smile/dict/stringintdict.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILELIST_H__
#include <smile/smiletypes/smilelist.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEFUNCTION_H__
#include <smile/smiletypes/smilefunction.h>
#endif

#ifndef __SMILE_EVAL_BYTECODE_H__
#include <smile/eval/bytecode.h>
#endif

#ifndef __SMILE_EVAL_OPCODE_H__
#include <smile/eval/opcode.h>
#endif

#ifndef __SMILE_EVAL_CLOSURE_H__
#include <smile/eval/closure.h>
#endif

#ifndef __SMILE_EVAL_COMPILEDBLOCK_H__
#include <smile/eval/compiledblock.h>
#endif

typedef enum {
	COMPILE_FLAG_NORESULT = (1 << 0),		// Try to compile this expr only for its side-effects; avoid leaving anything on the stack.
} CompileFlags;

//-------------------------------------------------------------------------------------------------
//  Type declarations.

/// <summary>
/// This represents a single compiled function (i.e., a [fn ...] expression).  It may be
/// incomplete, which is what the 'isCompiled' flag answers.
/// </summary>
typedef struct CompilerFunctionStruct {
	struct CompilerFunctionStruct *parent;	// The parent function that contains this function (NULL if none).
	Int functionDepth;						// The number of functions deep in which this function was declared.
	Int numArgs;							// The number of arguments to this function (derived from 'args').
	Int32 localSize;						// The total number of locals required by this function.
	Int32 localMax;							// The current maximum number of locals in the localNames array.
	Symbol *localNames;						// The names of all the locals in this function, in order.
	Int currentStackDepth;					// The current depth of the temporary stack.
	Int stackSize;							// The total number of stack temporaries required by this function.
	Bool isCompiled;						// Whether this function is fully or only partially compiled.
	SmileList args;							// This function's arguments, from its raw expression.
	SmileObject body;						// This function's body, from its raw expression.
	ClosureInfo closureInfo;				// The ClosureInfo object needed to actually eval this function.
	Int currentSourceLocation;				// The current source location, if any.
	UserFunctionInfo userFunctionInfo;		// The UserFunctionInfo object that is being generated from this.

	struct TillContinuationInfoStruct **tillInfos;	// The till-continuation-info objects collected during the compile.
	Int numTillInfos;						// The number of till-continuation-info objects collected.
	Int maxTillInfos;						// The maximum number of till-continuation-info objects in the array.
} *CompilerFunction;

typedef struct CompileScopeStruct {
	struct CompileScopeStruct *parent;		// The parent scope (NULL if none).
	Int32Dict symbolDict;					// How to resolve symbols in this current scope.
	Int kind;								// What kind of scope this is (using the PARSESCOPE_* enumeration).
	CompilerFunction function;				// The function that directly contains this scope.
} *CompileScope;

typedef struct CompiledLocalSymbolStruct {
	Symbol symbol;							// The name of this symbol.
	Int kind;								// What kind of symbol it is (using the PARSEDECL_* enumeration).
	Int index;								// The index of this symbol in its collection.
	Bool wasRead;							// Whether this variable has been read from.
	Bool wasReadDeep;						// Whether this variable has been read from in a child function's closure.
	Bool wasWritten;						// Whether this variable has been written to.
	Bool wasWrittenDeep;					// Whether this variable has been written to in a child function's closure.
	CompileScope scope;						// The scope that contains this symbol.
} *CompiledLocalSymbol;

typedef struct TillContinuationInfoStruct {
	Int tillIndex;							// The index of this till-info in the compiler's collection of till-info objects.
	UserFunctionInfo userFunctionInfo;		// The user function this till object belongs to.
	Bool realContinuationNeeded;			// Whether we need a true escape continuation, or whether this is leftover data.
	Int32 numSymbols;							// The number of symbols (flags) defined by this till continuation.
	struct CompiledTillSymbolStruct **symbols;	// The symbols (flags) defined by this till continuation.
	Int32 *branchTargetAddresses;			// The branch target addresses of each symbol (flag), after address resolution.
	IntermediateInstruction *branchTargetInstructions;	// The branch targets of each symbol (flag), before address resolution.
} *TillContinuationInfo;

typedef struct TillContinuationStruct {
	TillContinuationInfo tillContinuationInfo;	// The full data about this till-continuation.
	Closure closure;						// The closure to escape back to (NULL if this TillContinuation is no longer viable).
} *TillContinuation;

typedef struct CompiledTillSymbolStruct {
	struct CompiledLocalSymbolStruct base;	// The base struct this inherits from.
		
	Int tillIndex;							// The index of this symbol in the till's flag collection.
	IntermediateInstruction whenLabel;		// The address of the 'when' clause's label-instruction (0 = no when clause).
} *CompiledTillSymbol;

typedef struct CompiledSourceLocationStruct {
	String filename;						// The source filename this code came from.
	Int32 line;								// The source line this code started on.
	Int32 column;							// The source column (character index) this code started on.
	Symbol assignedName;					// The name of the symbol(s) being constructed, if any.
} *CompiledSourceLocation;

/// <summary>
/// This represents all of the different tables of data collected during a compile.
/// </summary>
typedef struct CompiledTablesStruct {
	UserFunctionInfo globalFunctionInfo;	// The global (outermost) function.
	ClosureInfo globalClosureInfo;			// The global ClosureInfo object that contains all of the external variables.
		
	String *strings;						// The constant strings collected during the compile (unordered).
	Int numStrings;							// The number of strings in the array.
	Int maxStrings;							// The maximum number of strings in the array.
	StringIntDict stringLookup;				// A lookup table for de-duping constant strings.
		
	UserFunctionInfo *userFunctions;		// The function definitions collected during the compile (unordered).
	Int numUserFunctions;					// The number of functions in the array.
	Int maxUserFunctions;					// The maximum number of functions in the array.
		
	SmileObject *objects;					// The constant objects collected during the compile.
	Int numObjects;							// The number of constant objects collected.
	Int maxObjects;							// The maximum number of constant objects in the array.
		
	TillContinuationInfo *tillInfos;		// The till-continuation-info objects collected during the compile.
	Int numTillInfos;						// The number of till-continuation-info objects collected.
	Int maxTillInfos;						// The maximum number of till-continuation-info objects in the array.
		
	struct CompiledSourceLocationStruct *sourcelocations;	// Source code locations.
	Int numSourceLocations;					// The number of source-code locations in the struct.
	Int maxSourceLocations;					// The maximum number of source-code locations in the struct.
} *CompiledTables;

/// <summary>
/// This represents the data collected during a compile process, and current state of the compile.
/// </summary>
typedef struct CompilerStruct {
	CompiledTables compiledTables;			// The tables collected during the parse.
	CompilerFunction currentFunction;		// The current function being compiled (all code exists in a function).
	CompileScope currentScope;				// The current compile scope (for resolving symbols).
	SmileList firstMessage, lastMessage;	// Any errors/warnings encountered while compiling.
} *Compiler;

//-------------------------------------------------------------------------------------------------
//  External API.

SMILE_API_FUNC CompiledTables CompiledTables_Create(void);

SMILE_API_FUNC Compiler Compiler_Create(void);
SMILE_API_FUNC CompilerFunction Compiler_BeginFunction(Compiler compiler, SmileList args, SmileObject body);
SMILE_API_FUNC void Compiler_EndFunction(Compiler compiler);
SMILE_API_FUNC CompiledBlock Compiler_CompileExpr(Compiler compiler, SmileObject expr, CompileFlags compileFlags);
SMILE_API_FUNC UserFunctionInfo Compiler_CompileGlobal(Compiler compiler, SmileObject expr);
SMILE_API_FUNC Int Compiler_AddString(Compiler compiler, String string);
SMILE_API_FUNC Int Compiler_AddObject(Compiler compiler, SmileObject obj);
SMILE_API_FUNC Int Compiler_AddUserFunctionInfo(Compiler compiler, UserFunctionInfo userFunctionInfo);
SMILE_API_FUNC TillContinuationInfo Compiler_AddTillContinuationInfo(Compiler compiler, UserFunctionInfo userFunctionInfo, Int numOffsets);
SMILE_API_FUNC Int Compiler_AddNewSourceLocation(Compiler compiler, String filename, Int line, Int column, Symbol assignedName);
SMILE_API_FUNC CompileScope Compiler_BeginScope(Compiler compiler, Int kind);
SMILE_API_FUNC CompiledLocalSymbol CompileScope_DefineSymbol(CompileScope scope, Symbol symbol, Int kind, Int index);
SMILE_API_FUNC CompiledLocalSymbol CompileScope_FindSymbol(CompileScope compileScope, Symbol symbol);
SMILE_API_FUNC CompiledLocalSymbol CompileScope_FindSymbolHere(CompileScope compileScope, Symbol symbol);
SMILE_API_FUNC ClosureInfo Compiler_SetupClosureInfoForCompilerFunction(Compiler compiler, CompilerFunction compilerFunction);
SMILE_API_FUNC void Compiler_ResolveTillBranchTargets(TillContinuationInfo *tillInfos, Int numTillInfos);

//-------------------------------------------------------------------------------------------------
//  Inline functions.

Inline void Compiler_SetGlobalClosureInfo(Compiler compiler, ClosureInfo globalClosureInfo)
{
	compiler->compiledTables->globalClosureInfo = globalClosureInfo;
}

Inline void Compiler_EndScope(Compiler compiler)
{
	compiler->currentScope = compiler->currentScope->parent;
}

Inline void Compiler_AddMessage(Compiler compiler, ParseMessage message)
{
	LIST_APPEND(compiler->firstMessage, compiler->lastMessage, message);
}

#endif