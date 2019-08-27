#ifndef __SMILE_CRYPTO_BASE64_H__
#define __SMILE_CRYPTO_BASE64_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

// Okay, yeah, so base-64 encoding isn't *really* a strong cryptographic operation,
// but from the perspective of the Roman Empire, even rot-13 was valid cryptography
// (there's a reason they call it a Caesar cypher!), so base-64 encoding can
// still fit at least nominally within the crypto library, since there's not really
// a "data encoding" namespace elsewhere.

SMILE_API_FUNC String Base64Encode(const Byte *buffer, Int length, Bool wrapLines);
SMILE_API_FUNC Byte *Base64Decode(String text, Int *length);

#endif