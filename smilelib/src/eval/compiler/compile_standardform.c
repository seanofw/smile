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

static CompiledBlock Compiler_CompileOneArgument(Compiler compiler, SmileList args, Int compileFlags, const char *name, Int op);
static CompiledBlock Compiler_CompileTwoArguments(Compiler compiler, SmileList args, Int compileFlags, const char *name, Int op);

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
/// <param name="compileFlags">Flags controlling the desired compile behavior.</param>
/// <param name="result">Metadata about what was compiled.</param>
/// <returns>True if this form is known and compiling was attempted (even if it resulted in an error);
/// false if the symbol is unknown.<returns>
CompiledBlock Compiler_CompileStandardForm(Compiler compiler, Symbol symbol, SmileList args, CompileFlags compileFlags)
{
	switch (symbol) {

		// Assignment.
		case SMILE_SPECIAL_SYMBOL__SET:
			return Compiler_CompileSetf(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__OPSET:
			return Compiler_CompileOpEquals(compiler, args, compileFlags);

		// Control flow.
		case SMILE_SPECIAL_SYMBOL__IF:
			return Compiler_CompileIf(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__WHILE:
			return Compiler_CompileWhile(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__TILL:
			return Compiler_CompileTill(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__CATCH:
			return Compiler_CompileCatch(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__RETURN:
			return Compiler_CompileReturn(compiler, args, compileFlags);

		// Expression-evaluation control.
		case SMILE_SPECIAL_SYMBOL__FN:
			return Compiler_CompileFn(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__QUOTE:
			return Compiler_CompileQuote(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__PROG1:
			return Compiler_CompileProg1(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__PROGN:
			return Compiler_CompileProgN(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__SCOPE:
			return Compiler_CompileScope(compiler, args, compileFlags);

		// Object creation and type testing.
		case SMILE_SPECIAL_SYMBOL__NEW:
			return Compiler_CompileNew(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__DOT:
			return Compiler_CompileDot(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__IS:
			return Compiler_CompileTwoArguments(compiler, args, compileFlags, "$is", Op_Is);
		case SMILE_SPECIAL_SYMBOL__TYPEOF:
			return Compiler_CompileOneArgument(compiler, args, compileFlags, "$typeof", Op_TypeOf);

		// Reference comparison.
		case SMILE_SPECIAL_SYMBOL__EQ:
			return Compiler_CompileTwoArguments(compiler, args, compileFlags, "===", Op_SuperEq);
		case SMILE_SPECIAL_SYMBOL__NE:
			return Compiler_CompileTwoArguments(compiler, args, compileFlags, "!==", Op_SuperNe);

		// Logical operations.
		case SMILE_SPECIAL_SYMBOL__AND:
			return Compiler_CompileAnd(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__OR:
			return Compiler_CompileOr(compiler, args, compileFlags);
		case SMILE_SPECIAL_SYMBOL__NOT:
			return Compiler_CompileOneArgument(compiler, args, compileFlags, "$not", Op_Not);

		// Don't know what this is, so it's likely an evaluation of whatever's in scope.
		default:
			return NULL;
	}
}

/// <summary>
/// Compile the argument to a form that takes exactly one argument and has no special type requirements
/// or macro-like behaviors.
/// </summary>
/// <param name="compiler">The compiler that is compiling this form.</param>
/// <param name="args">The arguments to this form (not including the name of the form itself).</param>
/// <param name="name">The name of this form (for error-reporting).</param>
/// <param name="op">The Op_* instruction to emit.</param>
/// <returns>The original source location, which should be reverted to after emitting the opcode(s) for this form.<returns>
static CompiledBlock Compiler_CompileOneArgument(Compiler compiler, SmileList args, Int compileFlags, const char *name, Int op)
{
	Int oldSourceLocation;
	CompiledBlock compiledBlock, childBlock;

	// Must be an expression of the form [op x].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_Format("Cannot compile [%s]: Expression is not well-formed.", name)));
		return CompiledBlock_CreateError();
	}

	// Return value is a derivative value, so anything we may construct in the next expression has no name.
	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);

	// Make sure the first argument's instructions get tagged with the correct source line.
	Compiler_SetSourceLocationFromList(compiler, args);

	// Compile the expression.
	compiledBlock = CompiledBlock_Create();
	childBlock = Compiler_CompileExpr(compiler, args->a, compileFlags);
	Compiler_MakeStackMatchCompileFlags(compiler, childBlock, compileFlags);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Make sure the operation instruction is tagged with the correct source line, and with the original assignment symbol.
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	Compiler_SetSourceLocationFromList(compiler, args);
	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		EMIT0(op, -1 + 1);
	}
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	return compiledBlock;
}

/// <summary>
/// Compile the arguments to a form that takes exactly two arguments and has no special type requirements
/// or macro-like behaviors.
/// </summary>
/// <param name="compiler">The compiler that is compiling this form.</param>
/// <param name="args">The arguments to this form (not including the name of the form itself).</param>
/// <param name="name">The name of this form (for error-reporting).</param>
/// <returns>The original source location, which should be reverted to after emitting the opcode(s) for this form.<returns>
static CompiledBlock Compiler_CompileTwoArguments(Compiler compiler, SmileList args, Int compileFlags, const char *name, Int op)
{
	Int oldSourceLocation;
	CompiledBlock compiledBlock, childBlock;

	// Must be an expression of the form [op x y].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST
		|| SMILE_KIND(((SmileList)args->d)->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_Format("Cannot compile [%s]: Expression is not well-formed.", name)));
		return CompiledBlock_CreateError();
	}

	// Return value is a derivative value, so anything we may construct in the next expression has no name.
	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);

	// Make sure the first argument's instructions get tagged with the correct source line.
	Compiler_SetSourceLocationFromList(compiler, args);

	compiledBlock = CompiledBlock_Create();

	// Compile the first expression.
	childBlock = Compiler_CompileExpr(compiler, args->a, compileFlags);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_MakeStackMatchCompileFlags(compiler, childBlock, compileFlags);

	// Make sure the second argument's instructions get tagged with the correct source line.
	Compiler_SetSourceLocationFromList(compiler, (SmileList)args->d);

	// Compile the second expression.
	childBlock = Compiler_CompileExpr(compiler, ((SmileList)args->d)->a, compileFlags);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_MakeStackMatchCompileFlags(compiler, childBlock, compileFlags);

	// Make sure the operation instruction is tagged with the correct source line, and with the original assignment symbol.
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	Compiler_SetSourceLocationFromList(compiler, args);
	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		EMIT0(op, -2 + 1);
	}
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	return compiledBlock;
}
