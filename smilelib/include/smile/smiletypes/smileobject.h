
#ifndef __SMILE_SMILETYPES_OBJECT_H__
#define __SMILE_SMILETYPES_OBJECT_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_DICT_POINTERSET_H__
#include <smile/dict/pointerset.h>
#endif

#ifndef __SMILE_ENV_ENV_H__
#include <smile/env/env.h>
#endif

#ifndef __SMILE_ENV_SYMBOLTABLE_H__
#include <smile/env/symboltable.h>
#endif

#ifndef __SMILE_SMILETYPES_KIND_H__
#include <smile/smiletypes/kind.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

//-------------------------------------------------------------------------------------------------
//  The core Object declaration itself

/// <summary>
/// These four fields are shared by all Smile objects, and must appear at the start of each
/// object's data in memory.
/// </summary>
#define DECLARE_BASE_OBJECT_PROPERTIES \
	UInt32 kind;	/* What kind of native object this is, from the SMILE_KIND enumeration */ \
	SmileVTable vtable;	/* A pointer to this object's virtual table, which must match SMILE_VTABLE_TYPE. */ \
	SmileObject base	/* A pointer to the "base" object for this object, i.e., the object from which this object inherits any properties. */

/// <summary>
/// This is the core Smile "Object" object itself.  Only one instance of this will ever exist.
/// </summary>
struct SmileObjectInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  The argument struct, used for passing objects around in an unboxed form (where appropriate).

/// <summary>
/// The unboxed portion of an argument, if such a portion exists.  This is at most 64 bits.
/// </summary>
typedef union {
	Bool b;
	Byte i8;
	Int16 i16;
	Int32 i32;
	Int64 i64;
	Real32 r32;
	Real64 r64;
	Float32 f32;
	Float64 f64;
	Symbol symbol;
} SmileUnboxedData;

/// <summary>
/// This shape represents a single function argument or local variable.  It consists of a pointer
/// to a real SmileObject (usually on the heap), and, if that object is unboxed, the unboxed data
/// immediately adjacent to it.  Note that unlike most other typedefs in the interpreter, the name
/// SmileArg refers to a STRUCT, not to a pointer to it.
/// </summary>
typedef struct {
	SmileObject obj;	// A pointer to the object instance itself.
	SmileUnboxedData unboxed;	// Any unboxed data associated with this arg, if this is an unboxed type.
} SmileArg;

//-------------------------------------------------------------------------------------------------
//  The virtual table common to all objects.

