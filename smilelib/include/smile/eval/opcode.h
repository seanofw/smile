#ifndef __SMILE_EVAL_OPCODE_H__
#define __SMILE_EVAL_OPCODE_H__

enum Opcode {

	// 0x: State management.
	// 1x: Core load instructions.
	// 2x: Float/real load instructions.
	// 3x: Basic load/store operations.
	// 4x: Branch/call instructions.
	// 5x: Scoping instructions, and type testing.
	// 6x: Shorthand load instructions.
	// 7x: Shorthand store instructions.
	// 8x: Shorthand call instructions.
	// 9x: Special-object instructions.
	// Ax-Ex: Reserved for future use.
	// Fx: Temporary instructions (for compiler internal use).

	Op_Nop	= 0x00,	//  0	; Do nothing
	Op_Dup1	= 0x01,	// +1	; Duplicate the top item on the work stack.
	Op_Dup2	= 0x02,	// +1	; Duplicate the item one below the top item on the work stack.
	Op_Dup	= 0x03,	// +1 | int32	; Duplicate the item N below the top item on the work stack.
	Op_04	= 0x04,		
	Op_Pop1	= 0x05,	// -1	; Pop the top item off the work stack and discard it.
	Op_Pop2	= 0x06,	// -2	; Pop the top two items off the work stack and discard them.
	Op_Pop	= 0x07,	// -n | int32	; Pop the top N items off the work stack and discard them.
	Op_08	= 0x08,		
	Op_Rep1	= 0x09,	// -1	; Pop the top 2 items, and then re-push what was previously the topmost item.
	Op_Rep2	= 0x0A,	// -2	; Pop the top 3 items, and then re-push what was previously the topmost item.
	Op_Rep	= 0x0B,	// -n | int32	; Pop the top N+1 items, and then re-push what was previously the topmost item.
	Op_0C	= 0x0C,		
	Op_0D	= 0x0D,		
	Op_0E	= 0x0E,		
	Op_Brk	= 0x0F,	//  0	; Break into debugger immediately.
				
	Op_LdNull	= 0x10,	// +1	; Push null onto the work stack.
	Op_LdBool	= 0x11,	// +1 | bool	; Push the given boolean value onto the work stack.
	Op_LdStr	= 0x12,	// +1 | int32	; Push the given string value onto the work stack (by string table index).
	Op_LdSym	= 0x13,	// +1 | int32	; Push the given symbol value onto the work stack.
	Op_LdObj	= 0x14,	// +1 | int32	; Push the given literal object onto the work stack (by object table index).
	Op_LdClos	= 0x15,	// +1	; Push the current-closure object onto the work stack.
	Op_16	= 0x16,		
	Op_17	= 0x17,		
	Op_Ld8	= 0x18,	// +1 | byte	; Push the given byte value onto the work stack.
	Op_Ld16	= 0x19,	// +1 | int16	; Push the given int16 value onto the work stack.
	Op_Ld32	= 0x1A,	// +1 | int32	; Push the given int32 value onto the work stack.
	Op_Ld64	= 0x1B,	// +1 | int64	; Push the given int64 value onto the work stack.
	Op_Ld128	= 0x1C,	// +1 | index	; Push the given int128 value (object index) onto the work stack.
	Op_1D	= 0x1D,		
	Op_1E	= 0x1E,		
	Op_1F	= 0x1F,		
				
