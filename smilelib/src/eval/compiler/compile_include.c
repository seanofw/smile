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
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/env/modules.h>

// Form: [$include moduleid membername]
//    or [$include moduleid membername localname]
CompiledBlock Compiler_CompileInclude(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	Int length;
	Int64 moduleId;
	Symbol memberName, localName;
	CompiledBlock compiledBlock;
	IntermediateInstruction instr;
	Int oldSourceLocation;
	Int memberOffset;
	ModuleInfo moduleInfo;

	// Make sure this is a well-formed list of two to three elements.
	length = SmileList_Length(args);
	if (length < 2 || length > 3) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$include]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	// Make sure we have a valid module reference.
	if (SMILE_KIND(args->a) != SMILE_KIND_INTEGER64
		|| (moduleId = ((SmileInteger64)args->a)->value) >= Int32Max
		|| (moduleInfo = ModuleInfo_GetModuleById((Int32)moduleId)) == NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$include]: Module is not a valid ID.")));
		return CompiledBlock_CreateError();
	}

	// Make sure the member name is a symbol.
	args = (SmileList)args->d;
	if (SMILE_KIND(args->a) != SMILE_KIND_SYMBOL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$include]: Module member name is not a symbol.")));
		return CompiledBlock_CreateError();
	}
	memberName = ((SmileSymbol)args->a)->symbol;

	// Make sure the member name actually is something this module exposes.
	memberOffset = ModuleInfo_GetExposedValueClosureOffset(moduleInfo, memberName);
	if (memberOffset < 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_Format("Cannot compile [$include]: Module \"%S\" does not expose anything named \"%S\".",
				moduleInfo->name, SymbolTable_GetName(Smile_SymbolTable, memberName))));
		return CompiledBlock_CreateError();
	}

	// If there's a local name, make sure it's valid too.
	args = (SmileList)args->d;
	if (SMILE_KIND(args) == SMILE_KIND_NULL) {
		// No local name.
		localName = memberName;
	}
	else if (SMILE_KIND(args->a) != SMILE_KIND_SYMBOL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$include]: Module member name is not a symbol.")));
		return CompiledBlock_CreateError();
	}
	else {
		localName = ((SmileSymbol)args->a)->symbol;
	}

	// Make the instructions that perform the include.
	compiledBlock = CompiledBlock_Create();
	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, localName);
	Compiler_SetSourceLocationFromList(compiler, args);

	EMIT2(Op_LdInclude, +1, i2.a = (Int32)moduleId, i2.b = (Int32)memberOffset);

	Compiler_CompileStoreVariable(compiler, localName, compileFlags, compiledBlock);
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	return compiledBlock;
}
