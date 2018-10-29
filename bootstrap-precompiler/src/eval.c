
#include "stdafx.h"
#include "eval.h"

static Bool DeclareVariableForCompiler(VarInfo varInfo, void *param)
{
	CompileScope_DefineSymbol((CompileScope)param, varInfo->symbol, PARSEDECL_GLOBAL, 0);
	return True;
}

EvalResult EvalInScope(Compiler compiler, ClosureInfo globalClosureInfo, SmileObject expression)
{
	CompileScope compileScope;
	UserFunctionInfo globalFunction;
	EvalResult result;

	// Set up the compiler...
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

SmileObject EvalExpr(Compiler compiler, SmileObject expr, ClosureInfo closureInfo, String filename)
{
	EvalResult evalResult;
	Bool hasErrors = False;
	SmileObject result;

	evalResult = EvalInScope(compiler, closureInfo, expr);

	switch (evalResult->evalResultKind) {

		case EVAL_RESULT_EXCEPTION:
			{
				String exceptionMessage = (String)SMILE_VCALL1(evalResult->exception, getProperty, Smile_KnownSymbols.message);
				SmileSymbol exceptionKindWrapped = (SmileSymbol)SMILE_VCALL1(evalResult->exception, getProperty, Smile_KnownSymbols.kind);
				String displayMessage, stackTraceMessage;
				Symbol exceptionKind;
				SmileObject stackTrace;

				if (SMILE_KIND(exceptionMessage) != SMILE_KIND_STRING)
					exceptionMessage = String_Empty;
				if (SMILE_KIND(exceptionKindWrapped) != SMILE_KIND_SYMBOL)
					exceptionKind = SymbolTable_GetSymbolC(Smile_SymbolTable, "unknown-error");
				else exceptionKind = exceptionKindWrapped->symbol;

				displayMessage = String_Format("!Exception: %S (%S)\n",
					exceptionMessage,
					SymbolTable_GetName(Smile_SymbolTable, exceptionKind));
				fwrite((const char *)String_GetBytes(displayMessage), 1, String_Length(displayMessage), stderr);
				fflush(stderr);

				stackTrace = SMILE_VCALL1(evalResult->exception, getProperty, Smile_KnownSymbols.stack_trace);
				stackTraceMessage = Smile_FormatStackTrace((SmileList)stackTrace);
				fwrite((const char *)String_GetBytes(stackTraceMessage), 1, String_Length(stackTraceMessage), stderr);
				fflush(stderr);

				result = NullObject;
				hasErrors = True;
			}
			break;

		case EVAL_RESULT_BREAK:
			{
				Closure closure;
				CompiledTables compiledTables;
				ByteCodeSegment segment;
				String message;
				ByteCode byteCode;

				Eval_GetCurrentBreakpointInfo(&closure, &compiledTables, &segment, &byteCode);

				if (byteCode->sourceLocation > 0 && byteCode->sourceLocation < compiledTables->numSourceLocations) {
					CompiledSourceLocation sourceLocation = &compiledTables->sourcelocations[byteCode->sourceLocation];
					message = String_Format("%S: Stopped at breakpoint in \"%S\", line %d.\r\n",
						filename, sourceLocation->filename, sourceLocation->line);
				}
				else {
					message = String_Format("%S: Stopped at breakpoint.\r\n", filename);
				}
				fwrite(String_GetBytes(message), 1, String_Length(message), stderr);
				fflush(stderr);
				result = NullObject;
				hasErrors = True;
			}
			break;

		default:
			result = evalResult->value;
			hasErrors = False;
			break;
	}

	return result;
}
