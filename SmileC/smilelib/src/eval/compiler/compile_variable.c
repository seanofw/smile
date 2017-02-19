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

// Form: symbol
void Compiler_CompileVariable(Compiler compiler, Symbol symbol, Bool store)
{
	Int offset;
	Int functionDepth;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	CompiledLocalSymbol localSymbol = CompileScope_FindSymbol(compiler->currentScope, symbol);

	if (localSymbol == NULL || localSymbol->kind == PARSEDECL_GLOBAL || localSymbol->kind == PARSEDECL_PRIMITIVE) {
		// Don't know what this is, or it's explictly global, so it comes from a dictionary load from an outer closure.
		if (store) {
			EMIT1(Op_StX, -1, symbol = symbol);
			if (localSymbol != NULL)
				localSymbol->wasWrittenDeep = True;
		}
		else {
			EMIT1(Op_LdX, +1, symbol = symbol);
			if (localSymbol != NULL)
				localSymbol->wasReadDeep = True;
		}
		return;
	}

	functionDepth = compiler->currentFunction->functionDepth - localSymbol->scope->function->functionDepth;

	switch (localSymbol->kind) {

	case PARSEDECL_ARGUMENT:
		if (functionDepth <= 7) {
			if (store) {
				EMIT1(Op_StArg0 + functionDepth, 0, index = localSymbol->index);	// Leaves the value on the stack.
				if (functionDepth == 0) localSymbol->wasWritten = True;
				else localSymbol->wasWrittenDeep = True;
			}
			else {
				EMIT1(Op_LdArg0 + functionDepth, +1, index = localSymbol->index);
				if (functionDepth == 0) localSymbol->wasRead = True;
				else localSymbol->wasReadDeep = True;
			}
		}
		else {
			if (store) {
				EMIT2(Op_StArg, 0, i2.a = functionDepth, i2.b = localSymbol->index);	// Leaves the value on the stack.
			}
			else {
				EMIT2(Op_LdArg, +1, i2.a = functionDepth, i2.b = localSymbol->index);
			}
		}
		break;

	case PARSEDECL_VARIABLE:
		if (functionDepth <= 7) {
			if (store) {
				EMIT1(Op_StLoc0 + functionDepth, 0, index = localSymbol->index);	// Leaves the value on the stack.
				if (functionDepth == 0) localSymbol->wasWritten = True;
				else localSymbol->wasWrittenDeep = True;
			}
			else {
				EMIT1(Op_LdLoc0 + functionDepth, +1, index = localSymbol->index);
				if (functionDepth == 0) localSymbol->wasRead = True;
				else localSymbol->wasReadDeep = True;
			}
		}
		else {
			if (store) {
				EMIT2(Op_StLoc, 0, i2.a = functionDepth, i2.b = localSymbol->index);	// Leaves the value on the stack.
			}
			else {
				EMIT2(Op_LdLoc, +1, i2.a = functionDepth, i2.b = localSymbol->index);
			}
		}
		break;

	default:
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, NULL,
			String_FromC("Cannot compile symbol:  Fatal internal error.")));
		break;
	}
}
