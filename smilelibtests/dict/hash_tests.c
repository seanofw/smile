//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2017 Sean Werkema
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

#include "../stdafx.h"
#include <smile/crypto/sha2.h>

TEST_SUITE(HashTests)

//-------------------------------------------------------------------------------------------------
//  Creation Tests.

START_TEST(HashOracleShouldBeAtLeastSomewhatUnpredictable)
{
#	define NUM_TRIES (32)

	UInt32 hashOracles[NUM_TRIES];
	Int i;
	Int duplicates;

	for (i = 0; i < NUM_TRIES; i++) {
		Smile_Init();
		hashOracles[i] = Smile_HashOracle;
	}

	duplicates = 0;
	for (i = 0; i < NUM_TRIES - 1; i++) {
		if (hashOracles[i] == hashOracles[i + 1])
			duplicates++;
	}

	ASSERT(duplicates < NUM_TRIES / 2);

#	undef NUM_TRIES
}
END_TEST

START_TEST(InitHashTableShouldDistributeRandomValuesEvenly)
{
#	define NUM_TRIES (4096)

	Byte hashes[NUM_TRIES];
	Int i;
	Int duplicates;
	UInt64 hash;

	Smile_Init();

	Smile_HashOracle = 1;
	Smile_InitHashTable(Smile_HashOracle);

	for (i = 0; i < NUM_TRIES; i++) {
		hash = ((UInt64)i);
		hash += (hash >> 28);		// Fold high 28 bits of 56-bit value down to low 28 bits
		hash += (hash >> 14);		// Fold high 14 bits of 28-bit value down to low 14 bits
		hash = Smile_HashTable[hash & 0x3FFF];

		hashes[i] = hash & 0xFF;	// Only look at the lowest bits
	}

	duplicates = 0;
	for (i = 0; i < NUM_TRIES - 1; i++) {
		if (hashes[i] == hashes[i + 1])
			duplicates++;
	}

	ASSERT(duplicates < (NUM_TRIES * (1.0/256) * 2));	// We expect ~(1/256)*4096 = ~16 to be successive duplicates.
}
END_TEST

#include "hash_tests.generated.inc"