	Op_20	= 0x20,		
	Op_LdR16	= 0x21,	// +1 | real16	; Push the given real16 value onto the work stack.
	Op_LdR32	= 0x22,	// +1 | real32	; Push the given real32 value onto the work stack.
	Op_LdR64	= 0x23,	// +1 | real64	; Push the given real64 value onto the work stack.
	Op_LdR128	= 0x24,	// +1 | int32	; Push the given real128 value onto the work stack (by real128 table).
	Op_25	= 0x25,		
	Op_26	= 0x26,		
	Op_27	= 0x27,		
	Op_28	= 0x28,		
	Op_LdF16	= 0x29,	// +1 | float16	; Push the given float16 value onto the work stack.
	Op_LdF32	= 0x2A,	// +1 | float32	; Push the given float32 value onto the work stack.
	Op_LdF64	= 0x2B,	// +1 | float64	; Push the given float64 value onto the work stack.
	Op_LdF128	= 0x2C,	// +1 | int32	; Push the given float128 value onto the work stack (by real128 table).
	Op_2D	= 0x2D,		
	Op_2E	= 0x2E,		
	Op_2F	= 0x2F,		
				
	Op_LdLoc	= 0x30,	// +1 | int32, int32	; Load the indexed local variable in the given relative-indexed scope onto the work stack.
	Op_StLoc	= 0x31,	//  0 | int32, int32	; Store the value of the stack top into the indexed local variable in the given relative-indexed scope.
	Op_StpLoc	= 0x32,	// -1 | int32, int32	; Store and pop the value of the stack top into the indexed local variable in the given relative-indexed scope.
	Op_33	= 0x33,		
	Op_LdArg	= 0x34,	// +1 | int32, int32	; Load the value of the given function's argument onto the work stack.  (function index, arg index)
	Op_StArg	= 0x35,	//  0 | int32, int32	; Store the value of the stack top into the given function's argument.  (function index, arg index)
	Op_StpArg	= 0x36,	// -1 | int32, int32	; Store and pop the value of the stack top into the given function's argument.  (function index, arg index)
	Op_37	= 0x37,		
	Op_LdX	= 0x38,	// +1 | int32	; Load the value of the given named variable (global) onto the work stack.
	Op_StX	= 0x39,	//  0 | int32	; Store the value of the stack top into the given named variable (global).
	Op_StpX	= 0x3A,	// -1 | int32	; Store and pop the value of the stack top into the given named variable (global).
	Op_3B	= 0x3B,		
	Op_3C	= 0x3C,		
	Op_3D	= 0x3D,		
	Op_3E	= 0x3E,		
	Op_3F	= 0x3F,		
				
	Op_LdArg0	= 0x40,	// +1 | int32	; Load the current function's argument (by index) onto the work stack.
	Op_LdArg1	= 0x41,	// +1 | int32	; Load the parent function's argument (by index) onto the work stack.
	Op_LdArg2	= 0x42,	// +1 | int32	; Load the parent-parent function's argument (by index) onto the work stack.
	Op_LdArg3	= 0x43,	// +1 | int32	; Load the parent-parent-parent function's argument (by index) onto the work stack.
	Op_LdArg4	= 0x44,	// +1 | int32	; etc.
	Op_LdArg5	= 0x45,	// +1 | int32	; etc.
	Op_LdArg6	= 0x46,	// +1 | int32	; etc.
	Op_LdArg7	= 0x47,	// +1 | int32	; etc.
	Op_LdLoc0	= 0x48,	// +1 | int32	; Load the indexed local variable in the current function onto the work stack.
	Op_LdLoc1	= 0x49,	// +1 | int32	; Load the indexed local variable in the parent function onto the work stack.
	Op_LdLoc2	= 0x4A,	// +1 | int32	; Load the indexed local variable in the parent-parent function onto the work stack.
	Op_LdLoc3	= 0x4B,	// +1 | int32	; Load the indexed local variable in the parent-parent-parent function onto the work stack.
	Op_LdLoc4	= 0x4C,	// +1 | int32	; etc.
	Op_LdLoc5	= 0x4D,	// +1 | int32	; etc.
	Op_LdLoc6	= 0x4E,	// +1 | int32	; etc.
	Op_LdLoc7	= 0x4F,	// +1 | int32	; etc.
				
