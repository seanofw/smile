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
#include <smile/dict/pointerset.h>

TEST_SUITE(PointerSetTests)

//-------------------------------------------------------------------------------------------------
//  Creation Tests.

START_TEST(CanCreatePointerSets)
{
	PointerSet set = PointerSet_Create();
	ASSERT(set != NULL);
	ASSERT(PointerSet_Count(set) == 0);
}
END_TEST

START_TEST(CanAddPointersIntoPointerSetsAndRetrieveThemAgain)
{
	static char *foo = "foo";
	static char *bar = "bar";
	static char *baz = "baz";
	static char *qux = "qux";

	PointerSet set = PointerSet_Create();
	ASSERT(set != NULL);
	ASSERT(PointerSet_Count(set) == 0);

	PointerSet_Add(set, foo);
	PointerSet_Add(set, bar);
	PointerSet_Add(set, baz);
	ASSERT(PointerSet_Count(set) == 3);

	ASSERT(PointerSet_Contains(set, foo));
	ASSERT(PointerSet_Contains(set, bar));
	ASSERT(PointerSet_Contains(set, baz));
	ASSERT(!PointerSet_Contains(set, qux));
}
END_TEST

START_TEST(CanHandleForcedHashCollisionsDuringAddition)
{
	Int32 i;

	PointerSet set = PointerSet_Create();
	ASSERT(set != NULL);
	ASSERT(PointerSet_Count(set) == 0);

	// Insert 1024 values, all into the same bucket.
	for (i = 0; i < 1024; i++) {
		PointerSet_Add(set, (void *)(PtrInt)((i << 16) ^ 0xABAD1DEA));
	}
	ASSERT(PointerSet_Count(set) == 1024);

	// Read out all 1024 values from the same bucket and make sure they contain the expected data.
	for (i = 0; i < 1024; i++) {
		ASSERT(PointerSet_Contains(set, (void *)(PtrInt)((i << 16) ^ 0xABAD1DEA)));
	}
}
END_TEST

START_TEST(CanAddALotOfDataIntoADictionaryReliably)
{
	UInt32 seed, i;

	PointerSet set = PointerSet_Create();
	ASSERT(set != NULL);
	ASSERT(PointerSet_Count(set) == 0);

	// Fill up the set with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 10000; i++) {
		PointerSet_Add(set, (void *)(PtrInt)(seed << 3));
		seed = (seed * 69069) + 127;
	}
	ASSERT(PointerSet_Count(set) == 10000);

	// Verify the set still has all those values.
	seed = 31415;
	for (i = 0; i < 10000; i++) {
		ASSERT(PointerSet_Contains(set, (void *)(PtrInt)(seed << 3)));
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(CanRemoveItems)
{
	UInt32 seed, i;
	Bool removed, exists;

	PointerSet set = PointerSet_Create();
	ASSERT(set != NULL);
	ASSERT(PointerSet_Count(set) == 0);

	// Fill up the set with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		PointerSet_Add(set, (void *)(PtrInt)(seed << 3));
		seed = (seed * 69069) + 127;
	}
	ASSERT(PointerSet_Count(set) == 0x4000);

	// Remove 1/16 of the values, and do it in bunches just to be annoying.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		if ((i & 0xF00) == 0) {
			removed = PointerSet_Remove(set, (void *)(PtrInt)(seed << 3));
			ASSERT(removed);
		}
		seed = (seed * 69069) + 127;
	}
	ASSERT(PointerSet_Count(set) == 0x4000 - 0x400);

	// Now make sure the set contains known data.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		exists = PointerSet_Contains(set, (void *)(PtrInt)(seed << 3));
		if ((i & 0xF00) == 0) {
			ASSERT(!exists);
		}
		else {
			ASSERT(exists);
		}
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(GetKeysReturnsEverything)
{
	void **keys;
	Int32 i;
	Bool keyCheckBits[256];

	PointerSet set = PointerSet_Create();
	ASSERT(set != NULL);
	ASSERT(PointerSet_Count(set) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		PointerSet_Add(set, (void *)(PtrInt)((i * 12) ^ 0xABAD1DEA));
	}
	ASSERT(PointerSet_Count(set) == 256);

	// Pull out all the keys.
	keys = PointerSet_GetAll(set);

	// Now make sure they're all forms of 0xABAD1DEA and divisible by 12 and and that none are missing.
	MemZero(keyCheckBits, sizeof(Bool) * 256);
	for (i = 0; i < 256; i++) {
		ASSERT((((PtrInt)keys[i] ^ 0xABAD1DEA) % 12) == 0);
		keyCheckBits[((PtrInt)keys[i] ^ 0xABAD1DEA) / 12] = True;
	}
	for (i = 0; i < 256; i++) {
		ASSERT(keyCheckBits[i]);
	}
}
END_TEST

#include "pointerset_tests.generated.inc"
