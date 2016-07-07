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

//-------------------------------------------------------------------------------------------------
//  ParserSyntaxClass and ParserSyntaxNode functions.

/// <summary>
/// Create a new, empty parser syntax class (top-level nonterminal class).  The
/// resulting syntax class will have a copy-on-write reference count of 1.
/// </summary>
/// <returns>The new, empty syntax class.</returns>
Inline ParserSyntaxClass ParserSyntaxClass_CreateNew(void)
{
	ParserSyntaxClass cls = GC_MALLOC_STRUCT(struct ParserSyntaxClassStruct);

	cls->referenceCount = 1;
	cls->rootDict = NULL;
	cls->isRootNonterminal = False;

	return cls;
}

/// <summary>
/// Helper function that correctly makes copy-on-write clones of each key of a
/// ParseSyntaxClass's root dictionary.
/// </summary>
static void *ParserSyntaxClass_DictClone(Int32 key, void *value, void *param)
{
	UNUSED(key);
	UNUSED(param);

	if (value != NULL)
		((ParserSyntaxNode)value)->referenceCount++;
	return value;
}

/// <summary>
/// Given a syntax class that may or may not be unique, make it unique by virtually
/// cloning it, so that updates may be safely applied to it.  If the incoming class
/// has a reference count of 1 or more, this will make a shallow clone with a reference
/// count of 1 and return the clone; if the incoming class has a reference count of 1
/// or less, it will be returned directly.
/// </summary>
/// <param name="cls">The original, possibly non-unique class.</param>
/// <returns>The unique-ified syntax class.</returns>
Inline ParserSyntaxClass ParserSyntaxClass_VFork(ParserSyntaxClass cls)
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

/// <summary>
/// Find or create a syntax class object that holds all the rules associated with
/// a given nonterminal class, like 'STMT', within the current syntax table.
/// </summary>
/// <param name="table">A pointer to the syntax table in which to find or create the syntax class.
/// If the table is non-unique and a class must be created, the table will be virtually forked.</param>
/// <param name="nonterminal">The nonterminal describing which syntax class to find or create.</param>
/// <returns>A new or preexisting syntax class that contains all of the rules for the
/// given nonterminal class.  If newly-created, it will be correctly registered with
/// the syntax table.</returns>
static ParserSyntaxClass ParserSyntaxClass_FindOrCreate(ParserSyntaxTable *table, Symbol nonterminal)
{
	ParserSyntaxClass syntaxClass;

	// Find or create the syntax class.
	if (Int32Dict_TryGetValue((*table)->syntaxClasses, nonterminal, &syntaxClass))
		return syntaxClass;

	// Virtually fork the syntax table, if it's not unique.  This makes a shallow copy of
	// the table, propagating its reference count out to the syntax classes it points to.
	if ((*table)->referenceCount > 1) {
		*table = ParserSyntaxTable_VFork(*table);
	}

	// Didn't find it, so create it.
	syntaxClass = ParserSyntaxClass_CreateNew();
	Int32Dict_Add((*table)->syntaxClasses, nonterminal, syntaxClass);

	// We now have to remember certain special nonterminals when they are registered so
	// that the parser doesn't have to work too hard to parse custom syntax rules.  These
	// nine nonterminals are the hook points that allow anyone to glue the root of their
	// own custom syntax rules into the standard Smile parsing logic.

	// Optimization note:
	//
	// By using a switch statement on special symbols here, we ensure that the cost to
	// track the special classes is still O(1) time (it would be O(n) or at best O(lg n)
	// if we used if-statements against arbitary symbols).

	switch (nonterminal) {
	case SMILE_SPECIAL_SYMBOL_STMT:
		(*table)->stmtClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_EXPR:
		(*table)->exprClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_CMP:
		(*table)->cmpClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_ADDSUB:
		(*table)->addSubClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_MULDIV:
		(*table)->mulDivClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_BINARY:
		(*table)->binaryClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_UNARY:
		(*table)->unaryClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_POSTFIX:
		(*table)->postfixClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_TERM:
		(*table)->termClass = syntaxClass;
		break;
	}

	return syntaxClass;
}

