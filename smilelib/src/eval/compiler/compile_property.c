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

// Form: expr.symbol
void Compiler_CompileProperty(Compiler compiler, SmilePair pair, Bool store)
{
	Symbol symbol;
	Int offset;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;

	if (SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(pair, getSourceLocation),
			String_FromC("Cannot compile pair: right side must be a symbol.")));
	}

	// Evaluate the left side first, which will leave the left side on the stack.
	Compiler_CompileExpr(compiler, pair->left);

	// Extract the property named by the symbol on the right side, leaving the property's value on the stack.
	symbol = ((SmileSymbol)(pair->right))->symbol;
	if (store) {
		EMIT1(Op_StProp, -2, symbol = symbol);
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
		else if (symbol == Smile_KnownSymbols.left) {
			EMIT0(Op_LdLeft, -1 + 1);
		}
		else if (symbol == Smile_KnownSymbols.right) {
			EMIT0(Op_LdRight, -1 + 1);
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
}
