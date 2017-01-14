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

TEST_SUITE(StringDictTests)

//-------------------------------------------------------------------------------------------------
//  Creation Tests.

START_TEST(CanCreateStringDicts)
{
	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);
}
END_TEST

START_TEST(CanAddValuesIntoStringDictsAndRetrieveThemAgain)
{
	const char *text;

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	StringDict_AddC(dict, "soup", "chicken");
	StringDict_AddC(dict, "nuts", "walnut");
	StringDict_AddC(dict, "fruit", "banana");
	ASSERT(StringDict_Count(dict) == 3);

	text = (const char *)StringDict_GetValueC(dict, "soup");
	ASSERT(!strcmp(text, "chicken"));

	text = (const char *)StringDict_GetValueC(dict, "nuts");
	ASSERT(!strcmp(text, "walnut"));

	text = (const char *)StringDict_GetValueC(dict, "fruit");
	ASSERT(!strcmp(text, "banana"));
}
END_TEST

START_TEST(CanHandleSimpleAdditionAndLookup)
{
	Int i;
	PtrInt value;

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Insert 1024 values into whatever bucket the hashes suggest.
	for (i = 0; i < 1024; i++) {
		StringDict_Add(dict, String_Format("%u", i << 16), (void *)(PtrInt)(i ^ 0xABAD1DEA));
	}
	ASSERT(StringDict_Count(dict) == 1024);

	// Read out all 1024 values from the same bucket and make sure they contain the expected data.
	for (i = 0; i < 1024; i++) {
		value = (PtrInt)StringDict_GetValue(dict, String_Format("%u", i << 16));
		ASSERT(value == (i ^ 0xABAD1DEA));
	}
}
END_TEST