	Op_StArg0	= 0x50,	//  0 | int32	; Store the value of the stack top into the current function's argument (by index).
	Op_StArg1	= 0x51,	//  0 | int32	; Store the value of the stack top into the parent function's argument (by index).
	Op_StArg2	= 0x52,	//  0 | int32	; Store the value of the stack top into the parent-parent function's argument (by index).
	Op_StArg3	= 0x53,	//  0 | int32	; Store the value of the stack top into the parent-parent-parent function's argument (by index).
	Op_StArg4	= 0x54,	//  0 | int32	; etc.
	Op_StArg5	= 0x55,	//  0 | int32	; etc.
	Op_StArg6	= 0x56,	//  0 | int32	; etc.
	Op_StArg7	= 0x57,	//  0 | int32	; etc.
	Op_StLoc0	= 0x58,	//  0 | int32	; Store the value of the stack top into the named variable (symbol) in the current function.
	Op_StLoc1	= 0x59,	//  0 | int32	; Store the value of the stack top into the named variable (symbol) in the parent function.
	Op_StLoc2	= 0x5A,	//  0 | int32	; Store the value of the stack top into the named variable (symbol) in the parent-parent function.
	Op_StLoc3	= 0x5B,	//  0 | int32	; Store the value of the stack top into the named variable (symbol) in the parent-parent-parent function.
	Op_StLoc4	= 0x5C,	//  0 | int32	; etc.
	Op_StLoc5	= 0x5D,	//  0 | int32	; etc.
	Op_StLoc6	= 0x5E,	//  0 | int32	; etc.
	Op_StLoc7	= 0x5F,	//  0 | int32	; etc.
				
	Op_StpArg0	= 0x60,	//  -1 | int32	; Store and pop the value of the stack top into the current function's argument (by index).
	Op_StpArg1	= 0x61,	//  -1 | int32	; Store and pop the value of the stack top into the parent function's argument (by index).
	Op_StpArg2	= 0x62,	//  -1 | int32	; Store and pop the value of the stack top into the parent-parent function's argument (by index).
	Op_StpArg3	= 0x63,	//  -1 | int32	; Store and pop the value of the stack top into the parent-parent-parent function's argument (by index).
	Op_StpArg4	= 0x64,	//  -1 | int32	; etc.
	Op_StpArg5	= 0x65,	//  -1 | int32	; etc.
	Op_StpArg6	= 0x66,	//  -1 | int32	; etc.
	Op_StpArg7	= 0x67,	//  -1 | int32	; etc.
	Op_StpLoc0	= 0x68,	//  -1 | int32	; Store and pop the value of the stack top into the named variable (symbol) in the current function.
	Op_StpLoc1	= 0x69,	//  -1 | int32	; Store and pop the value of the stack top into the named variable (symbol) in the parent function.
	Op_StpLoc2	= 0x6A,	//  -1 | int32	; Store and pop the value of the stack top into the named variable (symbol) in the parent-parent function.
	Op_StpLoc3	= 0x6B,	//  -1 | int32	; Store and pop the value of the stack top into the named variable (symbol) in the parent-parent-parent function.
	Op_StpLoc4	= 0x6C,	//  -1 | int32	; etc.
	Op_StpLoc5	= 0x6D,	//  -1 | int32	; etc.
	Op_StpLoc6	= 0x6E,	//  -1 | int32	; etc.
	Op_StpLoc7	= 0x6F,	//  -1 | int32	; etc.
				
