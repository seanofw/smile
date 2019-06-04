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

#include <smile/types.h>
#include <smile/atomic.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileloanword.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/parsing/internal/parseloanword.h>
#include <smile/internal/staticstring.h>

static Int32Dict _reservedKeywords;

static ParserLoanwordTable ParserLoanwordTable_Fork(ParserLoanwordTable table);

static void *LoanwordValueCloner(Int32 key, void *value, void *param)
{
	SmileLoanword oldLoanword, newLoanword;

	UNUSED(key);

	oldLoanword = (SmileLoanword)value;
	newLoanword = SmileLoanword_Create(oldLoanword->name, oldLoanword->regex, oldLoanword->replacement, oldLoanword->position);
	newLoanword->table = (ParserLoanwordTable)param;

	return newLoanword;
}

static ParserLoanwordTable ParserLoanwordTable_Fork(ParserLoanwordTable table)
{
	ParserLoanwordTable newTable = GC_MALLOC_STRUCT(struct ParserLoanwordTableStruct);

	newTable->referenceCount = 1;
	newTable->definitions = Int32Dict_Clone(table->definitions, LoanwordValueCloner, newTable);

	return newTable;
}

ParserLoanwordTable ParserLoanwordTable_CreateNew(void)
{
	ParserLoanwordTable newTable = GC_MALLOC_STRUCT(struct ParserLoanwordTableStruct);

	newTable->referenceCount = 1;
	newTable->definitions = Int32Dict_Create();

	return newTable;
}

Inline void FastAddReservedKeyword(Int32Dict dict, String keyword)
{
	Symbol symbol = SymbolTable_GetSymbol(Smile_SymbolTable, keyword);
	Int32Dict_Add(dict, symbol, NULL);
}

static void ParserLoanwordTable_SetupReservedKeywords(void)
{
	STATIC_STRING(brk, "brk");
	STATIC_STRING(include, "include");
	STATIC_STRING(loanword, "loanword");
	STATIC_STRING(syntax, "syntax");

	STATIC_STRING(html, "html");
	STATIC_STRING(json, "json");
	STATIC_STRING(xml, "xml");

	STATIC_STRING(error, "error");
	STATIC_STRING(line, "line");
	STATIC_STRING(warning, "warning");

	STATIC_STRING(declare, "declare");
	STATIC_STRING(define, "define");
	STATIC_STRING(else_, "else");
	STATIC_STRING(export, "export");
	STATIC_STRING(if_, "if");
	STATIC_STRING(import, "import");
	STATIC_STRING(macro, "macro");
	STATIC_STRING(pragma, "pragma");

	Int32Dict reservedKeywords = Int32Dict_Create();

	FastAddReservedKeyword(reservedKeywords, brk);
	FastAddReservedKeyword(reservedKeywords, include);
	FastAddReservedKeyword(reservedKeywords, loanword);
	FastAddReservedKeyword(reservedKeywords, syntax);

	FastAddReservedKeyword(reservedKeywords, html);
	FastAddReservedKeyword(reservedKeywords, json);
	FastAddReservedKeyword(reservedKeywords, xml);

	FastAddReservedKeyword(reservedKeywords, error);
	FastAddReservedKeyword(reservedKeywords, line);
	FastAddReservedKeyword(reservedKeywords, warning);

	FastAddReservedKeyword(reservedKeywords, declare);
	FastAddReservedKeyword(reservedKeywords, define);
	FastAddReservedKeyword(reservedKeywords, else_);
	FastAddReservedKeyword(reservedKeywords, export);
	FastAddReservedKeyword(reservedKeywords, if_);
	FastAddReservedKeyword(reservedKeywords, import);
	FastAddReservedKeyword(reservedKeywords, macro);
	FastAddReservedKeyword(reservedKeywords, pragma);

	_reservedKeywords = reservedKeywords;
}

Bool ParserLoanwordTable_AddRule(Parser parser, ParserLoanwordTable *resultTable, SmileLoanword loanword)
{
	SmileLoanword previousLoanword;

	if (_reservedKeywords == NULL)
		ParserLoanwordTable_SetupReservedKeywords();

	// Make sure we have a writable copy of the loanword table.
	ParserLoanwordTable table = *resultTable;
	if (table->referenceCount > 1)
		*resultTable = table = ParserLoanwordTable_Fork(table);

	// Try to add it.
	if (Int32Dict_ContainsKey(_reservedKeywords, loanword->name)) {

		// Can't replace the special built-in forms, no matter what scope you're in.
		Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, loanword->position,
			String_Format("Cannot declare \"#%S\"; this loanword name is reserved.", loanword->name)));
		return False;
	}
	else if (Int32Dict_TryGetValue(table->definitions, loanword->name, (void **)&previousLoanword)) {

		// Couldn't add it because it already existed.  This may or may not be an error, depending
		// on why it already exists.
		if (previousLoanword->table == table) {

			// Redeclaration in the same scope.  That's a programmer error.
			Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, loanword->position,
				String_Format("Cannot redeclare \"#%S\"; this loanword name was already declared on line %d.",
					SymbolTable_GetName(Smile_SymbolTable, loanword->name), previousLoanword->position->line)));
			return False;
		}

		// Just a lexical sharing from an outer scope, so it's okay to redeclare it.
	}

	// Assign it.
	Int32Dict_SetValue(table->definitions, loanword->name, loanword);
	loanword->table = table;
	return True;
}
