
#ifndef __SMILE_ENV_KNOWNOBJECTS_H__
#define __SMILE_ENV_KNOWNOBJECTS_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

struct KnownObjectsStruct {

	SmileNull NullInstance;

	SmileBool TrueObj;
	SmileBool FalseObj;
	SmileBool BooleanObjs[2];

	SmileChar Chars[256];
	SmileUni Unis[1024];

	SmileByte Bytes[256];
	SmileByte ZeroByte;
	SmileByte OneByte;
	SmileByte NegOneByte;

	SmileInteger16 SmallInt16s[201];
	SmileInteger16 ZeroInt16;
	SmileInteger16 OneInt16;
	SmileInteger16 NegOneInt16;

	SmileInteger32 SmallInt32s[201];
	SmileInteger32 ZeroInt32;
	SmileInteger32 OneInt32;
	SmileInteger32 NegOneInt32;

	SmileInteger64 SmallInt64s[201];
	SmileInteger64 ZeroInt64;
	SmileInteger64 OneInt64;
	SmileInteger64 NegOneInt64;

	SmileInteger128 SmallInt128s[201];
	SmileInteger128 ZeroInt128;
	SmileInteger128 OneInt128;
	SmileInteger128 NegOneInt128;

	SmileSymbol ObjectSymbol;
	SmileSymbol ListSymbol;
	SmileSymbol RegexSymbol;

	// The twenty-three core special forms.
	SmileSymbol _setSymbol, _opsetSymbol, _includeSymbol;
	SmileSymbol _ifSymbol, _whileSymbol, _tillSymbol;
	SmileSymbol _fnSymbol, _quoteSymbol, _scopeSymbol, _prog1Symbol, _prognSymbol, _returnSymbol, _catchSymbol;
	SmileSymbol _notSymbol, _orSymbol, _andSymbol, _eqSymbol, _neSymbol;
	SmileSymbol _newSymbol, _dotSymbol, _indexSymbol, _isSymbol, _typeofSymbol;

	// Method names used at parse-time.
	SmileSymbol getMemberSymbol;
	SmileSymbol setMemberSymbol;
	SmileSymbol ofSymbol;
	SmileSymbol combineSymbol;
	SmileSymbol consSymbol;
	SmileSymbol typeSymbol;
	SmileSymbol defaultSymbol;
	SmileSymbol restSymbol;
	SmileSymbol joinSymbol;
	SmileSymbol rangeToSymbol;

	// Operator names used at parse-time.
	SmileSymbol eqSymbol, neSymbol, ltSymbol, gtSymbol, leSymbol, geSymbol;
	SmileSymbol plusSymbol, minusSymbol, starSymbol, slashSymbol;
};

extern void KnownObjects_Setup(struct KnownObjectsStruct *knownObjects, struct KnownSymbolsStruct *knownSymbols);

#endif
