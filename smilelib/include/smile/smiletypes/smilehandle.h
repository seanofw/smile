
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
/// Handles may optionally provide useful methods for them to interact with
/// the type system as more "normal" objects.
/// </summary>
typedef struct SmileHandleMethodsStruct {
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
	Bool (*end)(SmileHandle handle, Bool userInvoked);

	/// <summary>
	/// This will be invoked to convert the handle to a boolean value, in if-statements and such.
	/// If omitted, the handle will always be converted to 'true'.
	/// </summary>
	Bool (*toBool)(SmileHandle handle, SmileUnboxedData unboxedData);

	/// <summary>
	/// This will be invoked to convert the handle to a string.  If omitted, the handle will be
	/// converted to its symbol name.
	/// </summary>
	String (*toString)(SmileHandle handle, SmileUnboxedData unboxedData);

	/// <summary>
	/// Get a property of the handle.  If omitted, this will return NullObject.
	/// </summary>
	SmileObject (*getProperty)(SmileHandle handle, Symbol symbol);

	/// <summary>
	/// Ask a handle if it has a property.  If omitted, this will return False.
	/// </summary>
	Bool (*hasProperty)(SmileHandle handle, Symbol symbol);

	/// <summary>
	/// Set a property on a handle.  If omitted, this will raise an exception.
	/// </summary>
	void (*setProperty)(SmileHandle handle, Symbol symbol, SmileObject value);

	/// <summary>
	/// Get a list of all property names of the handle.  If omitted, this will
	/// return NullObject.
	/// </summary>
	SmileList (*getPropertyNames)(SmileHandle handle);
} *SmileHandleMethods;

struct SmileHandleInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileHandleMethods methods;		// Methods for acting like a "normal" object.
	Symbol handleKind;				// What kind of handle this is (unique a "class" name, like 'file')
	void *ptr;						// A pointer to the handle's actual data.
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileHandle_VTable;

SMILE_API_FUNC SmileHandle SmileHandle_Create(SmileObject base, SmileHandleMethods methods, Symbol handleKind, void *ptr);

#endif
