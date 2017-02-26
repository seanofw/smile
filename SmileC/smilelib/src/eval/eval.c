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

Closure _closure;
CompiledTables _compiledTables;
ByteCodeSegment _segment;
ByteCode _byteCode;

EscapeContinuation _exceptionContinuation;

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

EvalResult Eval_Run(CompiledTables tables, UserFunctionInfo functionInfo)
{
	_compiledTables = tables;
	_segment = functionInfo->byteCodeSegment;
	_closure = Closure_CreateGlobal(tables->globalClosureInfo, NULL);
	_byteCode = &_segment->byteCodes[0];

	_closure = Closure_CreateLocal(&functionInfo->closureInfo, _closure, NULL, NULL, 0);

	_exceptionContinuation = EscapeContinuation_Create(ESCAPE_KIND_EXCEPTION);

	return Eval_Continue();
}

EvalResult Eval_Continue(void)
{
	EvalResult evalResult;

	// Set up the exception continuation using setjmp/longjmp.
	if (!setjmp(_exceptionContinuation->jump)) {
		_exceptionContinuation->isValid = True;

		// Evaluate the expression for real.
		if (Eval_RunCore()) {

			// Expression evaluated normally.
			evalResult = EvalResult_Create(EVAL_RESULT_VALUE);
			evalResult->value = Closure_PopTemp(_closure);

			_exceptionContinuation->isValid = False;
			return evalResult;
		}
		else {
			// Hit a breakpoint.
			evalResult = EvalResult_Create(EVAL_RESULT_BREAK);

			_exceptionContinuation->isValid = False;
			return evalResult;
		}
	}
	else {
		// Expression threw an uncaught exception.
		evalResult = EvalResult_Create(EVAL_RESULT_EXCEPTION);
		evalResult->exception = _exceptionContinuation->result;

		_exceptionContinuation->isValid = False;
		return evalResult;
	}
}

// Ensure that we've stored any of eval's core registers in the global state, so that they can be
// safely mutated or recorded by external actors.
#define STORE_REGISTERS \
	(_byteCode = byteCode)

// Reload eval's core registers from the global state, which is really the system of record for them.
#define LOAD_REGISTERS \
	(closure = _closure, byteCode = _byteCode)

