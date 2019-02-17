
#ifndef __SMILE_SMILETYPES_HANDLE_H__
#define __SMILE_SMILETYPES_HANDLE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

/// <summary>
/// This is a cleanup function for a handle.  It will be invoked explicitly by a user call to
/// the 'end' method (in which case 'userInvoked' will be True), or, if necessary, implicitly by
/// a GC finalizer (in which case 'userInvoked' will be False).  Its job is to release all
/// resources held by the handle, whatever they may be.  If 'userInvoked' is False, this must
/// not allocate memory on the GC heap.
/// </summary>
/// <param name="handle">The handle that is to be ended/closed/cleaned-up.</param>
/// <param name="userInvoked">Whether this was triggered by an explicit user call in code,
/// either by [handle.end] or by the 'auto' keyword (True), or whether this was triggered
/// late by the garbage collector (False).</param>
/// <returns>True if the handle could be successfully ended (or was already successfully ended),
/// False if the attempt to end the handle failed.</returns>
typedef Bool (*SmileHandleEnd)(SmileHandle handle, Bool userInvoked);

struct SmileHandleInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileHandleEnd end;		// Clean up the handle, implicitly or explicitly
	Symbol handleKind;		// What kind of handle this is (unique a "class" name, like 'file')
	void *ptr;				// A pointer to the handle's actual data.
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileHandle_VTable;

SMILE_API_FUNC SmileHandle SmileHandle_Create(SmileObject base, SmileHandleEnd end, Symbol handleKind, void *ptr);

#endif
