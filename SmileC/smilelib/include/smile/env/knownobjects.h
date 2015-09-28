
#ifndef __SMILE_ENV_KNOWNOBJECTS_H__
#define __SMILE_ENV_KNOWNOBJECTS_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

struct KnownObjectsStruct {

	SmileObject Object;
	SmileNull Null;

	SmileBool TrueObj;
	SmileBool FalseObj;

	SmileInteger32 SmallInt32s[200];
	SmileInteger32 ZeroInt32;
	SmileInteger32 OneInt32;

	SmileInteger64 SmallInt64s[200];
	SmileInteger64 ZeroInt64;
	SmileInteger64 OneInt64;
};

extern void KnownObjects_Preload(struct KnownObjectsStruct *knownObjects);

#endif