START_TEST(CanAddALotOfDataIntoADictionaryReliably)
{
	UInt32 seed, i;
	void *ptr;

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 100000; i++) {
		StringDict_Add(dict, String_Format("%u", seed), (void *)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringDict_Count(dict) == 100000);

	// Verify the dictionary still has all those values.
	seed = 31415;
	for (i = 0; i < 100000; i++) {
		ptr = StringDict_GetValue(dict, String_Format("%u", seed));
		ASSERT(ptr == (void *)i);
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(SetValueCanBothAddAndUpdate)
{
	UInt32 seed, i;
	void *ptr;

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		StringDict_SetValue(dict, String_Format("%u", seed), (void *)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringDict_Count(dict) == 1000);

	// Replace 500 of those values with new values.
	seed = 31415;
	for (i = 0; i < 500; i++) {
		StringDict_SetValue(dict, String_Format("%u", seed), (void *)(i ^ 0xFFFF));
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringDict_Count(dict) == 1000);

	// Verify the dictionary still has both the modified and original values.
	seed = 31415;
	for (i = 0; i < 500; i++) {
		ptr = StringDict_GetValue(dict, String_Format("%u", seed));
		ASSERT(ptr == (void *)(i ^ 0xFFFF));
		seed = (seed * 69069) + 127;
	}
	for (; i < 1000; i++) {
		ptr = StringDict_GetValue(dict, String_Format("%u", seed));
		ASSERT(ptr == (void *)i);
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(ContainsKeyCanAnswerWhenThingsExist)
{
	UInt32 seed, i;

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		StringDict_SetValue(dict, String_Format("%u", seed), (void *)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringDict_Count(dict) == 1000);

	// Verify the dictionary still the original values and nothing unexpected.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		if (!StringDict_ContainsKey(dict, String_Format("%u", seed))) {
			ASSERT(False);
		}
		seed = (seed * 69069) + 127;
	}
	for (; i < 2000; i++) {
		if (StringDict_ContainsKey(dict, String_Format("%u", seed))) {
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

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		StringDict_SetValue(dict, String_Format("%u", seed), (void *)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringDict_Count(dict) == 1000);

	// Verify the dictionary still the original values and nothing unexpected.
	seed = 31415;
	for (i = 0; i < 1000; i++) {
		if (!StringDict_TryGetValue(dict, String_Format("%u", seed), &ptr)) {
			ASSERT(False);
		}
		ASSERT(ptr == (void *)i);
		seed = (seed * 69069) + 127;
	}
	for (; i < 2000; i++) {
		if (StringDict_TryGetValue(dict, String_Format("%u", seed), &ptr)) {
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

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Fill up the dictionary with a whole bunch of values.  We use a simple PRNG to
	// generate evenly-distributed values.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		StringDict_Add(dict, String_Format("%u", seed), (void *)i);
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringDict_Count(dict) == 0x4000);

	// Remove 1/16 of the values, and do it in bunches just to be annoying.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		if ((i & 0xF00) == 0) {
			removed = StringDict_Remove(dict, String_Format("%u", seed));
			ASSERT(removed);
		}
		seed = (seed * 69069) + 127;
	}
	ASSERT(StringDict_Count(dict) == 0x4000 - 0x400);

	// Now make sure the dictionary contains known data.
	seed = 31415;
	for (i = 0; i < 0x4000; i++) {
		exists = StringDict_TryGetValue(dict, String_Format("%u", seed), &ptr);
		if ((i & 0xF00) == 0) {
			ASSERT(!exists);
		}
		else {
			ASSERT(exists && (void *)i == ptr);
		}
		seed = (seed * 69069) + 127;
	}
}
END_TEST

START_TEST(CanHandleRepeatedMutation)
{
	Int i, retry;
	void *ptr;
	Bool found;

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	for (retry = 0; retry < 3; retry++) {
		// Insert 1024 values, all into the same bucket.
		for (i = 0; i < 1024; i++) {
			StringDict_Add(dict, String_Format("%d", i << 16), (void *)(PtrInt)(i ^ 0xABAD1DEA));
		}
		ASSERT(StringDict_Count(dict) == 1024);

		// Remove 512 of those values from the middle of the bucket.
		for (i = 256; i < 768; i++) {
			ASSERT(StringDict_Remove(dict, String_Format("%d", i << 16)));
		}
		// And also the very first and very last ones.
		ASSERT(StringDict_Remove(dict, String_Format("%d", 0 << 16)));
		ASSERT(StringDict_Remove(dict, String_Format("%d", 1023 << 16)));
		ASSERT(StringDict_Count(dict) == 510);

		// Read out all 510 values from the same bucket and make sure they contain the expected data.
		for (i = -1; i <= 1024; i++) {
			found = StringDict_TryGetValue(dict, String_Format("%d", i << 16), &ptr);
			if ((i >= 1 && i < 256) || (i >= 768 && i < 1023)) {
				ASSERT(found);
				ASSERT((PtrInt)ptr == (i ^ 0xABAD1DEA));
			}
			else {
				ASSERT(!found);
			}
		}

		// Reinsert the first and last entries.
		StringDict_Add(dict, String_Format("%d", 0 << 16), (void *)(PtrInt)(0 ^ 0xABAD1DEA));
		StringDict_Add(dict, String_Format("%d", 1023 << 16), (void *)(PtrInt)(1023 ^ 0xABAD1DEA));
		ASSERT(StringDict_Count(dict) == 512);

		// Verify that we have 512 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = StringDict_TryGetValue(dict, String_Format("%d", i << 16), &ptr);
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
			StringDict_Add(dict, String_Format("%d", i << 16), (void *)(PtrInt)(i ^ 0xABAD1DEA));
		}
		ASSERT(StringDict_Count(dict) == 1024);

		// Verify that we have 1024 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = StringDict_TryGetValue(dict, String_Format("%d", i << 16), &ptr);
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
			ASSERT(StringDict_Remove(dict, String_Format("%d", i << 16)));
		}
		ASSERT(StringDict_Count(dict) == 0);

		// Verify that we have 0 usable entries.
		for (i = -1; i <= 1024; i++) {
			found = StringDict_TryGetValue(dict, String_Format("%d", i << 16), &ptr);
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

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		StringDict_Add(dict, String_Format("%u", i * 12), (void *)(i ^ 0xABAD1DEA));
	}
	ASSERT(StringDict_Count(dict) == 256);

	// Pull out all the keys.
	keys = StringDict_GetKeys(dict);

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
	void **values;
	void *value;
	Int32 i;
	Bool checkBits[256];

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		StringDict_Add(dict, String_Format("%u", i * 12), (void *)(i ^ 0xABAD1DEA));
	}
	ASSERT(StringDict_Count(dict) == 256);

	// Pull out all the values.
	values = StringDict_GetValues(dict);

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
	StringDictKeyValuePair *pairs;
	Int32 i, keyNumber;
	Bool keyCheckBits[256];
	Bool valueCheckBits[256];

	StringDict dict = StringDict_Create();
	ASSERT(dict != NULL);
	ASSERT(StringDict_Count(dict) == 0);

	// Insert 256 values, with keys that should cause some hash collisions.
	for (i = 0; i < 256; i++) {
		StringDict_Add(dict, String_Format("%u", i * 12), (void *)(i ^ 0xABAD1DEA));
	}
	ASSERT(StringDict_Count(dict) == 256);

	// Pull out all the values.
	pairs = StringDict_GetAll(dict);

	// Now make sure they're all divisible by 12 and forms of 0xABAD1DEA and that none are missing.
	MemZero(keyCheckBits, sizeof(Bool) * 256);
	MemZero(valueCheckBits, sizeof(Bool) * 256);
	for (i = 0; i < 256; i++) {
		keyNumber = atoi(String_ToC(pairs[i].key));
		ASSERT(keyNumber % 12 == 0);
		keyCheckBits[keyNumber / 12] = True;
		ASSERT(((PtrInt)(pairs[i].value) ^ 0xABAD1DEA) < 256);
		valueCheckBits[(PtrInt)(pairs[i].value) ^ 0xABAD1DEA] = True;
	}
	for (i = 0; i < 256; i++) {
		ASSERT(keyCheckBits[i]);
		ASSERT(valueCheckBits[i]);
	}
}
END_TEST

#include "stringdict_tests.generated.inc"
