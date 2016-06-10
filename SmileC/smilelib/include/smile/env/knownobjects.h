
#ifndef __SMILE_ENV_KNOWNOBJECTS_H__
#define __SMILE_ENV_KNOWNOBJECTS_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

struct KnownObjectsStruct {

	SmileObject Object;
	SmileNull NullInstance;

	SmileBool TrueObj;
	SmileBool FalseObj;

	SmileByte Bytes[256];
	SmileByte ZeroByte;
	SmileByte OneByte;

	SmileInteger16 SmallInt16s[201];
	SmileInteger16 ZeroInt16;
	SmileInteger16 OneInt16;

	SmileInteger32 SmallInt32s[201];
	SmileInteger32 ZeroInt32;
	SmileInteger32 OneInt32;

	SmileInteger64 SmallInt64s[201];
	SmileInteger64 ZeroInt64;
	SmileInteger64 OneInt64;

	SmileChar Chars[256];

	SmileUChar UChars[256];

	SmileSymbol ObjectSymbol;
	SmileSymbol ListSymbol;
	SmileSymbol PairSymbol;
	SmileSymbol RangeSymbol;

	SmileSymbol fnSymbol, quoteSymbol;
	SmileSymbol joinSymbol, ofSymbol;
	SmileSymbol prognSymbol, scopeSymbol;
	SmileSymbol opEqualsSymbol;
	SmileSymbol equalsSymbol;
	SmileSymbol typeSymbol;
	SmileSymbol newSymbol;

	SmileSymbol andSymbol;
	SmileSymbol orSymbol;
	SmileSymbol notSymbol;

	SmileSymbol eqSymbol;
	SmileSymbol neSymbol;
	SmileSymbol ltSymbol;
	SmileSymbol gtSymbol;
	SmileSymbol leSymbol;
	SmileSymbol geSymbol;
	SmileSymbol supereqSymbol;
	SmileSymbol superneSymbol;
	SmileSymbol isSymbol;

	SmileSymbol plusSymbol;
	SmileSymbol minusSymbol;
	SmileSymbol starSymbol;
	SmileSymbol slashSymbol;

	SmileSymbol typeofSymbol;

	SmileSymbol getMemberSymbol;
	SmileSymbol setMemberSymbol;
};

extern void KnownObjects_Preload(struct KnownObjectsStruct *knownObjects, struct KnownSymbolsStruct *knownSymbols);

#endif