	Op_LdProp	= 0x70,	// -1, +1 | int32	; Retrieve the given property from the object on the stack top, or null if there is no such property.
	Op_StProp	= 0x71,	// -1, +1 | int32	; Store the stack top into the given property of the given object.  Results in the stack top value.
	Op_StpProp	= 0x72,	// -2 | int32	; Store and pop the stack top into the given property of the given object.
	Op_73	= 0x73,		
	Op_LdMember	= 0x74,	// -2, +1	; Call 'get-member', passing member (top-1) and object (top-2).
	Op_StMember	= 0x75,	// -2, +1	; Call 'set-member', passing value (top-1), member (top-2), and object (top-3).  Results in the stack top value.
	Op_StpMember	= 0x76,	// -3	; Call 'set-member', passing value (top-1), member (top-2), and object (top-3).  Pops the stack top value.
	Op_77	= 0x77,		
	Op_78	= 0x78,		
	Op_79	= 0x79,		
	Op_7A	= 0x7A,		
	Op_7B	= 0x7B,		
	Op_7C	= 0x7C,		
	Op_7D	= 0x7D,		
	Op_7E	= 0x7E,		
	Op_7F	= 0x7F,		
				
	Op_Cons	= 0x80,	// -2, +1	; Create a new List object from the given a/d values on the work stack.
	Op_Car	= 0x81,	// -1, +1	; Retrieve the 'a' property from the List on the stack top (UNDEFINED if not a List or Null).
	Op_Cdr	= 0x82,	// -1, +1	; Retrieve the 'd' property from the List on the stack top (UNDEFINED if not a List or Null).
	Op_83	= 0x83,		
	Op_NewPair	= 0x84,	// -2, +1	; Create a new Pair object from the given left/right values on the work stack.
	Op_Left	= 0x85,	// -1, +1	; Retrieve the 'left' property from the Pair on the stack top (UNDEFINED if not a Pair).
	Op_Right	= 0x86,	// -1, +1	; Retrieve the 'right' property from the Pair on the stack top (UNDEFINED if not a Pair).
	Op_87	= 0x87,		
	Op_NewFn	= 0x88,	// +1 | int32	; Push a new function instance that comes from the given compiled function (by function table index).
	Op_NewObj	= 0x89,	// -(n*2+1), +1 | int32	; Create a new object from the 'n' property decls and base object on the work stack.
	Op_8A	= 0x8A,		
	Op_SuperEq	= 0x8B,	// -2, +1	; Push true if a (top-2) and b (top-1) are identical references, false otherwise.
	Op_SuperNe	= 0x8C,	// -2, +1	; Push false if a (top-2) and b (top-1) are identical references, true otherwise.
	Op_Not	= 0x8D,	// -1, +1	; Convert the stack top to boolean, and then logically invert it.
	Op_Is	= 0x8E,	// -2, +1	; Push true if a (top-2) is derived from b (top-1); push false if not.
	Op_TypeOf	= 0x8F,	// -1, +1	; Push the formal type symbol corresponding to the type of the stack top.
				
	Op_Call0	= 0x90,	// -1, +1	; Call the given function with 0 arguments.  Function must be on the stack.
	Op_Call1	= 0x91,	// -2, +1	; Call the given function with 1 argument.  Function and argument must be on the stack.
	Op_Call2	= 0x92,	// -3, +1	; Call the given function with 2 arguments.  Function and arguments must be on the stack.
	Op_Call3	= 0x93,	// -4, +1	; Call the given function with 3 arguments.  Function and arguments must be on the stack.
	Op_Call4	= 0x94,	// -5, +1	; Call the given function with 4 arguments.  Function and arguments must be on the stack.
	Op_Call5	= 0x95,	// -6, +1	; Call the given function with 5 arguments.  Function and arguments must be on the stack.
	Op_Call6	= 0x96,	// -7, +1	; Call the given function with 6 arguments.  Function and arguments must be on the stack.
	Op_Call7	= 0x97,	// -8, +1	; Call the given function with 7 arguments.  Function and arguments must be on the stack.
	Op_Met0	= 0x98,	// -1, +1 | int32	; Call the given named method with 0 arguments.  'This' object must be on the stack.
	Op_Met1	= 0x99,	// -2, +1 | int32	; Call the given named method with 1 argument.  'This' object and arguments must be on the stack.
	Op_Met2	= 0x9A,	// -3, +1 | int32	; Call the given named method with 2 arguments.  'This' object and arguments must be on the stack.
	Op_Met3	= 0x9B,	// -4, +1 | int32	; Call the given named method with 3 arguments.  'This' object and arguments must be on the stack.
	Op_Met4	= 0x9C,	// -5, +1 | int32	; Call the given named method with 4 arguments.  'This' object and arguments must be on the stack.
	Op_Met5	= 0x9D,	// -6, +1 | int32	; Call the given named method with 5 arguments.  'This' object and arguments must be on the stack.
	Op_Met6	= 0x9E,	// -7, +1 | int32	; Call the given named method with 6 arguments.  'This' object and arguments must be on the stack.
	Op_Met7	= 0x9F,	// -8, +1 | int32	; Call the given named method with 7 arguments.  'This' object and arguments must be on the stack.
				
