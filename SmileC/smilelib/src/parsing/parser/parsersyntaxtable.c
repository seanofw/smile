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

	syntaxNode->isNonterminal = False;
	syntaxNode->referenceCount = 1;
	syntaxNode->nextDict = NULL;

	syntaxNode->name = name;
	syntaxNode->variable = variable;

	syntaxNode->repetitionKind = (Int8)repetitionKind;
	syntaxNode->repetitionSep = (Int8)repetitionSep;

	syntaxNode->replacement = NullObject;

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
	cls->nextDict = NULL;
	cls->isNonterminal = False;

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
	newCls->nextDict = Int32Dict_Clone(cls->nextDict, ParserSyntaxClass_DictClone, NULL);
	newCls->isNonterminal = cls->isNonterminal;

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
	Int32DictKeyValuePair firstPair;
	Int32Dict nextDict;
	Bool nextIsNonterminal;

	nextIsNonterminal = (parent == NULL ? newCls->isNonterminal : parent->isNonterminal);
	nextDict = (parent == NULL ? newCls->nextDict : parent->nextDict);

	if (nextDict == NULL) {
		// No dictionary at this level yet.

		if (parent == NULL) {
			// Empty root dictionary, so create it, and add the first node.
			if (newCls->referenceCount > 1) {
				newCls = ParserSyntaxClass_VFork(newCls);
			}
			newCls->nextDict = Int32Dict_CreateWithSize(4);
			newCls->isNonterminal = (variable != 0);
			syntaxNode = ParserSyntaxNode_CreateInternal(name, variable, repetitionKind, repetitionSep);
			Int32Dict_Add(newCls->nextDict, name, syntaxNode);
		}
		else {
			// Make sure we're not adding a nonterminal in such a way that the resulting tree
			// is no longer deterministically parseable.
			if (variable != 0 && parent->replacement != NullObject) {
				Parser_AddError(parser, position, "Cannot add syntax rule because it forks the tree on the nonterminal \"%S %S\".",
					SymbolTable_GetName(Smile_SymbolTable, name),
					SymbolTable_GetName(Smile_SymbolTable, variable));
				return False;
			}

			// There is no child dictionary yet, so create it, and add the first node.
			if (newCls->referenceCount > 1) {
				newCls = ParserSyntaxClass_VFork(newCls);
			}
			parent->nextDict = Int32Dict_CreateWithSize(4);
			parent->isNonterminal = (variable != 0);
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
				return False;
			}
			if (syntaxNode->name != name || syntaxNode->variable != variable
				|| syntaxNode->repetitionKind != repetitionKind || syntaxNode->repetitionSep != repetitionSep) {
				// Error: Can't fork nonterminal --> different nonterminal.
				Parser_AddError(parser, position, "Cannot add syntax rule because it forks the tree on the nonterminal \"%S %S\".",
					SymbolTable_GetName(Smile_SymbolTable, name),
					SymbolTable_GetName(Smile_SymbolTable, variable));
				return False;
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
				return False;
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
static ParserSyntaxTable ParserSyntaxTable_VFork(ParserSyntaxTable table)
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
/// Test to ensure that this new rule with an initial nonterminal will not result
/// in an infinite loop when trying to match it during the actual parsing.
/// </summary>
/// <remarks>
/// <p>This starts at the given root for the new rule (i.e., the nonterminal/class that is
/// about to have a new rule assigned to it), and recursively traverses through the list
/// of all possible initial nonterminals pointed to by its next state.  This sequence
/// must be a directed list that terminates, and not circular; and this function ensures
/// that the precondition that it is a directed list continues to hold.</p>
///
/// <p>What kind of rules can generate an invalid structure?  Consider the simple rules below:</p>
///
/// <ul>
/// <li><code>add ::= mul '+' mul | mul</code></li>
/// <li><code>mul ::= add '*' add | add</code></li>
/// </ul>
///
/// <p>In this grammar, the author is clearly trying to write a grammar for arithmetic,
/// but got the precedence rules wrong in the defintion of 'mul'.  Because 'add' is
/// defined in terms of 'mul', and 'mul' is defined in terms of 'add', this grammar
/// would result in an infinite loop trying to resolve either 'add' or 'mul':  The
/// parser would arrive in the 'add' rule, see that 'mul' is the first nonterminal,
/// go to the 'mul' rule, see that 'add' is the first nonterminal, go to the 'add'
/// rule, and so on.  This grammar is neither LL(k) nor LR(k), and in fact, would cause
/// most other parser-generators to fail with an error as well.</p>
///
/// </p>(There are scenarios that involve more legitimate grammars than this one, but
/// this is an easy case to understand.)</p>
///
/// <p>Other parser-generators can handle some variations on this that pop up in other
/// LL(k) and LR(k) grammars, but the Smile internal parser cannot.  So to ensure that
/// the Smile parser doesn't loop forever, we recursively walk the tree from the root
/// of each new rule that starts with a nonterminal and make sure that we never arrive
/// back at the same root.</p>
///
/// <p>This search takes at most O(n) time, where n is the number of rules, and
/// potentially O(n) stack space for the recursion.  However, the number of rules is
/// usually bounded in practice, and is rarely greater than some small constant k.</p>
/// </remarks>
/// <param name="table">The syntax table that contains all the current rules.</param>
/// <param name="rootNonterminal">The root (class symbol) of the would-be new rule.</param>
/// <param name="firstNonterminal">The first nonterminal symbol of the would-be new rule.</param>
/// <returns>True if this rule is valid, or false if it would result in an
/// infinitely-circular grammar.</returns>
static Bool ParserSyntaxTable_ValidateRuleWithInitialNonterminal(ParserSyntaxTable table, Symbol rootNonterminal, Symbol firstNonterminal)
{
	Symbol currentNonterminal;
	ParserSyntaxClass syntaxClass;
	Int32DictKeyValuePair pair;

	// In this algorithm, we simply walk the list.  Previous invocations of this function
	// have ensured that the initial nonterminals of all the rules so far must form a list,
	// so we don't need to use algorithms the "trail of breadcrumbs" or "tortoise-and-hare"
	// to ensure we don't have circles; we merely need to see if the current set of rules,
	// starting from 'nextNonterminal', would eventually get us back to the 'rootNonterminal'
	// that would own 'nextNonterminal' if this rule were added.
	for (currentNonterminal = firstNonterminal; currentNonterminal != rootNonterminal; ) {

		// Find the rule pointed-to by the current nonterminal.
		if (!Int32Dict_TryGetValue(table->syntaxClasses, currentNonterminal, &syntaxClass)) {
			// If we got here, then we ended up nowhere (i.e., this set of syntax rules is
			// incomplete). This is not necessarily an error, since we may not have
			// encountered the rest of the syntax rules yet.
			return True;
		}
	
		if (!syntaxClass->isNonterminal) {
			// This syntax class starts with a terminal, so it can't result in a loop.
			return True;
		}

		// By definition, the dictionary associated with a nonterminal must have one entry
		// in it, since you cannot fork rules on nonterminals.
		pair = Int32Dict_GetFirst(syntaxClass->nextDict);
	
		// Move to the nonterminal that starts this rule.
		currentNonterminal = ((ParserSyntaxNode)pair.value)->name;
	}

	// Found a recursive loop.
	return False;
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

				// If this is a leftmost nonterminal, then go make sure this new rule wouldn't
				// result in an infinite loop during parsing.
				if (numNodes == 0) {
					if (!ParserSyntaxTable_ValidateRuleWithInitialNonterminal(syntaxTable, rule->nonterminal, nonterminal->nonterminal)) {
						Parser_AddMessage(parser, ParseMessage_Create(PARSEMESSAGE_ERROR, rule->position,
							IllegalInitialNonterminalError));
						return False;
					}
				}

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