Bool Eval_RunCore(void)
{
	// We prefer keeping these pointers in registers, because they're used by nearly every instruction.
	register Closure closure;
	register ByteCode byteCode;

	// If there are registers leftover, the compiler's welcome to store these values in registers, but
	// they're primarily used for less-frequent operations, so it's okay if they end up on the stack.
	SmileObject target, value;

	LOAD_REGISTERS;

next:
	switch (byteCode->opcode) {
	
		//-------------------------------------------------------
		// 00-0F: Miscellaneous stack- and state-management

		case Op_Nop:
			byteCode++;
			goto next;
		
		case Op_Dup1:
			closure->stackTop[0] = closure->stackTop[-1];
			closure->stackTop++;
			byteCode++;
			goto next;

		case Op_Dup2:
			closure->stackTop[0] = closure->stackTop[-2];
			closure->stackTop++;
			byteCode++;
			goto next;

		case Op_Dup:
			closure->stackTop[0] = closure->stackTop[-byteCode->u.index];
			closure->stackTop++;
			byteCode++;
			goto next;
		
		case Op_Pop1:
			Closure_PopCount(closure, 1);
			byteCode++;
			goto next;

		case Op_Pop2:
			Closure_PopCount(closure, 2);
			byteCode++;
			goto next;
		
		case Op_Pop:
			Closure_PopCount(closure, byteCode->u.index);
			byteCode++;
			goto next;

		case Op_Rep1:
			closure->stackTop[-2] = closure->stackTop[-1];
			closure->stackTop--;
			byteCode++;
			goto next;

		case Op_Rep2:
			closure->stackTop[-3] = closure->stackTop[-1];
			closure->stackTop -= 2;
			byteCode++;
			goto next;

		case Op_Rep:
			closure->stackTop[-(byteCode->u.index + 1)] = closure->stackTop[-1];
			closure->stackTop -= byteCode->u.index;
			byteCode++;
			goto next;

		case Op_Brk:
			return False;
		
		//-------------------------------------------------------
		// 10-17: Special load instructions
		
		case Op_LdNull:
			Closure_PushTemp(closure, NullObject);
			byteCode++;
			goto next;

		case Op_LdBool:
			Closure_PushTemp(closure, Smile_KnownObjects.BooleanObjs[byteCode->u.boolean]);
			byteCode++;
			goto next;

		case Op_LdStr:
			Closure_PushTemp(closure, SmileString_Create(_compiledTables->strings[byteCode->u.index]));
			byteCode++;
			goto next;

		case Op_LdSym:
			Closure_PushTemp(closure, SmileSymbol_Create(byteCode->u.symbol));
			byteCode++;
			goto next;

		case Op_LdObj:
			Closure_PushTemp(closure, _compiledTables->objects[byteCode->u.index]);
			byteCode++;
			goto next;

		case Op_LdClos:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// 18-1F: Integer load instructions
		
		case Op_Ld8:
			Closure_PushTemp(closure, SmileByte_Create(byteCode->u.byte));
			byteCode++;
			goto next;

		case Op_Ld16:
			Closure_PushTemp(closure, SmileInteger16_Create(byteCode->u.int16));
			byteCode++;
			goto next;

		case Op_Ld32:
			Closure_PushTemp(closure, SmileInteger32_Create(byteCode->u.int32));
			byteCode++;
			goto next;

		case Op_Ld64:
			Closure_PushTemp(closure, SmileInteger64_Create(byteCode->u.int64));
			byteCode++;
			goto next;

		case Op_Ld128:
			Closure_PushTemp(closure, _compiledTables->objects[byteCode->u.index]);
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 20-27: Real load instructions

		case Op_LdR16:
		case Op_LdR32:
		case Op_LdR64:
		case Op_LdR128:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// 28-2F: Float load instructions

		case Op_LdF16:
		case Op_LdF32:
		case Op_LdF64:
		case Op_LdF128:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// 30-37: General-purpose local-variable/argument instructions
		
		case Op_LdLoc:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope(closure, byteCode->u.i2.a, byteCode->u.i2.b));
			byteCode++;
			goto next;

		case Op_StLoc:
			Closure_SetLocalVariableInScope(closure, byteCode->u.i2.a, byteCode->u.i2.b, Closure_GetTop(closure));
			byteCode++;
			goto next;

		case Op_StpLoc:
			Closure_SetLocalVariableInScope(closure, byteCode->u.i2.a, byteCode->u.i2.b, Closure_PopTemp(closure));
			byteCode++;
			goto next;

		case Op_LdArg:
			Closure_PushTemp(closure, Closure_GetArgumentInScope(closure, byteCode->u.i2.a, byteCode->u.i2.b));
			byteCode++;
			goto next;

		case Op_StArg:
			Closure_SetArgumentInScope(closure, byteCode->u.i2.a, byteCode->u.i2.b, Closure_GetTop(closure));
			byteCode++;
			goto next;

		case Op_StpArg:
			Closure_SetArgumentInScope(closure, byteCode->u.i2.a, byteCode->u.i2.b, Closure_PopTemp(closure));
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// 38-3F: Global (eXternal) variable instructions

		case Op_LdX:
			Closure_PushTemp(closure, Closure_GetGlobalVariable(closure, byteCode->u.symbol));
			byteCode++;
			goto next;

		case Op_StX:
			Closure_SetGlobalVariable(closure, byteCode->u.symbol, Closure_GetTop(closure));
			byteCode++;
			goto next;

		case Op_StpX:
			Closure_SetGlobalVariable(closure, byteCode->u.symbol, Closure_PopTemp(closure));
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// 40-4F: Optimized local-variable/argument load instructions
		
		case Op_LdArg0:
			Closure_PushTemp(closure, Closure_GetArgumentInScope0(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg1:
			Closure_PushTemp(closure, Closure_GetArgumentInScope1(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg2:
			Closure_PushTemp(closure, Closure_GetArgumentInScope2(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg3:
			Closure_PushTemp(closure, Closure_GetArgumentInScope3(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg4:
			Closure_PushTemp(closure, Closure_GetArgumentInScope4(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg5:
			Closure_PushTemp(closure, Closure_GetArgumentInScope5(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg6:
			Closure_PushTemp(closure, Closure_GetArgumentInScope6(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdArg7:
			Closure_PushTemp(closure, Closure_GetArgumentInScope7(closure, byteCode->u.index));
			byteCode++;
			goto next;
		
		case Op_LdLoc0:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope0(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc1:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope1(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc2:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope2(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc3:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope3(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc4:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope4(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc5:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope5(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc6:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope6(closure, byteCode->u.index));
			byteCode++;
			goto next;
		case Op_LdLoc7:
			Closure_PushTemp(closure, Closure_GetLocalVariableInScope7(closure, byteCode->u.index));
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 50-5F: Optimized local-variable/argument store instructions

		case Op_StArg0:
			Closure_SetArgumentInScope0(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StArg1:
			Closure_SetArgumentInScope1(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StArg2:
			Closure_SetArgumentInScope2(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StArg3:
			Closure_SetArgumentInScope3(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StArg4:
			Closure_SetArgumentInScope4(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StArg5:
			Closure_SetArgumentInScope5(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StArg6:
			Closure_SetArgumentInScope6(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StArg7:
			Closure_SetArgumentInScope7(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;

		case Op_StLoc0:
			Closure_SetLocalVariableInScope0(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StLoc1:
			Closure_SetLocalVariableInScope1(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StLoc2:
			Closure_SetLocalVariableInScope2(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StLoc3:
			Closure_SetLocalVariableInScope3(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StLoc4:
			Closure_SetLocalVariableInScope4(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StLoc5:
			Closure_SetLocalVariableInScope5(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StLoc6:
			Closure_SetLocalVariableInScope6(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		case Op_StLoc7:
			Closure_SetLocalVariableInScope7(closure, byteCode->u.index, Closure_GetTop(closure));
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 60-6F: Optimized local-variable/argument store-and-pop instructions

		case Op_StpArg0:
			Closure_SetArgumentInScope0(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpArg1:
			Closure_SetArgumentInScope1(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpArg2:
			Closure_SetArgumentInScope2(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpArg3:
			Closure_SetArgumentInScope3(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpArg4:
			Closure_SetArgumentInScope4(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpArg5:
			Closure_SetArgumentInScope5(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpArg6:
			Closure_SetArgumentInScope6(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpArg7:
			Closure_SetArgumentInScope7(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;

		case Op_StpLoc0:
			Closure_SetLocalVariableInScope0(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpLoc1:
			Closure_SetLocalVariableInScope1(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpLoc2:
			Closure_SetLocalVariableInScope2(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpLoc3:
			Closure_SetLocalVariableInScope3(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpLoc4:
			Closure_SetLocalVariableInScope4(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpLoc5:
			Closure_SetLocalVariableInScope5(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpLoc6:
			Closure_SetLocalVariableInScope6(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		case Op_StpLoc7:
			Closure_SetLocalVariableInScope7(closure, byteCode->u.index, Closure_PopTemp(closure));
			byteCode++;
			goto next;
		
		//-------------------------------------------------------
		// 70-7F: General-purporse property and member access

		case Op_LdProp:
			target = Closure_GetTop(closure);
			STORE_REGISTERS;
			value = SMILE_VCALL1(target, getProperty, byteCode->u.symbol);
			LOAD_REGISTERS;
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;
		case Op_StProp:
			target = Closure_GetTemp(closure, 1);
			value = Closure_GetTemp(closure, 0);
			STORE_REGISTERS;
			SMILE_VCALL2(target, setProperty, byteCode->u.symbol, value);
			LOAD_REGISTERS;
			Closure_PopCount(closure, 1);
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;
		case Op_StpProp:
			target = Closure_GetTemp(closure, 1);
			value = Closure_GetTemp(closure, 0);
			STORE_REGISTERS;
			SMILE_VCALL2(target, setProperty, byteCode->u.symbol, value);
			LOAD_REGISTERS;
			Closure_PopCount(closure, 2);
			byteCode++;
			goto next;

		case Op_LdMember:
		case Op_StMember:
		case Op_StpMember:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// 80-8F: Specialty type management

		case Op_Cons:
			value = (SmileObject)SmileList_Cons(Closure_GetTemp(closure, 1), Closure_GetTemp(closure, 0));
			Closure_PopCount(closure, 1);
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_Car:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_LIST) {
				value = ((SmileList)target)->a;
			}
			else {
				value = NullObject;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_Cdr:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_LIST) {
				value = ((SmileList)target)->a;
			}
			else {
				value = NullObject;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_NewPair:
			value = (SmileObject)SmilePair_Create(Closure_GetTemp(closure, 1), Closure_GetTemp(closure, 0));
			Closure_PopCount(closure, 1);
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_Left:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_PAIR) {
				value = ((SmilePair)target)->left;
			}
			else {
				value = NullObject;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_Right:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_PAIR) {
				value = ((SmilePair)target)->right;
			}
			else {
				value = NullObject;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_NewFn:
			value = (SmileObject)SmileFunction_CreateUserFunction(_compiledTables->userFunctions[byteCode->u.index], closure);
			Closure_PushTemp(closure, value);
			byteCode++;
			goto next;

		case Op_NewObj:
			goto unsupportedOpcode;
		
		case Op_SuperEq:
			target = Closure_GetTemp(closure, 0);
			value = Closure_GetTemp(closure, 1);
			Closure_PopCount(closure, 1);
			value = (SmileObject)SmileBool_FromBool(SMILE_VCALL1(target, compareEqual, value));
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_SuperNe:
			target = Closure_GetTemp(closure, 0);
			value = Closure_GetTemp(closure, 1);
			Closure_PopCount(closure, 1);
			value = (SmileObject)SmileBool_FromBool(1 - SMILE_VCALL1(target, compareEqual, value));
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;
		
		case Op_Not:
			value = Closure_GetTop(closure);
			if (SMILE_KIND(value) != SMILE_KIND_BOOL) {
				SmileBool smileBool;
				STORE_REGISTERS;
				smileBool = SmileBool_FromBool(1 - SMILE_VCALL(value, toBool));
				LOAD_REGISTERS;
				Closure_SetTop(closure, smileBool);
			}
			else {
				Closure_SetTop(closure, SmileBool_FromBool(1 - ((SmileBool)value)->value));
			}
			byteCode++;
			goto next;

		case Op_Is:
			target = Closure_GetTemp(closure, 0);
			value = Closure_GetTemp(closure, 1);
			Closure_PopCount(closure, 1);
			Closure_SetTop(closure, SmileBool_FromBool(SmileObject_Is(target, value)));
			byteCode++;
			goto next;

		case Op_TypeOf:
			goto unsupportedOpcode;
		
		//-------------------------------------------------------
		// 90-9F: Special-purpose function and method calls

		case Op_Call0:
			target = Closure_GetTemp(closure, 0);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, 0);
			LOAD_REGISTERS;
			goto next;

		case Op_Call1:
			target = Closure_GetTemp(closure, 1);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, 1);
			LOAD_REGISTERS;
			goto next;

		case Op_Call2:
			target = Closure_GetTemp(closure, 2);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, 2);
			LOAD_REGISTERS;
			goto next;
		
		case Op_Call3:
			target = Closure_GetTemp(closure, 3);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, 3);
			LOAD_REGISTERS;
			goto next;

		case Op_Call4:
			target = Closure_GetTemp(closure, 4);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, 4);
			LOAD_REGISTERS;
			goto next;

		case Op_Call5:
			target = Closure_GetTemp(closure, 5);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, 5);
			LOAD_REGISTERS;
			goto next;

		case Op_Call6:
			target = Closure_GetTemp(closure, 6);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, 6);
			LOAD_REGISTERS;
			goto next;

		case Op_Call7:
			target = Closure_GetTemp(closure, 7);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, 7);
			LOAD_REGISTERS;
			goto next;

		case Op_Met0:
			target = Closure_GetTemp(closure, 0);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 1);	// Invoke it, whatever it is.
			LOAD_REGISTERS;
			goto next;

		case Op_Met1:
			target = Closure_GetTemp(closure, 1);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 2);	// Invoke it, whatever it is.
			LOAD_REGISTERS;
			goto next;

		case Op_Met2:
			target = Closure_GetTemp(closure, 2);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 3);	// Invoke it, whatever it is.
			LOAD_REGISTERS;
			goto next;

		case Op_Met3:
			target = Closure_GetTemp(closure, 3);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 4);	// Invoke it, whatever it is.
			LOAD_REGISTERS;
			goto next;

		case Op_Met4:
			target = Closure_GetTemp(closure, 4);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 5);	// Invoke it, whatever it is.
			LOAD_REGISTERS;
			goto next;

		case Op_Met5:
			target = Closure_GetTemp(closure, 5);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 6);	// Invoke it, whatever it is.
			LOAD_REGISTERS;
			goto next;

		case Op_Met6:
			target = Closure_GetTemp(closure, 6);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 7);	// Invoke it, whatever it is.
			LOAD_REGISTERS;
			goto next;

		case Op_Met7:
			target = Closure_GetTemp(closure, 7);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.symbol);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, 8);	// Invoke it, whatever it is.
			LOAD_REGISTERS;
			goto next;

		case Op_TCall0:
		case Op_TCall1:
		case Op_TCall2:
		case Op_TCall3:
		case Op_TCall4:
		case Op_TCall5:
		case Op_TCall6:
		case Op_TCall7:
		case Op_TMet0:
		case Op_TMet1:
		case Op_TMet2:
		case Op_TMet3:
		case Op_TMet4:
		case Op_TMet5:
		case Op_TMet6:
		case Op_TMet7:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// B0-BF: Flow control
		
		case Op_Jmp:
			byteCode += byteCode->u.index;
			goto next;

		case Op_Bt:
			value = Closure_PopTemp(closure);
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
			value = Closure_PopTemp(closure);
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
			target = Closure_GetTemp(closure, byteCode->u.i2.b + 1);	// Get the target object
			byteCode++;	
			STORE_REGISTERS;	
			target = SMILE_VCALL1(target, getProperty, byteCode[-1].u.i2.a);	// Turn it into a function (hopefully)
			SMILE_VCALL1(target, call, byteCode[-1].u.i2.b + 1);	// Invoke it.
			LOAD_REGISTERS;
			goto next;

		case Op_TMet:
			goto unsupportedOpcode;
		
		case Op_Call:
			target = Closure_GetTemp(closure, byteCode->u.index);
			byteCode++;
			STORE_REGISTERS;
			SMILE_VCALL1(target, call, byteCode[-1].u.index);
			LOAD_REGISTERS;
			goto next;

		case Op_TCall:
			goto unsupportedOpcode;

		case Op_NewTill:
		case Op_EndTill:
		case Op_TillEsc:
			goto unsupportedOpcode;

		case Op_Try:
		case Op_EndTry:
			goto unsupportedOpcode;

		case Op_Ret:
		do_return:
			if (closure->returnClosure == NULL) {
				return True;
			}
			else {
				// Get the return value off the top of the closure.
				value = Closure_GetTop(closure);
			
				// Reset which closure we're running against.
				_segment = closure->returnSegment;
				_byteCode = byteCode = _segment->byteCodes + closure->returnPc;
				_closure = closure = closure->returnClosure;
			
				// Push the function's return value onto the current closure.
				Closure_PushTemp(closure, value);
				goto next;
			}

		//-------------------------------------------------------
		// C0-C7: Optimized arithmetic method access
			
		case Op_Add:
		case Op_Sub:
		case Op_Mul:
		case Op_Div:
		case Op_Mod:
		case Op_Rem:
		case Op_RangeTo:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// C8-CF: Optimized comparison-method access
		
		case Op_Eq:
		case Op_Ne:
		case Op_Lt:
		case Op_Gt:
		case Op_Le:
		case Op_Ge:
		case Op_Cmp:
		case Op_Compare:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// D0-D7: Optimized binary sequence method access
		
		case Op_Each:
		case Op_Map:
		case Op_Where:
		case Op_Count:
		case Op_Any:
		case Op_Join:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// D8-DF: Optimized unary sequence method access
		
		case Op_UCount:
		case Op_UAny:
		case Op_UJoin:
		case Op_Neg:
			goto unsupportedOpcode;

		case Op_Bool:
			value = Closure_GetTop(closure);
			if (SMILE_KIND(value) != SMILE_KIND_BOOL) {
				value = (SmileObject)SmileBool_FromBool(SMILE_VCALL(value, toBool));
				Closure_SetTop(closure, value);
			}
			byteCode++;
			goto next;

		case Op_Int:
		case Op_String:
		case Op_Hash:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// E0-E7: Optimized type-query method access

		case Op_NullQ:
		case Op_ListQ:
		case Op_PairQ:
		case Op_FnQ:
		case Op_BoolQ:
		case Op_IntQ:
		case Op_StringQ:
		case Op_SymbolQ:
			goto unsupportedOpcode;

		//-------------------------------------------------------
		// E8-EF: Special-purpose optimized property access

		case Op_LdA:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_LIST) {
				value = ((SmileList)target)->a;
			}
			else {
				STORE_REGISTERS;
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.a);
				LOAD_REGISTERS;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_LdD:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_LIST) {
				value = ((SmileList)target)->d;
			}
			else {
				STORE_REGISTERS;
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.d);
				LOAD_REGISTERS;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;
		
		case Op_LdLeft:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_PAIR) {
				value = ((SmilePair)target)->left;
			}
			else {
				STORE_REGISTERS;
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.left);
				LOAD_REGISTERS;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_LdRight:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_PAIR) {
				value = ((SmilePair)target)->right;
			}
			else {
				STORE_REGISTERS;
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.right);
				LOAD_REGISTERS;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_LdStart:
			target = Closure_GetTop(closure);
			STORE_REGISTERS;
			value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.start);
			LOAD_REGISTERS;
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_LdEnd:
			target = Closure_GetTop(closure);
			STORE_REGISTERS;
			value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.end);
			LOAD_REGISTERS;
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		case Op_LdCount:
			target = Closure_GetTop(closure);
			STORE_REGISTERS;
			value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.count);
			LOAD_REGISTERS;
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;
		
		case Op_LdLength:
			target = Closure_GetTop(closure);
			if (SMILE_KIND(target) == SMILE_KIND_STRING) {
				value = (SmileObject)SmileInteger32_Create(((SmileString)target)->string.length);
			}
			else {
				STORE_REGISTERS;
				value = SMILE_VCALL1(target, getProperty, Smile_KnownSymbols.length);
				LOAD_REGISTERS;
			}
			Closure_SetTop(closure, value);
			byteCode++;
			goto next;

		//-------------------------------------------------------
		// F0-FF: Miscellaneous internal constructs
		
		case Op_StateMachStart:
			// Repeatedly invoke the Smile function on the top of the stack, calling the given C
			// function in between.  This provides a way for things like List.each and List.map
			// to do their job while not recursing deeper on the C stack, which makes many
			// externally-looping C things able to be safely interrupted (like by a cofunction).
			// It also means that after compile, the C stack size is finite (more-or-less, depending
			// on what external things you may call).

			// Call the given C state-machine startup function first (same semantics as the body call below).
			{
				Int argc;

				byteCode++;
				STORE_REGISTERS;
				if ((argc = ((ClosureStateMachine)closure)->stateMachineStart((ClosureStateMachine)closure)) >= 0) {
					LOAD_REGISTERS;
					STORE_REGISTERS;
					SMILE_VCALL1(Closure_GetTemp(closure, argc), call, argc);
					LOAD_REGISTERS;
					goto next;
				}
				else {
					LOAD_REGISTERS;
					goto do_return;
				}
			}

		case Op_StateMachBody:
			// Call the given C state-machine function body.  If it returns a SmileFunction, we need to
			// then invoke that, which may involve switching closures and running user code for a while.
			// But while in this closure, we continue to hold on this instruction until the state
			// machine tells us it is done.  This all ends up forming a much tighter loop than it may
			// initially look; it's not as tight as something like `while (cond) { Eval(fn); }`, but
			// it's still pretty tight, and unlike that loop, it is safely interruptible.
			{
				Int argc;

				STORE_REGISTERS;
				if ((argc = ((ClosureStateMachine)closure)->stateMachineBody((ClosureStateMachine)closure)) >= 0) {
					LOAD_REGISTERS;
					STORE_REGISTERS;
					SMILE_VCALL1(Closure_GetTemp(closure, argc), call, argc);
					LOAD_REGISTERS;
					goto next;
				}
				else {
					LOAD_REGISTERS;
					goto do_return;
				}
			}
		
		case Op_Label:
			// Label is treated the same as a NOP, if it still somehow exists at eval-time.
			byteCode++;
			goto next;
			
		//-------------------------------------------------------
		
		case Op_04: case Op_08: case Op_0C: case Op_0D: case Op_0E:
		case Op_16: case Op_17: case Op_1D: case Op_1E: case Op_1F:
		case Op_20: case Op_25: case Op_26: case Op_27: case Op_28: case Op_2D: case Op_2E: case Op_2F:
		case Op_33: case Op_37: case Op_3B: case Op_3C: case Op_3D: case Op_3E: case Op_3F:
		case Op_73: case Op_77: case Op_78: case Op_79: case Op_7A: case Op_7B: case Op_7C: case Op_7D: case Op_7E: case Op_7F:
		case Op_83: case Op_87: case Op_8A:
		case Op_B3: case Op_BE:
		case Op_C6:
		case Op_D3: case Op_D7:
		case Op_F2: case Op_F3: case Op_F4: case Op_F5: case Op_F6: case Op_F7:
		case Op_F8: case Op_F9: case Op_FA: case Op_FB: case Op_FC: case Op_FD: case Op_FE:
			STORE_REGISTERS;
			Smile_Abort_FatalError(String_ToC(String_Format("Compiler bug: Unknown opcode 0x%02X", byteCode->opcode)));
		
		unsupportedOpcode:
			STORE_REGISTERS;
			Smile_Abort_FatalError(String_ToC(String_Format("Eval: Unsuported opcode 0x%02X", byteCode->opcode)));
	}

	STORE_REGISTERS;
	Smile_Abort_FatalError(String_ToC(String_Format("Eval bug: Unhandled opcode 0x%02X", byteCode->opcode)));
	return False;
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

/*
#define FAST_INLINE_BINARY_OP(__type__, __kind__, __resultType__, __resultKind__, __op__) { \
		__type__ a, b; \
		__resultType__ result; \
		if ((b = (__type__)closure->stacktop[-1])->kind == (__kind__)) { \
			result = (__resultType__)&closure->unboxed[closure->stacktop - closure->base - 2]; \
			a = (__type__)(*--closure->stacktop); \
			closure->stacktop[-1] = (SmileObject)dest; \
			result->kind = (__resultKind__); \
			result->vtable = __resultType__##_VTable; \
			(__op__); \
			_byteCode++; \
			goto next; \
		} \
	}

case Op_Add:
	switch (closure->stacktop[-2]->kind) {
		case SMILE_KIND_BYTE:
			FAST_INLINE_BINARY_OP(SmileByte, SMILE_KIND_BYTE, SmileByte, SMILE_KIND_BYTE,
				{ result->value = a->value + b->value; })
		case SMILE_KIND_INTEGER16:
			FAST_INLINE_BINARY_OP(SmileInteger16, SMILE_KIND_INTEGER16, SmileInteger16, SMILE_KIND_INTEGER16,
				{ result->value = a->value + b->value; })
		case SMILE_KIND_INTEGER32:
			FAST_INLINE_BINARY_OP(SmileInteger32, SMILE_KIND_INTEGER32, SmileInteger32, SMILE_KIND_INTEGER32,
				{ result->value = a->value + b->value; })
		case SMILE_KIND_INTEGER64:
			FAST_INLINE_BINARY_OP(SmileInteger64, SMILE_KIND_INTEGER64, SmileInteger64, SMILE_KIND_INTEGER64,
				{ result->value = a->value + b->value; })
	}
	symbol = SMILE_SPECIAL_SYMBOL_PLUS;
	goto method2;

case Op_Sub:
	switch (closure->stacktop[-2]->kind) {
		case SMILE_KIND_BYTE:
			FAST_INLINE_BINARY_OP(SmileByte, SMILE_KIND_BYTE, SmileByte, SMILE_KIND_BYTE,
				{ result->value = a->value - b->value; })
		case SMILE_KIND_INTEGER16:
			FAST_INLINE_BINARY_OP(SmileInteger16, SMILE_KIND_INTEGER16, SmileInteger16, SMILE_KIND_INTEGER16,
				{ result->value = a->value - b->value; })
		case SMILE_KIND_INTEGER32:
			FAST_INLINE_BINARY_OP(SmileInteger32, SMILE_KIND_INTEGER32, SmileInteger32, SMILE_KIND_INTEGER32,
				{ result->value = a->value - b->value; })
		case SMILE_KIND_INTEGER64:
			FAST_INLINE_BINARY_OP(SmileInteger64, SMILE_KIND_INTEGER64, SmileInteger64, SMILE_KIND_INTEGER64,
				{ result->value = a->value - b->value; })
	}
	symbol = SMILE_SPECIAL_SYMBOL_MINUS;
	goto method2;

case Op_Mul:
	switch (closure->stacktop[-2]->kind) {
		case SMILE_KIND_BYTE:
			FAST_INLINE_BINARY_OP(SmileByte, SMILE_KIND_BYTE, SmileByte, SMILE_KIND_BYTE,
				{ result->value = a->value * b->value; })
		case SMILE_KIND_INTEGER16:
			FAST_INLINE_BINARY_OP(SmileInteger16, SMILE_KIND_INTEGER16, SmileInteger16, SMILE_KIND_INTEGER16,
				{ result->value = a->value * b->value; })
		case SMILE_KIND_INTEGER32:
			FAST_INLINE_BINARY_OP(SmileInteger32, SMILE_KIND_INTEGER32, SmileInteger32, SMILE_KIND_INTEGER32,
				{ result->value = a->value * b->value; })
		case SMILE_KIND_INTEGER64:
			FAST_INLINE_BINARY_OP(SmileInteger64, SMILE_KIND_INTEGER64, SmileInteger64, SMILE_KIND_INTEGER64,
				{ result->value = a->value * b->value; })
	}
	symbol = SMILE_SPECIAL_SYMBOL_STAR;
	goto method2;

case Op_Div:
	switch (closure->stacktop[-2]->kind) {
		case SMILE_KIND_BYTE:
			FAST_INLINE_BINARY_OP(SmileByte, SMILE_KIND_BYTE, SmileByte, SMILE_KIND_BYTE,
				{ if (b->value == 0) Smile_ThrowException(); result->value = a->value / b->value; })
		case SMILE_KIND_INTEGER16:
			FAST_INLINE_BINARY_OP(SmileInteger16, SMILE_KIND_INTEGER16, SmileInteger16, SMILE_KIND_INTEGER16,
				{ if (b->value == 0) Smile_ThrowException(); result->value = a->value / b->value; })
		case SMILE_KIND_INTEGER32:
			FAST_INLINE_BINARY_OP(SmileInteger32, SMILE_KIND_INTEGER32, SmileInteger32, SMILE_KIND_INTEGER32,
				{ if (b->value == 0) Smile_ThrowException(); result->value = a->value / b->value; })
		case SMILE_KIND_INTEGER64:
			FAST_INLINE_BINARY_OP(SmileInteger64, SMILE_KIND_INTEGER64, SmileInteger64, SMILE_KIND_INTEGER64,
				{ if (b->value == 0) Smile_ThrowException(); result->value = a->value / b->value; })
	}
	symbol = SMILE_SPECIAL_SYMBOL_SLASH;
	goto method2;

case Op_Eq:
	switch (closure->stacktop[-2]->kind) {
		case SMILE_KIND_BYTE:
			FAST_INLINE_BINARY_OP(SmileByte, SMILE_KIND_BYTE, SmileBool, SMILE_KIND_BOOL,
				{ result->value = a->value == b->value; })
		case SMILE_KIND_INTEGER16:
			FAST_INLINE_BINARY_OP(SmileInteger16, SMILE_KIND_INTEGER16, SmileBool, SMILE_KIND_BOOL,
				{ result->value = a->value == b->value; })
		case SMILE_KIND_INTEGER32:
			FAST_INLINE_BINARY_OP(SmileInteger32, SMILE_KIND_INTEGER32, SmileBool, SMILE_KIND_BOOL,
				{ result->value = a->value == b->value; })
		case SMILE_KIND_INTEGER64:
			FAST_INLINE_BINARY_OP(SmileInteger64, SMILE_KIND_INTEGER64, SmileBool, SMILE_KIND_BOOL,
				{ result->value = a->value == b->value; })
	}
	symbol = SMILE_SPECIAL_SYMBOL_EQ;
	goto method2;

case Op_Ne:
	switch (closure->stacktop[-2]->kind) {
		case SMILE_KIND_BYTE:
			FAST_INLINE_BINARY_OP(SmileByte, SMILE_KIND_BYTE, SmileBool, SMILE_KIND_BOOL,
				{ result->value = a->value != b->value; })
		case SMILE_KIND_INTEGER16:
			FAST_INLINE_BINARY_OP(SmileInteger16, SMILE_KIND_INTEGER16, SmileBool, SMILE_KIND_BOOL,
				{ result->value = a->value != b->value; })
		case SMILE_KIND_INTEGER32:
			FAST_INLINE_BINARY_OP(SmileInteger32, SMILE_KIND_INTEGER32, SmileBool, SMILE_KIND_BOOL,
				{ result->value = a->value != b->value; })
		case SMILE_KIND_INTEGER64:
			FAST_INLINE_BINARY_OP(SmileInteger64, SMILE_KIND_INTEGER64, SmileBool, SMILE_KIND_BOOL,
				{ result->value = a->value != b->value; })
	}
	symbol = SMILE_SPECIAL_SYMBOL_NE;
	goto method2;
*/