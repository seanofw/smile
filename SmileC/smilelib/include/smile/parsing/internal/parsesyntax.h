
#ifndef __SMILE_PARSING_INTERNAL_PARSESYNTAX_H__
#define __SMILE_PARSING_INTERNAL_PARSESYNTAX_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif
#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif
#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif
#ifndef __SMILE_PARSING_LEXER_H__
#include <smile/parsing/lexer.h>
#endif

typedef struct ParserSyntaxNodeStruct *ParserSyntaxNode;
typedef struct ParserSyntaxClassStruct *ParserSyntaxClass;
typedef struct ParserSyntaxTableStruct *ParserSyntaxTable;

//-------------------------------------------------------------------------------------------------
//  Internal types

/// <summary>
/// A single node in a syntax tree.  <em>NOT</em> copy-on-write.
/// Each node may be one of several possible token kinds:  NAME  ,  ;  :  (  )  {  }  [  ]
/// Note that root nodes must always be KEYWORDs, and VARIABLEs may not be
/// followed by certain tokens, depending on what class of nonterminal they are.
/// </summary>
struct ParserSyntaxNodeStruct {
	Int32 referenceCount;	// For copy-on-write behavior.
		
	Int8 repetitionKind;	// Either 0 (no repetition), '?' for optional, '*' for zero-or-more, '+' for one-or-more.
	Int8 repetitionSep;	// Either 0 (no separator), ',' for comma, or ';' for semicolon.
	Bool isNextNonterminal;	// Does the dictionary consist of a single nonterminal?
	Int8 reserved;	// (reserved for padding, always zero)
		
	Symbol name;	// The keyword/symbol or nonterminal name.
	Symbol variable;	// The variable to emit on a nonterminal match, 0 if this is a keyword/symbol.
		
	SmileList replacement;	// The replacement form, if this is the final node in the chain.
		
	Int32Dict nextDict;	// Possible next states, keyed by keyword/symbol or nonterminal name.
};

/// <summary>
/// A single syntax precedence class.  Precedence classes are wide trees composed of ParserSyntaxNodes, with
/// each root branch starting with a unique (named) terminal.
/// </summary>
struct ParserSyntaxClassStruct {
	Int referenceCount;	// For copy-on-write behavior.
		
	Int32Dict rootDict;	// Possible root states, keyed by keyword/symbol or nonterminal name.
	Bool isRootNonterminal;	// Does the dictionary consist of a single nonterminal?
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
		
	// Special syntax classes, for fast lookup.  These trees correspond to known built-in syntax-production names.
	ParserSyntaxClass stmtClass;	
	ParserSyntaxClass exprClass;	
	ParserSyntaxClass cmpClass;	// Note: RValue of CMP (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass addSubClass;	// Note: RValue of ADDSUB (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass mulDivClass;	// Note: RValue of MULDIV (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass binaryClass;	// Note: RValue of BINARY (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass unaryClass;	
	ParserSyntaxClass postfixClass;	// Note: RValue of POSTFIX (or more restrictive form) is assumed to exist before tree.
	ParserSyntaxClass termClass;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_FUNC ParserSyntaxTable ParserSyntaxTable_CreateNew(void);
SMILE_API_FUNC ParserSyntaxTable ParserSyntaxTable_VFork(ParserSyntaxTable table);
SMILE_API_FUNC ParseError ParserSyntaxTable_AddRule(ParserSyntaxTable *table, SmileSyntax rule);
SMILE_API_FUNC ParseError ParserSyntaxTable_SetupDefaultRules(ParserSyntaxTable *table);

#endif
