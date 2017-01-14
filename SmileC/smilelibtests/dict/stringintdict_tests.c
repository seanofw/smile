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
#include <stdlib.h>

TEST_SUITE(StringIntDictTests)

//-------------------------------------------------------------------------------------------------
//  Creation Tests.

START_TEST(CanCreateStringIntDicts)
{
	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);
}
END_TEST

START_TEST(CanAddValuesIntoStringIntDictsAndRetrieveThemAgain)
{
	Int value;

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	StringIntDict_AddC(dict, "soup", 314159);
	StringIntDict_AddC(dict, "nuts", 42);
	StringIntDict_AddC(dict, "fruit", 1010101);
	ASSERT(StringIntDict_Count(dict) == 3);

	value = StringIntDict_GetValueC(dict, "soup");
	ASSERT(value == 314159);

	value = StringIntDict_GetValueC(dict, "nuts");
	ASSERT(value == 42);

	value = StringIntDict_GetValueC(dict, "fruit");
	ASSERT(value == 1010101);
}
END_TEST

START_TEST(CanHandleSimpleAdditionAndLookup)
{
	Int i;
	Int value;

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Insert 1024 values into whatever bucket the hashes suggest.
	for (i = 0; i < 1024; i++) {
		StringIntDict_Add(dict, String_Format("%u", i << 16), i ^ 0xABAD1DEA);
	}
	ASSERT(StringIntDict_Count(dict) == 1024);

	// Read out all 1024 values from the same bucket and make sure they contain the expected data.
	for (i = 0; i < 1024; i++) {
		value = StringIntDict_GetValue(dict, String_Format("%u", i << 16));
		ASSERT(value == (i ^ 0xABAD1DEA));
	}
}
END_TEST

START_TEST(CanAddALotOfDataIntoADictionaryReliably)
{
	UInt32 seed, i;
	Int value;

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 100000; i++) {
		StringIntDict_Add(dict, String_Format("%u", seed), i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringIntDict_Count(dict) == 100000);

	// Verify the dictionary still has all those values.
	seed = 31415;
	for (i = 0; i < 100000; i++) {
		value = StringIntDict_GetValue(dict, String_Format("%u", seed));
		ASSERT(value == i);
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(SetValueCanBothAddAndUpdate)
{
	UInt32 seed, i;
	Int value;

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		StringIntDict_SetValue(dict, String_Format("%u", seed), i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringIntDict_Count(dict) == 1000);

	// Replace 500 of those values with new values.
	seed = 31415;
	for (i = 0; i < 500; i++) {
		StringIntDict_SetValue(dict, String_Format("%u", seed), i ^ 0xFFFF);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringIntDict_Count(dict) == 1000);

	// Verify the dictionary still has both the modified and original values.
	seed = 31415;
	for (i = 0; i < 500; i++) {
		value = StringIntDict_GetValue(dict, String_Format("%u", seed));
		ASSERT(value == (i ^ 0xFFFF));
		seed = (seed * 69069) + 127;
	}
	for (; i < 1000; i++) {
		value = StringIntDict_GetValue(dict, String_Format("%u", seed));
		ASSERT(value == i);
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(ContainsKeyCanAnswerWhenThingsExist)
{
	UInt32 seed, i;

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		StringIntDict_SetValue(dict, String_Format("%u", seed), i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringIntDict_Count(dict) == 1000);

	// Verify the dictionary still the original values and nothing unexpected.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		if (!StringIntDict_ContainsKey(dict, String_Format("%u", seed))) {
			ASSERT(False);
		}
		seed = (seed * 69069) + 127;
	}
	for (; i < 2000; i++) {
		if (StringIntDict_ContainsKey(dict, String_Format("%u", seed))) {
			ASSERT(False);
		}
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(TryGetValueCanAnswerWhenThingsExist)
{
	UInt32 seed, i;
	Int value;

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		StringIntDict_SetValue(dict, String_Format("%u", seed), i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringIntDict_Count(dict) == 1000);

	// Verify the dictionary still the original values and nothing unexpected.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		if (!StringIntDict_TryGetValue(dict, String_Format("%u", seed), &value)) {
			ASSERT(False);
		}
		ASSERT(value == i);
		seed = (seed * 69069) + 127;
	}
	for (; i < 2000; i++) {
		if (StringIntDict_TryGetValue(dict, String_Format("%u", seed), &value)) {
			ASSERT(False);
		}
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(CanRemoveItemsByKey)
{
	UInt32 seed, i;
	Int value;
	Bool removed, exists;

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		StringIntDict_Add(dict, String_Format("%u", seed), i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringIntDict_Count(dict) == 0x4000);

	// Remove 1/16 of the values, and do it in bunches just to be annoying.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		if ((i & 0xF00) == 0) {
			removed = StringIntDict_Remove(dict, String_Format("%u", seed));
			ASSERT(removed);
		}
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringIntDict_Count(dict) == 0x4000 - 0x400);

	// Now make sure the dictionary contains known data.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		exists = StringIntDict_TryGetValue(dict, String_Format("%u", seed), &value);
		if ((i & 0xF00) == 0) {
			ASSERT(!exists);
		}
		else {
			ASSERT(exists && i == value);
		}
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(CanHandleRepeatedMutation)
{
	Int i, retry;
	Int value;
	Bool found;

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	for (retry = 0; retry < 3; retry++) {
		// Insert 1024 values, all into the same bucket.
		for (i = 0; i < 1024; i++) {
			StringIntDict_Add(dict, String_Format("%d", i << 16), (i ^ 0xABAD1DEA));
		}
		ASSERT(StringIntDict_Count(dict) == 1024);

		// Remove 512 of those values from the middle of the bucket.
		for (i = 256; i < 768; i++) {
			ASSERT(StringIntDict_Remove(dict, String_Format("%d", i << 16)));
		}
		// And also the very first and very last ones.
		ASSERT(StringIntDict_Remove(dict, String_Format("%d", 0 << 16)));
		ASSERT(StringIntDict_Remove(dict, String_Format("%d", 1023 << 16)));
		ASSERT(StringIntDict_Count(dict) == 510);

		// Read out all 510 values from the same bucket and make sure they contain the expected data.
		for (i = -1; i <= 1024; i++) {
			found = StringIntDict_TryGetValue(dict, String_Format("%d", i << 16), &value);
			if ((i >= 1 && i < 256) || (i >= 768 && i < 1023)) {
				ASSERT(found);
				ASSERT(value == (i ^ 0xABAD1DEA));
			}
			else {
				ASSERT(!found);
			}
		}

		// Reinsert the first and last entries.
		StringIntDict_Add(dict, String_Format("%d", 0 << 16), (0 ^ 0xABAD1DEA));
		StringIntDict_Add(dict, String_Format("%d", 1023 << 16), (1023 ^ 0xABAD1DEA));
		ASSERT(StringIntDict_Count(dict) == 512);

		// Verify that we have 512 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = StringIntDict_TryGetValue(dict, String_Format("%d", i << 16), &value);
			if ((i >= 0 && i < 256) || (i >= 768 && i < 1024)) {
				ASSERT(found);
				ASSERT(value == (i ^ 0xABAD1DEA));
			}
			else {
				ASSERT(!found);
			}
		}

		// Reinsert the middle entries.
		for (i = 256; i < 768; i++) {
			StringIntDict_Add(dict, String_Format("%d", i << 16), (i ^ 0xABAD1DEA));
		}
		ASSERT(StringIntDict_Count(dict) == 1024);

		// Verify that we have 1024 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = StringIntDict_TryGetValue(dict, String_Format("%d", i << 16), &value);
			if (i >= 0 && i < 1024) {
				ASSERT(found);
				ASSERT(value == (i ^ 0xABAD1DEA));
			}
			else {
				ASSERT(!found);
			}
		}

		// Remove *all* of them.
		for (i = 0; i < 1024; i++) {
			ASSERT(StringIntDict_Remove(dict, String_Format("%d", i << 16)));
		}
		ASSERT(StringIntDict_Count(dict) == 0);

		// Verify that we have 0 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = StringIntDict_TryGetValue(dict, String_Format("%d", i << 16), &value);
			ASSERT(!found);
		}
	}
}
END_TEST

