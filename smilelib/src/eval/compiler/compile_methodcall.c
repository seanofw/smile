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
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

CompiledBlock Compiler_CompileMethodCall(Compiler compiler, SmileList dotArgs, SmileList args, CompileFlags compileFlags)
{
	Int length;
	SmileList temp;
	Symbol symbol;
	Int oldSourceLocation = compiler->currentFunction->currentSourceLocation;
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;

	// First, make sure the args are well-formed, and count how many of them there are.
	length = SmileList_Length(args);

	// Now make sure the [$dot] form is well-formed.
	if (!Compiler_ValidateDotArgs(compiler, dotArgs))
		return CompiledBlock_CreateError();

	// Make sure this is a valid method call form.
	if (length < 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, NULL,
			String_FromC("Cannot compile method call: Argument list is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	compiledBlock = CompiledBlock_Create();

	// Evaluate the left side of the pair (the object to invoke).
	Compiler_SetSourceLocationFromList(compiler, dotArgs);
	childBlock = Compiler_CompileExpr(compiler, LIST_FIRST(dotArgs), compileFlags & ~COMPILE_FLAG_NORESULT);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_EmitRequireResult(compiler, compiledBlock);
	symbol = ((SmileSymbol)LIST_SECOND(dotArgs))->symbol;

	// Evaluate all of the arguments.
	for (temp = args; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
		Compiler_SetSourceLocationFromList(compiler, temp);
		childBlock = Compiler_CompileExpr(compiler, temp->a, compileFlags & ~COMPILE_FLAG_NORESULT);
		CompiledBlock_AppendChild(compiledBlock, childBlock);
		Compiler_EmitRequireResult(compiler, compiledBlock);
	}

	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	// Invoke the method by symbol name.
	if (length <= 7) {
		// If this is the special get-member method, use the special fast instruction for that.
		if (length == 1 && symbol == Smile_KnownSymbols.get_member) {
			EMIT0(Op_LdMember, -2 + 1);
		}
		else {
			// Use a short form.
			EMIT1(Op_Met0 + length, -(length + 1) + 1, symbol = symbol);
		}
	}
	else {
		EMIT2(Op_Met, -(length + 1) + 1, i2.a = (Int32)length, i2.b = (Int32)symbol);
	}

	// If no result is desired, just discard whatever the method returned.
	Compiler_PopIfNecessary(compiler, compiledBlock, compileFlags);

	return compiledBlock;
}
