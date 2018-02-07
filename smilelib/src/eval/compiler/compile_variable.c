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

/// <summary>
/// Compile a variable load where we don't know what this is, or it's explictly global,
/// so it comes from a dictionary read/write from the current global closure.
/// </summary>
Inline CompiledBlock Compiler_CompileLoadGlobalVariable(Compiler compiler, Symbol symbol, CompiledLocalSymbol localSymbol, CompileFlags compileFlags)
{
	IntermediateInstruction instr;
	CompiledBlock compiledBlock = CompiledBlock_Create();

	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		if (symbol == Smile_KnownSymbols.true_) {
			EMIT1(Op_LdBool, +1, boolean = True);
		}
		else if (symbol == Smile_KnownSymbols.false_) {
			EMIT1(Op_LdBool, +1, boolean = False);
		}
		else if (symbol == Smile_KnownSymbols.null_) {
			EMIT0(Op_LdNull, +1);
		}
		else {
			EMIT1(Op_LdX, +1, symbol = symbol);
		}
		if (localSymbol != NULL)
			localSymbol->wasReadDeep = True;
	}

	return compiledBlock;
}

/// <summary>
/// Compile a variable store where we don't know what this is, or it's explictly global,
/// so it comes from a dictionary read/write from the current global closure.
/// </summary>
Inline void Compiler_CompileStoreGlobalVariable(Compiler compiler, Symbol symbol, CompiledLocalSymbol localSymbol, CompileFlags compileFlags, CompiledBlock compiledBlock)
{
	IntermediateInstruction instr;

	if (symbol == Smile_KnownSymbols.true_ || symbol == Smile_KnownSymbols.false_ || symbol == Smile_KnownSymbols.null_) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, NULL,
			String_Format("{0} is a constant, read-only global and cannot be assigned to.",
				SymbolTable_GetName(Smile_SymbolTable, symbol))));
		return;
	}

	if (compileFlags & COMPILE_FLAG_NORESULT) {
		EMIT1(Op_StpX, -1, symbol = symbol);
		if (localSymbol != NULL)
			localSymbol->wasWrittenDeep = True;
	}
	else {
		EMIT1(Op_StX, 0, symbol = symbol);
		if (localSymbol != NULL)
			localSymbol->wasWrittenDeep = True;
	}
}

/// <summary>
/// Compile a variable load to an argument of a function, either this function, or one of its ancestors.
/// </summary>
Inline CompiledBlock Compiler_CompileLoadArgument(Compiler compiler, CompiledLocalSymbol localSymbol, CompileFlags compileFlags)
{
	IntermediateInstruction instr;
	Int functionDepth = compiler->currentFunction->functionDepth - localSymbol->scope->function->functionDepth;
	CompiledBlock compiledBlock = CompiledBlock_Create();

	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		if (functionDepth <= 7) {
			localSymbol->wasReadDeep = True;
			EMIT1(Op_LdArg0 + functionDepth, +1, index = localSymbol->index);
		}
		else {
			if (functionDepth == 0) localSymbol->wasRead = True;
			else localSymbol->wasReadDeep = True;
			EMIT2(Op_LdArg, +1, i2.a = (Int32)functionDepth, i2.b = (Int32)localSymbol->index);
		}
	}

	return compiledBlock;
}

