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
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuchar.h>
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

Inline EvalResult EvalResult_Create(Int kind)
{
	EvalResult result = GC_MALLOC_STRUCT(struct EvalResultStruct);
	result->evalResultKind = kind;
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

		// Set up the return continuation using setjmp/longjmp.
		if (!setjmp(_returnContinuation->jump)) {
		
			// Evaluate the expression for real.
			if (Eval_RunCore()) {

				// Expression evaluated normally.
				evalResult = EvalResult_Create(EVAL_RESULT_VALUE);
				evalResult->value = Closure_PopTemp(_closure);
				return evalResult;
			}
			else {
				// Hit a breakpoint.
				evalResult = EvalResult_Create(EVAL_RESULT_BREAK);
				return evalResult;
			}
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

		case Op_LdCh:
			Closure_PushTemp(_closure, SmileChar_Create((Byte)byteCode->u.ch));
			byteCode++;
			goto next;

		case Op_LdUCh:
			Closure_PushTemp(_closure, SmileUChar_Create(byteCode->u.uch));
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
		
		default:
			Smile_Abort_FatalError(String_ToC(String_Format("Eval error: Unknown opcode 0x%02X", byteCode->opcode)));
	}

	return True;
}

void Smile_Throw(SmileObject thrownObject)
{
	_exceptionContinuation->result = thrownObject;
	longjmp(_exceptionContinuation->jump, 1);
}
