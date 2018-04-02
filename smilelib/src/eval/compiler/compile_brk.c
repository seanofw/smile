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

// Form: [$brk]
CompiledBlock Compiler_CompileBrk(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	CompiledBlock compiledBlock = CompiledBlock_Create();

	UNUSED(args);
	UNUSED(compileFlags);
	UNUSED(compiler);

	EMIT0(Op_Brk, 0);

	Compiler_MakeStackMatchCompileFlags(compiler, compiledBlock, compileFlags);

	return compiledBlock;
}