/// <summary>
/// Compile a variable store to an argument of a function, either this function, or one of its ancestors.
/// </summary>
Inline void Compiler_CompileStoreArgument(Compiler compiler, CompiledLocalSymbol localSymbol, CompileFlags compileFlags, CompiledBlock compiledBlock)
{
	IntermediateInstruction instr;
	Int functionDepth = compiler->currentFunction->functionDepth - localSymbol->scope->function->functionDepth;

	if (functionDepth <= 7) {
		if (compileFlags & COMPILE_FLAG_NORESULT) {
			if (functionDepth == 0) localSymbol->wasWritten = True;
			else localSymbol->wasWrittenDeep = True;
			EMIT1(Op_StpArg0 + functionDepth, -1, index = localSymbol->index);
		}
		else {
			if (functionDepth == 0) localSymbol->wasWritten = True;
			else localSymbol->wasWrittenDeep = True;
			EMIT1(Op_StArg0 + functionDepth, 0, index = localSymbol->index);
		}
	}
	else {
		if (compileFlags & COMPILE_FLAG_NORESULT) {
			localSymbol->wasWrittenDeep = True;
			EMIT2(Op_StpArg, -1, i2.a = (Int32)functionDepth, i2.b = (Int32)localSymbol->index);
		}
		else {
			localSymbol->wasWrittenDeep = True;
			EMIT2(Op_StArg, 0, i2.a = (Int32)functionDepth, i2.b = (Int32)localSymbol->index);
		}
	}
}

/// <summary>
/// Compile a variable load to a local variable of a function, either this function, or one of its ancestors.
/// </summary>
Inline CompiledBlock Compiler_CompileLoadLocalVariable(Compiler compiler, CompiledLocalSymbol localSymbol, CompileFlags compileFlags)
{
	IntermediateInstruction instr;
	Int functionDepth = compiler->currentFunction->functionDepth - localSymbol->scope->function->functionDepth;
	CompiledBlock compiledBlock = CompiledBlock_Create();

	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		if (functionDepth <= 7) {
			localSymbol->wasReadDeep = True;
			EMIT1(Op_LdLoc0 + functionDepth, +1, index = localSymbol->index);
		}
		else {
			if (functionDepth == 0) localSymbol->wasRead = True;
			else localSymbol->wasReadDeep = True;
			EMIT2(Op_LdLoc, +1, i2.a = (Int32)functionDepth, i2.b = (Int32)localSymbol->index);
		}
	}

	return compiledBlock;
}

/// <summary>
/// Compile a variable store to a local variable of a function, either this function, or one of its ancestors.
/// </summary>
Inline void Compiler_CompileStoreLocalVariable(Compiler compiler, CompiledLocalSymbol localSymbol, CompileFlags compileFlags, CompiledBlock compiledBlock)
{
	IntermediateInstruction instr;
	Int functionDepth = compiler->currentFunction->functionDepth - localSymbol->scope->function->functionDepth;

	if (functionDepth <= 7) {
		if (compileFlags & COMPILE_FLAG_NORESULT) {
			if (functionDepth == 0) localSymbol->wasWritten = True;
			else localSymbol->wasWrittenDeep = True;
			EMIT1(Op_StpLoc0 + functionDepth, -1, index = localSymbol->index);
		}
		else {
			if (functionDepth == 0) localSymbol->wasWritten = True;
			else localSymbol->wasWrittenDeep = True;
			EMIT1(Op_StLoc0 + functionDepth, 0, index = localSymbol->index);
		}
	}
	else {
		if (compileFlags & COMPILE_FLAG_NORESULT) {
			localSymbol->wasWrittenDeep = True;
			EMIT2(Op_StpLoc, -1, i2.a = (Int32)functionDepth, i2.b = (Int32)localSymbol->index);
		}
		else {
			localSymbol->wasWrittenDeep = True;
			EMIT2(Op_StLoc, 0, i2.a = (Int32)functionDepth, i2.b = (Int32)localSymbol->index);
		}
	}
}

