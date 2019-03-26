
#ifndef __SMILE_SMILETYPES_SMILELOANWORD_H__
#define __SMILE_SMILETYPES_SMILELOANWORD_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
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
/// This object represents the shape of a single #loanword declaration (a single substitution rule):
///	#loanword name: #/pattern/ => [replacement]
/// </summary>
struct SmileLoanwordInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	Symbol name;				// The unique name for this loanword.
	Regex regex;				// The regex pattern to capture for its content.
	SmileObject replacement;	// The replacement form to substitute; this is any legal Smile object.

	LexerPosition position;		// The source location at which this loanword was declared.
	ParserLoanwordTable table;	// Which parse table owns this declaration.

	Bool builtin;				// Whether this is a reserved, built-in loanword.
};

//-------------------------------------------------------------------------------------------------
//  Public object interface

SMILE_API_DATA SmileVTable SmileLoanword_VTable;

SMILE_API_FUNC SmileLoanword SmileLoanword_Create(Symbol name, Regex pattern, SmileObject replacement, LexerPosition position);
SMILE_API_FUNC Bool SmileLoanword_Equals(SmileLoanword a, SmileLoanword b);
SMILE_API_FUNC String SmileLoanword_ToString(SmileLoanword self);

#endif
