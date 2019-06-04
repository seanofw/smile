//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

#include "../stdafx.h"

TEST_SUITE(Int32DictTests)

//-------------------------------------------------------------------------------------------------
//  Creation Tests.

START_TEST(CanCreateInt32Dicts)
{
	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);
}
END_TEST

START_TEST(CanAddValuesIntoInt32DictsAndRetrieveThemAgain)
{
	const char *text;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	Int32Dict_Add(dict, 12345, "foo");
	Int32Dict_Add(dict, 54321, "bar");
	Int32Dict_Add(dict, 31415, "baz");
	ASSERT(Int32Dict_Count(dict) == 3);

	text = (const char *)Int32Dict_GetValue(dict, 12345);
	ASSERT(!strcmp(text, "foo"));

	text = (const char *)Int32Dict_GetValue(dict, 54321);
	ASSERT(!strcmp(text, "bar"));

	text = (const char *)Int32Dict_GetValue(dict, 31415);
	ASSERT(!strcmp(text, "baz"));
}
END_TEST

START_TEST(CanHandleForcedHashCollisionsDuringAddition)
{
	Int32 i;
	PtrInt value;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Insert 1024 values, all into the same bucket.
	for (i = 0; i < 1024; i++) {
		Int32Dict_Add(dict, i << 16, (void *)(PtrInt)(i ^ 0xABAD1DEA));
	}
	ASSERT(Int32Dict_Count(dict) == 1024);

	// Read out all 1024 values from the same bucket and make sure they contain the expected data.
	for (i = 0; i < 1024; i++) {
		value = (PtrInt)Int32Dict_GetValue(dict, i << 16);
		ASSERT(value == (i ^ 0xABAD1DEA));
	}
}
END_TEST