/// <summary>
/// Compile an invocation of a 'till' flag, which causes an immediate branch/escape to the appropriate
/// 'when'/'exit' clause of the 'till'.
/// </summary>
Inline CompiledBlock Compiler_CompileTillFlag(Compiler compiler, CompiledLocalSymbol localSymbol, CompileFlags compileFlags)
{
	CompiledTillSymbol tillSymbol;
	IntermediateInstruction instr, jmpInstr;
	Int functionDepth = compiler->currentFunction->functionDepth - localSymbol->scope->function->functionDepth;
	CompiledBlock compiledBlock = CompiledBlock_Create();

	UNUSED(compileFlags);

	tillSymbol = (CompiledTillSymbol)localSymbol;

	if (functionDepth == 0) {
		// Special case:  This is a flag triggered in the same function, so we
		// can use a simple Jmp to escape the till loop.

		// Emit the jump instruction, pointing at the when block.
		jmpInstr = EMIT0(Op_Jmp, 0);
		jmpInstr->p.branchTarget = tillSymbol->whenLabel;

		localSymbol->wasRead = True;
	}
	else {
		// General case:  We need to use the till loop's escape continuation, since
		// we're inside a nested function.  First, load the escape continuation itself
		// onto the stack.  (Every flag will reference the same escape continuation, so
		// the "simple" way of loading a local variable still works.)
		if (functionDepth <= 7) {
			EMIT1(Op_LdLoc0 + functionDepth, +1, index = localSymbol->index);
		}
		else {
			EMIT2(Op_LdLoc, +1, i2.a = (Int32)functionDepth, i2.b = (Int32)localSymbol->index);
		}
		localSymbol->wasReadDeep = True;

		// Now emit the special "till-escape" instruction to invoke the continuation.
		EMIT1(Op_TillEsc, -1, index = tillSymbol->tillIndex);
	}

	// No matter what was asked for, we return nothing, because we escaped.
	compiledBlock->blockFlags |= BLOCK_FLAG_ESCAPE;
	return compiledBlock;
}

// Form: symbol
CompiledBlock Compiler_CompileLoadVariable(Compiler compiler, Symbol symbol, CompileFlags compileFlags)
{
	CompiledLocalSymbol localSymbol = CompileScope_FindSymbol(compiler->currentScope, symbol);

	if (localSymbol == NULL || localSymbol->kind == PARSEDECL_GLOBAL || localSymbol->kind == PARSEDECL_PRIMITIVE) {
		return Compiler_CompileLoadGlobalVariable(compiler, symbol, localSymbol, compileFlags);
	}

	switch (localSymbol->kind) {
		case PARSEDECL_ARGUMENT:
			return Compiler_CompileLoadArgument(compiler, localSymbol, compileFlags);

		case PARSEDECL_VARIABLE:
			return Compiler_CompileLoadLocalVariable(compiler, localSymbol, compileFlags);

		case PARSEDECL_TILL:
			return Compiler_CompileTillFlag(compiler, localSymbol, compileFlags);

		default:
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, NULL,
				String_FromC("Cannot compile symbol:  Fatal internal error.")));
			return CompiledBlock_CreateError();
	}
}

// Form: symbol
void Compiler_CompileStoreVariable(Compiler compiler, Symbol symbol, CompileFlags compileFlags, CompiledBlock compiledBlock)
{
	CompiledLocalSymbol localSymbol = CompileScope_FindSymbol(compiler->currentScope, symbol);

	if (localSymbol == NULL || localSymbol->kind == PARSEDECL_GLOBAL || localSymbol->kind == PARSEDECL_PRIMITIVE) {
		Compiler_CompileStoreGlobalVariable(compiler, symbol, localSymbol, compileFlags, compiledBlock);
		return;
	}

	switch (localSymbol->kind) {
		case PARSEDECL_ARGUMENT:
			Compiler_CompileStoreArgument(compiler, localSymbol, compileFlags, compiledBlock);
			break;

		case PARSEDECL_VARIABLE:
			Compiler_CompileStoreLocalVariable(compiler, localSymbol, compileFlags, compiledBlock);
			break;

		case PARSEDECL_TILL:
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, NULL,
				String_Format("Cannot assign a value to till-flag \"%S\".",
					SymbolTable_GetName(Smile_SymbolTable, symbol))));
			break;

		default:
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, NULL,
				String_FromC("Cannot compile symbol:  Fatal internal error.")));
			compiledBlock->blockFlags |= BLOCK_FLAG_ERROR;
			break;
	}
}