#ifndef __BPC_EVAL_H__
#define __BPC_EVAL_H__

#ifndef __STDAFX_H__
#include "stdafx.h"
#endif

SmileObject EvalExpr(Compiler compiler, SmileObject expr, ClosureInfo closureInfo, String filename);

#endif