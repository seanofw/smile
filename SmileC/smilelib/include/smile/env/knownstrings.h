
#ifndef __SMILE_ENV_KNOWNSTRINGS_H__
#define __SMILE_ENV_KNOWNSTRINGS_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

struct KnownStringsStruct {
	SmileString Object;
	SmileString true_;
	SmileString false_;
};

#define KNOWN_SMILESTRING(__env__, __name__) \
	((__env__)->knownStrings.__name__)

#define KNOWN_STRING(__env__, __name__) \
	((String)&((__env__)->knownStrings.__name__->string))

void KnownStrings_Preload(SmileEnv env, struct KnownStringsStruct *knownStrings);

#endif
