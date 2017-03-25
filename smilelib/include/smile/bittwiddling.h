
#ifndef __SMILE_BITTWIDDLING_H__
#define __SMILE_BITTWIDDLING_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

Inline UInt32 NextPowerOfTwo32(UInt32 v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

Inline UInt64 NextPowerOfTwo64(UInt64 v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	v++;
	return v;
}

Inline Int NextPowerOfTwo(Int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;

	#if SizeofInt == 8
		v |= v >> 32;
	#endif

	v++;
	return v;
}

#endif