START_TEST(CanAddALotOfDataIntoADictionaryReliably)
{
	UInt32 seed, i;
	void *ptr;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 10000; i++) {
		Int32Dict_Add(dict, seed, (void *)(PtrInt)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(Int32Dict_Count(dict) == 10000);

	// Verify the dictionary still has all those values.
	seed = 31415;
	for (i = 0; i < 10000; i++) {
		ptr = Int32Dict_GetValue(dict, seed);
		ASSERT(ptr == (void *)(PtrInt)i);
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(SetValueCanBothAddAndUpdate)
{
	UInt32 seed, i;
	void *ptr;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		Int32Dict_SetValue(dict, seed, (void *)(PtrInt)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(Int32Dict_Count(dict) == 1000);

	// Replace 500 of those values with new values.
	seed = 31415;
	for (i = 0; i < 500; i++) {
		Int32Dict_SetValue(dict, seed, (void *)(PtrInt)(i ^ 0xFFFF));
		seed = (seed * 69069) + 127;
	}
	ASSERT(Int32Dict_Count(dict) == 1000);

	// Verify the dictionary still has both the modified and original values.
	seed = 31415;
	for (i = 0; i < 500; i++) {
		ptr = Int32Dict_GetValue(dict, seed);
		ASSERT(ptr == (void *)(PtrInt)(i ^ 0xFFFF));
		seed = (seed * 69069) + 127;
	}
	for (; i < 1000; i++) {
		ptr = Int32Dict_GetValue(dict, seed);
		ASSERT(ptr == (void *)(PtrInt)i);
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(ContainsKeyCanAnswerWhenThingsExist)
{
	UInt32 seed, i;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		Int32Dict_SetValue(dict, seed, (void *)(PtrInt)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(Int32Dict_Count(dict) == 1000);

	// Verify the dictionary still the original values and nothing unexpected.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		if (!Int32Dict_ContainsKey(dict, seed)) {
			ASSERT(False);
		}
		seed = (seed * 69069) + 127;
	}
	for (; i < 2000; i++) {
		if (Int32Dict_ContainsKey(dict, seed)) {
			ASSERT(False);
		}
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(TryGetValueCanAnswerWhenThingsExist)
{
	UInt32 seed, i;
	void *ptr;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		Int32Dict_SetValue(dict, seed, (void *)(PtrInt)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(Int32Dict_Count(dict) == 1000);

	// Verify the dictionary still the original values and nothing unexpected.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		if (!Int32Dict_TryGetValue(dict, seed, &ptr)) {
			ASSERT(False);
		}
		ASSERT(ptr == (void *)(PtrInt)i);
		seed = (seed * 69069) + 127;
	}
	for (; i < 2000; i++) {
		if (Int32Dict_TryGetValue(dict, seed, &ptr)) {
			ASSERT(False);
		}
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(CanRemoveItemsByKey)
{
	UInt32 seed, i;
	void *ptr;
	Bool removed, exists;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		Int32Dict_Add(dict, seed, (void *)(PtrInt)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(Int32Dict_Count(dict) == 0x4000);

	// Remove 1/16 of the values, and do it in bunches just to be annoying.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		if ((i & 0xF00) == 0) {
			removed = Int32Dict_Remove(dict, seed);
			ASSERT(removed);
		}
		seed = (seed * 69069) + 127;
	}
	ASSERT(Int32Dict_Count(dict) == 0x4000 - 0x400);

	// Now make sure the dictionary contains known data.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		exists = Int32Dict_TryGetValue(dict, seed, &ptr);
		if ((i & 0xF00) == 0) {
			ASSERT(!exists);
		}
		else {
			ASSERT(exists && (void *)(PtrInt)i == ptr);
		}
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(CanHandleForcedHashCollisionsDuringRemoval)
{
	Int32 i;
	PtrInt value;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Insert 1024 values, all into the same bucket.
	for (i = 0; i < 1024; i++) {
		Int32Dict_Add(dict, i << 16, (void *)(PtrInt)(i ^ 0xABAD1DEA));
	}
	ASSERT(Int32Dict_Count(dict) == 1024);

	// Remove 512 of those values from the middle of the bucket.
	for (i = 256; i < 768; i++) {
		Int32Dict_Remove(dict, i << 16);
	}
	// And also the very first and very last ones.
	Int32Dict_Remove(dict, 0 << 16);
	Int32Dict_Remove(dict, 1023 << 16);
	ASSERT(Int32Dict_Count(dict) == 510);

	// Read out all 510 values from the same bucket and make sure they contain the expected data.
	for (i = 1; i < 256; i++) {
		value = (PtrInt)Int32Dict_GetValue(dict, i << 16);
		ASSERT(value == (i ^ 0xABAD1DEA));
	}
	for (i = 768; i < 1023; i++) {
		value = (PtrInt)Int32Dict_GetValue(dict, i << 16);
		ASSERT(value == (i ^ 0xABAD1DEA));
	}
}
END_TEST

START_TEST(CanHandleRepeatedForcedHashCollisions)
{
	Int32 i, retry;
	void *ptr;
	Bool found;

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	for (retry = 0; retry < 3; retry++) {
		// Insert 1024 values, all into the same bucket.
		for (i = 0; i < 1024; i++) {
			Int32Dict_Add(dict, i << 16, (void *)(PtrInt)(i ^ 0xABAD1DEA));
		}
		ASSERT(Int32Dict_Count(dict) == 1024);

		// Remove 512 of those values from the middle of the bucket.
		for (i = 256; i < 768; i++) {
			ASSERT(Int32Dict_Remove(dict, i << 16));
		}
		// And also the very first and very last ones.
		ASSERT(Int32Dict_Remove(dict, 0 << 16));
		ASSERT(Int32Dict_Remove(dict, 1023 << 16));
		ASSERT(Int32Dict_Count(dict) == 510);

		// Read out all 510 values from the same bucket and make sure they contain the expected data.
		for (i = -1; i <= 1024; i++) {
			found = Int32Dict_TryGetValue(dict, i << 16, &ptr);
			if ((i >= 1 && i < 256) || (i >= 768 && i < 1023)) {
				ASSERT(found);
				ASSERT((PtrInt)ptr == (i ^ 0xABAD1DEA));
			}
			else {
				ASSERT(!found);
			}
		}

		// Reinsert the first and last entries.
		Int32Dict_Add(dict, 0 << 16, (void *)(PtrInt)(0 ^ 0xABAD1DEA));
		Int32Dict_Add(dict, 1023 << 16, (void *)(PtrInt)(1023 ^ 0xABAD1DEA));
		ASSERT(Int32Dict_Count(dict) == 512);

		// Verify that we have 512 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = Int32Dict_TryGetValue(dict, i << 16, &ptr);
			if ((i >= 0 && i < 256) || (i >= 768 && i < 1024)) {
				ASSERT(found);
				ASSERT((PtrInt)ptr == (i ^ 0xABAD1DEA));
			}
			else {
				ASSERT(!found);
			}
		}

		// Reinsert the middle entries.
		for (i = 256; i < 768; i++) {
			Int32Dict_Add(dict, i << 16, (void *)(PtrInt)(i ^ 0xABAD1DEA));
		}
		ASSERT(Int32Dict_Count(dict) == 1024);

		// Verify that we have 1024 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = Int32Dict_TryGetValue(dict, i << 16, &ptr);
			if (i >= 0 && i < 1024) {
				ASSERT(found);
				ASSERT((PtrInt)ptr == (i ^ 0xABAD1DEA));
			}
			else {
				ASSERT(!found);
			}
		}

		// Remove *all* of them.
		for (i = 0; i < 1024; i++) {
			ASSERT(Int32Dict_Remove(dict, i << 16));
		}
		ASSERT(Int32Dict_Count(dict) == 0);

		// Verify that we have 0 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = Int32Dict_TryGetValue(dict, i << 16, &ptr);
			ASSERT(!found);
		}
	}
}
END_TEST

START_TEST(GetKeysReturnsAllTheKeys)
{
	Int32 *keys;
	Int32 i, key;
	Bool checkBits[256];

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		Int32Dict_Add(dict, i * 12, (void *)(PtrInt)(i ^ 0xABAD1DEA));
	}
	ASSERT(Int32Dict_Count(dict) == 256);

	// Pull out all the keys.
	keys = Int32Dict_GetKeys(dict);

	// Now make sure they're all divisible by 12 and that none are missing.
	MemZero(checkBits, sizeof(Bool) * 256);
	for (i = 0; i < 256; i++) {
		key = keys[i];
		ASSERT(key % 12 == 0);
		checkBits[key / 12] = True;
	}
	for (i = 0; i < 256; i++) {
		ASSERT(checkBits[i]);
	}
}
END_TEST

START_TEST(GetValuesReturnsAllTheValues)
{
	void **values;
	void *value;
	Int32 i;
	Bool checkBits[256];

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		Int32Dict_Add(dict, i * 12, (void *)(PtrInt)(i ^ 0xABAD1DEA));
	}
	ASSERT(Int32Dict_Count(dict) == 256);

	// Pull out all the values.
	values = Int32Dict_GetValues(dict);

	// Now make sure they're all forms of 0xABAD1DEA and that none are missing.
	MemZero(checkBits, sizeof(Bool) * 256);
	for (i = 0; i < 256; i++) {
		value = values[i];
		ASSERT(((PtrInt)value ^ 0xABAD1DEA) < 256);
		checkBits[(PtrInt)value ^ 0xABAD1DEA] = True;
	}
	for (i = 0; i < 256; i++) {
		ASSERT(checkBits[i]);
	}
}
END_TEST

START_TEST(GetAllReturnsEverything)
{
	Int32DictKeyValuePair *pairs;
	Int32 i;
	Bool keyCheckBits[256];
	Bool valueCheckBits[256];

	Int32Dict dict = Int32Dict_Create();
	ASSERT(dict != NULL);
	ASSERT(Int32Dict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		Int32Dict_Add(dict, i * 12, (void *)(PtrInt)(i ^ 0xABAD1DEA));
	}
	ASSERT(Int32Dict_Count(dict) == 256);

	// Pull out all the values.
	pairs = Int32Dict_GetAll(dict);

	// Now make sure they're all divisible by 12 and forms of 0xABAD1DEA and that none are missing.
	MemZero(keyCheckBits, sizeof(Bool) * 256);
	MemZero(valueCheckBits, sizeof(Bool) * 256);
	for (i = 0; i < 256; i++) {
		ASSERT(pairs[i].key % 12 == 0);
		keyCheckBits[pairs[i].key / 12] = True;
		ASSERT(((PtrInt)(pairs[i].value) ^ 0xABAD1DEA) < 256);
		valueCheckBits[(PtrInt)(pairs[i].value) ^ 0xABAD1DEA] = True;
	}
	for (i = 0; i < 256; i++) {
		ASSERT(keyCheckBits[i]);
		ASSERT(valueCheckBits[i]);
	}
}
END_TEST

#include "int32dict_tests.generated.inc"
