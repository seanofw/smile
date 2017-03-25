
#ifndef __SMILE_SMILETYPES_SMILESYNTAX_H__
#define __SMILE_SMILETYPES_SMILESYNTAX_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif
#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif
#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif
#ifndef __SMILE_PARSING_LEXER_H__
#include <smile/parsing/lexer.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Object declarations

/// <summary>
/// This object represents the shape of a single #syntax declaration (a single production rule):
///	#syntax STMT [pattern] => [replacement]
/// </summary>
struct SmileSyntaxInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	Symbol nonterminal;	// The nonterminal on the left side of this rule.
	SmileList pattern;	// The syntax pattern to match against; this is either SmileSymbol or SmileNonterminal objects.
	SmileObject replacement;	// The replacement form to substitute; this is any legal Smile object.
		
	LexerPosition position;	// The source location at which this syntax rule was declared.
};

/// <summary>
/// This object represents the shape of a nonterminal in a #syntax declaration.
/// </summary>
struct SmileNonterminalInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	Symbol nonterminal;	// What kind of nonterminals this should match.
	Symbol name;	// The name to use in the substitution for the matched syntax construct.
	Symbol repeat;	// Either 0 (no repetition), '?' for optional, '*' for zero-or-more, '+' for one-or-more.
	Symbol separator;	// Either 0 (no separator), ',' for comma, or ';' for semicolon.
};

//-------------------------------------------------------------------------------------------------
//  Public object interface

SMILE_API_DATA SmileVTable SmileSyntax_VTable;

SMILE_API_FUNC SmileSyntax SmileSyntax_Create(Symbol nonterminal, SmileList pattern, SmileObject replacement, LexerPosition position);
SMILE_API_FUNC Bool SmileSyntax_Equals(SmileSyntax a, SmileSyntax b);
SMILE_API_FUNC String SmileSyntax_ToString(SmileSyntax self);

SMILE_API_DATA SmileVTable SmileNonterminal_VTable;

SMILE_API_FUNC SmileNonterminal SmileNonterminal_Create(Symbol nonterminal, Symbol name, Symbol repeat, Symbol separator);

#endif
