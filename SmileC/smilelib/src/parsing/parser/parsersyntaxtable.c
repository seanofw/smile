//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/parsing/internal/parsesyntax.h>

static ParserSyntaxNode ParserSyntaxNode_CreateInternal(Symbol name, Symbol variable)
{
	ParserSyntaxNode syntaxNode = GC_MALLOC_STRUCT(struct ParserSyntaxNodeStruct);

	syntaxNode->name = name;
	syntaxNode->variable = variable;

	syntaxNode->repetitionKind = 0;
	syntaxNode->repetitionSep = 0;
	syntaxNode->isNextNonterminal = False;
	syntaxNode->referenceCount = 1;

	syntaxNode->replacement = NullList;

	syntaxNode->nextDict = NULL;

	return syntaxNode;
}

ParserSyntaxClass ParserSyntaxClass_Extend(Parser parser, LexerPosition position,
	ParserSyntaxClass cls, ParserSyntaxNode parent,
	Symbol name, Symbol variable, Int repetitionKind, Int repetitionSep,
	ParserSyntaxNode *resultingNode)
{
	ParserSyntaxClass newCls = cls;
	ParserSyntaxNode syntaxNode;
	Int32DictKeyValuePair firstPair;
	Int32Dict nextDict;
	Bool nextIsNonterminal;

	if (newCls->referenceCount > 1) {
		newCls = ParserSyntaxClass_MakeUnique(newCls);
	}

	nextIsNonterminal = (parent == NULL ? newCls->isRootNonterminal : parent->isNextNonterminal);
	nextDict = (parent == NULL ? newCls->rootDict : parent->nextDict);

	if (nextDict == NULL) {
		// No dictionary at this level yet.

		if (parent == NULL) {
			// Empty root dictionary, so create it, and add the first node.
			newCls->rootDict = Int32Dict_CreateWithSize(4);
			newCls->isRootNonterminal = (variable != 0);
			syntaxNode = ParserSyntaxNode_CreateInternal(name, variable);
			Int32Dict_Add(newCls->rootDict, name, syntaxNode);
		}
		else {
			// Make sure we're not adding a nonterminal in such a way that the resulting tree
			// is no longer deterministically parseable.
			if (variable != 0 && parent->replacement != NullList) {
				Parser_AddError(parser, position, "Cannot add syntax rule because it forks the tree on the nonterminal \"%S %S\".",
					SymbolTable_GetName(Smile_SymbolTable, name),
					SymbolTable_GetName(Smile_SymbolTable, variable));
				return newCls;
			}

			// There is no child dictionary yet, so create it, and add the first node.
			parent->nextDict = Int32Dict_CreateWithSize(4);
			parent->isNextNonterminal = (variable != 0);
			syntaxNode = ParserSyntaxNode_CreateInternal(name, variable);
			Int32Dict_Add(parent->nextDict, name, syntaxNode);
		}
	}
	else {
		// Preexisting dictionary at this level.

		if (nextIsNonterminal) {
			// This dictionary contains only a single nonterminal.

			firstPair = Int32Dict_GetFirst(nextDict);
			syntaxNode = (ParserSyntaxNode)firstPair.value;
			if (variable == 0) {
				// Error: Can't fork nonterminal --> terminal.
				Parser_AddError(parser, position, "Cannot add syntax rule because it forks the tree on the nonterminal \"%S %S\".",
					SymbolTable_GetName(Smile_SymbolTable, syntaxNode->name),
					SymbolTable_GetName(Smile_SymbolTable, syntaxNode->variable));
				return newCls;
			}
			if (syntaxNode->name != name || syntaxNode->variable != variable
				|| syntaxNode->repetitionKind != repetitionKind || syntaxNode->repetitionSep != repetitionSep) {
				// Error: Can't fork nonterminal --> different nonterminal.
				Parser_AddError(parser, position, "Cannot add syntax rule because it forks the tree on the nonterminal \"%S %S\".",
					SymbolTable_GetName(Smile_SymbolTable, name),
					SymbolTable_GetName(Smile_SymbolTable, variable));
				return newCls;
			}
			else {
				// Just a nonterminal, correctly mimicked in a new rule.
				// Nothing to do here.
			}
		}
		else {
			// This dictionary is a collection of one or more terminals.

			if (variable != 0) {
				// Error: Can't fork terminal --> nonterminal.
				Parser_AddError(parser, position, "Cannot add syntax rule because it forks the tree on the nonterminal \"%S %S\".",
					SymbolTable_GetName(Smile_SymbolTable, name),
					SymbolTable_GetName(Smile_SymbolTable, variable));
				return newCls;
			}
			else if (Int32Dict_TryGetValue(nextDict, name, (void **)&syntaxNode)) {
				// Just a terminal, correctly mimicked in a new rule.
				// Nothing to do here.
			}
			else {
				// Forking the syntax tree at a terminal.
				syntaxNode = ParserSyntaxNode_CreateInternal(name, variable);
				Int32Dict_Add(nextDict, name, syntaxNode);
			}
		}
	}

	*resultingNode = syntaxNode;

	return newCls;
}

ParserSyntaxClass ParserSyntaxClass_CreateNew(void)
{
	ParserSyntaxClass cls = GC_MALLOC_STRUCT(struct ParserSyntaxClassStruct);

	cls->referenceCount = 1;
	cls->rootDict = NULL;
	cls->isRootNonterminal = False;

	return cls;
}

static void *ParserSyntaxClass_DictClone(Int32 key, void *value, void *param)
{
	UNUSED(key);
	UNUSED(param);

	if (value != NULL)
		((ParserSyntaxNode)value)->referenceCount++;
	return value;
}

ParserSyntaxClass ParserSyntaxClass_MakeUnique(ParserSyntaxClass cls)
{
	ParserSyntaxClass newCls;

	if (cls->referenceCount <= 1)
		return cls;

	newCls = GC_MALLOC_STRUCT(struct ParserSyntaxClassStruct);

	newCls->referenceCount = 1;
	newCls->rootDict = Int32Dict_Clone(cls->rootDict, ParserSyntaxClass_DictClone, NULL);
	newCls->isRootNonterminal = cls->isRootNonterminal;

	return newCls;
}

ParserSyntaxTable ParserSyntaxTable_CreateNew(void)
{
	ParserSyntaxTable syntaxTable = GC_MALLOC_STRUCT(struct ParserSyntaxTableStruct);

	syntaxTable->referenceCount = 1;
	syntaxTable->syntaxClasses = Int32Dict_Create();

	syntaxTable->stmtClass = NULL;
	syntaxTable->exprClass = NULL;
	syntaxTable->cmpClass = NULL;
	syntaxTable->addSubClass = NULL;
	syntaxTable->mulDivClass = NULL;
	syntaxTable->binaryClass = NULL;
	syntaxTable->unaryClass = NULL;
	syntaxTable->postfixClass = NULL;
	syntaxTable->termClass = NULL;

	return syntaxTable;
}

ParserSyntaxTable ParserSyntaxTable_MakeUnique(ParserSyntaxTable table)
{
	UNUSED(table);
	return NULL;
}

void ParserSyntaxTable_AddRule(ParserSyntaxTable table, SmileSyntax rule)
{
	UNUSED(table);
	UNUSED(rule);
}

void ParserSyntaxTable_SetupDefaultRules(ParserSyntaxTable table)
{
	UNUSED(table);
}
