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

#include <smile/eval/eval.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smileinteger128.h>

static EscapeContinuation _returnContinuation;
static EscapeContinuation _exceptionContinuation;
static Closure _closure;
static CompiledTables _compiledTables;
static CompiledFunction _function;
static ByteCodeSegment _segment;
static Int _pc;

EvalResult EvalResult_Create(Int kind)
{
	EvalResult result = GC_MALLOC_STRUCT(struct EvalResultStruct);
	result->evalResultKind = kind;
	result->value = NullObject;
	result->exception = NullObject;
	result->parseMessages = NULL;
	result->numMessages = 0;
	return result;
}

EvalResult Eval_Run(CompiledTables tables, CompiledFunction function)
{
	_compiledTables = tables;
	_function = function;
	_segment = function->byteCodeSegment;
	_closure = Closure_CreateGlobal(tables->globalClosureInfo, NULL);
	_pc = 0;

	_closure = Closure_CreateLocal(function->closureInfo, _closure);

	_exceptionContinuation = EscapeContinuation_Create(ESCAPE_KIND_EXCEPTION);
	_returnContinuation = EscapeContinuation_Create(ESCAPE_KIND_RETURN);

	return Eval_Continue();
}

EvalResult Eval_Continue(void)
{
	EvalResult evalResult;

	// Set up the exception continuation using setjmp/longjmp.
	if (!setjmp(_exceptionContinuation->jump)) {
		_exceptionContinuation->isValid = True;

		// Set up the return continuation using setjmp/longjmp.
		if (!setjmp(_returnContinuation->jump)) {
			_returnContinuation->isValid = True;
		
			// Evaluate the expression for real.
			if (Eval_RunCore()) {

				// Expression evaluated normally.
				evalResult = EvalResult_Create(EVAL_RESULT_VALUE);
				evalResult->value = Closure_PopTemp(_closure);

				_returnContinuation->isValid = False;
				_exceptionContinuation->isValid = False;
				return evalResult;
			}
			else {
				// Hit a breakpoint.
				evalResult = EvalResult_Create(EVAL_RESULT_BREAK);

				_returnContinuation->isValid = False;
				_exceptionContinuation->isValid = False;
				return evalResult;
			}
		}
		else {
			// Expression invoked explicit 'return'.
			evalResult = EvalResult_Create(EVAL_RESULT_VALUE);
			evalResult->value = _returnContinuation->result;

			_returnContinuation->isValid = False;
			_exceptionContinuation->isValid = False;
			return evalResult;
		}
	}
	else {
		// Expression threw an uncaught exception.
		evalResult = EvalResult_Create(EVAL_RESULT_EXCEPTION);
		evalResult->exception = _exceptionContinuation->result;

		_returnContinuation->isValid = False;
		_exceptionContinuation->isValid = False;
		return evalResult;
	}
}

