#ifndef __SMILE_SMILETYPES_OBJECTKIND_H__
#define __SMILE_SMILETYPES_OBJECTKIND_H__

//-----------------------------------------------------------------------------
// Object kind enumeration.

typedef enum {
		
	// Special masks.	
	SMILE_KIND_MASK					= ((1 << 8) - 1),	// Bottom eight bits are the *actual* kind; any upper bits are flags.
	SMILE_KIND_LIST_BIT				= (1 << 0),			// Must be a power of two.

	SMILE_KIND_UNBOXED_MIN			= 0x00,
	SMILE_KIND_UNBOXED_MAX			= 0x0F,
	SMILE_KIND_UNBOXABLE_MIN		= 0x10,
	SMILE_KIND_UNBOXABLE_MAX		= 0x1F,
	SMILE_KIND_SPECIAL_AGGREGATE_MIN = 0x40,
	SMILE_KIND_SPECIAL_AGGREGATE_MAX = 0x7F,

	// Unboxed types.	
	SMILE_KIND_UNBOXED_BYTE			= 0x00,
	SMILE_KIND_UNBOXED_INTEGER16	= 0x01,
	SMILE_KIND_UNBOXED_INTEGER32	= 0x02,
	SMILE_KIND_UNBOXED_INTEGER64	= 0x03,
	SMILE_KIND_UNBOXED_BOOL			= 0x04,
	SMILE_KIND_UNBOXED_FLOAT32		= 0x06,
	SMILE_KIND_UNBOXED_FLOAT64		= 0x07,
	SMILE_KIND_UNBOXED_SYMBOL		= 0x08,
	SMILE_KIND_UNBOXED_REAL32		= 0x0A,
	SMILE_KIND_UNBOXED_REAL64		= 0x0B,
	SMILE_KIND_UNBOXED_CHAR			= 0x0C,
	SMILE_KIND_UNBOXED_UNI			= 0x0D,

	// Boxed versions of unboxable types.	
	SMILE_KIND_BYTE					= 0x10,
	SMILE_KIND_INTEGER16			= 0x11,
	SMILE_KIND_INTEGER32			= 0x12,
	SMILE_KIND_INTEGER64			= 0x13,
	SMILE_KIND_BOOL					= 0x14,
	SMILE_KIND_FLOAT32				= 0x16,
	SMILE_KIND_FLOAT64				= 0x17,
	SMILE_KIND_SYMBOL				= 0x18,
	SMILE_KIND_REAL32				= 0x1A,
	SMILE_KIND_REAL64				= 0x1B,
	SMILE_KIND_CHAR					= 0x1C,
	SMILE_KIND_UNI					= 0x1D,
		
	// The special aggregate types.	
	SMILE_KIND_NULL					= 0x20,		// Must be a number with a zero in the SMILE_KIND_LIST_BIT position.
	SMILE_KIND_LIST					= (SMILE_KIND_NULL | SMILE_KIND_LIST_BIT),
	SMILE_KIND_PRIMITIVE			= 0x22,		// The type of the one-and-only primitive object at the root of the object hierarchy.
	SMILE_KIND_USEROBJECT			= 0x24,
	SMILE_KIND_STRING				= 0x25,
		
	// Opaque handles.	
	SMILE_KIND_TILL_CONTINUATION	= 0x2A,
	SMILE_KIND_HANDLE				= 0x2B,
	SMILE_KIND_CLOSURE				= 0x2C,
	SMILE_KIND_FACADE				= 0x2D,
	SMILE_KIND_MACRO				= 0x2E,
	SMILE_KIND_FUNCTION				= 0x2F,
		
	// Bigger numeric types.	
	SMILE_KIND_INTEGER128			= 0x30,
	SMILE_KIND_BIGINT				= 0x31,
	SMILE_KIND_FLOAT128				= 0x34,
	SMILE_KIND_BIGFLOAT				= 0x35,
	SMILE_KIND_REAL128				= 0x38,
	SMILE_KIND_BIGREAL				= 0x39,
	SMILE_KIND_TIMESTAMP			= 0x3F,

	// Range versions of numeric types.	
	SMILE_KIND_BYTERANGE			= 0x40,
	SMILE_KIND_INTEGER16RANGE		= 0x41,
	SMILE_KIND_INTEGER32RANGE		= 0x42,
	SMILE_KIND_INTEGER64RANGE		= 0x43,
	SMILE_KIND_FLOAT32RANGE			= 0x46,
	SMILE_KIND_FLOAT64RANGE			= 0x47,
	SMILE_KIND_REAL32RANGE			= 0x4A,
	SMILE_KIND_REAL64RANGE			= 0x4B,
	SMILE_KIND_CHARRANGE			= 0x4C,
	SMILE_KIND_UNIRANGE				= 0x4D,

	// Raw buffer types.
	SMILE_KIND_BYTEARRAY			= 0x50,
		
	// Types used for parsing.	
	SMILE_KIND_SYNTAX				= 0xF0,
	SMILE_KIND_NONTERMINAL			= 0xF1,
	SMILE_KIND_LOANWORD				= 0xF2,

	// Internal types used during parsing.	
	SMILE_KIND_PARSEDECL			= 0xFE,
	SMILE_KIND_PARSEMESSAGE			= 0xFF,
		
	//----------------------	
	// Object security bits.	
		
	SMILE_SECURITY_READONLY			= 0,
	SMILE_SECURITY_WRITABLE			= (1 << 8),
	SMILE_SECURITY_APPENDABLE		= (1 << 9),
	SMILE_SECURITY_READWRITEAPPEND	= (SMILE_SECURITY_WRITABLE | SMILE_SECURITY_APPENDABLE),
		
	SMILE_SECURITY_HASKEY			= (1 << 10),
	SMILE_SECURITY_UNFROZEN			= (1 << 11),
		
	//----------------------	
	// Miscellaneous flags.	
		
	SMILE_FLAG_EXTERNAL_FUNCTION	= (1 << 12),

} SmileKind;

#endif