/// <summary>
/// Declare the type of a Smile object virtual table.
/// </summary>
/// <param name="__name__">The name of the virtual table type, like "struct FooObjectVTable".
/// (This is not a string, but the actual identifier to use in code.)</param>
/// <param name="__type__">The type of object this virtual table's methods manipulate, like "FooObject".
/// (Again, this is not a string, but the actual identifier to use in code.)</param>
/// <returns>A struct declaration that matches the given virtual table.</returns>
/// <remarks>
/// <h3>Documentation for the standard fourteen Smile object methods</h3>
///
/// <hr />
/// <h4>Base operations:</h4>
///
/// <code>compareEqual</code>:	<p>Compare this object against another object, which could be an object of any type.
///	Return true if they are logically equal, false if they are not.  This must follow the
///	rules listed below:</p>
///	<ul>
///	<li>Objects may only be equal if they are of the same SMILE_KIND;</li>
///	<li>Equality is commutative; that is, "a == b" has the same meaning as "b == a".</li>
///	<li>The equality function must be defined consistent with the hash() function below.</li>
///	<li>The equality function should run in constant time.</li>
///	<li>The only object that may equal the Primitive instance is the Primitive instance.</li>
///	<li>The only object that may equal the Null instance is the Null instance.</li>
///	</ul>
///	
/// <code>hash</code>:	<p>Generate a 32-bit hash code for this object that would allow it to be suitable for
///	use as a dictionary key.  The following rules for hash codes must be observed:</p>
///	<ul>
///	<li>Hashing must be consistent; that is, hash(x) must equal hash(x) no matter how
///			many times it is invoked nor where or when it is invoked for the same x.</li>
///	<li>Hashing must be consistent with object equality; that is, if a ==b, then
///			hash(a) == hash(b).</li>
///	<li>Hash codes should be as close to uniformly-randomly-distributed as possible.</li>
///	</ul>
///	
/// <code>deepEqual</code>:	<p>Compare this object against another object, which could be an object of any type.
///	Return true if they are logically equal, false if they are not.  This must follow the
///	rules listed below:</p>
///	<ul>
///	<li>Objects may only be equal if they are of the same SMILE_KIND;</li>
///	<li>Equality is commutative; that is, "a == b" has the same meaning as "b == a".</li>
///	<li>The equality function must be defined consistent with the hash() function below.</li>
///	<li>The equality function should attempt to compare all data of both objects,
///	recursively if necessary, and may run in O(n) time.</li>
///	<li>The only object that may equal the Primitive instance is the Primitive instance.</li>
///	<li>The only object that may equal the Null instance is the Null instance.</li>
///	</ul>
///	
/// <hr />	
/// <h4>Security operations:</h4>	
///	
/// <code>setSecurityKey</code>:	<p>Change the security key of this object to be the provided object instance.  Security
///	keys allow an object to be locked down to prevent alteration by unauthorized parties.
///	Any object may be used as a security key.  By default, all objects start with Null as
///	their security key.  This requires you to pass the object's current security key in order
///	to be authorized to set a new one.  Not all object types participate in security; but
///	notably, UserObject and Façade do, and they alone are sufficient to be able to apply
///	security to all other objects.</p>
///	
/// <code>setSecurity</code>:	<p>Change the current security flags applied to this object, from the SMILE_SECURITY_*
///	enum.  This replaces the current security flags, in full.  You may only alter this object's
///	security if you provide the correct security key for it.  An object may choose to ignore
///	or alter requests to change its security, according to its own internal rules (for example,
///	an always-immutable object type, like String, may always prohibit attempts to
///	force it to allow WRITE access to its data).</p>
///	
/// <code>getSecurity</code>:	<p>Get the current security flags applied to this object, from the SMILE_SECURITY_*
///	enum.  All objects may be queried as to their security, and must respond according to
///	what they permit.</p>
///	
/// <hr />	
/// <h4>Property operations:</h4>	
///	
/// <code>getProperty</code>:	<p>Retrieve a property of this object, by its symbol.  If no such property exists, this
///	must return the Null object.  (Note that this necessarily implies that there is no difference
///	as far as getProperty() is concerned between a nonexistent property and a property
///	containing Null.).  If this object prohibits READ security access, this should return
///	the Null object.</p>
///	
/// <code>setProperty</code>:	<p>Add or update a property of this object, by its symbol. If no such property exists,
///	this <em>may</em> create the property. Some objects may prohibit property additions
///	and modifications; some may only allow specific values for some properties; and some
///	may observe the security rules when adding or updating properties.  No guarantees are
///	placed on the behavior of this method.</p>
///	
/// <code>hasProperty</code>:	<p>Determine whether this object has the named property.  Should return true if
///	the object has this property, or false if it does not.  This method usually (but not always!)
///	matches whether getProperty() for this object would return a non-Null value.  If this
///	object prohibits READ access, thish should usually return false for all symbols.</p>
///	
/// <code>getPropertyNames</code>:	<p>Retrieve a list of symbols that identify all of the properties of this object.  This
///	data is informative, not normative.  In general, it <em>should</em> return a list
///	that contains all the symbols for which hasProperty() would return true.  If this object
///	prohibits READ access, this should usually return Null (the empty list).</p>
///		
/// <hr />	
/// <h4>Conversion operations:</h4>	
///	
/// <code>toBool</code>:	<p>Construct a Boolean representation of this object.  This conversion will be used
///	in many places, including if-statements, and should be based on the content of the
///	object:  For example, empty strings convert to false, but nonempty strings convert to
///	true.  Numeric values convert to false if they are zero, and true otherwise.  Most objects
///	other than numbers and strings should return true, but there can be legitimate reasons
///	for a nontrivial object to be false (a closed network stream, for example).</p>
///	
/// <code>toInteger32</code>:	<p>Construct an Integer32 representation of this object.  This conversion is used in
///	several places, and should be based on the content of the object:  Strings and arrays, for
///	example, return their length.  This should generally be nonzero for most objects, but
///	there can be legitimate reasons for a nontrivial object to be zero (an empty queue, for
///	example).</p>
///	
/// <code>toFloat64</code>:	<p>Construct a Float64 representation of this object.  This conversion is used in only
///	a few specific places.  It should be based on the content of the object:  Strings and arrays, for
///	example, return their length.  This should generally be nonzero for most objects, but
///	there can be legitimate reasons for a nontrivial object to be zero (the cartesian length of a
///	3D vector, for example).</p>
///	
/// <code>toReal64</code>:	<p>Construct a Real64 representation of this object.  This conversion is used in only
///	a few specific places.  It should be based on the content of the object:  Strings and arrays, for
///	example, return their length.  This should generally be nonzero for most objects, but
///	there can be legitimate reasons for a nontrivial object to be zero (the cartesian length of a
///	3D vector, for example).</p>
///	
/// <code>toString</code>:	<p>Construct a string representation of this object.  This conversion is used in many
///	places, and is intended to be <em>human-readable</em>, not necessarily a formal
///	serialization of the object.  It is used during debugging to observe the state of objects, and
///	is used by all of the <code>print</code> methods for outputting objects to streams.
///	Despite the fact that is it not intended to be a serialization method, many of the standard
///	built-in objects produce strings that are equivalent to a serialized form.</p>
///	
/// <hr />	
/// <h4>Special operations:</h4>	
///	
/// <code>call</code>:	<p>Invoke this object as a function.  The number of arguments will be provided directly, and
///	the arguments themselves will be on the temporary stack of the current closure.</p>
///
/// <code>getSourceLocation</code>:	<p>Get the location in the source code where this object was created, if known.</p>
///	
/// <code>unbox</code>:	<p>Copy this object into the provided argument container, unboxing it (if
///	appropriate).  This is the opposite of the 'box' operation, below.</p>
///	
/// <code>box</code>:	<p>Copy the given argument container's value onto the heap, boxing it (if necessary)
///	and returning the boxed value.  This is the opposite of the 'unbox' operation, above.</p>
///
/// </remarks>
#define SMILE_VTABLE_TYPE(__name__, __type__) \
	__name__ { \
		Bool (*compareEqual)(__type__ self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData); \
		Bool (*deepEqual)(__type__ self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers); \
		UInt32 (*hash)(__type__ self); \
		\
		void (*setSecurityKey)(__type__ self, SmileObject newSecurityKey, SmileObject oldSecurityKey); \
		void (*setSecurity)(__type__ self, Int security, SmileObject securityKey); \
		Int (*getSecurity)(__type__ self); \
		\
		SmileObject (*getProperty)(__type__ self, Symbol propertyName); \
		void (*setProperty)(__type__ self, Symbol propertyName, SmileObject value); \
		Bool (*hasProperty)(__type__ self, Symbol propertyName); \
		SmileList (*getPropertyNames)(__type__ self); \
		\
		Bool (*toBool)(__type__ self, SmileUnboxedData unboxedData); \
		Int32 (*toInteger32)(__type__ self, SmileUnboxedData unboxedData); \
		Float64 (*toFloat64)(__type__ self, SmileUnboxedData unboxedData); \
		Real64 (*toReal64)(__type__ self, SmileUnboxedData unboxedData); \
		String (*toString)(__type__ self, SmileUnboxedData unboxedData); \
		\
		void (*call)(__type__ self, Int argc, Int extra); \
		LexerPosition (*getSourceLocation)(__type__ self); \
		\
		SmileArg (*unbox)(__type__ self); \
		SmileObject (*box)(SmileArg src); \
	}

