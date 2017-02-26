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

#include <smile/eval/compiler.h>
#include <smile/eval/compiler_internal.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smileinteger128.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

/// <summary>
/// Compile the given *single* expression.
/// </summary>
/// <param name="compiler">The compiler that will be compiling this expression.</param>
/// <param name="expr">The expression to compile.</param>
/// <returns>The offset of the first instruction of this expression in the current function's ByteCodeSegment.</returns>
/// <remarks>
/// This is the core of the compiler:  It transforms expressions into bytecode, adding that new
/// bytecode to the end of the ByteCodeSegment of the current function.
///
/// This function's behavior is undefined (i.e., broken) if the compiler has a NULL 'currentFunction'.
///
/// Note that you must have evaluated all macros before calling this function:  In particular,
/// this means special handling for the [$scope] expression.  This will compile a full [$scope],
/// but it compiles that [$scope] as-is, and does not evaluate macros.
///
/// Note also that this function does not compile nested functions:  It merely creates new
/// CompilerFunction objects for them, with their 'isCompiled' flags set to False.
/// </remarks>
Int Compiler_CompileExpr(Compiler compiler, SmileObject expr)
{
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	Int startIndex = segment->numByteCodes;
	SmileList list;
	Int argCount, index, offset, oldSourceLocation;

	switch (SMILE_KIND(expr)) {

		// Primitive constants.
	case SMILE_KIND_NULL:
		EMIT0(Op_LdNull, +1);
		break;
	case SMILE_KIND_BOOL:
		EMIT1(Op_LdBool, +1, boolean = ((SmileBool)expr)->value);
		break;
	case SMILE_KIND_STRING:
		EMIT1(Op_LdStr, +1, index = Compiler_AddString(compiler, (String)&((SmileString)expr)->string));
		break;
	case SMILE_KIND_PRIMITIVE:
		EMIT1(Op_LdObj, +1, index = Compiler_AddObject(compiler, Smile_KnownBases.Primitive));
		break;

		// Integer constants evaluate to themselves.
	case SMILE_KIND_BYTE:
		EMIT1(Op_Ld8, +1, byte = ((SmileByte)expr)->value);
		break;
	case SMILE_KIND_INTEGER16:
		EMIT1(Op_Ld16, +1, int16 = ((SmileInteger16)expr)->value);
		break;
	case SMILE_KIND_INTEGER32:
		EMIT1(Op_Ld32, +1, int32 = ((SmileInteger32)expr)->value);
		break;
	case SMILE_KIND_INTEGER64:
		EMIT1(Op_Ld64, +1, int64 = ((SmileInteger64)expr)->value);
		break;
	case SMILE_KIND_INTEGER128:
		Smile_Abort_FatalError("Integer128 is not yet supported.");
		break;
	case SMILE_KIND_BIGINT:
		Smile_Abort_FatalError("BigInt is not yet supported.");
		break;

		// Real constants evaluate to themselves.
	case SMILE_KIND_REAL16:
	case SMILE_KIND_REAL32:
	case SMILE_KIND_REAL64:
	case SMILE_KIND_REAL128:
	case SMILE_KIND_BIGREAL:
		Smile_Abort_FatalError("Reals are not yet supported.");
		break;

		// Float constants evaluate to themselves.
	case SMILE_KIND_FLOAT16:
	case SMILE_KIND_FLOAT32:
	case SMILE_KIND_FLOAT64:
	case SMILE_KIND_FLOAT128:
	case SMILE_KIND_BIGFLOAT:
		Smile_Abort_FatalError("Floats are not yet supported.");
		break;

		// User data evaluates to iteself.
	case SMILE_KIND_FUNCTION:
	case SMILE_KIND_HANDLE:
	case SMILE_KIND_NONTERMINAL:
	case SMILE_KIND_USEROBJECT:
	case SMILE_KIND_CLOSURE:
	case SMILE_KIND_FACADE:
		Smile_Abort_FatalError("These special forms are not yet supported.");
		break;

		// Intermediate forms (if they somehow make it this far) throw errors.
	case SMILE_KIND_MACRO:
	case SMILE_KIND_PARSEDECL:
	case SMILE_KIND_PARSEMESSAGE:
		Smile_Abort_FatalError("Intermediate forms are not supported.");
		break;

		// Simple pairs resolve by performing a property lookup on the object in question,
		// which may walk up the base chain for that object.
	case SMILE_KIND_PAIR:
		oldSourceLocation = compiler->currentFunction->currentSourceLocation;
		Compiler_SetSourceLocationFromPair(compiler, (SmilePair)expr);
		Compiler_CompileProperty(compiler, (SmilePair)expr, False);
		compiler->currentFunction->currentSourceLocation = oldSourceLocation;
		break;

		// Symbols (variables) resolve to whatever the current closure (or any base closure) says they are.
	case SMILE_KIND_SYMBOL:
		Compiler_CompileVariable(compiler, ((SmileSymbol)expr)->symbol, False);
		break;

		// Syntax objects resolve to themselves, like most other special user data does.
	case SMILE_KIND_SYNTAX:
		index = Compiler_AddObject(compiler, expr);
		EMIT1(Op_LdObj, +1, index = index);
		break;

		// In general, lists resolve by evaluating their arguments and then passing the results
		// to the function described by the first argument.  There are, however, a few forms which
		// evaluate specially, more forms than McCarthy's Lisp had, but still relatively few overall.
	case SMILE_KIND_LIST:
		list = (SmileList)expr;
		oldSourceLocation = Compiler_SetSourceLocationFromList(compiler, list);

		switch (SMILE_KIND(list->a)) {

			// Invocation of a method on an object, of the form [obj.method ...].
		case SMILE_KIND_PAIR:
			Compiler_CompileMethodCall(compiler, (SmilePair)list->a, (SmileList)list->d);
			break;

			// Either invocation of a known built-in (like [$if] or [$set] or [$scope]), or resolution
			// of a named function in the current scope.
		case SMILE_KIND_SYMBOL:
			if (SMILE_KIND(list->d) != SMILE_KIND_LIST && SMILE_KIND(list->d) != SMILE_KIND_NULL) {
				Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(list, getSourceLocation),
					String_FromC("Cannot compile list: List is not well-formed.")));
			}
			if (Compiler_CompileStandardForm(compiler, ((SmileSymbol)list->a)->symbol, (SmileList)list->d))
				break;
			// ...fall-thru to default case...

			// Resolve the given expression, and then call it, passing the rest of the list as
			// arguments (after they have been evaluated).
		default:
			// Resolve each element of the list.  The first element will become the function,
			// and the rest will become the arguments.
			argCount = 0;
			for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list), argCount++) {
				Compiler_SetSourceLocationFromList(compiler, list);
				Compiler_CompileExpr(compiler, list->a);
			}

			Compiler_RevertSourceLocation(compiler, oldSourceLocation);

			if (argCount == 0) {
				// [] just becomes null.
				EMIT0(Op_LdNull, +1);
				break;
			}

			// Under the hood, Call does some magic:
			//   1.  If the would-be function is actually a function, it is called directly;
			//   2.  Otherwise, if it is not a function, but it is an object that has an 'fn' method,
			//        then that method is called instead;
			//   3.  Otherwise, if it does not have an 'fn' method, then Call attempts to invoke
			//        [x.does-not-understand `fn ...] on it.
			//   4.  Otherwise, if it does not have a 'does-not-understand' method, a run-time exception is thrown.
			EMIT1(Op_Call, +1 - argCount, index = argCount - 1);
			break;
		}

		compiler->currentFunction->currentSourceLocation = oldSourceLocation;
		break;

	default:
		Smile_Abort_FatalError(String_ToC(String_Format("Cannot compile unknown/unsupported object type 0x%02X.", SMILE_KIND(expr))));
		break;
	}

	return startIndex;
}
