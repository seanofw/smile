#ifndef __SMILE_NUMERIC_REALSHARED_H__
#define __SMILE_NUMERIC_REALSHARED_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#define REAL_ROUND_TO_NEAREST 0		// The default.
#define REAL_ROUND_DOWN       1
#define REAL_ROUND_UP         2
#define REAL_ROUND_TO_ZERO    3
#define REAL_ROUND_TIES_AWAY  4

#define REAL_FLAG_INVALID     0x01
#define REAL_FLAG_UNNORMAL    0x02
#define REAL_FLAG_DIVBYZERO   0x04
#define REAL_FLAG_OVERFLOW    0x08
#define REAL_FLAG_UNDERFLOW   0x10
#define REAL_FLAG_INEXACT     0x20

#define REAL_KIND_POS_ZERO		0
#define REAL_KIND_NEG_ZERO		(0x80 | REAL_KIND_POS_ZERO)
#define REAL_KIND_POS_NUM		1
#define REAL_KIND_NEG_NUM		(0x80 | REAL_KIND_POS_NUM)
#define REAL_KIND_POS_INF		2
#define REAL_KIND_NEG_INF		(0x80 | REAL_KIND_POS_INF)
#define REAL_KIND_POS_QNAN		3
#define REAL_KIND_NEG_QNAN		(0x80 | REAL_KIND_POS_QNAN)
#define REAL_KIND_POS_SNAN		4
#define REAL_KIND_NEG_SNAN		(0x80 | REAL_KIND_POS_SNAN)

SMILE_API_FUNC UInt32 Real_GetRoundingMode(void);
SMILE_API_FUNC void Real_SetRoundingMode(UInt32 mode);
SMILE_API_FUNC UInt32 Real_GetFlags(void);
SMILE_API_FUNC void Real_SetFlags(UInt32 flags);

#endif