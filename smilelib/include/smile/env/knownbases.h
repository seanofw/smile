
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
	SmileUserObject         StringArray;
	SmileUserObject         SymbolArray;
	SmileUserObject         CharArray;
	SmileUserObject         UniArray;
	SmileUserObject         NumericArray;
	SmileUserObject           IntegerArrayBase;
	SmileUserObject             ByteArray, Integer16Array, Integer32Array, Integer64Array, Integer128Array;
	SmileUserObject           RealArrayBase;
	SmileUserObject             Real32Array, Real64Array, Real128Array;
	SmileUserObject           FloatArrayBase;
	SmileUserObject             Float32Array, Float64Array, Float128Array;
	SmileUserObject       MapBase;
	SmileUserObject         Map;
	SmileUserObject         SymbolMap;
	SmileUserObject         StringMap;
	SmileUserObject         CharMap;
	SmileUserObject         UniMap;
	SmileUserObject         NumericMap;
	SmileUserObject           IntegerMapBase;
	SmileUserObject             ByteMap, Integer16Map, Integer32Map, Integer64Map, Integer128Map;
	SmileUserObject           RealMapBase;
	SmileUserObject             Real32Map, Real64Map, Real128Map;
	SmileUserObject           FloatMapBase;
	SmileUserObject             Float32Map, Float64Map, Float128Map;
	SmileUserObject       SetBase;
	SmileUserObject         Set;
	SmileUserObject         SymbolSet;
	SmileUserObject         StringSet;
	SmileUserObject         CharSet;
	SmileUserObject         UniSet;
	SmileUserObject         NumericSet;
	SmileUserObject           IntegerSetBase;
	SmileUserObject             ByteSet, Integer16Set, Integer32Set, Integer64Set, Integer128Set;
	SmileUserObject           RealSetBase;
	SmileUserObject             Real32Set, Real64Set, Real128Set;
	SmileUserObject           FloatSetBase;
	SmileUserObject             Float32Set, Float64Set, Float128Set;
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
