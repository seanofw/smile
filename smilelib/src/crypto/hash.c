//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <smile/crypto/dicthash.h>
#include <smile/crypto/sha2.h>

/// <summary>
/// This table holds 16384 random 32-bit values, and is used for efficiently hashing
/// single-value objects like pointers and integers.
/// </summary>
UInt32 Smile_HashTable[SMILE_HASHTABLE_SIZE];

/// <summary>
/// Reset the global hash table to new values based on the given basis.
/// </summary>
/// <param name="hashBasis">The basis value to use when constructing the new global
/// hash table.</param>
void Smile_InitHashTable(UInt32 hashBasis)
{
	UInt32 i;
	Byte bytes[16];

	bytes[0] = 0;
	bytes[1] = 0;
	bytes[2] = (Byte)(hashBasis & 0xFF);
	bytes[3] = (Byte)((hashBasis >> 8) & 0xFF);
	bytes[4] = (Byte)((hashBasis >> 16) & 0xFF);
	bytes[5] = (Byte)((hashBasis >> 24) & 0xFF);
	bytes[6] = 0;
	bytes[7] = 0;
	bytes[8] = 0;
	bytes[9] = 0;
	bytes[10] = 0;
	bytes[11] = 0;
	bytes[12] = 0;
	bytes[13] = 0;
	bytes[14] = 0;
	bytes[15] = 0;

	// For the standard hash table of size 14 bits (16384 entries), we generate 1024 SHA-512 hashes.
	for (i = 0; i < SMILE_HASHTABLE_SIZE / 16; i++) {
		bytes[0] ^= (Byte)( i        & 0xFF);
		bytes[1] ^= (Byte)((i >>  8) & 0xFF);

		// From the combined hash basis and offset i, generate sixteen more provably-random
		// 32-bit values in the hash table, using the SHA-512 hash algorithm as our
		// random-value generator.
		Sha512((Byte *)Smile_HashTable + i * 64, bytes, 16);

		// Include 128 bits from the last round in the next round, so that we get the
		// effect of using the hash in CBC mode instead of in ECB mode.  This ensures that
		// each subsequent value in the table is about as unique and random as possible,
		// and that the hash table's values are reasonably unguessable.  This isn't *perfectly*
		// crypto-secure CBC mode, but we don't need that, since we only end up with 16K
		// worth of hash codes anyway.
		bytes[0] ^= ((Byte *)Smile_HashTable + i * 64)[0];
		bytes[1] ^= ((Byte *)Smile_HashTable + i * 64)[1];
		bytes[2] ^= ((Byte *)Smile_HashTable + i * 64)[2];
		bytes[3] ^= ((Byte *)Smile_HashTable + i * 64)[3];
		bytes[4] ^= ((Byte *)Smile_HashTable + i * 64)[4];
		bytes[5] ^= ((Byte *)Smile_HashTable + i * 64)[5];
		bytes[6] ^= ((Byte *)Smile_HashTable + i * 64)[6];
		bytes[7] ^= ((Byte *)Smile_HashTable + i * 64)[7];
		bytes[8] ^= ((Byte *)Smile_HashTable + i * 64)[8];
		bytes[9] ^= ((Byte *)Smile_HashTable + i * 64)[9];
		bytes[10] ^= ((Byte *)Smile_HashTable + i * 64)[10];
		bytes[11] ^= ((Byte *)Smile_HashTable + i * 64)[11];
		bytes[12] ^= ((Byte *)Smile_HashTable + i * 64)[12];
		bytes[13] ^= ((Byte *)Smile_HashTable + i * 64)[13];
		bytes[14] ^= ((Byte *)Smile_HashTable + i * 64)[14];
		bytes[15] ^= ((Byte *)Smile_HashTable + i * 64)[15];
	}
}
