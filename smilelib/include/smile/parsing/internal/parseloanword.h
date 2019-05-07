
#ifndef __SMILE_PARSING_INTERNAL_PARSELOANWORD_H__
#define __SMILE_PARSING_INTERNAL_PARSELOANWORD_H__

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

/// <summary>
/// A loanword table describes the complete set of loanword rules that exist within a single scope (ParseScope).
/// Every scope gets its own unique loanword table, copied from its parent scope.  (But to save on memory
/// and performance, we use copy-on-write sharing, so *actual* new loanword tables don't get created unless
/// absolutely necessary.)
/// </summary>
struct ParserLoanwordTableStruct {
	Int referenceCount;		// For copy-on-write behavior.
	Int32Dict definitions;
};

SMILE_API_FUNC ParserLoanwordTable ParserLoanwordTable_CreateNew(void);
SMILE_API_FUNC void ParserLoanwordTable_AddReservedKeywords(ParserLoanwordTable table);
SMILE_API_FUNC Bool ParserLoanwordTable_AddRule(Parser parser, ParserLoanwordTable *table, SmileLoanword loanword);

/// <summary>
/// Increase the reference count for the given loanword table, so that it knows
/// to fork itself if it is subsequently modified.
/// </summary>
Inline void ParserLoanwordTable_AddRef(ParserLoanwordTable table)
{
	table->referenceCount++;
}

/// <summary>
/// Decrease the reference count for the given loanword table, so that it knows
/// it no longer needs to fork itself if it is subsequently modified.
/// </summary>
Inline void ParserLoanwordTable_RemoveRef(ParserLoanwordTable table)
{
	table->referenceCount--;
}

#endif