START_TEST(GetKeysReturnsAllTheKeys)
{
	String *keys;
	String key;
	Int32 i, keyNumber;
	Bool checkBits[256];

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		StringIntDict_Add(dict, String_Format("%u", i * 12), (i ^ 0xABAD1DEA));
	}
	ASSERT(StringIntDict_Count(dict) == 256);

	// Pull out all the keys.
	keys = StringIntDict_GetKeys(dict);

	// Now make sure they're all divisible by 12 and that none are missing.
	MemZero(checkBits, sizeof(Bool) * 256);
	for (i = 0; i < 256; i++) {
		key = keys[i];
		keyNumber = atoi(String_ToC(key));
		ASSERT(keyNumber % 12 == 0);
		checkBits[keyNumber / 12] = True;
	}
	for (i = 0; i < 256; i++) {
		ASSERT(checkBits[i]);
	}
}
END_TEST

START_TEST(GetValuesReturnsAllTheValues)
{
	Int *values;
	Int value;
	Int32 i;
	Bool checkBits[256];

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		StringIntDict_Add(dict, String_Format("%u", i * 12), (i ^ 0xABAD1DEA));
	}
	ASSERT(StringIntDict_Count(dict) == 256);

	// Pull out all the values.
	values = StringIntDict_GetValues(dict);

	// Now make sure they're all forms of 0xABAD1DEA and that none are missing.
	MemZero(checkBits, sizeof(Bool) * 256);
	for (i = 0; i < 256; i++) {
		value = values[i];
		ASSERT((value ^ 0xABAD1DEA) < 256);
		checkBits[value ^ 0xABAD1DEA] = True;
	}
	for (i = 0; i < 256; i++) {
		ASSERT(checkBits[i]);
	}
}
END_TEST

START_TEST(GetAllReturnsEverything)
{
	StringIntDictKeyValuePair *pairs;
	Int32 i, keyNumber;
	Bool keyCheckBits[256];
	Bool valueCheckBits[256];

	StringIntDict dict = StringIntDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringIntDict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		StringIntDict_Add(dict, String_Format("%u", i * 12), (i ^ 0xABAD1DEA));
	}
	ASSERT(StringIntDict_Count(dict) == 256);

	// Pull out all the values.
	pairs = StringIntDict_GetAll(dict);

	// Now make sure they're all divisible by 12 and forms of 0xABAD1DEA and that none are missing.
	MemZero(keyCheckBits, sizeof(Bool) * 256);
	MemZero(valueCheckBits, sizeof(Bool) * 256);
	for (i = 0; i < 256; i++) {
		keyNumber = atoi(String_ToC(pairs[i].key));
		ASSERT(keyNumber % 12 == 0);
		keyCheckBits[keyNumber / 12] = True;
		ASSERT((pairs[i].value ^ 0xABAD1DEA) < 256);
		valueCheckBits[pairs[i].value ^ 0xABAD1DEA] = True;
	}
	for (i = 0; i < 256; i++) {
		ASSERT(keyCheckBits[i]);
		ASSERT(valueCheckBits[i]);
	}
}
END_TEST

#include "stringintdict_tests.generated.inc"
