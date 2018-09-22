#ifndef __BCP_LOAD_AND_PARSE_H__
#define __BCP_LOAD_AND_PARSE_H__

#ifndef __STDAFX_H__
#include "stdafx.h"
#endif

String LoadFile(String filename);
SmileObject Parse(String text, String filename, ClosureInfo closureInfo);

#endif