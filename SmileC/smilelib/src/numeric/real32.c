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

#include <smile/numeric/real32.h>
#include <smile/numeric/real64.h>
#include <smile/numeric/real128.h>

SMILE_API Real32 Real32_Zero = { 0 };
SMILE_API Real32 Real32_One = { 1 };

SMILE_API Real32 Real32_FromInt32(Int32 int32)
{
	UNUSED(int32);
	return Real32_Zero;
}

SMILE_API Real32 Real32_FromInt64(Int64 int64)
{
	UNUSED(int64);
	return Real32_Zero;
}

SMILE_API Real64 Real32_ToReal64(Real32 real32)
{
	UNUSED(real32);
	return Real64_Zero;
}

SMILE_API Real128 Real32_ToReal128(Real32 real32)
{
	UNUSED(real32);
	return Real128_Zero;
}
