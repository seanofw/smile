
#ifndef __SMILE_SMILETYPES_SMILEPAIR_H__
#define __SMILE_SMILETYPES_SMILEPAIR_H__

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
//  Type declarations

struct SmilePairInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileObject left;
	SmileObject right;
};

struct SmilePairWithSourceInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileObject left;
	SmileObject right;
	LexerPosition position;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmilePair_VTable;

SMILE_API_FUNC SmilePair SmilePair_Create(SmileObject left, SmileObject right);
SMILE_API_FUNC SmilePair SmilePair_CreateWithSource(SmileObject left, SmileObject right, LexerPosition position);

#endif
