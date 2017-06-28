
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
typedef void (*SmileHandleEnd)(void *handle, Bool userInvoked);

struct SmileHandleInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Symbol handleName;	// What kind of handle this is (unique a "class" name, like 'file')
	Int32 costEstimate;	// How "expensive" this handle is, relative to a one-byte memory allocation.
	void *handle;	// The raw handle itself
	SmileHandleEnd end;	// Clean up the handle, implicitly or explicitly
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileHandle_VTable;

SMILE_API_FUNC SmileHandle SmileHandle_Create(SmileObject base, Symbol handleName, Int32 costEstimate, void *handle, SmileHandleEnd end);

#endif
