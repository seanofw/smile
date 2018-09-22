#ifndef __BCP_ERROR_H__
#define __BCP_ERROR_H__

#ifndef __STDAFX_H__
#include "stdafx.h"
#endif

void Error(const char *filename, Int line, const char *format, ...);
Bool PrintParseMessages(Parser parser);

#endif