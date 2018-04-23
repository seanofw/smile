
#ifndef __SMILE_SMILETYPES_TILLCONTINUATION_H__
#define __SMILE_SMILETYPES_TILLCONTINUATION_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

/// <summary>
/// A TillContinuation is very much like a full continuation, with two key differences:
/// First, rather than having a single, hardcoded continuation instruction that exactly matches
/// the instruction after the continuation was created, we have an array of possible continuation
/// target instructions, chosen in advance; and second, the 'EndTill' instruction can effectively
/// destroy an instance of this.  Because of the way the compiler emits NewTill, EndTill, and
/// TillEsc instructions, this can only ever be used in the form of an escape continuation,
/// but it is actually a real, full continuation under the hood.
/// </summary>
struct SmileTillContinuationInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	Closure closure;				// The closure in which we were executing.
	ByteCodeSegment segment;		// The segment containing the code that can be executed.
	Int32 stackTop;					// How deep the closure stack was.

	Int32 numBranchTargetAddresses;	// How many entries exist in the branch target array.
	Int32 *branchTargetAddresses;	// An array of allowed branch targets (copied from TillContinuationInfo).
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileTillContinuation_VTable;

SMILE_API_FUNC SmileTillContinuation SmileTillContinuation_Create(SmileObject base, Closure closure,
	ByteCodeSegment segment, Int32 *branchTargetAddresses, Int32 numBranchTargetAddresses);

#endif
