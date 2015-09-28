#ifndef __SMILE_SMILETYPES_OBJECTKIND_H__
#define __SMILE_SMILETYPES_OBJECTKIND_H__

//-----------------------------------------------------------------------------
// Object kind enumeration.

// Special intrinsic types.
#define SMILE_KIND_NULL					0x00		// Must be 0x00 or stuff will break!
#define SMILE_KIND_LIST					0x01		// Must be 0x01 or stuff will break!
#define SMILE_KIND_BOOL					0x02
#define SMILE_KIND_SYMBOL				0x03
#define SMILE_KIND_CHAR					0x04
#define SMILE_KIND_UCHAR				0x05
#define SMILE_KIND_STRING				0x06
#define SMILE_KIND_OBJECT				0x07

// Other aggregations of data.
#define SMILE_KIND_USEROBJECT			0x08
#define SMILE_KIND_PAIR					0x09
#define SMILE_KIND_WEAKREF				0x0A		// Not yet supported.

// Opaque handles.
#define SMILE_KIND_HANDLE				0x0B
#define SMILE_KIND_CLOSURE				0x0C
#define SMILE_KIND_FACADE				0x0D
#define SMILE_KIND_MACRO				0x0E		// Not yet supported.
#define SMILE_KIND_FUNCTION				0x0F

// Integer numeric types.
#define SMILE_KIND_INTEGER16			0x10
#define SMILE_KIND_INTEGER32			0x11
#define SMILE_KIND_INTEGER64			0x12
#define SMILE_KIND_INTEGER128			0x13		// Not yet supported.

// Binary floating-point types.
#define SMILE_KIND_FLOAT16				0x14		// Not yet supported.
#define SMILE_KIND_FLOAT32				0x15
#define SMILE_KIND_FLOAT64				0x16
#define SMILE_KIND_FLOAT128				0x17		// Not yet supported.

// Decimal floating-point types.
#define SMILE_KIND_REAL16				0x18		// Not yet supported.
#define SMILE_KIND_REAL32				0x19
#define SMILE_KIND_REAL64				0x1A
#define SMILE_KIND_REAL128				0x1B		// Not yet supported.

// Other numeric types.
#define SMILE_KIND_BIGINT				0x1C		// Not yet supported.
#define SMILE_KIND_BIGFLOAT				0x1D		// Not yet supported.
#define SMILE_KIND_BIGREAL				0x1E		// Not yet supported.
#define SMILE_KIND_BYTE					0x1F

// Custom aggregate forms of the atomic types.
#define SMILE_KIND_ATOMIC				0x00
#define SMILE_KIND_ARRAYOF				0x20
#define SMILE_KIND_RANGEOF				0x40
#define SMILE_KIND_MAPOF				0x60

//-----------------------------------------------------------------------------
// Object security bits.

#define SMILE_SECURITY_READONLY			0
#define SMILE_SECURITY_WRITABLE			(1 << 8)
#define SMILE_SECURITY_APPENDABLE		(1 << 9)
#define SMILE_SECURITY_READWRITEAPPEND	(SMILE_SECURITY_WRITABLE | SMILE_SECURITY_APPENDABLE)

#define SMILE_SECURITY_HASKEY			(1 << 10)

//-----------------------------------------------------------------------------
// Miscellaneous flags.

#define SMILE_FLAG_LISTWITHSOURCE		(1 << 11)

#endif
