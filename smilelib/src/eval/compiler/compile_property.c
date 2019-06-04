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

Bool Compiler_ValidateIndexArgs(Compiler compiler, SmileList indexArgs)
{
	Int numArgs;

	numArgs = SmileList_SafeLength(indexArgs);

	if (numArgs < 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(indexArgs, getSourceLocation),
			String_FromC("Cannot compile $index expression; it is malformed.")));
		return False;
	}

	if (numArgs < 2) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(indexArgs, getSourceLocation),
			String_FromC("Cannot compile $index expression; it has fewer than the required two arguments.")));
		return False;
	}

	if (numArgs > 2) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(indexArgs, getSourceLocation),
			String_FromC("Cannot compile $index expression; it has more than the required two arguments.")));
		return False;
	}

	return True;
}

Bool Compiler_ValidateDotArgs(Compiler compiler, SmileList dotArgs)
{
	Int numArgs;

	numArgs = SmileList_SafeLength(dotArgs);

	if (numArgs < 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(dotArgs, getSourceLocation),
			String_FromC("Cannot compile $dot expression; it is malformed.")));
		return False;
	}

	if (numArgs < 2) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(dotArgs, getSourceLocation),
			String_FromC("Cannot compile $dot expression; it has fewer than the required two arguments.")));
		return False;
	}

	if (numArgs > 2) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(dotArgs, getSourceLocation),
			String_FromC("Cannot compile $dot expression; it has more than the required two arguments.")));
		return False;
	}

	if (SMILE_KIND(LIST_SECOND(dotArgs)) != SMILE_KIND_SYMBOL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(dotArgs, getSourceLocation),
			String_FromC("Cannot compile $dot: second argument must be a symbol.")));
		return False;
	}

	return True;
}

// Form: [$dot expr symbol]
CompiledBlock Compiler_CompileLoadProperty(Compiler compiler, SmileList dotArgs, CompileFlags compileFlags)
{
	Symbol symbol;
	IntermediateInstruction instr;
	CompiledBlock compiledBlock, childBlock;

	if (!Compiler_ValidateDotArgs(compiler, dotArgs))
		return CompiledBlock_CreateError();

	// Evaluate the left side first, which will leave the left side on the stack.
	compiledBlock = CompiledBlock_Create();
	childBlock = Compiler_CompileExpr(compiler, LIST_FIRST(dotArgs), 0);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_EmitRequireResult(compiler, compiledBlock);

	// Extract the property named by the symbol on the right side, leaving the property's value on the stack.
	symbol = ((SmileSymbol)LIST_SECOND(dotArgs))->symbol;
	if (compileFlags & COMPILE_FLAG_NORESULT) {
		// If they don't want anything, then don't even bother to dereference the data, and
		// discard anything we have computed so far.
		EMIT0(Op_Pop1, -1);
	}
	else {
		// If this is one of the special common properties of one of the built-in core shapes,
		// emit a short property-load instruction for it.  Otherwise, emit a general-purpose
		// propery-load instruction. 
		if (symbol == Smile_KnownSymbols.a) {
			EMIT0(Op_LdA, -1 + 1);
		}
		else if (symbol == Smile_KnownSymbols.d) {
			EMIT0(Op_LdD, -1 + 1);
		}
		else if (symbol == Smile_KnownSymbols.start) {
			EMIT0(Op_LdStart, -1 + 1);
		}
		else if (symbol == Smile_KnownSymbols.end) {
			EMIT0(Op_LdEnd, -1 + 1);
		}
		else if (symbol == Smile_KnownSymbols.count) {
			EMIT0(Op_LdCount, -1 + 1);
		}
		else if (symbol == Smile_KnownSymbols.length) {
			EMIT0(Op_LdLength, -1 + 1);
		}
		else {
			EMIT1(Op_LdProp, -1 + 1, symbol = symbol);
		}
	}

	return compiledBlock;
}

// Form: [$index expr index]
CompiledBlock Compiler_CompileLoadMember(Compiler compiler, SmileList indexArgs, CompileFlags compileFlags)
{
	CompiledBlock compiledBlock, childBlock;

	if (!Compiler_ValidateIndexArgs(compiler, indexArgs))
		return CompiledBlock_CreateError();

	// Evaluate the left side first, which will leave the left side on the stack.
	compiledBlock = CompiledBlock_Create();
	childBlock = Compiler_CompileExpr(compiler, LIST_FIRST(indexArgs), 0);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_EmitRequireResult(compiler, compiledBlock);

	// Evaluate the index next, which leaves the index above it on the stack.
	childBlock = Compiler_CompileExpr(compiler, LIST_SECOND(indexArgs), 0);
	CompiledBlock_AppendChild(compiledBlock, childBlock);
	Compiler_EmitRequireResult(compiler, compiledBlock);

	// Now call the object's get-member method (assuming it has such a method).
	EMIT0(Op_LdMember, -1);

	if (compileFlags & COMPILE_FLAG_NORESULT) {
		// If they don't want anything, then discard the result of the call.
		EMIT0(Op_Pop1, -1);
	}

	return compiledBlock;
}
