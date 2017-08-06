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

// Form: [$fn [args...] body]
CompiledBlock Compiler_CompileFn(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	CompilerFunction compilerFunction;
	CompileScope scope;
	SmileList functionArgs;
	SmileObject functionBody;
	Int i;
	ClosureInfo closureInfo;
	UserFunctionInfo userFunctionInfo;
	Int functionIndex;
	String errorMessage;
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;
	ByteCodeSegment byteCodeSegment;

	Int oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);

	// The [$fn] expression must be of the form:  [$fn [args...] body].
	if (SMILE_KIND(args) != SMILE_KIND_LIST
		|| (SMILE_KIND(args->a) != SMILE_KIND_LIST && SMILE_KIND(args->a) != SMILE_KIND_NULL)
		|| SMILE_KIND(args->d) != SMILE_KIND_LIST || SMILE_KIND(((SmileList)args->d)->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$fn]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	// Create the function.
	functionArgs = (SmileList)args->a;
	functionBody = ((SmileList)args->d)->a;
	userFunctionInfo = UserFunctionInfo_Create(compiler->currentFunction->userFunctionInfo, SMILE_VCALL(args, getSourceLocation),
		functionArgs, functionBody, &errorMessage);
	if (userFunctionInfo == NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(functionArgs, getSourceLocation), errorMessage));
		return CompiledBlock_CreateError();
	}
	functionIndex = Compiler_AddUserFunctionInfo(compiler, userFunctionInfo);
	compilerFunction = Compiler_BeginFunction(compiler, functionArgs, functionBody);
	compilerFunction->userFunctionInfo = userFunctionInfo;
	compilerFunction->numArgs = userFunctionInfo->numArgs;

	// Begin a new symbol scope for this function.
	scope = Compiler_BeginScope(compiler, PARSESCOPE_FUNCTION);
	compiledBlock = CompiledBlock_Create();

	// Declare the argument symbols, so that they can be correctly resolved.
	for (i = 0; i < userFunctionInfo->numArgs; i++) {
		Symbol name = userFunctionInfo->args[i].name;
		CompileScope_DefineSymbol(scope, name, PARSEDECL_ARGUMENT, i);
	}

	// Compile the body.
	Compiler_SetSourceLocationFromList(compiler, (SmileList)args->d);
	childBlock = Compiler_CompileExpr(compiler, functionBody, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Emit a return instruction at the end.
	Compiler_SetSourceLocationFromList(compiler, args);
	EMIT0(Op_Ret, -1);

	// We're done intermediate-compiling this function.
	Compiler_EndScope(compiler);
	Compiler_EndFunction(compiler);

	// Now transform it into finished bytecodes.
	byteCodeSegment = CompiledBlock_Finish(compiledBlock, False);
	userFunctionInfo->byteCodeSegment = byteCodeSegment;
	compilerFunction->stackSize = compiledBlock->maxStackDepth;

	// Make a suitable closure decriptor for it, and an actual function object.
	closureInfo = Compiler_MakeClosureInfoForCompilerFunction(compiler, compilerFunction);
	MemCpy(&userFunctionInfo->closureInfo, closureInfo, sizeof(struct ClosureInfoStruct));

	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	compiledBlock = CompiledBlock_Create();

	// Finally, emit an instruction to load a new instance of this function onto its parent's stack.
	EMIT1(Op_NewFn, 1, index = functionIndex);

	return compiledBlock;
}
