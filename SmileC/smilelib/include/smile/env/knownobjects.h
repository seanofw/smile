
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

	SmileInteger32 SmallInt32s[201];
	SmileInteger32 ZeroInt32;
	SmileInteger32 OneInt32;

	SmileInteger64 SmallInt64s[201];
	SmileInteger64 ZeroInt64;
	SmileInteger64 OneInt64;

	SmileSymbol ListSymbol;
	SmileSymbol joinSymbol, ofSymbol;
	SmileSymbol prognSymbol, scopeSymbol;
	SmileSymbol opEqualsSymbol;
	SmileSymbol equalsSymbol;
};

extern void KnownObjects_Preload(struct KnownObjectsStruct *knownObjects, struct KnownSymbolsStruct *knownSymbols);

#endif
