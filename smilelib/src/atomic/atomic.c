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

#include <smile/atomic.h>

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY) && SMILE_COMPILER == SMILE_COMPILER_MSVC
	#include "atomic_windows_msvc.inc"
#elif SMILE_OS == SMILE_OS_LINUX && SMILE_COMPILER == SMILE_COMPILER_GCC
	#include "atomic_linux_gcc.inc"
#else
	#error Atomic operations suitable for the current CPU/OS combination are not defined in 'atomic.c'.
#endif
