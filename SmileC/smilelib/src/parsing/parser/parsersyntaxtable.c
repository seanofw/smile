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
#include <smile/atomic.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>
#include <smile/parsing/internal/parsesyntax.h>

static ParserSyntaxNode ParserSyntaxNode_CreateInternal(Symbol name, Symbol variable, Int repetitionKind, Int repetitionSep);

static ParserSyntaxClass ParserSyntaxClass_CreateNew(void);
static void *ParserSyntaxClass_DictClone(Int32 key, void *value, void *param);
static ParserSyntaxClass ParserSyntaxClass_VFork(ParserSyntaxClass cls);
static ParserSyntaxClass ParserSyntaxClass_FindOrCreate(ParserSyntaxTable *table, Symbol nonterminal);
static Bool ParserSyntaxClass_Extend(Parser parser, LexerPosition position,
	ParserSyntaxClass cls, ParserSyntaxNode parent,
	Symbol name, Symbol variable, Int repetitionKind, Int repetitionSep,
	ParserSyntaxNode *resultingNode, ParserSyntaxClass *resultingCls);

static ParserSyntaxTable ParserSyntaxTable_VFork(ParserSyntaxTable table);
static Bool ParserSyntaxTable_ValidateRuleWithInitialNonterminal(ParserSyntaxTable table, Symbol rootNonterminal, Symbol firstNonterminal);

static UInt32 UniqueNodeID = 0;

//-------------------------------------------------------------------------------------------------
// Static data.

STATIC_STRING(InvalidSyntaxRuleError, "Invalid syntax rule pattern; patterns are lists of only symbols and nonterminals.");
STATIC_STRING(SyntaxRuleTooDeepError, "Syntax rule's pattern is too big (>256 nodes).");
STATIC_STRING(IllegalInitialNonterminalError, "Syntax rule produces an illegal grammar with infinite left recursion on the initial nonterminals.");
STATIC_STRING(IllegalRepeatSymbolError, "Syntax rule contains an unknown repeat symbol \"%S\" in nonterminal.");
STATIC_STRING(IllegalSeparatorSymbolError, "Syntax rule contains an unknown separator symbol \"%S\" in nonterminal.");
STATIC_STRING(CantRepeatFirstNonterminalError, "Cannot use '?' or '*' repeats on the first item in a syntax pattern.");
STATIC_STRING(DuplicateSyntaxRuleError, "Duplicate syntax rule pattern; patterns must be unique within a syntax class.");
STATIC_STRING(NullReplacementError, "Syntax rules must not have null/[] as their replacement form.");

//-------------------------------------------------------------------------------------------------
//  ParserSyntaxNode functions.

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
static ParserSyntaxNode ParserSyntaxNode_CreateInternal(Symbol name, Symbol variable, Int repetitionKind, Int repetitionSep)
{
	ParserSyntaxNode syntaxNode = GC_MALLOC_STRUCT(struct ParserSyntaxNodeStruct);

	syntaxNode->referenceCount = 1;
	syntaxNode->nextTerminals = NULL;
	syntaxNode->nextNonterminals = NULL;

	syntaxNode->replacement = NullObject;

	syntaxNode->name = name;
	syntaxNode->variable = variable;

	syntaxNode->repetitionKind = (Int8)repetitionKind;
	syntaxNode->repetitionSep = (Int8)repetitionSep;

	syntaxNode->nodeID = Atomic_IncrementInt32(&UniqueNodeID);

	return syntaxNode;
}

//-------------------------------------------------------------------------------------------------
//  ParserSyntaxClass functions.

