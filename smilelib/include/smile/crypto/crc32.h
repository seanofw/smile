
#ifndef __SMILE_CRYPTO_CRC32_H__
#define __SMILE_CRYPTO_CRC32_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

//-----------------------------------------------------------------------------
// CRC-32 Implementation (NOT SECURE).

SMILE_API_FUNC UInt32 Crc32_Update(UInt32 crc, const Byte *buffer, Int length);

Inline UInt32 Crc32_Init(void)
{
	return 0;
}

Inline UInt32 Crc32_Finish(UInt32 crc)
{
	return crc;
}

Inline UInt32 Crc32(const Byte *buffer, Int length)
{
	return Crc32_Update(0, buffer, length);
}

#endif
