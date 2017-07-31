#ifndef __SMILE_EVAL_EVAL_H__
#define __SMILE_EVAL_EVAL_H__

#ifndef __SMILE_ENV_ENV_H__
#include <smile/env/env.h>
#endif

#ifndef __SMILE_EVAL_CLOSURE_H__
#include <smile/eval/closure.h>
#endif

#ifndef __SMILE_EVAL_COMPILER_H__
#include <smile/eval/compiler.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#include <setjmp.h>

#define ESCAPE_KIND_RETURN	0
#define ESCAPE_KIND_EXCEPTION	1

#define EVAL_RESULT_VALUE	0
#define EVAL_RESULT_EXCEPTION	1
#define EVAL_RESULT_BREAK	2
#define EVAL_RESULT_PARSEERRORS	3

typedef struct EscapeContinuationStruct {
	Int escapeKind;	// What kind of escape continuation this is, from the ESCAPE_KIND_* enumeration.
	jmp_buf jump;	// The target stack frame to jump to.
	Bool isValid;	// Whether the jmp_buf can safely be invoked.
	SmileObject result;	// The resulting value from invoking the escape (a return value, or an exception object).
} *EscapeContinuation;

struct EvalResultStruct {
	Int evalResultKind;	// How eval() exited, from the EVAL_RESULT_* enumeration.
		
	SmileObject value;	// The value resulting from the evaluation.
	SmileObject exception;	// The exception thrown.
		
	ParseMessage *parseMessages;	// For "wrapper" functions, this holds an array of any parse errors/warnings that were generated (NULL if none).
	Int numMessages;	// For "wrapper" functions, this is the number of parse errors/warnings that were generated.
};

//-------------------------------------------------------------------------------------------------
//  The core Smile eval() function

SMILE_API_FUNC EvalResult EvalResult_Create(Int kind);

SMILE_API_FUNC EvalResult Eval_Run(CompiledTables tables, UserFunctionInfo function);
SMILE_API_FUNC EvalResult Eval_Continue(void);
SMILE_API_FUNC ClosureStateMachine Eval_BeginStateMachine(StateMachine stateMachineStart, StateMachine stateMachineBody);

SMILE_API_FUNC void Eval_GetCurrentBreakpointInfo(Closure *closure, CompiledTables *compiledTables, ByteCodeSegment *segment, ByteCode *byteCode);

Inline EscapeContinuation EscapeContinuation_Create(Int escapeKind)
{
	EscapeContinuation escapeContinuation = GC_MALLOC_STRUCT(struct EscapeContinuationStruct);
	escapeContinuation->escapeKind = escapeKind;
	escapeContinuation->result = NullObject;
	escapeContinuation->isValid = False;
	return escapeContinuation;
}

#endif

