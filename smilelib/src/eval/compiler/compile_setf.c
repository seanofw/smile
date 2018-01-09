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

// Form [$set symbol value].
Inline CompiledBlock Compiler_CompileSetSymbol(Compiler compiler, SmileList args,
	SmileSymbol symbol, SmileObject value, CompileFlags compileFlags)
{
	Int oldSourceLocation;
	CompiledBlock compiledBlock, childBlock;

	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, symbol);
	Compiler_SetSourceLocationFromList(compiler, (SmileList)args->d);

	// Load the value to store.
	compiledBlock = CompiledBlock_Create();
	childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Store it, leaving a duplicate on the stack.
	Compiler_SetSourceLocationFromList(compiler, args);
	Compiler_CompileStoreVariable(compiler, symbol, compileFlags, compiledBlock);
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	return compiledBlock;
}

// Form [$set [$dot object symbol] value].
Inline CompiledBlock Compiler_CompileSetProperty(Compiler compiler, SmileList args,
	SmileList dotArgs, SmileObject value, CompileFlags compileFlags)
{
	Symbol symbol;
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;
	Int oldSourceLocation = compiler->currentFunction->currentSourceLocation;

	symbol = ((SmileSymbol)LIST_SECOND(dotArgs))->symbol;

	Compiler_SetSourceLocationFromList(compiler, (SmileList)args->d);

	// Evaluate the left side first.
	compiledBlock = CompiledBlock_Create();
	childBlock = Compiler_CompileExpr(compiler, LIST_FIRST(dotArgs), compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	Compiler_SetAssignedSymbol(compiler, symbol);
	Compiler_SetSourceLocationFromList(compiler, args);

	// Evaluate the value second.  (Doing this second ensures that everything always
	// evaluates left-to-right, the order in which it was written.)
	childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Assign the property.
	if (compileFlags & COMPILE_FLAG_NORESULT) {
		EMIT1(Op_StpProp, -2, symbol = symbol);
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
		return compiledBlock;
	}
	else {
		EMIT1(Op_StProp, -1, symbol = symbol);
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
		return compiledBlock;
	}
}

// Form [$set [[$dot object get-member] index] value].
Inline CompiledBlock Compiler_CompileSetMember(Compiler compiler, SmileList args,
	SmileList dest, SmileList dotArgs, SmileObject value, CompileFlags compileFlags)
{
	Symbol symbol;
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;
	Int oldSourceLocation = compiler->currentFunction->currentSourceLocation;
	SmileObject index;

	// This is probably of the form [$set [[$dot obj get-member] index] value].  Make sure that the
	// inner list is well-formed, has two elements, the first element is a pair, and the right
	// side of the first element is the special symbol "get-member".
	if (SmileList_Length((SmileList)dest) != 2
		|| ((SmileSymbol)LIST_FIRST(dotArgs))->symbol != SMILE_SPECIAL_SYMBOL_GET_MEMBER) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$set]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	index = LIST_SECOND((SmileList)dest);

	// Okay.  We now have pair->left, pair->right, index, and value.  Let's compile them.
	compiledBlock = CompiledBlock_Create();
	Compiler_SetSourceLocationFromList(compiler, dotArgs);
	childBlock = Compiler_CompileExpr(compiler, LIST_FIRST(dotArgs), compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_SetSourceLocationFromList(compiler, (SmileList)((SmileList)dest)->d);
	childBlock = Compiler_CompileExpr(compiler, index, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_SetSourceLocationFromList(compiler, (SmileList)args->d);
	childBlock = Compiler_CompileExpr(compiler, value, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_SetSourceLocationFromList(compiler, args);

	if (compileFlags & COMPILE_FLAG_NORESULT) {
		EMIT0(Op_StpMember, -3);
		return compiledBlock;
	}
	else {
		EMIT0(Op_LdNull, +1);
		EMIT0(Op_StMember, -2);
		return compiledBlock;
	}
}

// Form: [$set lvalue rvalue]
CompiledBlock Compiler_CompileSetf(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	Int length;
	SmileObject dest, value;
	SmileList dotArgs;

	// There are three possible legal forms for the arguments:
	//
	//   [$set symbol value]
	//   [$set [$dot obj property] value]
	//   [$set [[$dot obj get-member] index] value]
	//
	// We have to determine which one of these we have, and compile an appropriate
	// assignment (or method invocation) accordingly.

	// Make sure this is a well-formed list of exactly two elements.
	length = SmileList_Length(args);
	if (length != 2) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$set]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	// Get the destination object, and the value to be assigned.
	dest = args->a;
	value = ((SmileList)args->d)->a;

	switch (SMILE_KIND(dest)) {

	case SMILE_KIND_SYMBOL:
		// This is of the form [$set symbol value].
		return Compiler_CompileSetSymbol(compiler, args, ((SmileSymbol)dest)->symbol, value, compileFlags);

	case SMILE_KIND_LIST:
		// This is either of these two forms:
		//
		//    - [$set [$dot obj property] value]
		//    - [$set [[$dot obj get-member] index] value]
		//
		// Whichever it is, determine that fact absolutely, and then compile it to the correct behavior.

		if (SMILE_KIND(((SmileList)dest)->a) == SMILE_KIND_SYMBOL
			&& ((SmileSymbol)((SmileList)dest)->a)->symbol == SMILE_SPECIAL_SYMBOL__DOT) {

			// We found [$set [$dot ... ] value].  So this is probably the property-access form.
			// Make sure the [$dot ... ] is well-formed, and then compile this for real.
			dotArgs = LIST_REST((SmileList)dest);
			if (!Compiler_ValidateDotArgs(compiler, dotArgs))
				return CompiledBlock_CreateError();

			return Compiler_CompileSetProperty(compiler, args, dotArgs, value, compileFlags);
		}
		else if (SMILE_KIND(((SmileList)dest)->a) == SMILE_KIND_LIST
			&& SMILE_KIND(((SmileList)((SmileList)dest)->a)->a) == SMILE_KIND_SYMBOL
			&& ((SmileSymbol)((SmileList)((SmileList)dest)->a)->a)->symbol == SMILE_SPECIAL_SYMBOL__DOT) {

			// We found [$set [[$dot ... ] ... ] value].  So this is probably the get-member form.
			// Make sure the [$dot ... ] is well-formed, and then compile this for real.
			dotArgs = LIST_REST(LIST_FIRST((SmileList)dest));
			if (!Compiler_ValidateDotArgs(compiler, dotArgs))
				return CompiledBlock_CreateError();

			return Compiler_CompileSetMember(compiler, args, (SmileList)dest, dotArgs, value, compileFlags);
		}
		else {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
				String_FromC("Cannot compile [$set]: Expression is not well-formed.")));
			return CompiledBlock_CreateError();
		}

	default:
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$set]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}
}
