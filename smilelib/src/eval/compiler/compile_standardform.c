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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static Int Compiler_CompileOneArgument(Compiler compiler, SmileList args, const char *name);
static Int Compiler_CompileTwoArguments(Compiler compiler, SmileList args, const char *name);

/// <summary>
/// Compile the standard 20 well-known forms, like [$set] and [$fn] and [$quote], if possible.
/// If the given symbol matches the name of a well-known form, then compile that form and return
/// true; if it's the name of an unknown form (i.e., likely a call to a user function), do nothing,
/// and return false.  In most cases of known forms, this just dispatches to a special function
/// that knows how to *actually* compile that form.
/// </summary>
/// <param name="compiler">The compiler that is compiling this form.</param>
/// <param name="args">The arguments to this form (not including the name of the form itself).</param>
/// <param name="symbol">The symbolic name of this form.</param>
/// <returns>True if this form is known and compiling was attempted (even if it resulted in an error);
/// false if the symbol is unknown.<returns>
Bool Compiler_CompileStandardForm(Compiler compiler, Symbol symbol, SmileList args)
{
	Int offset;
	Int oldSourceLocation;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;

	switch (symbol) {

		// Assignment.
		case SMILE_SPECIAL_SYMBOL__SET:
			Compiler_CompileSetf(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__OPSET:
			Compiler_CompileOpEquals(compiler, args);
			return True;

		// Control flow.
		case SMILE_SPECIAL_SYMBOL__IF:
			Compiler_CompileIf(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__WHILE:
			Compiler_CompileWhile(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__TILL:
			Compiler_CompileTill(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__CATCH:
			Compiler_CompileCatch(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__RETURN:
			Compiler_CompileReturn(compiler, args);
			return True;

		// Expression-evaluation control.
		case SMILE_SPECIAL_SYMBOL__FN:
			Compiler_CompileFn(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__QUOTE:
			Compiler_CompileQuote(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__PROG1:
			Compiler_CompileProg1(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__PROGN:
			Compiler_CompileProgN(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__SCOPE:
			Compiler_CompileScope(compiler, args);
			return True;

		// Object creation and type testing.
		case SMILE_SPECIAL_SYMBOL__NEW:
			Compiler_CompileNew(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__IS:
			oldSourceLocation = Compiler_CompileTwoArguments(compiler, args, "$is");
			EMIT0(Op_Is, -2 + 1);
			Compiler_RevertSourceLocation(compiler, oldSourceLocation);
			return True;
		case SMILE_SPECIAL_SYMBOL__TYPEOF:
			oldSourceLocation = Compiler_CompileOneArgument(compiler, args, "$typeof");
			EMIT0(Op_TypeOf, -1 + 1);
			Compiler_RevertSourceLocation(compiler, oldSourceLocation);
			return True;

		// Reference comparison.
		case SMILE_SPECIAL_SYMBOL__EQ:
			oldSourceLocation = Compiler_CompileTwoArguments(compiler, args, "===");
			EMIT0(Op_SuperEq, -2 + 1);
			Compiler_RevertSourceLocation(compiler, oldSourceLocation);
			return True;
		case SMILE_SPECIAL_SYMBOL__NE:
			oldSourceLocation = Compiler_CompileTwoArguments(compiler, args, "!==");
			EMIT0(Op_SuperNe, -2 + 1);
			Compiler_RevertSourceLocation(compiler, oldSourceLocation);
			return True;

		// Logical operations.
		case SMILE_SPECIAL_SYMBOL__AND:
			Compiler_CompileAnd(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__OR:
			Compiler_CompileOr(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__NOT:
			oldSourceLocation = Compiler_CompileOneArgument(compiler, args, "$not");
			EMIT0(Op_Not, -1 + 1);
			Compiler_RevertSourceLocation(compiler, oldSourceLocation);
			return True;

		// Don't know what this is, so it's likely an evaluation of whatever's in scope.
		default:
			return False;
	}
}

/// <summary>
/// Compile the argument to a form that takes exactly one argument and has no special type requirements
/// or macro-like behaviors.
/// </summary>
/// <param name="compiler">The compiler that is compiling this form.</param>
/// <param name="args">The arguments to this form (not including the name of the form itself).</param>
/// <param name="name">The name of this form (for error-reporting).</param>
/// <returns>The original source location, which should be reverted to after emitting the opcode(s) for this form.<returns>
static Int Compiler_CompileOneArgument(Compiler compiler, SmileList args, const char *name)
{
	Int oldSourceLocation;

	// Must be an expression of the form [op x].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_Format("Cannot compile [%s]: Expression is not well-formed.", name)));
		return compiler->currentFunction->currentSourceLocation;
	}

	// Return value is a derivative value, so anything we may construct in the next expression has no name.
	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);

	// Make sure the first argument's instructions get tagged with the correct source line.
	Compiler_SetSourceLocationFromList(compiler, args);

	// Compile the expression.
	Compiler_CompileExpr(compiler, args->a);

	// Make sure the operation instruction is tagged with the correct source line, and with the original assignment symbol.
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	Compiler_SetSourceLocationFromList(compiler, args);

	return oldSourceLocation;
}

/// <summary>
/// Compile the arguments to a form that takes exactly two arguments and has no special type requirements
/// or macro-like behaviors.
/// </summary>
/// <param name="compiler">The compiler that is compiling this form.</param>
/// <param name="args">The arguments to this form (not including the name of the form itself).</param>
/// <param name="name">The name of this form (for error-reporting).</param>
/// <returns>The original source location, which should be reverted to after emitting the opcode(s) for this form.<returns>
static Int Compiler_CompileTwoArguments(Compiler compiler, SmileList args, const char *name)
{
	Int oldSourceLocation;

	// Must be an expression of the form [op x y].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST
		|| SMILE_KIND(((SmileList)args->d)->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_Format("Cannot compile [%s]: Expression is not well-formed.", name)));
		return compiler->currentFunction->currentSourceLocation;
	}

	// Return value is a derivative value, so anything we may construct in the next expression has no name.
	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);

	// Make sure the first argument's instructions get tagged with the correct source line.
	Compiler_SetSourceLocationFromList(compiler, args);

	// Compile the first expression.
	Compiler_CompileExpr(compiler, args->a);

	// Make sure the second argument's instructions get tagged with the correct source line.
	Compiler_SetSourceLocationFromList(compiler, (SmileList)args->d);

	// Compile the second expression.
	Compiler_CompileExpr(compiler, ((SmileList)args->d)->a);

	// Make sure the operation instruction is tagged with the correct source line, and with the original assignment symbol.
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	Compiler_SetSourceLocationFromList(compiler, args);

	return oldSourceLocation;
}
