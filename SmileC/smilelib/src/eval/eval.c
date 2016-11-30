//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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

#include <smile/eval/eval.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>

static EscapeContinuation _returnContinuation;
static EscapeContinuation _exceptionContinuation;
static Closure _closure;
static CompiledTables _compiledTables;
static CompiledFunction _function;
static ByteCodeSegment _segment;
static Int _pc;

void Eval_Run(Int pc);

Inline EvalResult EvalResult_Create(Int kind)
{
	EvalResult result = GC_MALLOC_STRUCT(struct EvalResultStruct);
	result->evalResultKind = kind;
	return result;
}

EvalResult Eval_RunOuter(CompiledTables tables, CompiledFunction function, ByteCodeSegment segment, Int pc, Closure globalClosure)
{
	EvalResult evalResult;
	SmileObject value;

	_compiledTables = tables;
	_function = function;
	_segment = segment;
	_pc = pc;
	_closure = globalClosure;

	_exceptionContinuation = EscapeContinuation_Create(ESCAPE_KIND_EXCEPTION);
	_returnContinuation = EscapeContinuation_Create(ESCAPE_KIND_RETURN);

	// Set up the exception continuation using setjmp/longjmp.
	if (!setjmp(_exceptionContinuation->jump)) {

		// Set up the return continuation using setjmp/longjmp.
		if (!setjmp(_returnContinuation->jump)) {
		
			// Evaluate the expression for real.
			Eval_Run(_pc);
			value = NULL;	// TODO: FIXME: Pop the result from the stack.
		
			// Expression evaluated normally.
			evalResult = EvalResult_Create(EVAL_RESULT_VALUE);
			evalResult->value = value;
			return evalResult;
		}
		else {
			// Expression invoked explicit 'return'.
			evalResult = EvalResult_Create(EVAL_RESULT_VALUE);
			evalResult->value = _returnContinuation->result;
			return evalResult;
		}
	}
	else {
		// Expression threw an uncaught exception.
		evalResult = EvalResult_Create(EVAL_RESULT_EXCEPTION);
		evalResult->exception = _exceptionContinuation->result;
		return evalResult;
	}
}

void Eval_Run(Int pc)
{
	UNUSED(pc);
}

void Smile_Throw(SmileObject thrownObject)
{
	_exceptionContinuation->result = thrownObject;
	longjmp(_exceptionContinuation->jump, 1);
}
