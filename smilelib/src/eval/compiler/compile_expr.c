//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

// TODO: FIXME: HACK:  This should properly use the new type model to determine if
// the first argument is a type object or an ordinary object.  But since we don't
// have the new type model, we just test to see if:
//
//   1. 'obj' (the first argument) is a symbol; and
//   2. the symbol starts with a capital letter.
//
// This is utterly hacktastic, but it works well enough that we can continue building out
// the methods in the standard library even though we don't have a 'type' keyword yet.
static Bool IsStaticInvocation(SmileList dotArgs)
{
	// Test #1:  Is the first argument a symbol?
	if (SMILE_KIND(dotArgs) == SMILE_KIND_LIST
		&& SMILE_KIND(dotArgs->a) == SMILE_KIND_SYMBOL) {

		// Test #2:  Is the symbol's first character an uppercase letter?
		// This is a lot of function calls, but still runs in amortized O(1) time.
		Symbol symbol = ((SmileSymbol)dotArgs->a)->symbol;
		String symbolText = SymbolTable_GetName(Smile_SymbolTable, symbol);
		const Byte *text = String_GetBytes(symbolText);
		if (*text >= 'A' && *text <= 'Z') {
			return True;
		}
	}

	return False;
}

/// <summary>
/// Compile the given *single* expression.
/// </summary>
/// <param name="compiler">The compiler that will be compiling this expression.</param>
/// <param name="expr">The expression to compile.</param>
/// <param name="compileFlags">Flags that control the compile behavior.</param>
/// <returns>A result struct indicating the results of the compile.</returns>
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
CompiledBlock Compiler_CompileExpr(Compiler compiler, SmileObject expr, CompileFlags compileFlags)
{
	SmileList list;
	Int argCount, index, oldSourceLocation;
	CompiledBlock compiledBlock;
	IntermediateInstruction instr;

	// Compile an expression that evaluates to a constant of some type.
#define COMPILE_PRIMITIVE_EXPR(__op__, __expr__) \
	do { \
		compiledBlock = CompiledBlock_Create(); \
		if (!(compileFlags & COMPILE_FLAG_NORESULT)) { \
			EMIT1(__op__, +1, __expr__); \
		} \
		return compiledBlock; \
	} while (0)

	switch (SMILE_KIND(expr)) {

		// Primitive constants.
		case SMILE_KIND_NULL:
			compiledBlock = CompiledBlock_Create();
			if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
				EMIT0(Op_LdNull, +1);
			}
			return compiledBlock;
		case SMILE_KIND_BOOL:
			COMPILE_PRIMITIVE_EXPR(Op_LdBool, boolean = ((SmileBool)expr)->value);
		case SMILE_KIND_STRING:
			COMPILE_PRIMITIVE_EXPR(Op_LdStr, index = Compiler_AddString(compiler, (String)expr));
		case SMILE_KIND_PRIMITIVE:
			COMPILE_PRIMITIVE_EXPR(Op_LdObj, index = Compiler_AddObject(compiler, Smile_KnownBases.Primitive));
		case SMILE_KIND_CHAR:
			COMPILE_PRIMITIVE_EXPR(Op_LdChar, ch = ((SmileChar)expr)->ch);
		case SMILE_KIND_UNI:
			COMPILE_PRIMITIVE_EXPR(Op_LdUni, uni = ((SmileUni)expr)->code);

		// Integer constants evaluate to themselves.
		case SMILE_KIND_BYTE:
			COMPILE_PRIMITIVE_EXPR(Op_Ld8, byte = ((SmileByte)expr)->value);
		case SMILE_KIND_INTEGER16:
			COMPILE_PRIMITIVE_EXPR(Op_Ld16, int16 = ((SmileInteger16)expr)->value);
		case SMILE_KIND_INTEGER32:
			COMPILE_PRIMITIVE_EXPR(Op_Ld32, int32 = ((SmileInteger32)expr)->value);
		case SMILE_KIND_INTEGER64:
			COMPILE_PRIMITIVE_EXPR(Op_Ld64, int64 = ((SmileInteger64)expr)->value);
		case SMILE_KIND_INTEGER128:
			Smile_Abort_FatalError("Integer128 is not yet supported.");
		case SMILE_KIND_BIGINT:
			Smile_Abort_FatalError("BigInt is not yet supported.");

		// Real constants evaluate to themselves.
		case SMILE_KIND_REAL32:
			COMPILE_PRIMITIVE_EXPR(Op_LdR32, real32 = ((SmileReal32)expr)->value);
		case SMILE_KIND_REAL64:
			COMPILE_PRIMITIVE_EXPR(Op_LdR64, real64 = ((SmileReal64)expr)->value);
		case SMILE_KIND_REAL128:
		case SMILE_KIND_BIGREAL:
			Smile_Abort_FatalError("Real128 and BigReal are not yet supported.");

		// Float constants evaluate to themselves.
		case SMILE_KIND_FLOAT32:
			COMPILE_PRIMITIVE_EXPR(Op_LdF32, float32 = ((SmileFloat32)expr)->value);
		case SMILE_KIND_FLOAT64:
			COMPILE_PRIMITIVE_EXPR(Op_LdF64, float64 = ((SmileFloat64)expr)->value);
		case SMILE_KIND_FLOAT128:
		case SMILE_KIND_BIGFLOAT:
			Smile_Abort_FatalError("Float128 and BigFloat are not yet supported.");

		// User data evaluates to iteself.
		case SMILE_KIND_TILL_CONTINUATION:
		case SMILE_KIND_FUNCTION:
		case SMILE_KIND_HANDLE:
		case SMILE_KIND_USEROBJECT:
		case SMILE_KIND_NONTERMINAL:
		case SMILE_KIND_CLOSURE:
		case SMILE_KIND_FACADE:
			COMPILE_PRIMITIVE_EXPR(Op_LdObj, index = Compiler_AddObject(compiler, expr));

		// Intermediate forms (if they somehow make it this far) throw errors.
		case SMILE_KIND_MACRO:
		case SMILE_KIND_PARSEDECL:
		case SMILE_KIND_PARSEMESSAGE:
			Smile_Abort_FatalError("Intermediate forms are not supported.");

		// Symbols (variables) resolve to whatever the current closure (or any base closure) says they are.
		case SMILE_KIND_SYMBOL:
			return Compiler_CompileLoadVariable(compiler, ((SmileSymbol)expr)->symbol, compileFlags);

		// Syntax objects resolve to themselves, like most other special user data does.
		case SMILE_KIND_SYNTAX:
		case SMILE_KIND_LOANWORD:
			compiledBlock = CompiledBlock_Create();
			if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
				index = Compiler_AddObject(compiler, expr);
				EMIT1(Op_LdObj, +1, index = index);
			}
			return compiledBlock;

		// In general, lists resolve by evaluating their arguments and then passing the results
		// to the function described by the first argument.  There are, however, a few forms which
		// evaluate specially, more forms than McCarthy's Lisp had, but still relatively few overall.
		case SMILE_KIND_LIST:
			list = (SmileList)expr;
			oldSourceLocation = Compiler_SetSourceLocationFromList(compiler, list);

			switch (SMILE_KIND(list->a)) {

				// Indirect invocation, possibly of the form [[$dot obj method] ...], which needs to
				// be compiled as a method call.
				case SMILE_KIND_LIST:
					{
						SmileList subList = (SmileList)(list->a);
						if (SMILE_KIND(subList->a) == SMILE_KIND_SYMBOL
							&& ((SmileSymbol)subList->a)->symbol == SMILE_SPECIAL_SYMBOL__DOT) {
							if (IsStaticInvocation((SmileList)subList->d)) {
								// When 'obj' is known to be a $type object, don't invoke it like a
								// method (passing 'obj' as the first parameter), but simply invoke
								// its function like an ordinary function call instead.
								goto defaultListForm;
							}
							compiledBlock = Compiler_CompileMethodCall(compiler, (SmileList)subList->d, (SmileList)list->d, compileFlags);
							compiler->currentFunction->currentSourceLocation = oldSourceLocation;
							return compiledBlock;
						}
					}
					goto defaultListForm;

				// Either invocation of a known built-in (like [$if] or [$set] or [$scope]), or resolution
				// of a named function in the current scope.
				case SMILE_KIND_SYMBOL:
					if (SMILE_KIND(list->d) != SMILE_KIND_LIST && SMILE_KIND(list->d) != SMILE_KIND_NULL) {
						Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(list, getSourceLocation),
							String_FromC("Cannot compile list: List is not well-formed.")));
						return CompiledBlock_CreateError();
					}
					if ((compiledBlock = Compiler_CompileStandardForm(compiler, ((SmileSymbol)list->a)->symbol, (SmileList)list->d, compileFlags)) != NULL) {
						compiler->currentFunction->currentSourceLocation = oldSourceLocation;
						return compiledBlock;
					}
					// ...fall-thru to default case...

				// Resolve the given expression, and then call it, passing the rest of the list as
				// arguments (after they have been evaluated).
				default:
				defaultListForm:
					compiledBlock = CompiledBlock_Create();

					// Resolve each element of the list.  The first element will become the function,
					// and the rest will become the arguments.
					argCount = 0;
					for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list), argCount++) {
						CompiledBlock argumentBlock;
						Compiler_SetSourceLocationFromList(compiler, list);
						argumentBlock = Compiler_CompileExpr(compiler, list->a, compileFlags & ~COMPILE_FLAG_NORESULT);
						Compiler_EmitRequireResult(compiler, argumentBlock);
						CompiledBlock_AppendChild(compiledBlock, argumentBlock);
					}

					Compiler_RevertSourceLocation(compiler, oldSourceLocation);

					if (argCount == 0) {
						// [] just becomes null (or possibly nothing, if no result is desired).
						if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
							EMIT0(Op_LdNull, +1);
						}
						compiler->currentFunction->currentSourceLocation = oldSourceLocation;
						return compiledBlock;
					}

					// Under the hood, Call does some magic:
					//   1.  If the would-be function is actually a function, it is called directly;
					//   2.  Otherwise, if it is not a function, but it is an object that has an 'fn' method,
					//        then that method is called instead;
					//   3.  Otherwise, if it does not have an 'fn' method, then Call attempts to invoke
					//        [x.does-not-understand `fn ...] on it.
					//   4.  Otherwise, if it does not have a 'does-not-understand' method, a run-time exception is thrown.
					EMIT1(Op_Call, +1 - argCount, index = argCount - 1);
					compiler->currentFunction->currentSourceLocation = oldSourceLocation;
					Compiler_PopIfNecessary(compiler, compiledBlock, compileFlags);
					return compiledBlock;
			}

		default:
			Smile_Abort_FatalError(String_ToC(String_Format("Cannot compile unknown/unsupported object type 0x%02X.", SMILE_KIND(expr))));
	}
}
