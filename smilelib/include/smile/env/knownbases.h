
#ifndef __SMILE_ENV_KNOWNBASES_H__
#define __SMILE_ENV_KNOWNBASES_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

struct KnownBasesStruct {

	// These are the core base objects that all other types derive from.
	SmileObject     Primitive;
	SmileUserObject   Object;
	SmileUserObject     Number;
	SmileUserObject       IntegerBase;
	SmileUserObject         Byte, Integer16, Integer32, Integer64, Integer128;
	SmileUserObject       RealBase;
	SmileUserObject         Real32, Real64, Real128;
	SmileUserObject       FloatBase;
	SmileUserObject         Float32, Float64, Float128;
	SmileUserObject     Enumerable;
	SmileUserObject       List;
	SmileUserObject       String;
	SmileUserObject       Range;
	SmileUserObject         NumericRange;
	SmileUserObject           IntegerRangeBase;
	SmileUserObject             ByteRange, Integer16Range, Integer32Range, Integer64Range, Integer128Range;
	SmileUserObject           RealRangeBase;
	SmileUserObject             Real32Range, Real64Range, Real128Range;
	SmileUserObject           FloatRangeBase;
	SmileUserObject             Float32Range, Float64Range, Float128Range;
	SmileUserObject       ArrayBase;
	SmileUserObject         Array;
	SmileUserObject         BoolArray;
	SmileUserObject         SymbolArray;
	SmileUserObject         StringArray;
	SmileUserObject         NumericArray;
	SmileUserObject           IntegerArrayBase;
	SmileUserObject             ByteArray, Integer16Array, Integer32Array, Integer64Array, Integer128Array;
	SmileUserObject           RealArrayBase;
	SmileUserObject             Real32Array, Real64Array, Real128Array;
	SmileUserObject           FloatArrayBase;
	SmileUserObject             Float32Array, Float64Array, Float128Array;
	SmileUserObject       MapBase;
	SmileUserObject         Map;
	SmileUserObject         StringMap;
	SmileUserObject         SymbolMap;
	SmileUserObject         NumericMap;
	SmileUserObject           IntegerMapBase;
	SmileUserObject             ByteMap, Integer16Map, Integer32Map, Integer64Map, Integer128Map;
	SmileUserObject           RealMapBase;
	SmileUserObject             Real32Map, Real64Map, Real128Map;
	SmileUserObject           FloatMapBase;
	SmileUserObject             Float32Map, Float64Map, Float128Map;
	SmileUserObject     Fn;
	SmileUserObject     Bool;
	SmileUserObject     Char;
	SmileUserObject     Uni;
	SmileUserObject     Symbol;
	SmileUserObject     Exception;
	SmileUserObject     Handle;
};

extern void KnownBases_Preload(struct KnownBasesStruct *knownBases);

extern void KnownBases_Setup(struct KnownBasesStruct *knownBases);

#endif
