#ifndef __SMILE_SMILETYPES_OBJECTKIND_H__
#define __SMILE_SMILETYPES_OBJECTKIND_H__

//-----------------------------------------------------------------------------
// Object kind enumeration.

typedef enum {
		
	// Special masks.	
	SMILE_KIND_MASK	= ((1 << 8) - 1),	// Bottom eight bits are the *actual* kind; any upper bits are flags.
	SMILE_KIND_LIST_BIT	= (1 << 0),	// Must be a power of two.
		
	// Unboxed types.	
	SMILE_KIND_UNBOXED_BYTE	= 0x00,
	SMILE_KIND_UNBOXED_INTEGER16	= 0x01,
	SMILE_KIND_UNBOXED_INTEGER32	= 0x02,
	SMILE_KIND_UNBOXED_INTEGER64	= 0x03,
	SMILE_KIND_UNBOXED_BOOL	= 0x04,
	SMILE_KIND_UNBOXED_FLOAT16	= 0x05,
	SMILE_KIND_UNBOXED_FLOAT32	= 0x06,
	SMILE_KIND_UNBOXED_FLOAT64	= 0x07,
	SMILE_KIND_UNBOXED_SYMBOL	= 0x08,
	SMILE_KIND_UNBOXED_REAL16	= 0x09,
	SMILE_KIND_UNBOXED_REAL32	= 0x0A,
	SMILE_KIND_UNBOXED_REAL64	= 0x0B,
		
	// Special intrinsic types.	
	SMILE_KIND_NULL	= 0x10,	// Must be a number with a zero in the SMILE_KIND_LIST_BIT position.
	SMILE_KIND_LIST	= (SMILE_KIND_NULL | SMILE_KIND_LIST_BIT),
	SMILE_KIND_PRIMITIVE	= 0x12,	// The type of the one-and-only primitive object at the root of the object hierarchy.
	SMILE_KIND_BOOL	= 0x13,
	SMILE_KIND_SYMBOL	= 0x14,
	SMILE_KIND_STRING	= 0x15,
		
	// Other aggregations of data.	
	SMILE_KIND_USEROBJECT	= 0x18,
	SMILE_KIND_PAIR	= 0x19,
		
	// Opaque handles.	
	SMILE_KIND_HANDLE	= 0x1A,
	SMILE_KIND_BYTE	= 0x1B,
	SMILE_KIND_CLOSURE	= 0x1C,
	SMILE_KIND_FACADE	= 0x1D,
	SMILE_KIND_MACRO	= 0x1E,
	SMILE_KIND_FUNCTION	= 0x1F,
		
	// Integer numeric types.	
	SMILE_KIND_INTEGER16	= 0x20,
	SMILE_KIND_INTEGER32	= 0x21,
	SMILE_KIND_INTEGER64	= 0x22,
	SMILE_KIND_INTEGER128	= 0x23,
		
	// Binary floating-point types.	
	SMILE_KIND_FLOAT16	= 0x24,
	SMILE_KIND_FLOAT32	= 0x25,
	SMILE_KIND_FLOAT64	= 0x26,
	SMILE_KIND_FLOAT128	= 0x27,
		
	// Decimal floating-point types.	
	SMILE_KIND_REAL16	= 0x28,
	SMILE_KIND_REAL32	= 0x29,
	SMILE_KIND_REAL64	= 0x2A,
	SMILE_KIND_REAL128	= 0x2B,
		
	// Other numeric types.	
	SMILE_KIND_BIGINT	= 0x2C,
	SMILE_KIND_BIGFLOAT	= 0x2D,
	SMILE_KIND_BIGREAL	= 0x2E,
		
	// Types used for parsing.	
	SMILE_KIND_SYNTAX	= 0xF0,
	SMILE_KIND_NONTERMINAL	= 0xF1,
		
	// Internal types used during parsing.	
	SMILE_KIND_PARSEDECL	= 0xFE,
	SMILE_KIND_PARSEMESSAGE	= 0xFF,
		
	//----------------------	
	// Object security bits.	
		
	SMILE_SECURITY_READONLY	= 0,
	SMILE_SECURITY_WRITABLE	= (1 << 8),
	SMILE_SECURITY_APPENDABLE	= (1 << 9),
	SMILE_SECURITY_READWRITEAPPEND	= (SMILE_SECURITY_WRITABLE | SMILE_SECURITY_APPENDABLE),
		
	SMILE_SECURITY_HASKEY	= (1 << 10),
		
	//----------------------	
	// Miscellaneous flags.	
		
	SMILE_FLAG_WITHSOURCE	= (1 << 11),
	SMILE_FLAG_EXTERNAL_FUNCTION	= (1 << 12),

} SmileKind;

#endif