	Op_TCall0	= 0xA0,	// -1, +1	; Jump to the given function with 0 arguments, as a tail-call.  Function must be on the stack.
	Op_TCall1	= 0xA1,	// -2, +1	; Jump to the given function with 1 argument, as a tail-call.  Function and argument must be on the stack.
	Op_TCall2	= 0xA2,	// -3, +1	; Jump to the given function with 2 arguments, as a tail-call.  Function and arguments must be on the stack.
	Op_TCall3	= 0xA3,	// -4, +1	; Jump to the given function with 3 arguments, as a tail-call.  Function and arguments must be on the stack.
	Op_TCall4	= 0xA4,	// -5, +1	; Jump to the given function with 4 arguments, as a tail-call.  Function and arguments must be on the stack.
	Op_TCall5	= 0xA5,	// -6, +1	; Jump to the given function with 5 arguments, as a tail-call.  Function and arguments must be on the stack.
	Op_TCall6	= 0xA6,	// -7, +1	; Jump to the given function with 6 arguments, as a tail-call.  Function and arguments must be on the stack.
	Op_TCall7	= 0xA7,	// -8, +1	; Jump to the given function with 7 arguments, as a tail-call.  Function and arguments must be on the stack.
	Op_TMet0	= 0xA8,	// -1, +1 | int32	; Jump to the given named method with 0 arguments, as a tail-call.  'This' object must be on the stack.
	Op_TMet1	= 0xA9,	// -2, +1 | int32	; Jump to the given named method with 1 argument, as a tail-call.  'This' object and arguments must be on the stack.
	Op_TMet2	= 0xAA,	// -3, +1 | int32	; Jump to the given named method with 2 arguments, as a tail-call.  'This' object and arguments must be on the stack.
	Op_TMet3	= 0xAB,	// -4, +1 | int32	; Jump to the given named method with 3 arguments, as a tail-call.  'This' object and arguments must be on the stack.
	Op_TMet4	= 0xAC,	// -5, +1 | int32	; Jump to the given named method with 4 arguments, as a tail-call.  'This' object and arguments must be on the stack.
	Op_TMet5	= 0xAD,	// -6, +1 | int32	; Jump to the given named method with 5 arguments, as a tail-call.  'This' object and arguments must be on the stack.
	Op_TMet6	= 0xAE,	// -7, +1 | int32	; Jump to the given named method with 6 arguments, as a tail-call.  'This' object and arguments must be on the stack.
	Op_TMet7	= 0xAF,	// -8, +1 | int32	; Jump to the given named method with 7 arguments, as a tail-call.  'This' object and arguments must be on the stack.
				