/// <summary>
/// Create a single node (eventually to be used in the syntax tree), filling in its
/// 'name' and 'variable' fields with the provided symbols, and with default values for
/// all the other fields.  This doesn't attach the node to anything; it just allocates it
/// and sets up its fields.
/// </summary>
/// <param name="name">The keyword/symbol or nonterminal name.</param>
/// <param name="variable">The variable to emit on a nonterminal match, 0 if this is a keyword/symbol.</param>
/// <param name="repetitionKind">The kind of repetition to use, if this is a nonterminal reference.
/// Either 0 (no repetition), '?' for optional, '*' for zero-or-more, '+' for one-or-more.</param>
/// <param name="repetitionSep">The separator for the repetition, if this is a nonterminal reference.
/// Either 0 (no separator), ',' for comma, or ';' for semicolon.</param>
/// <returns>The newly-created syntax node.</returns>
Inline ParserSyntaxNode ParserSyntaxNode_CreateInternal(Symbol name, Symbol variable, Int repetitionKind, Int repetitionSep)
{
	ParserSyntaxNode syntaxNode = GC_MALLOC_STRUCT(struct ParserSyntaxNodeStruct);

	syntaxNode->name = name;
	syntaxNode->variable = variable;

	syntaxNode->repetitionKind = (Int8)repetitionKind;
	syntaxNode->repetitionSep = (Int8)repetitionSep;
	syntaxNode->isNextNonterminal = False;
	syntaxNode->referenceCount = 1;

	syntaxNode->replacement = NullList;

	syntaxNode->nextDict = NULL;

	return syntaxNode;
}

/// <summary>
/// Extend the given syntax class (nonterminal-specific tree) with the data describing a new
/// node that will belong to some parent node within it.
/// </summary>
///
/// <remarks>
/// <p>This function applies copy-on-write behavior to the syntax class, virtually-forking the
/// syntax class if multiple syntax tables point to it.  After ensuring that it is unique, this
/// function then allocates and injects a new tree node under the provided parent.  Because
/// calling this may result in a new syntax class coming into existence, this returns a pointer
/// to the syntax class that contains the resulting node, which may or may not be the original
/// syntax class.</p>
///
/// <p>Note that if there is a preexisting node under the parent that matches the provided data,
/// this function does <em>not</em> allocate a new node, but merely returns the preexisting node.</p>
///
/// <p>And, finally, note also that this function expressly prohibits forking the tree on a
/// nonterminal node, to match the Smile syntax rules:  If the parent node already has a node
/// under it, and that node represents a nonterminal expression (i.e., its variable field != 0),
/// then the provided data <em>must</em> match that node or a parse error will be generated.  In
/// all other circumstances, the node can just be added to the tree (or matched/returned).</p>
/// </remarks>
///
/// <param name="parser">The parser that owns this syntax tree.</param>
/// <param name="position">The lexer position at which the new node was parsed.</param>
/// <param name="cls">The syntax class to extend.</param>
/// <param name="parent">A parent node within this syntax class under which the new child node
/// will be added.</param>
/// <param name="name">The keyword/symbol or nonterminal name.</param>
/// <param name="variable">The variable to emit on a nonterminal match, 0 if this is a keyword/symbol.</param>
/// <param name="repetitionKind">The kind of repetition to use, if this is a nonterminal reference.
/// Either 0 (no repetition), '?' for optional, '*' for zero-or-more, '+' for one-or-more.</param>
/// <param name="repetitionSep">The separator for the repetition, if this is a nonterminal reference.
/// Either 0 (no separator), ',' for comma, or ';' for semicolon.</param>
/// <param name="resultingNode">This will be filled in with a pointer to the newly-created child node.</param>
///
/// <returns>Either the original syntax class, or a modified copy of it, depending on whether the
/// copy-on-write rule needed to be applied.</returns>
static ParserSyntaxClass ParserSyntaxClass_Extend(Parser parser, LexerPosition position,
	ParserSyntaxClass cls, ParserSyntaxNode parent,
	Symbol name, Symbol variable, Int repetitionKind, Int repetitionSep,
	ParserSyntaxNode *resultingNode)
{
	ParserSyntaxClass newCls = cls;
	ParserSyntaxNode syntaxNode;
	Int32DictKeyValuePair firstPair;
	Int32Dict nextDict;
	Bool nextIsNonterminal;

	nextIsNonterminal = (parent == NULL ? newCls->isRootNonterminal : parent->isNextNonterminal);
	nextDict = (parent == NULL ? newCls->rootDict : parent->nextDict);

	if (nextDict == NULL) {
		// No dictionary at this level yet.

		if (parent == NULL) {
			// Empty root dictionary, so create it, and add the first node.
			if (newCls->referenceCount > 1) {
				newCls = ParserSyntaxClass_VFork(newCls);
			}
			newCls->rootDict = Int32Dict_CreateWithSize(4);
			newCls->isRootNonterminal = (variable != 0);
			syntaxNode = ParserSyntaxNode_CreateInternal(name, variable, repetitionKind, repetitionSep);
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
			if (newCls->referenceCount > 1) {
				newCls = ParserSyntaxClass_VFork(newCls);
			}
			parent->nextDict = Int32Dict_CreateWithSize(4);
			parent->isNextNonterminal = (variable != 0);
			syntaxNode = ParserSyntaxNode_CreateInternal(name, variable, repetitionKind, repetitionSep);
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
				if (newCls->referenceCount > 1) {
					newCls = ParserSyntaxClass_VFork(newCls);
				}
				syntaxNode = ParserSyntaxNode_CreateInternal(name, variable, repetitionKind, repetitionSep);
				Int32Dict_Add(nextDict, name, syntaxNode);
			}
		}
	}

	// Finally, now that we either found or created it, return it.
	*resultingNode = syntaxNode;

	// And return the (possibly-cloned/new) syntax class.
	return newCls;
}

