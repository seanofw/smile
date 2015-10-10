
#ifndef __SMILE_ENV_KNOWNBASES_H__
#define __SMILE_ENV_KNOWNBASES_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

struct KnownBasesStruct {

	SmileUserObject ActorBase;
	SmileUserObject		BoolBase;
	SmileUserObject		StringBase;
	SmileUserObject		CharBase;
	SmileUserObject		UCharBase;
	SmileUserObject		SymbolBase;
	SmileUserObject		NumberBase;
	SmileUserObject			IntegerBase;
	SmileUserObject				ByteBase;
	SmileUserObject				Integer16Base;
	SmileUserObject				Integer32Base;
	SmileUserObject				Integer64Base;
	SmileUserObject				Integer128Base;
	SmileUserObject			FloatBase;
	SmileUserObject				Float16Base;
	SmileUserObject				Float32Base;
	SmileUserObject				Float64Base;
	SmileUserObject				Float128Base;
	SmileUserObject			RealBase;
	SmileUserObject				Real16Base;
	SmileUserObject				Real32Base;
	SmileUserObject				Real64Base;
	SmileUserObject				Real128Base;
	SmileUserObject		PairBase;
	SmileUserObject		EnumerableBase;
	SmileUserObject			ListBase;
	SmileUserObject			ArrayBase;
	SmileUserObject				NumericArrayBase;
	SmileUserObject					IntegerArrayBase;
	SmileUserObject						ByteArrayBase;
	SmileUserObject						Integer16ArrayBase;
	SmileUserObject						Integer32ArrayBase;
	SmileUserObject						Integer64ArrayBase;
	SmileUserObject						Integer128ArrayBase;
	SmileUserObject					FloatArrayBase;
	SmileUserObject						Float16ArrayBase;
	SmileUserObject						Float32ArrayBase;
	SmileUserObject						Float64ArrayBase;
	SmileUserObject						Float128ArrayBase;
	SmileUserObject					RealArrayBase;
	SmileUserObject						Real16ArrayBase;
	SmileUserObject						Real32ArrayBase;
	SmileUserObject						Real64ArrayBase;
	SmileUserObject						Real128ArrayBase;
	SmileUserObject			RangeBase;
	SmileUserObject				NumericRangeBase;
	SmileUserObject					IntegerRangeBase;
	SmileUserObject						ByteRangeBase;
	SmileUserObject						Integer16RangeBase;
	SmileUserObject						Integer32RangeBase;
	SmileUserObject						Integer64RangeBase;
	SmileUserObject						Integer128RangeBase;
	SmileUserObject					FloatRangeBase;
	SmileUserObject						Float16RangeBase;
	SmileUserObject						Float32RangeBase;
	SmileUserObject						Float64RangeBase;
	SmileUserObject						Float128RangeBase;
	SmileUserObject					RealRangeBase;
	SmileUserObject						Real16RangeBase;
	SmileUserObject						Real32RangeBase;
	SmileUserObject						Real64RangeBase;
	SmileUserObject						Real128RangeBase;
};

extern void KnownBases_Preload(struct KnownBasesStruct *knownBases, SmileObject baseObject);

#endif
