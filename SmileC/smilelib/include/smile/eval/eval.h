#ifndef __SMILE_EVAL_EVAL_H__
#define __SMILE_EVAL_EVAL_H__

#ifndef __SMILE_ENV_ENV_H__
#include <smile/env/env.h>
#endif

#ifndef __SMILE_ENV_CLOSURE_H__
#include <smile/env/closure.h>
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

typedef struct EscapeContinuationStruct {
	Int escapeKind;	// What kind of escape continuation this is, from the ESCAPE_KIND_* enumeration.
	jmp_buf jump;	// The target stack frame to jump to.
	SmileObject result;	// The resulting value from invoking the escape (a return value, or an exception object).
} *EscapeContinuation;

typedef struct EvalResultStruct {
	Int evalResultKind;	// How eval() exited, from the EVAL_RESULT_* enumeration.
	SmileObject value;	// The value resulting from the evaluation.
	SmileObject exception;	// The exception thrown.
} *EvalResult;

//-------------------------------------------------------------------------------------------------
//  The core Smile eval() function

SMILE_API_FUNC EvalResult Smile_Eval(SmileObject expr, Closure closure);

SMILE_API_FUNC EvalResult Eval_RunOuter(CompiledTables tables, CompiledFunction function,
	ByteCodeSegment segment, Int pc, Closure globalClosure);
SMILE_API_FUNC void Eval_Run(Int pc);

SMILE_API_FUNC void Smile_Throw(SmileObject expr);

Inline EscapeContinuation EscapeContinuation_Create(Int escapeKind)
{
	EscapeContinuation escapeContinuation = GC_MALLOC_STRUCT(struct EscapeContinuationStruct);
	escapeContinuation->escapeKind = escapeKind;
	escapeContinuation->result = NullObject;
	return escapeContinuation;
}

#endif