/// <summary>
/// Declare an instance of a Smile object virtual table.
/// </summary>
/// <remarks>
/// <p>This should be followed by a curly-brace block that contains pointers to the methods of the
/// virtual table, like this:</p>
/// <pre>
///	SMILE_VTABLE(FooObjectVTable, FooObject) {
///		FooObject_CompareEqual,
///		FooObject_Hash,
///		...
///	}
/// </pre>
/// <p>This generates a static data struct named <em>FooObjectVTable</em> that is of a proper
/// VTable type and that is filled in with the methods named in the curly-brace block.</p>
/// </remarks>
/// <param name="__name__">The name of the virtual table declaration, like "FooObjectVTable".
/// (This is not a string, but the actual identifier to use in code.)</param>
/// <param name="__type__">The type of object this virtual table's methods manipulate, like "FooObject".
/// (Again, this is not a string, but the actual identifier to use in code.)</param>
/// <returns>Code that declares an instance of the virtual table as static data.</returns>
#define SMILE_VTABLE(__name__, __type__) \
	SMILE_VTABLE_TYPE(struct __name__##Int, __type__); \
	struct __name__##Int __name__##Data; \
	\
	SmileVTable __name__ = (SmileVTable)&__name__##Data; \
	\
	struct __name__##Int __name__##Data =

/// <summary>
/// Get whatever kind of native Smile object this object is.
/// </summary>
/// <param name="obj">The object to get the kind of.</param>
/// <returns>The kind of this object, from the SMILE_KIND_* enum.</returns>
#define SMILE_KIND(obj) ((obj)->kind & SMILE_KIND_MASK)

/// <summary>
/// Perform a virtual call to the given method on the object, passing no arguments.
/// </summary>
/// <param name="__obj__">The object whose method you would like to call.</param>
/// <param name="__method__">The name of the method to call, like "toString" (without quotes).</param>
/// <returns>The return value from the method.</returns>
#define SMILE_VCALL(__obj__, __method__) \
	((__obj__)->vtable->__method__((SmileObject)(__obj__)))

/// <summary>
/// Perform a virtual call to the given method on the object, passing one argument.
/// </summary>
/// <param name="__obj__">The object whose method you would like to call.</param>
/// <param name="__arg1__">The first argument to pass to the method.</param>
/// <param name="__method__">The name of the method to call, like "getProperty" (without quotes).</param>
/// <returns>The return value from the method.</returns>
#define SMILE_VCALL1(__obj__, __method__, __arg1__) \
	((__obj__)->vtable->__method__((SmileObject)(__obj__), __arg1__))

/// <summary>
/// Perform a virtual call to the given method on the object, passing two arguments.
/// </summary>
/// <param name="__obj__">The object whose method you would like to call.</param>
/// <param name="__arg1__">The first argument to pass to the method.</param>
/// <param name="__arg2__">The second argument to pass to the method.</param>
/// <param name="__method__">The name of the method to call, like "setProperty" (without quotes).</param>
/// <returns>The return value from the method.</returns>
#define SMILE_VCALL2(__obj__, __method__, __arg1__, __arg2__) \
	((__obj__)->vtable->__method__((SmileObject)(__obj__), __arg1__, __arg2__))

/// <summary>
/// Perform a virtual call to the given method on the object, passing three arguments.
/// </summary>
/// <param name="__obj__">The object whose method you would like to call.</param>
/// <param name="__arg1__">The first argument to pass to the method.</param>
/// <param name="__arg2__">The second argument to pass to the method.</param>
/// <param name="__arg3__">The third argument to pass to the method.</param>
/// <param name="__method__">The name of the method to call, like "compareEqual" (without quotes).</param>
/// <returns>The return value from the method.</returns>
#define SMILE_VCALL3(__obj__, __method__, __arg1__, __arg2__, __arg3__) \
	((__obj__)->vtable->__method__((SmileObject)(__obj__), __arg1__, __arg2__, __arg3__))

/// <summary>
/// Perform a virtual call to the given method on the object, passing four arguments.
/// </summary>
/// <param name="__obj__">The object whose method you would like to call.</param>
/// <param name="__arg1__">The first argument to pass to the method.</param>
/// <param name="__arg2__">The second argument to pass to the method.</param>
/// <param name="__arg3__">The third argument to pass to the method.</param>
/// <param name="__arg4__">The fourth argument to pass to the method.</param>
/// <param name="__method__">The name of the method to call, like "deepEqual" (without quotes).</param>
/// <returns>The return value from the method.</returns>
#define SMILE_VCALL4(__obj__, __method__, __arg1__, __arg2__, __arg3__, __arg4__) \
	((__obj__)->vtable->__method__((SmileObject)(__obj__), __arg1__, __arg2__, __arg3__, __arg4__))

/// <summary>
/// Perform a virtual call to the getProperty method on the object.
/// </summary>
/// <param name="__obj__">The object whose getProperty method you would like to call.</param>
/// <param name="__name__">The name of the property you would like getProperty to return.</param>
/// <returns>The resulting property value, or NullObject if no property exists by that name.</returns>
#define SMILE_GET_PROPERTY(__obj__, __name__) \
	((__obj__)->vtable->getProperty((SmileObject)(__obj__), __name__))

//-------------------------------------------------------------------------------------------------
//  Declare the core SmileObject itself, its virtual table, and common (external) operations
//  for working with it.

SMILE_VTABLE_TYPE(struct SmileVTableInt, SmileObject);

SMILE_API_DATA SmileVTable SmileObject_VTable;

SMILE_API_FUNC SmileObject SmileObject_Create(void);

//-------------------------------------------------------------------------------------------------
// These aren't core operations, but they're commonly-needed.

SMILE_API_FUNC String SmileObject_Stringify(SmileObject obj);
SMILE_API_FUNC const char *SmileObject_StringifyToC(SmileObject obj);

SMILE_API_FUNC Bool SmileObject_IsRegularList(SmileObject list);
SMILE_API_FUNC Bool SmileObject_ContainsNestedList(SmileObject obj);

SMILE_API_FUNC Bool SmileObject_DeepCompare(SmileObject self, SmileObject other);
SMILE_API_FUNC Bool SmileArg_DeepCompare(SmileArg self, SmileArg other);

SMILE_API_FUNC String SmileKind_GetName(Int kind);

//-------------------------------------------------------------------------------------------------
//  Inline operations on SmileObject.

Inline Bool SmileObject_Is(SmileObject self, SmileObject possibleParentOrSelf)
{
	for (;;) {
		if (self == possibleParentOrSelf) return True;
		if (SMILE_KIND(self) == SMILE_KIND_PRIMITIVE) return False;
		self = self->base;
	}
}

Inline Bool SmileObject_IsList(SmileObject self)
{
	register UInt32 kind = SMILE_KIND(self);
	return kind == SMILE_KIND_LIST || kind == SMILE_KIND_NULL;
}

Inline Bool SmileObject_IsListWithSource(SmileObject self)
{
	return SmileObject_IsList(self) && (self->kind & SMILE_FLAG_WITHSOURCE);
}

Inline Bool SmileObject_IsPair(SmileObject self)
{
	return SMILE_KIND(self) == SMILE_KIND_PAIR;
}

Inline Bool SmileObject_IsPairWithSource(SmileObject self)
{
	return SmileObject_IsPair(self) && (self->kind & SMILE_FLAG_WITHSOURCE);
}

Inline Bool SmileObject_IsSymbol(SmileObject self)
{
	return SMILE_KIND(self) == SMILE_KIND_SYMBOL;
}

Inline Bool SmileObject_IsNull(SmileObject self)
{
	return SMILE_KIND(self) == SMILE_KIND_NULL;
}

/// <summary>
/// Promote a boxed object up to a full function argument.
/// </summary>
/// <param name="obj">The object to be wrapped up as a SmileArg (with no unboxed data attached).</param>
/// <returns>The same object, in a SmileArg wrapper.</returns>
Inline SmileArg SmileArg_From(SmileObject obj)
{
	SmileArg arg;
	arg.obj = obj;
	return arg;
}

/// <summary>
/// Perform a virtual call to the given argument's special 'box' method, resulting in a real
/// object on the heap.
/// </summary>
/// <param name="arg">The argument containing the object whose 'box' method you would like to call.
/// This is a SmileArg* (pointer), not a SmileArg (struct).</param>
/// <returns>The boxed value, as a SmileObject.</returns>
Inline SmileObject SmileArg_Box(SmileArg arg)
{
	if (arg.obj->kind < 0x10)
		return arg.obj->vtable->box(arg);
	else
		return arg.obj;
}

/// <summary>
/// Perform a virtual call to the given object's special 'unbox' method, resulting in an argument
/// that can be pushed onto a call stack.
///
/// Warning: The behavior of this function is UNDEFINED if the object refers to the special instance
///          of an already-unboxed object.  Do not call this if the object is already unboxed.
/// </summary>
/// <param name="obj">The object whose 'unbox' method you would like to call.</param>
/// <returns>The possibly-unboxed value, as a SmileArg.</returns>
Inline SmileArg SmileArg_Unbox(SmileObject obj)
{
	if (obj->kind < 0x20)
		return obj->vtable->unbox(obj);
	else {
		SmileArg dest;
		dest.obj = obj;
		return dest;
	}
}

#endif