Bool Eval_RunCore(void)
{
	register ByteCode byteCode = _segment->byteCodes + _pc;
	SmileObject target, value;

next:
	switch (byteCode->opcode) {
	
		//-------------------------------------------------------
		// 00-0F: Miscellaneous stack- and state-management

		case Op_Nop:
			byteCode++;
			goto next;
		
		case Op_Dup1:
			_closure->stackTop[0] = _closure->stackTop[-1];
			_closure->stackTop++;
			byteCode++;
			goto next;

		case Op_Dup2:
			_closure->stackTop[0] = _closure->stackTop[-2];
			_closure->stackTop++;
			byteCode++;
			goto next;

		case Op_Dup:
			_closure->stackTop[0] = _closure->stackTop[-byteCode->u.index];
			_closure->stackTop++;
			byteCode++;
			goto next;
		
		case Op_Pop1:
			Closure_PopCount(_closure, 1);
			byteCode++;
			goto next;

		case Op_Pop2:
			Closure_PopCount(_closure, 2);
			byteCode++;
			goto next;
		
		case Op_Pop:
			Closure_PopCount(_closure, byteCode->u.index);
			byteCode++;
			goto next;

		case Op_Rep1:
			_closure->stackTop[-2] = _closure->stackTop[-1];
			_closure->stackTop--;
			byteCode++;
			goto next;

		case Op_Rep2:
			_closure->stackTop[-3] = _closure->stackTop[-1];
			_closure->stackTop -= 2;
			byteCode++;
			goto next;

		case Op_Rep:
			_closure->stackTop[-(byteCode->u.index + 1)] = _closure->stackTop[-1];
			_closure->stackTop -= byteCode->u.index;
			byteCode++;
			goto next;

		case Op_Brk:
			_pc = byteCode - _segment->byteCodes;
			return False;
		
		//-------------------------------------------------------
		// 10-17: Special load instructions
		
		case Op_LdNull:
			Closure_PushTemp(_closure, NullObject);
			byteCode++;
			goto next;

		case Op_LdBool:
			Closure_PushTemp(_closure, Smile_KnownObjects.BooleanObjs[byteCode->u.boolean]);
			byteCode++;
			goto next;

		case Op_LdStr:
			Closure_PushTemp(_closure, SmileString_Create(_compiledTables->strings[byteCode->u.index]));
			byteCode++;
			goto next;

		case Op_LdSym:
			Closure_PushTemp(_closure, SmileSymbol_Create(byteCode->u.symbol));
			byteCode++;
			goto next;

		case Op_LdObj:
			Closure_PushTemp(_closure, _compiledTables->objects[byteCode->u.index]);
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 18-1F: Integer load instructions
		
		case Op_Ld8:
			Closure_PushTemp(_closure, SmileByte_Create(byteCode->u.byte));
			byteCode++;
			goto next;

		case Op_Ld16:
			Closure_PushTemp(_closure, SmileInteger16_Create(byteCode->u.int16));
			byteCode++;
			goto next;

		case Op_Ld32:
			Closure_PushTemp(_closure, SmileInteger32_Create(byteCode->u.int32));
			byteCode++;
			goto next;

		case Op_Ld64:
			Closure_PushTemp(_closure, SmileInteger64_Create(byteCode->u.int64));
			byteCode++;
			goto next;

		case Op_Ld128:
			Closure_PushTemp(_closure, _compiledTables->objects[byteCode->u.index]);
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 20-27: Real load instructions

		//-------------------------------------------------------
		// 28-2F: Float load instructions

		//-------------------------------------------------------
		// 30-37: General-purpose local-variable/argument instructions
		
		case Op_LdLoc:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope(_closure, byteCode->u.i2.a, byteCode->u.i2.b));
			byteCode++;
			goto next;

		case Op_StLoc:
			Closure_SetLocalVariableInScope(_closure, byteCode->u.i2.a, byteCode->u.i2.b, Closure_GetTop(_closure));
			byteCode++;
			goto next;

		case Op_StpLoc:
			Closure_SetLocalVariableInScope(_closure, byteCode->u.i2.a, byteCode->u.i2.b, Closure_PopTemp(_closure));
			byteCode++;
			goto next;

		case Op_LdArg:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope(_closure, byteCode->u.i2.a, byteCode->u.i2.b));
			byteCode++;
			goto next;

		case Op_StArg:
			Closure_SetArgumentInScope(_closure, byteCode->u.i2.a, byteCode->u.i2.b, Closure_GetTop(_closure));
			byteCode++;
			goto next;

		case Op_StpArg:
			Closure_SetArgumentInScope(_closure, byteCode->u.i2.a, byteCode->u.i2.b, Closure_PopTemp(_closure));
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// 38-3F: Global variable instructions

		case Op_LdX:
			Closure_PushTemp(_closure, Closure_GetGlobalVariable(_closure, byteCode->u.symbol));
			byteCode++;
			goto next;

		case Op_StX:
			Closure_SetGlobalVariable(_closure, byteCode->u.symbol, Closure_GetTop(_closure));
			byteCode++;
			goto next;

		case Op_StpX:
			Closure_SetGlobalVariable(_closure, byteCode->u.symbol, Closure_PopTemp(_closure));
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// 40-4F: Optimized local-variable/argument load instructions
		
		case Op_LdArg0:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope0(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg1:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope1(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg2:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope2(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg3:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope3(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg4:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope4(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg5:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope5(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg6:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope6(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg7:
			Closure_PushTemp(_closure, Closure_GetArgumentInScope7(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		
		case Op_LdLoc0:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope0(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc1:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope1(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc2:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope2(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc3:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope3(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc4:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope4(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc5:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope5(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc6:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope6(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc7:
			Closure_PushTemp(_closure, Closure_GetLocalVariableInScope7(_closure, byteCode->u.index));
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 50-5F: Optimized local-variable/argument store instructions

		case Op_StArg0:
			Closure_SetArgumentInScope0(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StArg1:
			Closure_SetArgumentInScope1(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StArg2:
			Closure_SetArgumentInScope2(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StArg3:
			Closure_SetArgumentInScope3(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StArg4:
			Closure_SetArgumentInScope4(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StArg5:
			Closure_SetArgumentInScope5(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StArg6:
			Closure_SetArgumentInScope6(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StArg7:
			Closure_SetArgumentInScope7(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;

		case Op_StLoc0:
			Closure_SetLocalVariableInScope0(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StLoc1:
			Closure_SetLocalVariableInScope1(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StLoc2:
			Closure_SetLocalVariableInScope2(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StLoc3:
			Closure_SetLocalVariableInScope3(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StLoc4:
			Closure_SetLocalVariableInScope4(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StLoc5:
			Closure_SetLocalVariableInScope5(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StLoc6:
			Closure_SetLocalVariableInScope6(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		case Op_StLoc7:
			Closure_SetLocalVariableInScope7(_closure, byteCode->u.index, Closure_GetTop(_closure));
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 60-6F: Optimized local-variable/argument store-and-pop instructions

		case Op_StpArg0:
			Closure_SetArgumentInScope0(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpArg1:
			Closure_SetArgumentInScope1(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpArg2:
			Closure_SetArgumentInScope2(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpArg3:
			Closure_SetArgumentInScope3(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpArg4:
			Closure_SetArgumentInScope4(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpArg5:
			Closure_SetArgumentInScope5(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpArg6:
			Closure_SetArgumentInScope6(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpArg7:
			Closure_SetArgumentInScope7(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;

		case Op_StpLoc0:
			Closure_SetLocalVariableInScope0(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpLoc1:
			Closure_SetLocalVariableInScope1(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpLoc2:
			Closure_SetLocalVariableInScope2(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpLoc3:
			Closure_SetLocalVariableInScope3(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpLoc4:
			Closure_SetLocalVariableInScope4(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpLoc5:
			Closure_SetLocalVariableInScope5(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpLoc6:
			Closure_SetLocalVariableInScope6(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		case Op_StpLoc7:
			Closure_SetLocalVariableInScope7(_closure, byteCode->u.index, Closure_PopTemp(_closure));
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 70-7F: General-purporse property and member access

		case Op_LdProp:
			target = Closure_GetTop(_closure);
			value = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;
		case Op_StProp:
			target = Closure_GetTemp(_closure, 1);
			value = Closure_GetTemp(_closure, 0);
			SMILE_VCALL2(target, setProperty, byteCode->u.symbol, value);
			Closure_PopCount(_closure, 1);
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;
		case Op_StpProp:
			target = Closure_GetTemp(_closure, 1);
			value = Closure_GetTemp(_closure, 0);
			SMILE_VCALL2(target, setProperty, byteCode->u.symbol, value);
			Closure_PopCount(_closure, 2);
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// 80-8F: Specialty type management

		case Op_SuperEq:
			target = Closure_GetTemp(_closure, 0);
			value = Closure_GetTemp(_closure, 1);
			Closure_PopCount(_closure, 1);
			value = (SmileObject)SmileBool_FromBool(SMILE_VCALL1(target, compareEqual, value));
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_SuperNe:
			target = Closure_GetTemp(_closure, 0);
			value = Closure_GetTemp(_closure, 1);
			Closure_PopCount(_closure, 1);
			value = (SmileObject)SmileBool_FromBool(1 - SMILE_VCALL1(target, compareEqual, value));
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;
		
		case Op_Bool:
			value = Closure_GetTop(_closure);
			if (SMILE_KIND(value) != SMILE_KIND_BOOL) {
				value = (SmileObject)SmileBool_FromBool(SMILE_VCALL(value, toBool));
				Closure_SetTop(_closure, value);
			}
			byteCode++;
			goto next;

		case Op_Not:
			value = Closure_GetTop(_closure);
			if (SMILE_KIND(value) != SMILE_KIND_BOOL) {
				Closure_SetTop(_closure, SmileBool_FromBool(1 - SMILE_VCALL(value, toBool)));
			}
			else {
				Closure_SetTop(_closure, SmileBool_FromBool(1 - ((SmileBool)value)->value));
			}
			byteCode++;
			goto next;

		case Op_Is:
			target = Closure_GetTemp(_closure, 0);
			value = Closure_GetTemp(_closure, 1);
			Closure_PopCount(_closure, 1);
			Closure_SetTop(_closure, SmileBool_FromBool(SmileObject_Is(target, value)));
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// 90-9F: Special-purpose function and method calls

		case Op_Met0:
			target = Closure_GetTemp(_closure, 0);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 1);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Met1:
			target = Closure_GetTemp(_closure, 1);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 2);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Met2:
			target = Closure_GetTemp(_closure, 2);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 3);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Met3:
			target = Closure_GetTemp(_closure, 3);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 4);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Met4:
			target = Closure_GetTemp(_closure, 4);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 5);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Met5:
			target = Closure_GetTemp(_closure, 5);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 6);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Met6:
			target = Closure_GetTemp(_closure, 6);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 7);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Met7:
			target = Closure_GetTemp(_closure, 7);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 8);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Call0:
			target = Closure_GetTemp(_closure, 0);
			SMILE_VCALL1(target, call, 0);
			byteCode++;
			goto next;

		case Op_Call1:
			target = Closure_GetTemp(_closure, 1);
			SMILE_VCALL1(target, call, 1);
			byteCode++;
			goto next;

		case Op_Call2:
			target = Closure_GetTemp(_closure, 2);
			SMILE_VCALL1(target, call, 2);
			byteCode++;
			goto next;

		case Op_Call3:
			target = Closure_GetTemp(_closure, 3);
			SMILE_VCALL1(target, call, 3);
			byteCode++;
			goto next;

		case Op_Call4:
			target = Closure_GetTemp(_closure, 4);
			SMILE_VCALL1(target, call, 4);
			byteCode++;
			goto next;

		case Op_Call5:
			target = Closure_GetTemp(_closure, 5);
			SMILE_VCALL1(target, call, 5);
			byteCode++;
			goto next;

		case Op_Call6:
			target = Closure_GetTemp(_closure, 6);
			SMILE_VCALL1(target, call, 6);
			byteCode++;
			goto next;

		case Op_Call7:
			target = Closure_GetTemp(_closure, 7);
			SMILE_VCALL1(target, call, 7);
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// B0-BF: Flow control
		
		case Op_Jmp:
			byteCode += byteCode->u.index;
			goto next;

		case Op_Bt:
			value = Closure_PopTemp(_closure);
			if (SMILE_KIND(value) != SMILE_KIND_BOOL) {
				if (SMILE_VCALL(value, toBool)) {
					byteCode += byteCode->u.index;
				}
				else {
					byteCode++;
				}
			}
			else {
				if (((SmileBool)value)->value) {
					byteCode += byteCode->u.index;
				}
				else {
					byteCode++;
				}
			}
			goto next;

		case Op_Bf:
			value = Closure_PopTemp(_closure);
			if (SMILE_KIND(value) != SMILE_KIND_BOOL) {
				if (SMILE_VCALL(value, toBool)) {
					byteCode++;
				}
				else {
					byteCode += byteCode->u.index;
				}
			}
			else {
				if (((SmileBool)value)->value) {
					byteCode++;
				}
				else {
					byteCode += byteCode->u.index;
				}
			}
			goto next;

		case Op_Met:
			target = Closure_GetTemp(_closure, byteCode->u.i2.b + 1);	// Get the target object
			target = SMILE_VCALL1(target, getProperty, byteCode->u.i2.a);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, byteCode->u.i2.b + 1);	// Invoke it, whatever it is.
			byteCode++;
			goto next;

		case Op_Call:
			target = Closure_GetTemp(_closure, byteCode->u.index);
			SMILE_VCALL1(target, call, byteCode->u.index);
			byteCode++;
			goto next;
		
		case Op_Ret:
			return True;

		//-------------------------------------------------------
		// C0-CF: Object construction, and special property access

		case Op_Cons:
			value = (SmileObject)SmileList_Cons(Closure_GetTemp(_closure, 1), Closure_GetTemp(_closure, 0));
			Closure_PopCount(_closure, 1);
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_Car:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_LIST) {
				value = ((SmileList)target)->a;
			}
			else {
				value = NullObject;
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_Cdr:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_LIST) {
				value = ((SmileList)target)->a;
			}
			else {
				value = NullObject;
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_NewPair:
			value = (SmileObject)SmilePair_Create(Closure_GetTemp(_closure, 1), Closure_GetTemp(_closure, 0));
			Closure_PopCount(_closure, 1);
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_Left:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_PAIR) {
				value = ((SmilePair)target)->left;
			}
			else {
				value = NullObject;
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_Right:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_PAIR) {
				value = ((SmilePair)target)->right;
			}
			else {
				value = NullObject;
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// D0-DF: Special-purpose optimized property access

		case Op_LdA:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_LIST) {
				value = ((SmileList)target)->a;
			}
			else {
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.a);
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_LdD:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_LIST) {
				value = ((SmileList)target)->d;
			}
			else {
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.d);
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;
		
		case Op_LdLeft:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_PAIR) {
				value = ((SmilePair)target)->left;
			}
			else {
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.left);
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_LdRight:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_PAIR) {
				value = ((SmilePair)target)->right;
			}
			else {
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.right);
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_LdStart:
			target = Closure_GetTop(_closure);
			value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.start);
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_LdEnd:
			target = Closure_GetTop(_closure);
			value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.end);
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;

		case Op_LdCount:
			target = Closure_GetTop(_closure);
			value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.count);
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;
		
		case Op_LdLength:
			target = Closure_GetTop(_closure);
			if (SMILE_KIND(target) == SMILE_KIND_STRING) {
				value = (SmileObject)SmileInteger32_Create(((SmileString)target)->string.length);
			}
			else {
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.length);
			}
			Closure_SetTop(_closure, value);
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// F0-FF: Miscellaneous internal constructs
		
		case Op_Label:
			byteCode++;
			goto next;
			
		//-------------------------------------------------------
		
		default:
			Smile_Abort_FatalError(String_ToC(String_Format("Eval error: Unknown opcode 0x%02X", byteCode->opcode)));
	}

	return True;
}

//-------------------------------------------------------------------------------------------------

void Smile_Throw(SmileObject thrownObject)
{
	SmileObject kindObject, messageObject;
	String message;

	if (_exceptionContinuation != NULL && _exceptionContinuation->isValid) {
		_exceptionContinuation->result = thrownObject;

		if (SMILE_KIND(thrownObject) == SMILE_KIND_USEROBJECT) {
			// Do stuff to fill in the "stack-trace" property on this object, if that's possible.
		}

		longjmp(_exceptionContinuation->jump, 1);
	}
	else {
		kindObject = SmileUserObject_Get(thrownObject, Smile_KnownSymbols.kind);
		messageObject = SmileUserObject_Get(thrownObject, Smile_KnownSymbols.message);
		message = String_Format("%S: %S", SMILE_VCALL(kindObject, toString), SMILE_VCALL(messageObject, toString));
		
		Smile_Abort_FatalError(String_ToC(message));
	}
}

//-------------------------------------------------------------------------------------------------

Bool SmileUserFunction_Call(SmileFunction self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	return True;
}

//-------------------------------------------------------------------------------------------------

static Int PerformTypeChecks(Int argc, SmileObject *argv, Int numTypeChecks, const Byte *typeChecks)
{
	Int i;

	if (argc > numTypeChecks) {
		Byte mask, compare;
	
		// If there are more args than type checks, perform the type checks that are defined...
		for (i = 0; i < numTypeChecks; i++) {
			if ((argv[i]->kind & typeChecks[i * 2]) != typeChecks[i * 2 + 1])
				return i;
		}

		mask = typeChecks[i * 2 - 2];
		compare = typeChecks[i * 2 - 1];
	
		// ...and then repeatedly use the last type check for the rest of the arguments.
		for (; i < argc; i++) {
			if ((argv[i]->kind & mask) != compare)
				return i;
		}
	
		return -1;
	}

	// If there are fewer or equal args to type checks, perform the checks as-is,
	// wherever they match positions.  We use a switch statement (computed goto) to
	// optimize away the comparison loop if possible.
	switch (argc) {
	
		default:
			for (i = 0; i < argc; i++) {
				if ((argv[i]->kind & typeChecks[i * 2]) != typeChecks[i * 2 + 1])
					return i;
			}
			return -1;
		
		case 7:
			if ((argv[6]->kind & typeChecks[12]) != typeChecks[13])
				return 6;
		case 6:
			if ((argv[5]->kind & typeChecks[10]) != typeChecks[11])
				return 5;
		case 5:
			if ((argv[4]->kind & typeChecks[8]) != typeChecks[9])
				return 4;
		case 4:
			if ((argv[3]->kind & typeChecks[6]) != typeChecks[7])
				return 3;
		case 3:
			if ((argv[2]->kind & typeChecks[4]) != typeChecks[5])
				return 2;
		case 2:
			if ((argv[1]->kind & typeChecks[2]) != typeChecks[3])
				return 1;
		case 1:
			if ((argv[0]->kind & typeChecks[0]) != typeChecks[1])
				return 0;
		
		case 0:
			return -1;
	}
}

//-------------------------------------------------------------------------------------------------

Bool SmileExternalFunction_NoCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv;

	argv = _closure->stackTop -= argc;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_MinCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv;

	if (argc < self->u.externalFunctionInfo.minArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' requires at least %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.minArgs, argc));
	}

	argv = _closure->stackTop -= argc;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_MaxCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv;

	if (argc > self->u.externalFunctionInfo.maxArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' allows at most %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.maxArgs, argc));
	}

	argv = _closure->stackTop -= argc;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_MinMaxCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv;

	if (argc < self->u.externalFunctionInfo.minArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' requires at least %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.minArgs, argc));
	}
	if (argc > self->u.externalFunctionInfo.maxArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' allows at most %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.maxArgs, argc));
	}

	argv = _closure->stackTop -= argc;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_ExactCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv;

	if (argc != self->u.externalFunctionInfo.minArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' requires exactly %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.minArgs, argc));
	}

	argv = _closure->stackTop -= argc;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_TypesCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv = _closure->stackTop - argc;
	Int failingArg;

	if ((failingArg = PerformTypeChecks(argc, argv, self->u.externalFunctionInfo.numArgsToTypeCheck, self->u.externalFunctionInfo.argTypeChecks)) >= 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument %d to '%S' is of the wrong type.",
			failingArg+1, self->u.externalFunctionInfo.name));
	}
	
	_closure->stackTop = argv;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_MinTypesCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv = _closure->stackTop - argc;
	Int failingArg;

	if (argc < self->u.externalFunctionInfo.minArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' requires at least %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.minArgs, argc));
	}
	if ((failingArg = PerformTypeChecks(argc, argv, self->u.externalFunctionInfo.numArgsToTypeCheck, self->u.externalFunctionInfo.argTypeChecks)) >= 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument %d to '%S' is of the wrong type.",
			failingArg + 1, self->u.externalFunctionInfo.name));
	}

	_closure->stackTop = argv;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_MaxTypesCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv = _closure->stackTop - argc;
	Int failingArg;

	if (argc > self->u.externalFunctionInfo.maxArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' allows at most %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.maxArgs, argc));
	}
	if ((failingArg = PerformTypeChecks(argc, argv, self->u.externalFunctionInfo.numArgsToTypeCheck, self->u.externalFunctionInfo.argTypeChecks)) >= 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument %d to '%S' is of the wrong type.",
			failingArg + 1, self->u.externalFunctionInfo.name));
	}

	_closure->stackTop = argv;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_MinMaxTypesCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv = _closure->stackTop - argc;
	Int failingArg;

	if (argc < self->u.externalFunctionInfo.minArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' requires at least %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.minArgs, argc));
	}
	if (argc > self->u.externalFunctionInfo.maxArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' allows at most %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.maxArgs, argc));
	}
	if ((failingArg = PerformTypeChecks(argc, argv, self->u.externalFunctionInfo.numArgsToTypeCheck, self->u.externalFunctionInfo.argTypeChecks)) >= 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument %d to '%S' is of the wrong type.",
			failingArg + 1, self->u.externalFunctionInfo.name));
	}

	_closure->stackTop = argv;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}

Bool SmileExternalFunction_ExactTypesCheck_Call(SmileFunction self, Int argc)
{
	SmileObject *argv = _closure->stackTop - argc;
	Int failingArg;

	if (argc != self->u.externalFunctionInfo.minArgs) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' requires exactly %d arguments, but was called with %d.",
			self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.minArgs, argc));
	}
	if ((failingArg = PerformTypeChecks(argc, argv, self->u.externalFunctionInfo.numArgsToTypeCheck, self->u.externalFunctionInfo.argTypeChecks)) >= 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument %d to '%S' is of the wrong type.",
			failingArg + 1, self->u.externalFunctionInfo.name));
	}

	_closure->stackTop = argv;
	*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);

	return True;
}