//-------------------------------------------------------------------------------------------------
//  The public ParserSyntaxTable interface.

/// <summary>
/// Create a new, empty syntax table.
/// </summary>
/// <returns>The new, empty syntax table.</returns>
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

/// <summary>
/// Virtually fork a syntax table, if necessary:  That is, make a shallow copy-on-write
/// clone of the syntax table, increasing the reference counts of everything it points
/// to by 1.  The resulting syntax table will have its own unique syntax class dictionary,
/// and a reference count of 1.  Note that if this syntax table itself only has one reference,
/// the original syntax table will be returned, since no forking is needed.
/// </summary>
/// <param name="table">The syntax table to virtually fork.</param>
/// <returns>Either a new forked syntax table, or the original table if it was not referenced
/// anywhere else.</returns>
ParserSyntaxTable ParserSyntaxTable_VFork(ParserSyntaxTable table)
{
	ParserSyntaxTable newTable;

	newTable = GC_MALLOC_STRUCT(struct ParserSyntaxTableStruct);

	newTable->referenceCount = 1;
	newTable->syntaxClasses = Int32Dict_Clone(table->syntaxClasses, ParserSyntaxClass_DictClone, NULL);

	newTable->stmtClass = table->stmtClass;
	newTable->exprClass = table->exprClass;
	newTable->cmpClass = table->cmpClass;
	newTable->addSubClass = table->addSubClass;
	newTable->mulDivClass = table->mulDivClass;
	newTable->binaryClass = table->binaryClass;
	newTable->unaryClass = table->unaryClass;
	newTable->postfixClass = table->postfixClass;
	newTable->termClass = table->termClass;

	return newTable;
}

/// <summary>
/// Add a new syntax rule to the existing syntax table, virtually forking the table
/// as necessary.
/// </summary>
/// <param name="table">The table that will contain the new syntax rule.
/// If more than one scope references this table, it will be virtually forked, and
/// the new copy will be the one that gets the new rule.</param>
/// <param name="rule">The new rule to add to this syntax table.</param>
/// <returns>NULL on success, or a ParseError if the table contained another rule that
/// is incompatible with the rule being added.</returns>
ParseError ParserSyntaxTable_AddRule(ParserSyntaxTable *table, SmileSyntax rule)
{
	UNUSED(table);
	UNUSED(rule);

	return NULL;
}

/// <summary>
/// Set up the default syntax rules for normal Smile code.  This adds in the common
/// stuff like if-then-else and while-statements.
/// </summary>
/// <param name="table">The syntax table to add the default rules to.
/// If more than one scope references this table, it will be virtually forked, and
/// the new copy will be the one that gets the new rules.</param>
/// <returns>NULL on success, or a ParseError if the table contained any other rules
/// that are incompatible with the rules being added.</returns>
ParseError ParserSyntaxTable_SetupDefaultRules(ParserSyntaxTable *table)
{
	UNUSED(table);

	return NULL;
}
