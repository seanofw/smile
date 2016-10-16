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

Inline EvalResult EvalResult_Create(Int kind)
{
	EvalResult result = GC_MALLOC_STRUCT(struct EvalResultStruct);
	result->evalResultKind = kind;
	return result;
}

SMILE_API_FUNC EvalResult Smile_Eval(SmileObject expr, Closure closure)
{
	EvalResult evalResult;
	SmileObject value;

	_closure = closure;
	_exceptionContinuation = EscapeContinuation_Create(ESCAPE_KIND_EXCEPTION);
	_returnContinuation = EscapeContinuation_Create(ESCAPE_KIND_RETURN);

	// Set up the exception continuation using setjmp/longjmp.
	if (!setjmp(_exceptionContinuation->jump)) {

		// Set up the return continuation using setjmp/longjmp.
		if (!setjmp(_returnContinuation->jump)) {
		
			// Evaluate the expression for real.
			value = Smile_EvalInternal(expr);
		
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

Inline SmileObject EvalEquals(SmileObject target, SmileObject value)
{
	SmilePair pair;

	switch (SMILE_KIND(target)) {

		case SMILE_KIND_SYMBOL:
			Closure_Set(_closure, ((SmileSymbol)target)->symbol, value);
			if (!value->assignedSymbol) {
				value->assignedSymbol = ((SmileSymbol)target)->symbol;
			}
			return value;
		
		case SMILE_KIND_PAIR:
			pair = (SmilePair)target;
			target = Smile_EvalInternal(pair->left);
			if (SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL)
				Smile_Throw((SmileObject)SmileString_Create(String_Format("Cannot evaluate pair; there should be a symbol on the right side: %S", SmileObject_Stringify(pair->right))));
			SMILE_VCALL2(target, setProperty, ((SmileSymbol)pair->right)->symbol, value);
			return value;
		
		default:
			Smile_Abort_FatalError("Not yet supported:  Complex equals left-hand side.");
	}

	return NullObject;
}

Inline SmileObject EvalOpEquals(SmileList list)
{
	UNUSED(list);
	return NullObject;
}

Inline SmileObject EvalIf(SmileList list)
{
	UNUSED(list);
	return NullObject;
}

Inline SmileObject EvalWhile(SmileList list)
{
	UNUSED(list);
	return NullObject;
}

SmileObject Smile_EvalInternal(SmileObject expr)
{
	SmilePair pair;
	SmileList list;
	SmileObject target, result;

	switch (SMILE_KIND(expr)) {
	
		// Primitive constants evaluate to themselves.
		case SMILE_KIND_NULL:
		case SMILE_KIND_BOOL:
		case SMILE_KIND_CHAR:
		case SMILE_KIND_UCHAR:
		case SMILE_KIND_STRING:
		case SMILE_KIND_OBJECT:

		// Integer constants evaluate to themselves.
		case SMILE_KIND_BYTE:
		case SMILE_KIND_INTEGER16:
		case SMILE_KIND_INTEGER32:
		case SMILE_KIND_INTEGER64:
		case SMILE_KIND_INTEGER128:
		case SMILE_KIND_BIGINT:
		
		// Real constants evaluate to themselves.
		case SMILE_KIND_REAL16:
		case SMILE_KIND_REAL32:
		case SMILE_KIND_REAL64:
		case SMILE_KIND_REAL128:
		case SMILE_KIND_BIGREAL:
		
		// Float constants evaluate to themselves.
		case SMILE_KIND_FLOAT16:
		case SMILE_KIND_FLOAT32:
		case SMILE_KIND_FLOAT64:
		case SMILE_KIND_FLOAT128:
		case SMILE_KIND_BIGFLOAT:
		
		// User data evaluates to iteself.
		case SMILE_KIND_FUNCTION:
		case SMILE_KIND_HANDLE:
		case SMILE_KIND_NONTERMINAL:
		case SMILE_KIND_SYNTAX:
		case SMILE_KIND_USEROBJECT:
		case SMILE_KIND_CLOSURE:
		case SMILE_KIND_FACADE:
		case SMILE_KIND_MACRO:
		
		// Intermediate forms (if they somehow make it this far) also evaluate to themselves.
		case SMILE_KIND_PARSEDECL:
		case SMILE_KIND_PARSEMESSAGE:
			return expr;
		
		// Simple pairs resolve by performing a property lookup on the object in question, which may walk up the base chain for that object.
		case SMILE_KIND_PAIR:
			pair = (SmilePair)expr;
			target = Smile_EvalInternal(pair->left);
			if (SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL) {
				Smile_Throw((SmileObject)SmileString_Create(String_Format("Cannot evaluate pair; there should be a symbol on the right side: %S", SmileObject_Stringify(pair->right))));
			}
			result = SMILE_VCALL1(target, getProperty, ((SmileSymbol)pair->right)->symbol);
			return result;
		
		// Symbols (variables) resolve to whatever the current closure (or any base closure) says they are.
		// The only notable exception to this rule is 'till' objects, which immediately throw a symbol
		// exception when they are accessed.
		case SMILE_KIND_SYMBOL:
			result = Closure_Get(_closure, ((SmileSymbol)expr)->symbol);
			return result;
		
		// In general, lists resolve by evaluating their arguments and then passing the
		// results to the function described by the first argument.  There are a few special
		// forms which evaluate specially.
		case SMILE_KIND_LIST:
		
			// Resolve the first list element.  This will tell us what kind of function we're applying, a builtin
			// special form or a normal function-slash-method-call.
			list = (SmileList)expr;
			switch (SMILE_KIND(list->a)) {
			
				case SMILE_KIND_SYMBOL:
					switch (((SmileSymbol)list->a)->symbol) {
						case SMILE_SPECIAL_SYMBOL_EQUALS:
							return EvalEquals(LIST_SECOND(list), Smile_EvalInternal(LIST_THIRD(list)));
						case SMILE_SPECIAL_SYMBOL_OP_EQUALS:
							return EvalOpEquals(LIST_REST(list));
						case SMILE_SPECIAL_SYMBOL_IF:
							return EvalIf(LIST_REST(list));
						case SMILE_SPECIAL_SYMBOL_WHILE:
							return EvalWhile(LIST_REST(list));
						default:
							Smile_Throw((SmileObject)SmileString_Create(String_Format("Cannot call unknown function: %S", SmileObject_Stringify(list->a))));
					}
				
				default:
					Smile_Throw((SmileObject)SmileString_Create(String_Format("Cannot call object that is not a function: %S", SmileObject_Stringify(list->a))));
			}

		default:
			Smile_Throw((SmileObject)SmileString_Create(String_Format("Cannot evaluate object of kind %d: This is not an evaluable form.", SMILE_KIND(expr))));
	}

	return NullObject;
}

void Smile_Throw(SmileObject thrownObject)
{
	_exceptionContinuation->result = thrownObject;
	longjmp(_exceptionContinuation->jump, 1);
}
