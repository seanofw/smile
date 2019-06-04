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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

// Form [$opset op symbol value].
Inline CompiledBlock Compiler_CompileOpEqualsSymbol(Compiler compiler,
	Symbol op, SmileSymbol symbol, SmileObject value, CompileFlags compileFlags)
{
	Int oldSourceLocation;
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;

	compiledBlock = CompiledBlock_Create();

	// Load the source variable.
	childBlock = Compiler_CompileLoadVariable(compiler, symbol->symbol, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, symbol->symbol);

	// Load the value to store.
	childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Apply the operator.
	EMIT1(Op_Met1, -2 + 1, symbol = op);

	// Store the result back, leaving a duplicate on the stack.
	Compiler_CompileStoreVariable(compiler, symbol->symbol, compileFlags, compiledBlock);
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	return compiledBlock;
}

// Form [$opset op [$dot object symbol] value].
Inline CompiledBlock Compiler_CompileOpEqualsProperty(Compiler compiler, SmileList args,
	Symbol op, SmileList dotArgs, SmileObject value, CompileFlags compileFlags)
{
	Symbol symbol;
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;
	Int oldSourceLocation;
	
	symbol = ((SmileSymbol)LIST_SECOND(dotArgs))->symbol;

	oldSourceLocation = compiler->currentFunction->currentSourceLocation;
	Compiler_SetSourceLocationFromList(compiler, (SmileList)args->d);

	compiledBlock = CompiledBlock_Create();

	// Evaluate the object first.
	childBlock = Compiler_CompileExpr(compiler, dotArgs->a, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Duplicate it for later.
	EMIT0(Op_Dup1, +1);

	// Load the source property.
	EMIT1(Op_LdProp, 0, symbol = symbol);

	// Evaluate the value.
	childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Apply the operator.
	EMIT1(Op_Met1, -2 + 1, symbol = op);

	// Assign the property.
	if (compileFlags & COMPILE_FLAG_NORESULT) {
		EMIT1(Op_StpProp, -2, symbol = symbol);	// Leaves the value on the stack.
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	}
	else {
		EMIT1(Op_StProp, -1, symbol = symbol);	// Leaves the value on the stack.
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	}

	return compiledBlock;
}

// Form [$opset op [$index obj index] value].
Inline CompiledBlock Compiler_CompileOpEqualsMember(Compiler compiler,
	Symbol op, SmileList dotArgs, SmileObject value, CompileFlags compileFlags)
{
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;
	SmileObject index;
	SmileObject obj;

	obj = LIST_FIRST((SmileList)dotArgs);
	index = LIST_SECOND((SmileList)dotArgs);

	compiledBlock = CompiledBlock_Create();

	// Okay.  We now have obj, index, and value.  Let's compile the get-member call first.
	childBlock = Compiler_CompileExpr(compiler, obj, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	childBlock = Compiler_CompileExpr(compiler, index, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Duplicate obj and index.
	EMIT0(Op_Dup2, +1);
	EMIT0(Op_Dup2, +1);

	// Load the source from the given member.
	EMIT0(Op_LdMember, -2 + 1);

	// Evaluate the value.
	childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Apply the operator.
	EMIT1(Op_Met1, -2 + 1, symbol = op);

	// Store the result.
	if (compileFlags & COMPILE_FLAG_NORESULT) {
		EMIT0(Op_StpMember, -3);
	}
	else {
		EMIT0(Op_LdNull, +1);
		EMIT0(Op_StMember, -2);
	}
	return compiledBlock;
}

// Form: [$opset operator lvalue rvalue]
CompiledBlock Compiler_CompileOpEquals(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	Int length;
	SmileObject dest, value;
	SmileList dotArgs;
	Symbol op;

	// There are three possible legal forms for the arguments:
	//
	//   [$opset operator symbol value]
	//   [$opset operator [$dot obj property] value]
	//   [$opset operator [[$dot obj get-member] index] value]
	//
	// We have to determine which one of these we have, and compile an appropriate
	// assignment (or method invocation) accordingly.

	// Make sure this is a well-formed list of exactly three elements.
	length = SmileList_Length(args);
	if (length != 3) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$opset]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	// Get the operator symbol.
	if (SMILE_KIND(args->a) != SMILE_KIND_SYMBOL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$opset]: First argument must be an operator (method) name.")));
		return CompiledBlock_CreateError();
	}
	op = ((SmileSymbol)args->a)->symbol;
	args = (SmileList)args->d;

	// Get the destination object, and the value to be assigned.
	dest = args->a;
	value = ((SmileList)args->d)->a;

	switch (SMILE_KIND(dest)) {

		case SMILE_KIND_SYMBOL:
			// This is of the form [$opset op symbol value].
			return Compiler_CompileOpEqualsSymbol(compiler, op, ((SmileSymbol)dest), value, compileFlags);

		case SMILE_KIND_LIST:
			// This is either of these two forms:
			//
			//    - [$opset op [$dot obj property] value]
			//    - [$opset op [[$dot obj get-member] index] value]
			//
			// Whichever it is, determine that fact absolutely, and then compile it to the correct behavior.

			if (SMILE_KIND(((SmileList)dest)->a) == SMILE_KIND_SYMBOL
				&& ((SmileSymbol)((SmileList)dest)->a)->symbol == SMILE_SPECIAL_SYMBOL__DOT) {

				// We found [$opset op [$dot ... ] value].  So this is probably the property-access form.
				// Make sure the [$dot ... ] is well-formed, and then compile this for real.
				dotArgs = LIST_REST((SmileList)dest);
				if (!Compiler_ValidateDotArgs(compiler, dotArgs))
					return CompiledBlock_CreateError();

				return Compiler_CompileOpEqualsProperty(compiler, args, op, dotArgs, value, compileFlags);
			}
			else if (SMILE_KIND(((SmileList)dest)->a) == SMILE_KIND_SYMBOL
				&& ((SmileSymbol)((SmileList)dest)->a)->symbol == SMILE_SPECIAL_SYMBOL__INDEX) {

				// We found [$opset op [$index ... ] value].  So this is probably the get-member form.
				// Make sure the [$index ... ] is well-formed, and then compile this for real.
				dotArgs = LIST_REST((SmileList)dest);
				if (!Compiler_ValidateIndexArgs(compiler, dotArgs))
					return CompiledBlock_CreateError();

				return Compiler_CompileOpEqualsMember(compiler, op, dotArgs, value, compileFlags);
			}
			else {
				Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
					String_FromC("Cannot compile [$opset]: Expression is not well-formed.")));
				return CompiledBlock_CreateError();
			}

		default:
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
				String_FromC("Cannot compile [$opset]: Expression is not well-formed.")));
			return CompiledBlock_CreateError();
	}
}
