//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

#include <math.h>

#include <smile/gc.h>
#include <smile/mem.h>
#include <smile/string.h>
#include <smile/dict/int32dict.h>

String DictStats_ToString(DictStats stats)
{
	return String_Format(
		"DictStats:\n"
		"  heap total: %lu\n"
		"  heap alloc: %lu\n"
		"  heap free: %lu\n"
		"  bucket min size: %lu\n"
		"  bucket max size: %lu\n"
		"  bucket avg size: %u.%02u\n"
		"  bucket stddev: %u.%02u\n",
		stats->heapTotal,
		stats->heapAlloc,
		stats->heapFree,
		stats->bucketMin,
		stats->bucketMax,
		(int)stats->bucketAvg, (int)((stats->bucketAvg - floor(stats->bucketAvg)) * 100),
		(int)stats->bucketStdDev, (int)((stats->bucketStdDev - floor(stats->bucketStdDev)) * 100)
	);
}
