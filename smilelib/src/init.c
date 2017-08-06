//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#include <stdio.h>
#include <stdlib.h>

#include <smile/internal/types.h>
#include <smile/gc.h>
#include <smile/env/env.h>
#include <smile/env/symboltable.h>
#include <smile/env/knownsymbols.h>
#include <smile/env/knownobjects.h>
#include <smile/env/knownbases.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/text/smilesymbol.h>

extern void Smile_InitTicks(void);

/// <summary>
/// Whether or not the Smile runtime has been initialized yet.
/// </summary>
static Bool Smile_IsInitialized = False;

/// <summary>
/// The global hash oracle, which ensures that successive runs of the same program yield
/// different hash codes.  This ensures that people do not attempt to make assumptions about
/// the order or values of their hash codes.
/// </summary>
UInt32 Smile_HashOracle;

// The environment.
SymbolTable Smile_SymbolTable;
struct KnownSymbolsStruct Smile_KnownSymbols;
struct KnownObjectsStruct Smile_KnownObjects;
struct KnownBasesStruct Smile_KnownBases;

/// <summary>
/// Initialize the Smile runtime.  This must be performed at least once on startup.
/// </summary>
void Smile_Init(void)
{
	if (Smile_IsInitialized) return;

	Smile_InitTicks();

	GC_INIT();

	Smile_HashOracle = (UInt32)GetBaselineEntropy();

	Smile_ResetEnvironment();
}

void Smile_ResetEnvironment(void)
{
	// Clear out as many GC roots as we know about.
	Smile_SymbolTable = NULL;
	MemZero(&Smile_KnownSymbols, sizeof(struct KnownSymbolsStruct));
	MemZero(&Smile_KnownBases, sizeof(struct KnownBasesStruct));
	MemZero(&Smile_KnownObjects, sizeof(struct KnownObjectsStruct));

	// Now give the garbage collector a chance to make the world as clean as possible.
	GC_gcollect();

	// Make a symbol table for this environment.
	Smile_SymbolTable = SymbolTable_Create();

	// Preload the known symbols into this environment.
	KnownSymbols_PreloadSymbolTable(Smile_SymbolTable, &Smile_KnownSymbols);

	// Preload the known bases into this environment.  This must come first, or we can't
	// correctly instantiate any other Smile types.
	KnownBases_Preload(&Smile_KnownBases);

	// Preload the known objects into this environment.
	KnownObjects_Setup(&Smile_KnownObjects, &Smile_KnownSymbols);

	// Now that we have enough of a usable environment to make real objects, it's time to populate
	// all of the methods on the base objects, and to set up any other shared data they may need.
	KnownBases_Setup(&Smile_KnownBases);
}

/// <summary>
/// Shut down the Smile runtime.  This must be performed at least once on program shutdown.
/// </summary>
void Smile_End(void)
{
}

/// <summary>
/// Abort execution of the program because the runtime has run out of memory.
/// </summary>
void Smile_Abort_OutOfMemory(void)
{
	Smile_Abort_FatalError("Out of memory!");
}

/// <summary>
/// Abort execution of the program because the runtime has run out of memory.
/// </summary>
void Smile_Abort_FatalError(const char *message)
{
	fprintf(stderr, "\nFatal error:  %s\nAborting program.\n", message);
	exit(-1);
}

//-------------------------------------------------------------------------------------------------

void Smile_ThrowException(Symbol exceptionKind, String message)
{
	SmileObject exception = (SmileObject)Smile_CreateException(exceptionKind, message);
	Smile_Throw(exception);
}

void Smile_ThrowExceptionCV(const char *exceptionKind, const char *format, va_list v)
{
	SmileObject exception = (SmileObject)Smile_CreateExceptionCV(exceptionKind, format, v);
	Smile_Throw(exception);
}

void Smile_ThrowExceptionC(const char *exceptionKind, const char *format, ...)
{
	SmileObject exception;
	va_list v;

	va_start(v, format);
	exception = (SmileObject)Smile_CreateExceptionCV(exceptionKind, format, v);
	va_end(v);

	Smile_Throw(exception);
}

SmileUserObject Smile_CreateException(Symbol exceptionKind, String message)
{
	SmileUserObject exception = SmileUserObject_Create((SmileObject)Smile_KnownBases.Exception, Smile_KnownSymbols.Exception_);
	SmileUserObject_Set(exception, Smile_KnownSymbols.kind, SmileSymbol_Create(exceptionKind));
	SmileUserObject_Set(exception, Smile_KnownSymbols.message, message);
	return exception;
}

SmileUserObject Smile_CreateExceptionC(const char *exceptionKind, const char *format, ...)
{
	va_list v;
	Symbol symbol;
	String message;

	symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, exceptionKind);

	va_start(v, format);
	message = String_FormatV(format, v);
	va_end(v);

	return Smile_CreateException(symbol, message);
}

SmileUserObject Smile_CreateExceptionCV(const char *exceptionKind, const char *format, va_list v)
{
	Symbol symbol;
	String message;

	symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, exceptionKind);
	message = String_FormatV(format, v);

	return Smile_CreateException(symbol, message);
}

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)

#	define WIN32_LEAN_AND_MEAN
#	pragma warning(push)
#	pragma warning(disable: 4255)
#	include <windows.h>
#	pragma warning(pop)

	String Smile_Win32_GetErrorString(UInt32 errorCode)
	{
		LPWSTR messageBuffer;
		Int messageLength;
		String result;

		if (!errorCode)
			return String_Empty;

		messageBuffer = NULL;
		messageLength = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&messageBuffer,
			0,
			NULL
		);

		result = String_FromUtf16(messageBuffer, messageLength);

		LocalFree(messageBuffer);

		return result;
	}

#else

	String Smile_Win32_GetErrorString(UInt32 errorCode)
	{
		UNUSED(errorCode);

		return String_Empty;
	}

#endif

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)

#	include <errno.h>

	String Smile_Unix_GetErrorString(Int32 errorCode)
	{
		char buffer[1024];

		if (strerror_r(errorCode, buffer, 1023)) {
			return String_Format("unknown error %d", errorCode);
		}
		buffer[1023] = '\0';
		return String_FromC(buffer);
	}

#else

	String Smile_Unix_GetErrorString(Int32 errorCode)
	{
		UNUSED(errorCode);

		return String_Empty;
	}

#endif