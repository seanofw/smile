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
	Op_Pop1	= 0x05,	// -1	; Pop the top item off the work stack and discard it.
	Op_Pop2	= 0x06,	// -2	; Pop the top two items off the work stack and discard them.
	Op_Pop	= 0x07,	// -n | int32	; Pop the top N items off the work stack and discard them.
	Op_Brk	= 0x0F,	//  0	; Break into debugger immediately.
				
	Op_LdNull	= 0x10,	// +1	; Push null onto the work stack.
	Op_LdBool	= 0x11,	// +1 | bool	; Push the given boolean value onto the work stack.
	Op_LdCh	= 0x12,	// +1 | char	; Push the given char value onto the work stack.
	Op_LdUCh	= 0x13,	// +1 | uchar	; Push the given uchar value onto the work stack.
	Op_LdStr	= 0x14,	// +1 | int32	; Push the given string value onto the work stack (by string table index).
	Op_LdSym	= 0x15,	// +1 | int32	; Push the given symbol value onto the work stack.
	Op_LdObj	= 0x16,	// +1 | int32	; Push the given literal object onto the work stack (by object table index).
	Op_LdClos	= 0x17,	// +1	; Push the current-closure object onto the work stack.
				
	Op_Ld8	= 0x18,	// +1 | byte	; Push the given byte value onto the work stack.
	Op_Ld16	= 0x19,	// +1 | int16	; Push the given int16 value onto the work stack.
	Op_Ld32	= 0x1A,	// +1 | int32	; Push the given int32 value onto the work stack.
	Op_Ld64	= 0x1B,	// +1 | int64	; Push the given int64 value onto the work stack.
	Op_Ld128	= 0x1C,	// +1 | index	; Push the given int128 value (object index) onto the work stack.
				
	Op_LdR16	= 0x21,	// +1 | real16	; Push the given real16 value onto the work stack.
	Op_LdR32	= 0x22,	// +1 | real32	; Push the given real32 value onto the work stack.
	Op_LdR64	= 0x23,	// +1 | real64	; Push the given real64 value onto the work stack.
	Op_LdR128	= 0x24,	// +1 | int32	; Push the given real128 value onto the work stack (by real128 table).
				
	Op_LdF16	= 0x29,	// +1 | float16	; Push the given float16 value onto the work stack.
	Op_LdF32	= 0x2A,	// +1 | float32	; Push the given float32 value onto the work stack.
	Op_LdF64	= 0x2B,	// +1 | float64	; Push the given float64 value onto the work stack.
	Op_LdF128	= 0x2C,	// +1 | int32	; Push the given float128 value onto the work stack (by real128 table).
				
	Op_LdLoc	= 0x30,	// +1 | int32, int32	; Load the indexed local variable in the given relative-indexed scope onto the work stack.
	Op_StLoc	= 0x31,	//  0 | int32, int32	; Store the value of the stack top into the indexed local variable in the given relative-indexed scope.
	Op_LdArg	= 0x32,	// +1 | int32, int32	; Load the value of the given function's argument onto the work stack.  (function index, arg index)
	Op_StArg	= 0x33,	//  0 | int32, int32	; Store the value of the stack top into the given function's argument.  (function index, arg index)
	Op_LdX	= 0x34,	// +1 | int32	; Load the value of the given named variable (symbol) onto the work stack.
	Op_StX	= 0x35,	//  0 | int32	; Store the value of the stack top into the given named variable (symbol).
	Op_LdProp	= 0x38,	// -1, +1 | int32	; Retrieve the given property from the object on the stack top, or null if there is no such property.
	Op_StProp	= 0x39,	// -1, +1 | int32	; Store the stack top into the given property of the given object.  Results in the stack top value.
	Op_LdMember	= 0x3A,	// -2, +1	; Call 'get-member', passing member (top-1) and object (top-2).
	Op_StMember	= 0x3B,	// -2, +1	; Call 'set-member', passing value (top-1), member (top-2), and object (top-3).  Results in the stack top value.
				
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
				
	Op_Jmp	= 0x60,	//  0 | label	; Unconditional jump to the given label.
	Op_Bt	= 0x61,	// -1 | label	; Branch to the given label if the stack top is truthy.
	Op_Bf	= 0x62,	// -1 | label	; Branch to the given label if the stack top is falsy.
	Op_Met	= 0x64,	// -n, +1 | int32, int32	; Call the given method with 'n' arguments.  Target and arguments must all be on the stack.
	Op_Call	= 0x65,	// -(n+1), +1 | int32	; Call the given function with 'n' arguments.  Function and arguments must all be on the stack.
	Op_CallEsc	= 0x66,	// -(n+1), +1 | int32	; Call the given function with 'n' arguments as well as also passing an escape continuation.
	Op_CallTail	= 0x67,	// -(n+1), +1 | int32	; Jump to the given function with 'n' arguments as a tail-call, discarding the current scope.
	Op_LocalAlloc	= 0x6C,	//  0 | int32	; Allocate 'n' more local variables; construct any new local variables with null.
	Op_LocalFree	= 0x6D,	//  0 | int32	; Free 'n' unneeded local variables from the top of the local-variable stack.
	Op_Args	= 0x6E,	//  0 | int32	; Ensure current fn has 'n' arguments minimum; construct missing args with null.  Must be first instr of function.
	Op_Ret	= 0x6F,	//  0	; Return to caller, destroying the current function's dynamic scope.  Stack top must contain return value.
				
	Op_Call0	= 0x70,	// -1, +1	; Call the given function with 0 arguments.  Function must be on the stack.
	Op_Call1	= 0x71,	// -2, +1	; Call the given function with 1 argument.  Function and argument must be on the stack.
	Op_Call2	= 0x72,	// -3, +1	; Call the given function with 2 arguments.  Function and arguments must be on the stack.
	Op_Call3	= 0x73,	// -4, +1	; Call the given function with 3 arguments.  Function and arguments must be on the stack.
	Op_Call4	= 0x74,	// -5, +1	; Call the given function with 4 arguments.  Function and arguments must be on the stack.
	Op_Call5	= 0x75,	// -6, +1	; Call the given function with 5 arguments.  Function and arguments must be on the stack.
	Op_Call6	= 0x76,	// -7, +1	; Call the given function with 6 arguments.  Function and arguments must be on the stack.
	Op_Call7	= 0x77,	// -8, +1	; Call the given function with 7 arguments.  Function and arguments must be on the stack.
	Op_Met0	= 0x78,	// -1, +1 | int32	; Call the given named method with 0 arguments.  'This' object must be on the stack.
	Op_Unary	= 0x78,	// -1, +1 | int32	; Call the given named method with 0 arguments.  'This' object must be on the stack.
	Op_Met1	= 0x79,	// -2, +1 | int32	; Call the given named method with 1 argument.  'This' object and arguments must be on the stack.
	Op_Binary	= 0x79,	// -2, +1 | int32	; Call the given named method with 1 argument.  'This' object and arguments must be on the stack.
	Op_Met2	= 0x7A,	// -3, +1 | int32	; Call the given named method with 2 arguments.  'This' object and arguments must be on the stack.
	Op_Met3	= 0x7B,	// -4, +1 | int32	; Call the given named method with 3 arguments.  'This' object and arguments must be on the stack.
	Op_Met4	= 0x7C,	// -5, +1 | int32	; Call the given named method with 4 arguments.  'This' object and arguments must be on the stack.
	Op_Met5	= 0x7D,	// -6, +1 | int32	; Call the given named method with 5 arguments.  'This' object and arguments must be on the stack.
	Op_Met6	= 0x7E,	// -7, +1 | int32	; Call the given named method with 6 arguments.  'This' object and arguments must be on the stack.
	Op_Met7	= 0x7F,	// -8, +1 | int32	; Call the given named method with 7 arguments.  'This' object and arguments must be on the stack.
				
	Op_Begin	= 0x80,	//  0 | int32, int32	; Set up a new scope with 'n' locals and 'm' work-stack space, preserving a reference to the parent scope.
	Op_End	= 0x81,	//  0	; Revert to the parent scope.
	Op_Try	= 0x82,	//  0 | label, int32	; Set up a new exception scope, branching to 'label' function if an exception is raised, pushing exception
	Op_EndTry	= 0x83,	//  0	; Revert to the previous exception scope.
	Op_JmpEsc	= 0x84,	//  0 | label, int32	; Unconditional jump to the given label, creating a new scope with 'n' work-stack space and one local, an escape continuation.
	Op_Esc	= 0x85,	// -1 | label	; Unconditional jump to the given label, restoring state to the escape continuation on the stack top.
	Op_SuperEq	= 0x88,	// -2, +1	; Push true if a (top-2) and b (top-1) are identical references, false otherwise.
	Op_SuperNe	= 0x89,	// -2, +1	; Push false if a (top-2) and b (top-1) are identical references, true otherwise.
	Op_Not	= 0x8A,	// -1, +1	; Convert the stack top to boolean, and then logically invert it.
	Op_Bool	= 0x8B,	// -1, +1	; Convert the stack top to boolean.
	Op_Is	= 0x8C,	// -2, +1	; Push true if a (top-2) is derived from b (top-1); push false if not.
	Op_TypeOf	= 0x8D,	// -1, +1	; Push the formal type symbol corresponding to the type of the stack top.
				
	Op_Cons	= 0x90,	// -2, +1	; Create a new List object from the given a/d values on the work stack.
	Op_Car	= 0x91,	// -1, +1	; Retrieve the 'a' property from the List on the stack top.
	Op_Cdr	= 0x92,	// -1, +1	; Retrieve the 'd' property from the List on the stack top.
	Op_NewPair	= 0x94,	// -2, +1	; Create a new Pair object from the given left/right values on the work stack.
	Op_Left	= 0x95,	// -1, +1	; Retrieve the 'left' property from the Pair on the stack top.
	Op_Right	= 0x96,	// -1, +1	; Retrieve the 'right' property from the Pair on the stack top.
	Op_NewFn	= 0x98,	// +1 | int32	; Push a new function instance that comes from the given compiled function (by function table index).
	Op_NewObj	= 0x99,	// -(n*2+1), +1 | int32	; Create a new object from the 'n' property decls and base object on the work stack.
	Op_NewRange	= 0x9A,	// -2, +1	; Create a new Range object from the given start/end points on the work stack.
				
	Op_Label	= 0xFF,	//  0	; Branch target
};

SMILE_API_DATA String *Opcode_Names;

#endif
