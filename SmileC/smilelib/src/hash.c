/*
* hash_32 - 32 bit Fowler/Noll/Vo FNV-1a hash code
*
* @(#) $Revision: 5.1 $
* @(#) $Id: hash_32a.c,v 5.1 2009/06/30 09:13:32 chongo Exp $
* @(#) $Source: /usr/local/src/cmd/fnv/RCS/hash_32a.c,v $
*
***
*
* Fowler/Noll/Vo hash
*
* The basis of this hash algorithm was taken from an idea sent
* as reviewer comments to the IEEE POSIX P1003.2 committee by:
*
*      Phong Vo (http://www.research.att.com/info/kpv/)
*      Glenn Fowler (http://www.research.att.com/~gsf/)
*
* In a subsequent ballot round:
*
*      Landon Curt Noll (http://www.isthe.com/chongo/)
*
* improved on their algorithm.  Some people tried this hash
* and found that it worked rather well.  In an EMail message
* to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
*
* FNV hashes are designed to be fast while maintaining a low
* collision rate. The FNV speed allows one to quickly hash lots
* of data while maintaining a reasonable collision rate.  See:
*
*      http://www.isthe.com/chongo/tech/comp/fnv/index.html
*
* for more details as well as other forms of the FNV hash.
***
*
* To use the recommended 32 bit FNV-1a hash, pass FNV1_32A_INIT as the
* Fnv32_t hashval argument to fnv_32a_buf() or fnv_32a_str().
*
***
*
* Please do not copyright this code.  This code is in the public domain.
*
* LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
* INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
* EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
* CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
* USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
* OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*
* By:
*	chongo <Landon Curt Noll> /\oo/\
*      http://www.isthe.com/chongo/
*
* Share and Enjoy!	:-)
*
***
*
* Tweaked a bit for usage in Smile in 2015 by Sean Werkema.
* And still in the public domain, yo!
*/

#include <smile/types.h>
#include <smile/internal/types.h>

/// <summary>
/// RawHash:  Perform a 32 bit hash on a buffer.
/// The hash is guaranteed to always be the same value for the same sequence of bytes.
/// </summary>
/// <remarks>
/// This is currently implemented as a Fowler/Noll/Vo FNV-1a hash.
/// </remarks>
/// <param name="buffer">Start of buffer to hash.</param>
/// <param name="length">Length of buffer in bytes.</param>
/// <returns>32 bit hash of the buffer.</returns>
UInt32 Smile_RawHash(const void *buffer, Int length)
{
	Byte *bp = (Byte *)buffer;	// Start of buffer
	Byte *be = bp + length;		// Beyond end of buffer
	UInt32 hash = 0x811C9DC5UL;

	// FNV-1a hash each octet in the buffer
	while (bp < be) {

		// Xor the bottom with the current octet.
		hash ^= (UInt32)*bp++;

		// Multiply by the 32 bit FNV magic prime mod 2^32.
#if defined(NO_FNV_GCC_OPTIMIZATION)
		hash *= 0x01000193UL;
#else
		hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);
#endif
	}

	// Return our new hash value.
	return hash;
}
