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

// Form: [$new base [[sym1 val1] [sym2 val2] [sym3 val3] ...]]
void Compiler_CompileNew(Compiler compiler, SmileList args)
{
	Int numPairs;
	SmileList pairs, pair;
	Symbol symbol;
	SmileObject value;
	Int offset;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	Int oldSourceLocation;

	// Must be an expression of the form: [$new base [[sym1 val1] [sym2 val2] [sym3 val3] ...]]
	if (SmileList_Length(args) != 2) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$new]: Expression is not well-formed.")));
		return;
	}

	// Compile the base object reference.
	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);
	Compiler_SetSourceLocationFromList(compiler, args);
	Compiler_CompileExpr(compiler, LIST_FIRST(args));
	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	// Compile all the pairs.
	for (pairs = (SmileList)LIST_SECOND(args), numPairs = 0; SMILE_KIND(pairs) == SMILE_KIND_LIST; pairs = (SmileList)pairs->d, numPairs++) {
		pair = (SmileList)pairs->a;
		if (SMILE_KIND(pair) != SMILE_KIND_LIST) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
				String_FromC("Cannot compile [$new]: Expression is not well-formed.")));
			return;
		}
		if (SmileList_Length(pair) != 2 || SMILE_KIND(pair->a) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(pair, getSourceLocation),
				String_FromC("Cannot compile [$new]: Expression is not well-formed.")));
			return;
		}
		symbol = ((SmileSymbol)pair->a)->symbol;
		value = LIST_SECOND(pair);
		oldSourceLocation = Compiler_SetAssignedSymbol(compiler, symbol);
		Compiler_SetSourceLocationFromList(compiler, pair);
		EMIT1(Op_LdSym, +1, symbol = symbol);
		Compiler_CompileExpr(compiler, value);
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	}
	if (SMILE_KIND(pairs) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$new]: Expression is not well-formed.")));
		return;
	}

	// Create the new object.
	EMIT1(Op_NewObj, +1 - (numPairs * 2 + 1), int32 = numPairs);
}
