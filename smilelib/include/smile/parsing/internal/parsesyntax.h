
#ifndef __SMILE_PARSING_INTERNAL_PARSESYNTAX_H__
#define __SMILE_PARSING_INTERNAL_PARSESYNTAX_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif
#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif
#ifndef __SMILE_DICT_INT32INT32DICT_H__
#include <smile/dict/int32int32dict.h>
#endif
#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif
#ifndef __SMILE_PARSING_LEXER_H__
#include <smile/parsing/lexer.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Internal types

/// <summary>
/// A single node in a syntax tree.  <em>NOT</em> copy-on-write.
/// Each node may be one of several possible token kinds:  NAME  ,  ;  :  (  )  {  }  [  ]
/// Note that root nodes must always be KEYWORDs, and VARIABLEs may not be
/// followed by certain tokens, depending on what class of nonterminal they are.
/// </summary>
struct ParserSyntaxNodeStruct {
	UInt32 referenceCount;	// For copy-on-write behavior.
	UInt32 nodeID;	// A unique ID for this node (used for caching FOLLOW sets and transition tables for nonterminals).
	Int32Dict nextTerminals;	// Possible next states, keyed by keyword/symbol.
	Int32Dict nextNonterminals;	// Possible next states, keyed by nonterminal.
		
	SmileObject replacement;	// The replacement form, if this is the final node in the chain.
	Symbol *replacementVariables;	// All variable names from the pattern (only filled in if a replacement form is provided as well).
		
	Int8 repetitionKind;	// Either 0 (no repetition), '?' for optional, '*' for zero-or-more, '+' for one-or-more.
	Int8 repetitionSep;	// Either 0 (no separator), ',' for comma, or ';' for semicolon.
	UInt16 numReplacementVariables;	// The number of replacement variable names.
		
	Symbol name;	// The keyword/symbol or nonterminal name.
	Symbol variable;	// The variable to emit on a nonterminal match, 0 if this is a keyword/symbol.
};

/// <summary>
/// A single syntax precedence class.  Precedence classes are wide trees composed of ParserSyntaxNodes, with
/// each root branch starting with a unique (named) terminal.
/// </summary>
/// <remarks>Important: SyntaxNode structurally inherits from SyntaxClass.  Do not change
/// the layout of one struct without changing the layout of the other, because some code depends
/// on the layouts matching.</remarks>
struct ParserSyntaxClassStruct {
	UInt32 referenceCount;	// For copy-on-write behavior.
	UInt32 nodeID;	// A unique ID for this node (used for caching FOLLOW sets and transition tables for nonterminals).
	Int32Dict nextTerminals;	// Possible root states, keyed by keyword/symbol.
	Int32Dict nextNonterminals;	// Possible root states, keyed by nonterminal.
		
	SmileObject replacement;	// The replacement form (always NullObject for the class root).
};

/// <summary>
/// A syntax table describes the complete set of syntax rules that exist within a single scope (ParseScope).
/// Every scope gets its own unique syntax table, copied from its parent scope.  (But to save on memory
/// and performance, we use copy-on-write sharing, so *actual* new syntax tables don't get created unless
/// absolutely necessary.)
/// </summary>
struct ParserSyntaxTableStruct {
	Int referenceCount;	// For copy-on-write behavior.
	Int32Dict syntaxClasses;	// Key is syntax level, value is a SyntaxClass.
		
	Int32Dict firstSets;	// Cached FIRST sets for the nonterminals in this table.  Not copied when we fork the table.
	Int32Dict followSets;	// Cached FOLLOW sets for the nonterminal nodes in this table.  Not copied when we fork the table.
	Int32Dict transitionTables;	// Cached transition tables for every nonterminal node in this table.  Not copied when we fork the table.
		
	// Special syntax classes, for fast lookup.  These trees correspond to known built-in syntax-production names.
	ParserSyntaxClass stmtClass;	
	ParserSyntaxClass exprClass;	
	ParserSyntaxClass cmpExprClass;	// Note: RValue of CMPEXPR (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass addExprClass;	// Note: RValue of ADDEXPR (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass mulExprClass;	// Note: RValue of MULEXPR (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass binaryExprClass;	// Note: RValue of BINARYEXPR (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass prefixExprClass;	
	ParserSyntaxClass postfixExprClass;	// Note: RValue of POSTFIXEXPR (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass termClass;	
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_FUNC ParserSyntaxTable ParserSyntaxTable_CreateNew(void);
SMILE_API_FUNC Bool ParserSyntaxTable_AddRule(Parser parser, ParserSyntaxTable *table, SmileSyntax rule);
SMILE_API_FUNC Bool ParserSyntaxTable_SetupDefaultRules(Parser parser, ParserSyntaxTable *table);
SMILE_API_FUNC Int32Int32Dict ParserSyntaxTable_GetFirstSet(ParserSyntaxTable table, Symbol nonterminal);
SMILE_API_FUNC Int32Int32Dict ParserSyntaxTable_GetFollowSet(ParserSyntaxTable table, ParserSyntaxNode node);
SMILE_API_FUNC Int32Dict ParserSyntaxTable_GetTransitionTable(ParserSyntaxTable table, ParserSyntaxNode node);

/// <summary>
/// Increase the reference count for the given syntax table, so that it knows
/// to fork itself if it is subsequently modified.
/// </summary>
Inline void ParserSyntaxTable_AddRef(ParserSyntaxTable table)
{
	table->referenceCount++;
}

/// <summary>
/// Decrease the reference count for the given syntax table, so that it knows
/// it no longer needs to fork itself if it is subsequently modified.
/// </summary>
Inline void ParserSyntaxTable_RemoveRef(ParserSyntaxTable table)
{
	table->referenceCount--;
}

#endif