	Op_Jmp	= 0xB0,	//  0 | label	; Unconditional jump to the given label.
	Op_Bt	= 0xB1,	// -1 | label	; Branch to the given label if the stack top is truthy.
	Op_Bf	= 0xB2,	// -1 | label	; Branch to the given label if the stack top is falsy.
	Op_B3	= 0xB3,		
	Op_Met	= 0xB4,	// -n, +1 | int32, int32	; Call the given method with 'n' arguments.  Target and arguments must all be on the stack.
	Op_TMet	= 0xB5,	// -n, +1 | int32, int32	; Jump to the given method with 'n' arguments, as a tail-call.  Target and arguments must all be on the stack.
	Op_Call	= 0xB6,	// -(n+1), +1 | int32	; Call the given function with 'n' arguments.  Function and arguments must all be on the stack.
	Op_TCall	= 0xB7,	// -(n+1), +1 | int32	; Jump to the given function with 'n' arguments as a tail-call, discarding the current scope.
	Op_NewTill	= 0xB8,	// +1 | int32	; Load the given 'till' escape-continuation branch-table object.
	Op_EndTill	= 0xB9,	// -1	; Destroy (mark as unusable) the 'till' escape-continuation on the stack top.
	Op_TillEsc	= 0xBA,	// -1 | int32	; Invoke the escape continuation on the stack, escaping to the given indexed when clause.
	Op_Try	= 0xBC,	//  0 | label, int32	; Set up a new exception scope, branching to 'label' function if an exception is raised, pushing exception
	Op_EndTry	= 0xBD,	//  0	; Revert to the previous exception scope.
	Op_BE	= 0xBE,		
	Op_Ret	= 0xBF,	//  0	; Return to caller, destroying the current function's dynamic scope.  Stack top must contain return value.
				
	Op_Add	= 0xC0,	// -2, +1	; Invoke any binary '+' operator on the object on the stack top.
	Op_Sub	= 0xC1,	// -2, +1	; Invoke any binary '-' operator on the object on the stack top.
	Op_Mul	= 0xC2,	// -2, +1	; Invoke any binary '*' operator on the object on the stack top.
	Op_Div	= 0xC3,	// -2, +1	; Invoke any binary '/' operator on the object on the stack top.
	Op_Mod	= 0xC4,	// -2, +1	; Invoke any binary 'mod' operator on the object on the stack top.
	Op_Rem	= 0xC5,	// -2, +1	; Invoke any binary 'rem' operator on the object on the stack top.
	Op_C6	= 0xC6,		
	Op_RangeTo	= 0xC7,	// -2, +1	; Invoke any binary 'range-to' operator on the object on the stack top.
	Op_Eq	= 0xC8,	// -2, +1	; Invoke any binary '==' operator on the object on the stack top.
	Op_Ne	= 0xC9,	// -2, +1	; Invoke any binary '!=' operator on the object on the stack top.
	Op_Lt	= 0xCA,	// -2, +1	; Invoke any binary '<' operator on the object on the stack top.
	Op_Gt	= 0xCB,	// -2, +1	; Invoke any binary '>' operator on the object on the stack top.
	Op_Le	= 0xCC,	// -2, +1	; Invoke any binary '<=' operator on the object on the stack top.
	Op_Ge	= 0xCD,	// -2, +1	; Invoke any binary '>=' operator on the object on the stack top.
	Op_Cmp	= 0xCE,	// -2, +1	; Invoke any binary 'cmp' operator on the object on the stack top.
	Op_Compare	= 0xCF,	// -2, +1	; Invoke any binary 'compare' operator on the object on the stack top.
				