/// <summary>
/// Create a new, empty parser syntax class (top-level nonterminal class).  The
/// resulting syntax class will have a copy-on-write reference count of 1.
/// </summary>
/// <returns>The new, empty syntax class.</returns>
static ParserSyntaxClass ParserSyntaxClass_CreateNew(void)
{
	ParserSyntaxClass cls = GC_MALLOC_STRUCT(struct ParserSyntaxClassStruct);

	cls->referenceCount = 1;
	cls->nextTerminals = NULL;
	cls->nextNonterminals = NULL;
	cls->nodeID = Atomic_IncrementInt32(&UniqueNodeID);

	cls->replacement = NullObject;

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
static ParserSyntaxClass ParserSyntaxClass_VFork(ParserSyntaxClass cls)
{
	ParserSyntaxClass newCls;

	if (cls->referenceCount <= 1)
		return cls;

	newCls = GC_MALLOC_STRUCT(struct ParserSyntaxClassStruct);

	newCls->referenceCount = 1;
	newCls->nextTerminals = Int32Dict_Clone(cls->nextTerminals, ParserSyntaxClass_DictClone, NULL);
	newCls->nextNonterminals = Int32Dict_Clone(cls->nextNonterminals, ParserSyntaxClass_DictClone, NULL);

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
	case SMILE_SPECIAL_SYMBOL_CMPEXPR:
		(*table)->cmpExprClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_ADDEXPR:
		(*table)->addExprClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_MULEXPR:
		(*table)->mulExprClass = syntaxClass;
		break;
	case SMILE_SPECIAL_SYMBOL_BINARYEXPR:
		(*table)->binaryExprClass = syntaxClass;
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
/// <param name="resultingNode">This will be filled in with a pointer to the found or newly-created child node.</param>
/// <param name="resultingCls">This will be filled in with a pointer to the original or vforked syntax class,
/// wherever the new node was added.</param>
///
/// <returns>True on success, or False if an error occurred.  The parser will be updated to
/// include the error, if there was an error.  Note that the values of 'resultingNode' and 'resultingCls'
/// are meaningless if False is returned.</returns>
static Bool ParserSyntaxClass_Extend(Parser parser, LexerPosition position,
	ParserSyntaxClass cls, ParserSyntaxNode parent,
	Symbol name, Symbol variable, Int repetitionKind, Int repetitionSep,
	ParserSyntaxNode *resultingNode, ParserSyntaxClass *resultingCls)
{
	ParserSyntaxClass newCls = cls;
	ParserSyntaxNode syntaxNode;
	Int32Dict nextDict;

	if (parent == NULL) {
		parent = (ParserSyntaxNode)cls;
	}

	if ((!variable && parent->nextTerminals == NULL) || (variable && parent->nextNonterminals == NULL)) {
		// No dictionary at this level yet, so create one, and add the new node to it.
		if (newCls->referenceCount > 1) {
			newCls = ParserSyntaxClass_VFork(newCls);
		}
		nextDict = variable	? (parent->nextNonterminals	= Int32Dict_CreateWithSize(4))
			: (parent->nextTerminals	= Int32Dict_CreateWithSize(4));
		syntaxNode = ParserSyntaxNode_CreateInternal(name, variable, repetitionKind, repetitionSep);
		Int32Dict_Add(nextDict, name, syntaxNode);
	}
	else if (variable) {
		// This is a nonterminal that we'd like to add to a preexisting dictionary.
		
		if (!Int32Dict_TryGetValue(parent->nextNonterminals, name, &syntaxNode)) {
			// Nonterminal doesn't exist yet, so it's safe to add it.
			if (newCls->referenceCount > 1) {
				newCls = ParserSyntaxClass_VFork(newCls);
			}
			syntaxNode = ParserSyntaxNode_CreateInternal(name, variable, repetitionKind, repetitionSep);
			Int32Dict_Add(parent->nextNonterminals, name, syntaxNode);
		}
		if (syntaxNode->name == name && (syntaxNode->repetitionKind != repetitionKind || syntaxNode->repetitionSep != repetitionSep)) {
			// Error: Can't fork nonterminal --> nonterminal with different repeat behavior.
			Parser_AddError(parser, position, "Cannot add syntax rule because the nonterminal \"%S %S\" has different repeat behavior from other nonterminals in the same position in their rules.",
				SymbolTable_GetName(Smile_SymbolTable, name),
				SymbolTable_GetName(Smile_SymbolTable, variable));
			return False;
		}
		else {
			// A nonterminal correctly mimicked in a new rule.
			// Nothing to do here.
		}
	}
	else {
		// This is a nonterminal that we'd like to add to a preexisting dictionary.

		if (Int32Dict_TryGetValue(parent->nextTerminals, name, &syntaxNode)) {
			// Just a terminal, correctly mimicked in a new rule.
			// Nothing to do here.
		}
		else {
			// Terminal doesn't exist yet, so it's safe to add it.
			if (newCls->referenceCount > 1) {
				newCls = ParserSyntaxClass_VFork(newCls);
			}
			syntaxNode = ParserSyntaxNode_CreateInternal(name, variable, repetitionKind, repetitionSep);
			Int32Dict_Add(parent->nextTerminals, name, syntaxNode);
		}
	}

	// Finally, now that we either found or created it, return it.
	*resultingNode = syntaxNode;
	*resultingCls = newCls;

	// And return the (possibly-cloned/new) syntax class.
	return True;
}

//-------------------------------------------------------------------------------------------------
//  ParserSyntaxTable functions.

/// <summary>
/// Create a new, empty syntax table.
/// </summary>
/// <returns>The new, empty syntax table.</returns>
ParserSyntaxTable ParserSyntaxTable_CreateNew(void)
{
	ParserSyntaxTable syntaxTable = GC_MALLOC_STRUCT(struct ParserSyntaxTableStruct);

	syntaxTable->referenceCount = 1;
	syntaxTable->syntaxClasses = Int32Dict_Create();

	syntaxTable->firstSets = NULL;
	syntaxTable->followSets = NULL;

	syntaxTable->stmtClass = NULL;
	syntaxTable->exprClass = NULL;
	syntaxTable->cmpExprClass = NULL;
	syntaxTable->addExprClass = NULL;
	syntaxTable->mulExprClass = NULL;
	syntaxTable->binaryExprClass = NULL;
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
static ParserSyntaxTable ParserSyntaxTable_VFork(ParserSyntaxTable table)
{
	ParserSyntaxTable newTable;

	newTable = GC_MALLOC_STRUCT(struct ParserSyntaxTableStruct);

	newTable->referenceCount = 1;
	newTable->syntaxClasses = Int32Dict_Clone(table->syntaxClasses, ParserSyntaxClass_DictClone, NULL);

	newTable->firstSets = NULL;
	newTable->followSets = NULL;

	newTable->stmtClass = table->stmtClass;
	newTable->exprClass = table->exprClass;
	newTable->cmpExprClass = table->cmpExprClass;
	newTable->addExprClass = table->addExprClass;
	newTable->mulExprClass = table->mulExprClass;
	newTable->binaryExprClass = table->binaryExprClass;
	newTable->unaryClass = table->unaryClass;
	newTable->postfixClass = table->postfixClass;
	newTable->termClass = table->termClass;

	return newTable;
}

static void ParserSyntaxTable_RecursivelyComputeFirstSet(ParserSyntaxTable table, Symbol nonterminal, Int32Int32Dict firstSet, Int32Int32Dict nonterminalsSeen)
{
	ParserSyntaxClass syntaxClass;

	// See if we've already processed this nonterminal.  If so, abort.
	if (!Int32Int32Dict_Add(nonterminalsSeen, nonterminal, 0))
		return;

	// If we landed on one of the eight built-in rules, add the special "everything"
	// marker to the rule's first set.
	switch (nonterminal) {
		case SMILE_SPECIAL_SYMBOL_STMT:
		case SMILE_SPECIAL_SYMBOL_EXPR:
		case SMILE_SPECIAL_SYMBOL_CMPEXPR:
		case SMILE_SPECIAL_SYMBOL_ADDEXPR:
		case SMILE_SPECIAL_SYMBOL_MULEXPR:
		case SMILE_SPECIAL_SYMBOL_BINARYEXPR:
		case SMILE_SPECIAL_SYMBOL_UNARY:
		case SMILE_SPECIAL_SYMBOL_POSTFIX:
		case SMILE_SPECIAL_SYMBOL_TERM:
			Int32Int32Dict_Add(firstSet, -1, 0);
			Int32Int32Dict_Add(nonterminalsSeen, -1, 0);
			break;
	}

	// Find the rule pointed-to by the current nonterminal.
	if (!Int32Dict_TryGetValue(table->syntaxClasses, nonterminal, &syntaxClass)) {
		// If we got here, then we ended up nowhere (i.e., this set of syntax rules is
		// incomplete or broken or something).
		return;
	}

	if (syntaxClass->nextTerminals != NULL) {
		// This syntax class has terminals that start it, so add them to the FIRST set.
		Int32 *keys = Int32Dict_GetKeys(syntaxClass->nextTerminals);
		Int32 numKeys = Int32Dict_Count(syntaxClass->nextTerminals);
	
		while (numKeys--) {
			Int32Int32Dict_Add(firstSet, *keys++, 0);
		}
	}

	if (syntaxClass->nextNonterminals != NULL) {
		// This syntax class has nonterminals that start it, so recurse on them.
		Int32 *keys = Int32Dict_GetKeys(syntaxClass->nextNonterminals);
		Int32 numKeys = Int32Dict_Count(syntaxClass->nextNonterminals);
	
		while (numKeys--) {
			Symbol nextNonterminal = *keys++;
			ParserSyntaxTable_RecursivelyComputeFirstSet(table, nextNonterminal, firstSet, nonterminalsSeen);
		}
	}
}

/// <summary>
/// Compute the FIRST set, which describes which input symbols are matchable by the leftmost
/// terminals and nonterminals in all the rules for this nonterminal.  For example, given
/// this grammar:
///
///    addexpr ::= mulexpr '+' addexpr | mulexpr
///    mulexpr ::= unary '*' mulexpr | unary
///    unary ::= '-' unary | term
///    term ::= IDENT | NUMBER
///
/// The FIRST set for addexpr is ['-', IDENT, NUMBER].
/// </summary>
/// <param name="table">The syntax table that records the grammar containing this node.</param>
/// <param name="node">The node for which you would like to retrieve the FOLLOW set.</param>
/// <returns>The return value is a dictionary whose keys are the FOLLOW set (and whose values are all zero).</returns>
static Int32Int32Dict ParserSyntaxTable_ComputeFirstSet(ParserSyntaxTable table, Symbol nonterminal)
{
	Int32Int32Dict firstSet = Int32Int32Dict_Create();
	Int32Int32Dict nonterminalsSeen = Int32Int32Dict_Create();

	ParserSyntaxTable_RecursivelyComputeFirstSet(table, nonterminal, firstSet, nonterminalsSeen);

	return firstSet;
}

/// <summary>
/// Compute the FOLLOW set, which describes which input symbols should cause us to
/// exit the given nonterminal node.  For example, given this grammar:
///
///    addexpr ::= mulexpr '+' addexpr | mulexpr
///    mulexpr ::= unary '*' mulexpr | unary
///    unary ::= '-' unary | term
///    term ::= IDENT | NUMBER
///
/// The FOLLOW set for the first 'mulexpr' in the 'addexpr' rule is ['+'], which means
/// that while processing 'mulexpr' in that position or anything recursively deeper than
/// it, if we see a '+' symbol in the input, we should use it to escape the deeper rules
/// and resume processing the 'addexpr'.
/// </summary>
/// <param name="table">The syntax table that records the grammar containing this node.</param>
/// <param name="node">The node for which you would like to retrieve the FOLLOW set.</param>
/// <returns>The return value is a dictionary whose keys are the FOLLOW set (and whose values are all zero).</returns>
static Int32Int32Dict ParserSyntaxTable_ComputeFollowSet(ParserSyntaxTable table, ParserSyntaxNode node)
{
	Int32Int32Dict followSet = Int32Int32Dict_Create();

	if (node->nextTerminals != NULL) {
		// This node class has terminals that are in its next-state dictionary, so add them to the FOLLOW set.
		Int32 *terminals = Int32Dict_GetKeys(node->nextTerminals);
		Int32 numTerminals = Int32Dict_Count(node->nextTerminals);
	
		while (numTerminals--) {
			Int32Int32Dict_Add(followSet, *terminals++, 0);
		}
	}

	if (node->nextNonterminals != NULL) {
		// This syntax class has nonterminals that are in its next-state dictionary, so add the union of their
		// FIRST sets to the FOLLOW set.
		Int32 *nextNonterminals = Int32Dict_GetKeys(node->nextNonterminals);
		Int32 numNextNonterminals = Int32Dict_Count(node->nextNonterminals);

		while (numNextNonterminals--) {
			// Get the FIRST set for this nonterminal.
			Symbol nextNonterminal = *nextNonterminals++;
			Int32Int32Dict firstSet = ParserSyntaxTable_GetFirstSet(table, nextNonterminal);

			Int32 *terminals = Int32Int32Dict_GetKeys(firstSet);
			Int32 numTerminals = Int32Int32Dict_Count(firstSet);

			// Union the full FIRST set for this nonterminal into the FOLLOW set for this node.
			while (numTerminals--) {
				Int32Int32Dict_Add(followSet, *terminals++, 0);
			}
		}
	}

	return followSet;
}

/// <summary>
/// Compute the transition table that describes where to go from the given node.
/// This table is a straightforward dictionary mapping input symbols to next nodes.
/// </summary>
/// <param name="table">The syntax table that records the grammar containing this node.</param>
/// <param name="node">The node for which you would like to retrieve a transition-out table.</param>
/// <returns>A suitable transition table for that node, or NULL if the grammar is ambigious
/// (within the limitations of LL(1)) after this node.</returns>
static Int32Dict ParserSyntaxTable_ComputeTransitionTable(ParserSyntaxTable table, ParserSyntaxNode node)
{
	Int32Dict transitionTable = Int32Dict_Create();
	
	if (node->nextTerminals != NULL) {
		// This node has terminals that follow it, so create transitions for them.
		Int32DictKeyValuePair *pair = Int32Dict_GetAll(node->nextTerminals);
		Int32 numPairs = Int32Dict_Count(node->nextTerminals);

		for (; numPairs--; pair++) {
			if (!Int32Dict_Add(transitionTable, pair->key, pair->value)) {
				// Well, nuts.  We have a conflict, i.e., an ambiguous grammar.
				return NULL;
			}
		}
	}

	if (node->nextNonterminals != NULL) {
		// This node has nonterminals that follow it, so create transitions for their respective FIRST sets.
		Int32DictKeyValuePair *pair = Int32Dict_GetAll(node->nextNonterminals);
		Int32 numPairs = Int32Dict_Count(node->nextNonterminals);

		for (; numPairs--; pair++) {
			// Get the FIRST set for this nonterminal.
			Int32Int32Dict firstSet = ParserSyntaxTable_GetFirstSet(table, pair->key);

			Int32 *terminal = Int32Int32Dict_GetKeys(firstSet);
			Int32 numTerminals = Int32Int32Dict_Count(firstSet);

			// Union the full FIRST set for this nonterminal into the FOLLOW set for this node.
			for (; numTerminals--; terminal++) {
				if (!Int32Dict_Add(transitionTable, *terminal, pair->value)) {
					// Well, nuts.  We have a conflict, i.e., an ambiguous grammar.
					return NULL;
				}
			}
		}
	}

	return transitionTable;
}

Int32Int32Dict ParserSyntaxTable_GetFirstSet(ParserSyntaxTable table, Symbol nonterminal)
{
	Int32Int32Dict firstSet;

	if (table->firstSets == NULL) {
		table->firstSets = Int32Dict_Create();
	}

	if (!Int32Dict_TryGetValue(table->firstSets, nonterminal, &firstSet)) {
		firstSet = ParserSyntaxTable_ComputeFirstSet(table, nonterminal);
		Int32Dict_Add(table->firstSets, nonterminal, firstSet);
	}
	
	return firstSet;
}

/// <summary>
/// Get the FOLLOW set, which describes which input symbols should cause us to
/// exit the given nonterminal node.  For example, given this grammar:
///
///    addexpr ::= mulexpr '+' addexpr | mulexpr
///    mulexpr ::= unary '*' mulexpr | unary
///    unary ::= '-' unary | term
///    term ::= IDENT | NUMBER
///
/// The FOLLOW set for the first 'mulexpr' in the 'addexpr' rule is ['+'], which means
/// that while processing 'mulexpr' in that position or anything recursively deeper than
/// it, if we see a '+' symbol in the input, we should use it to escape the deeper rules
/// and resume processing the 'addexpr'.
/// </summary>
/// <param name="table">The syntax table that records the grammar containing this node.</param>
/// <param name="node">The node for which you would like to retrieve the FOLLOW set.</param>
/// <returns>The return value is a dictionary whose keys are the FOLLOW set (and whose values are all zero).</returns>
Int32Int32Dict ParserSyntaxTable_GetFollowSet(ParserSyntaxTable table, ParserSyntaxNode node)
{
	Int32Int32Dict followSet;

	if (table->followSets == NULL) {
		table->followSets = Int32Dict_Create();
	}

	if (!Int32Dict_TryGetValue(table->followSets, node->nodeID, &followSet)) {
		followSet = ParserSyntaxTable_ComputeFollowSet(table, node);
		Int32Dict_Add(table->firstSets, node->nodeID, followSet);
	}

	return followSet;
}

/// <summary>
/// Get the transition table that describes where to go from the given node.
/// This table is a straightforward dictionary mapping input symbols to next nodes.
/// </summary>
/// <param name="table">The syntax table that records the grammar containing this node.</param>
/// <param name="node">The node for which you would like to retrieve a transition-out table.</param>
/// <returns>A suitable transition table for that node, or NULL if the grammar is ambigious
/// (within the limitations of LL(1)) after this node.</returns>
Int32Dict ParserSyntaxTable_GetTransitionTable(ParserSyntaxTable table, ParserSyntaxNode node)
{
	Int32Dict transitionTable;

	if (table->transitionTables == NULL) {
		table->transitionTables = Int32Dict_Create();
	}

	if (!Int32Dict_TryGetValue(table->transitionTables, node->nodeID, &transitionTable)) {
		transitionTable = ParserSyntaxTable_ComputeTransitionTable(table, node);
		Int32Dict_Add(table->transitionTables, node->nodeID, transitionTable);
	}

	return transitionTable;
}

/// <summary>
/// Add a new syntax rule to the existing syntax table, virtually forking the table
/// as necessary.
/// </summary>
/// <param name="parser">The parser that owns this syntax table.  This will be used
/// for error-reporting if the rule is invalid.</param>
/// <param name="table">The table that will contain the new syntax rule.
/// If more than one scope references this table, it will be virtually forked, and
/// the new copy will be the one that gets the new rule.</param>
/// <param name="rule">The new rule to add to this syntax table.</param>
/// <returns>True on success, or False if one or more errors was produced.</returns>
Bool ParserSyntaxTable_AddRule(Parser parser, ParserSyntaxTable *table, SmileSyntax rule)
{
	ParserSyntaxTable syntaxTable;
	ParserSyntaxClass syntaxClass, newSyntaxClass;
	ParserSyntaxNode node, parentNode;
	SmileList pattern;
	SmileList replacementVariables, replacementVariablesTail;
	SmileNonterminal nonterminal;
	Int numNodes;
	Int repeatKind, repeatSeparator;
	Int numReplacementVariables;
	Symbol *destSymbol;
	
	numReplacementVariables = 0;
	replacementVariables = replacementVariablesTail = NullList;

	// First, ensure that the syntax table can be safely modified by vforking it as
	// necessary.
	syntaxTable = *table;
	if (syntaxTable->referenceCount > 1) {
		syntaxTable = ParserSyntaxTable_VFork(syntaxTable);
		*table = syntaxTable;
	}

	// Locate the appropriate class within the syntax table for the new rule's
	// nonterminal.  If no such class exists, create one.
	if (!Int32Dict_TryGetValue(syntaxTable->syntaxClasses, rule->nonterminal, &syntaxClass)) {
		// Don't have a preexisting class, so create one, and add it to the set of known
		// classes within this table.
		syntaxClass = ParserSyntaxClass_CreateNew();
		Int32Dict_Add(syntaxTable->syntaxClasses, rule->nonterminal, syntaxClass);

		// If this is a special class, glue it into the special slots in this syntax table.
		switch (rule->nonterminal) {
			case SMILE_SPECIAL_SYMBOL_STMT:
				syntaxTable->stmtClass = syntaxClass;
				break;
			case SMILE_SPECIAL_SYMBOL_EXPR:
				syntaxTable->exprClass = syntaxClass;
				break;
			case SMILE_SPECIAL_SYMBOL_CMPEXPR:
				syntaxTable->cmpExprClass = syntaxClass;
				break;
			case SMILE_SPECIAL_SYMBOL_ADDEXPR:
				syntaxTable->addExprClass = syntaxClass;
				break;
			case SMILE_SPECIAL_SYMBOL_MULEXPR:
				syntaxTable->mulExprClass = syntaxClass;
				break;
			case SMILE_SPECIAL_SYMBOL_BINARYEXPR:
				syntaxTable->binaryExprClass = syntaxClass;
				break;
			case SMILE_SPECIAL_SYMBOL_UNARY:
				syntaxTable->unaryClass = syntaxClass;
				break;
			case SMILE_SPECIAL_SYMBOL_POSTFIX:
				syntaxTable->postfixClass = syntaxClass;
				break;
			case SMILE_SPECIAL_SYMBOL_TERM:
				syntaxTable->termClass = syntaxClass;
				break;
		}
	}

	// The 'syntaxClass' variable now contains a valid root for this rule.
	// So walk down the rule's pattern and repeatedly invoke Extend() to search for
	// or build the tree of syntax nodes for it.
	numNodes = 0;
	parentNode = node = NULL;
	for (pattern = rule->pattern; pattern != NullList; pattern = LIST_REST(pattern)) {

		// As a safety check against broken dynamic data structures, we limit syntax trees
		// to a maximum depth of 256 nodes.  (There is no sensible grammar that requires more
		// than 256 nodes for a single rule that can't be broken up into smaller rules; in
		// fact, thanks to normal forms like CNF and GNF, you shouldn't technically need more
		// than 2 nodes per rule, so a limit of 256 is plenty.)
		if (numNodes >= 256) {
			Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position,
				SyntaxRuleTooDeepError));
			return False;
		}
	
		// Try to extend the rule with the next terminal or nonterminal in the pattern.
		switch (SMILE_KIND(pattern->a)) {
		
			default:
				Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position,
					InvalidSyntaxRuleError));
				return False;
		
			case SMILE_KIND_SYMBOL:
				// This is a terminal in the pattern (i.e., a keyword or symbol).
				if (!ParserSyntaxClass_Extend(parser, rule->position, syntaxClass, parentNode,
					((SmileSymbol)pattern->a)->symbol, 0, 0, 0,
					&node, &newSyntaxClass))
					return False;
				break;
			
			case SMILE_KIND_NONTERMINAL:
				// This is a nonterminal in the pattern (i.e., a reference to another rule).
				nonterminal = ((SmileNonterminal)pattern->a);

				// Figure out what kind of repeat they want, if any.
				if (nonterminal->repeat == Smile_KnownSymbols.question_mark) {
					repeatKind = '?';
					if (numNodes == 0) {
						Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position,
							CantRepeatFirstNonterminalError));
						return False;
					}
				}
				else if (nonterminal->repeat == Smile_KnownSymbols.star) {
					repeatKind = '*';
					if (numNodes == 0) {
						Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position,
							CantRepeatFirstNonterminalError));
						return False;
					}
				}
				else if (nonterminal->repeat == Smile_KnownSymbols.plus) {
					repeatKind = '+';
				}
				else if (nonterminal->repeat == 0) {
					repeatKind = 0;
				}
				else {
					Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position,
						String_FormatString(IllegalRepeatSymbolError, SymbolTable_GetName(Smile_SymbolTable, nonterminal->repeat))));
					return False;
				}
			
				// Figure out what kind of separator they want, if any.
				if (nonterminal->separator == Smile_KnownSymbols.comma) {
					repeatSeparator = ',';
				}
				else if (nonterminal->separator == Smile_KnownSymbols.semicolon) {
					repeatSeparator = ';';
				}
				else if (nonterminal->separator == 0) {
					repeatSeparator = 0;
				}
				else {
					Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position,
						String_FormatString(IllegalSeparatorSymbolError, SymbolTable_GetName(Smile_SymbolTable, nonterminal->separator))));
					return False;
				}
			
				// We have the requirements figured out now, so add the node to the syntax class.
				if (!ParserSyntaxClass_Extend(parser, rule->position, syntaxClass, parentNode,
					nonterminal->nonterminal, nonterminal->name, repeatKind, repeatSeparator,
					&node, &newSyntaxClass))
					return False;
			
				// Save this nonterminal's variable name; we'll need it later, when resolving the variables during application.
				LIST_APPEND(replacementVariables, replacementVariablesTail, SmileSymbol_Create(nonterminal->name));
				numReplacementVariables++;
				break;
		}
	
		parentNode = node;
		numNodes++;
	}

	if (node == NULL) {
		// Empty/null rule pattern is illegal :-/
		Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position,
			InvalidSyntaxRuleError));
		return False;
	}

	if (node->replacement != NullObject) {
		// We already have a parse rule here.  We can't legally replace it, even with an
		// identical rule, so we have to abort.
		Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position, DuplicateSyntaxRuleError));
		return False;
	}

	if (rule->replacement == NullObject) {
		Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position, NullReplacementError));
		return False;
	}

	// Finally!  Assign the replacement, creating the rule for real.
	node->replacement = rule->replacement;

	// And set up the replacement variables so all the substitution logic will work when parsing with it.
	node->replacementVariables = destSymbol = GC_MALLOC_RAW_ARRAY(Symbol, numReplacementVariables);
	node->numReplacementVariables = (UInt16)numReplacementVariables;
	for (; SMILE_KIND(replacementVariables) == SMILE_KIND_LIST; replacementVariables = LIST_REST(replacementVariables)) {
		*destSymbol++ = ((SmileSymbol)(replacementVariables->a))->symbol;
	}

	return True;
}

/// <summary>
/// Set up the default syntax rules for normal Smile code.  This adds in the common
/// stuff like if-then-else and while-statements.
/// </summary>
/// <param name="parser">The parser that owns this syntax table.  This will be used
/// for error-reporting if the rule is invalid.</param>
/// <param name="table">The syntax table to add the default rules to.
/// If more than one scope references this table, it will be virtually forked, and
/// the new copy will be the one that gets the new rules.</param>
/// <returns>True on success, or False if one or more errors was produced.</returns>
Bool ParserSyntaxTable_SetupDefaultRules(Parser parser, ParserSyntaxTable *table)
{
	UNUSED(parser);
	UNUSED(table);

	return True;
}
