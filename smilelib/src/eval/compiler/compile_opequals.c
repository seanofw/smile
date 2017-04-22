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

// Form: [$opset operator lvalue rvalue]
CompiledBlock Compiler_CompileOpEquals(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	Int length;
	SmileObject dest, value, index;
	SmilePair pair;
	Symbol symbol, op;
	Int oldSourceLocation;
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;

	// There are three possible legal forms for the arguments:
	//
	//   [$opset operator symbol value]
	//   [$opset operator obj.property value]
	//   [$opset operator [(obj.get-member) index] value]
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
		symbol = ((SmileSymbol)dest)->symbol;

		compiledBlock = CompiledBlock_Create();

		// Load the source variable.
		childBlock = Compiler_CompileLoadVariable(compiler, symbol, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitRequireResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		oldSourceLocation = Compiler_SetAssignedSymbol(compiler, symbol);

		// Load the value to store.
		childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitRequireResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// Apply the operator.
		EMIT1(Op_Met1, -2 + 1, symbol = op);

		// Store the result back, leaving a duplicate on the stack.
		Compiler_CompileStoreVariable(compiler, symbol, compileFlags, compiledBlock);
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
		return compiledBlock;

	case SMILE_KIND_PAIR:
		// This is probably of the form [$opset op obj.property value].  Make sure the right side
		// of the pair is a symbol.
		pair = (SmilePair)dest;
		if (SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
				String_FromC("Cannot compile [$opset]: Expression is not well-formed.")));
			return CompiledBlock_CreateError();
		}
		symbol = ((SmileSymbol)pair->right)->symbol;

		compiledBlock = CompiledBlock_Create();

		// Evaluate the left side first.
		childBlock = Compiler_CompileExpr(compiler, pair->left, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitRequireResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// Duplicate it for later.
		EMIT0(Op_Dup1, +1);

		// Load the source property.
		EMIT1(Op_LdProp, -1, symbol = symbol);

		oldSourceLocation = Compiler_SetAssignedSymbol(compiler, symbol);

		// Evaluate the value.
		childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitRequireResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// Apply the operator.
		EMIT1(Op_Met1, -2 + 1, symbol = op);

		// Assign the property.
		if (compileFlags & COMPILE_FLAG_NORESULT) {
			EMIT1(Op_StpProp, -1, symbol = symbol);	// Leaves the value on the stack.
			Compiler_RevertSourceLocation(compiler, oldSourceLocation);
		}
		else {
			EMIT1(Op_StProp, -1, symbol = symbol);	// Leaves the value on the stack.
			Compiler_RevertSourceLocation(compiler, oldSourceLocation);
		}
		return compiledBlock;

	case SMILE_KIND_LIST:
		// This is probably of the form [$set [(obj.get-member) index] value].  Make sure that the
		// inner list is well-formed, has two elements, the first element is a pair, and the right
		// side of the first element is the special symbol "get-member".
		if (SmileList_Length((SmileList)dest) != 2 || SMILE_KIND(((SmileList)dest)->a) != SMILE_KIND_PAIR) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
				String_FromC("Cannot compile [$opset]: Expression is not well-formed.")));
			return CompiledBlock_CreateError();
		}
		pair = (SmilePair)(((SmileList)dest)->a);
		if (SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
				String_FromC("Cannot compile [$opset]: Expression is not well-formed.")));
			return CompiledBlock_CreateError();
		}
		index = LIST_SECOND((SmileList)dest);

		compiledBlock = CompiledBlock_Create();

		// Okay.  We now have pair->left, pair->right, index, and value.  Let's compile the get-member call first.
		childBlock = Compiler_CompileExpr(compiler, pair->left, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitRequireResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);
		childBlock = Compiler_CompileExpr(compiler, index, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitRequireResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// Duplicate pair->left and index.
		EMIT0(Op_Dup2, +1);
		EMIT0(Op_Dup2, +1);

		// Load the source from the given member.
		EMIT0(Op_LdMember, -3 + 1);

		// Evaluate the value.
		childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitRequireResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// Apply the operator.
		EMIT1(Op_Met1, -2 + 1, symbol = op);

		// Store the result.
		if (compileFlags & COMPILE_FLAG_NORESULT) {
			EMIT0(Op_StpMember, -4);
		}
		else {
			EMIT0(Op_StMember, -3);
		}
		return compiledBlock;

	default:
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$opset]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}
}