	Op_Each	= 0xD0,	// -2, +1	; Invoke any binary 'each' operator on the object on the stack top.
	Op_Map	= 0xD1,	// -2, +1	; Invoke any binary 'map' operator on the object on the stack top.
	Op_Where	= 0xD2,	// -2, +1	; Invoke any binary 'where' operator on the object on the stack top.
	Op_D3	= 0xD3,		
	Op_Count	= 0xD4,	// -2, +1	; Invoke any binary 'count' operator on the object on the stack top.
	Op_Any	= 0xD5,	// -2, +1	; Invoke any binary 'any?' operator on the object on the stack top.
	Op_Join	= 0xD6,	// -2, +1	; Invoke any binary 'join' operator on the object on the stack top.
	Op_D7	= 0xD7,		
	Op_UCount	= 0xD8,	// -1, +1	; Invoke any unary 'count' operator on the object on the stack top.
	Op_UAny	= 0xD9,	// -1, +1	; Invoke any unary 'any?' operator on the object on the stack top.				
	Op_UJoin	= 0xDA,	// -1, +1	; Invoke any unary 'join' operator on the object on the stack top.
	Op_Neg	= 0xDB,	// -1, +1	; Invoke any unary '-' operator on the object on the stack top.
	Op_Bool	= 0xDC,	// -1, +1	; Invoke any unary 'bool' operator on the object on the stack top.	
	Op_Int	= 0xDD,	// -1, +1	; Invoke any unary 'int' operator on the object on the stack top.	
	Op_String	= 0xDE,	// -1, +1	; Invoke any unary 'string' operator on the object on the stack top.	
	Op_Hash	= 0xDF,	// -1, +1	; Invoke any unary 'hash' operator on the object on the stack top.	
				
	Op_NullQ	= 0xE0,	// -1, +1	; Invoke any unary 'null?' operator on the object on the stack top.
	Op_ListQ	= 0xE1,	// -1, +1	; Invoke any unary 'list?' operator on the object on the stack top.
	Op_PairQ	= 0xE2,	// -1, +1	; Invoke any unary 'pair?' operator on the object on the stack top.
	Op_FnQ	= 0xE3,	// -1, +1	; Invoke any unary 'fn?' operator on the object on the stack top.
	Op_BoolQ	= 0xE4,	// -1, +1	; Invoke any unary 'bool?' operator on the object on the stack top.
	Op_IntQ	= 0xE5,	// -1, +1	; Invoke any unary 'int?' operator on the object on the stack top.
	Op_StringQ	= 0xE6,	// -1, +1	; Invoke any unary 'string?' operator on the object on the stack top.
	Op_SymbolQ	= 0xE7,	// -1, +1	; Invoke any unary 'symbol?' operator on the object on the stack top.
	Op_LdA	= 0xE8,	// -1, +1	; Retrieve the 'a' property from the object on the stack top.
	Op_LdD	= 0xE9,	// -1, +1	; Retrieve the 'd' property from the object on the stack top.
	Op_LdLeft	= 0xEA,	// -1, +1	; Retrieve the 'left' property from the object on the stack top.
	Op_LdRight	= 0xEB,	// -1, +1	; Retrieve the 'right' property from the object on the stack top.
	Op_LdStart	= 0xEC,	// -1, +1	; Retrieve the 'start' property from the object on the stack top.
	Op_LdEnd	= 0xED,	// -1, +1	; Retrieve the 'end' property from the object on the stack top.
	Op_LdCount	= 0xEE,	// -1, +1	; Retrieve the 'count' property from the object on the stack top.
	Op_LdLength	= 0xEF,	// -1, +1	; Retrieve the 'length' property from the object on the stack top.
				
	Op_StateMachStart	= 0xF0,	//  0	; Special start-the-state-machine instruction.
	Op_StateMachBody	= 0xF1,	//  0	; Special repeatedly-invoke-the-state-machine instruction.
	Op_F2	= 0xF2,		
	Op_F3	= 0xF3,		
	Op_F4	= 0xF4,		
	Op_F5	= 0xF5,		
	Op_F6	= 0xF6,		
	Op_F7	= 0xF7,		
	Op_Pseudo	= 0xF8,	//  0	; First pseudo-op
	Op_F9	= 0xF9,		
	Op_FA	= 0xFA,		
	Op_FB	= 0xFB,		
	Op_FC	= 0xFC,		
	Op_EndBlock	= 0xFD,	//  0	; End of child basic block
	Op_Label	= 0xFE,	//  0	; Branch target		
	Op_Block	= 0xFF,	//  0	; Child basic block during intermediate-code generation
};

SMILE_API_DATA String *Opcode_Names;

#endif
