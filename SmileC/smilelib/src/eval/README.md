# Smile Eval

This folder contains the implementation of Smile's `eval()` function, the core "interpreter" itself.

The interpreter is composed of two major pieces:  There is a bytecode compiler, which transforms S-expressions into `ByteCode`, and an evaluator, which invokes the `ByteCode` instructions to run the program.

`eval` is a complicated beast, so this file attempts to provide something of a roadmap to its overall design.

-----------------

## The ByteCode Compiler

The compiler can be found in the  `compiler/` folder.  It recursively processes S-expressions, transforming them into an optimized byte-code representation.  (The list of optimizations is short, but growing.)  Because source code can be directly written as S-expressions, not merely emitted from the syntax-transforming part of the parser, the compiler also verifies that the S-expressions are correctly-structured (which may duplicate efforts of the parser, if the code really _did_ come from the syntax-transforming part of it).

The output of the compiler is a `CompiledTables` object, which contains collections of each of the artifacts of the compile:

- An array of constant `String` instances found in the source code.  The compiler compacts (unifies) identical strings as well.
- An array of `UserFunctionInfo` objects.  Each of these represents a single `[$fn]` declaration somewhere in the source code.  These are _not_ runtime function instances; they are metadata descriptions of how to correctly instantiate each function in the fastest way possible.
- An array of constant objects (`SmileObject` objects).  These are the results of any invocations of `[$quote]` that are not quoting symbols (which are stored as atoms in the global `SymbolTable`).
- An array of `TillContinuationInfo` objects.  Each of these represents a single `[$till]` declaration somewhere in the source code.  These are _not_ runtime till-continuation instances; they are metadata descriptions of how to correctly instantiate a till-continuation so that when a `[$till]` symbol is invoked, the correct escape may be made to the `[$till]`'s appropriate `when`-clause, no matter how deeply the user's code has recursed.
- An array of `CompiledSourceLocation` objects.  These are used for debugging, to connect `ByteCode` structures to the source files and lines that generated them (if possible; dynamic invocations of `eval` do not track source locations).
- Last, but not least, it contains a reference to the global `UserFunctionInfo` and `ClosureInfo` objects, the ones that form the root of the program.

Each of the `UserFunctionInfo` objects contains a `ClosureInfo`, which details about how large its closure must be, what its variable names were, and, critically, an actual `ByteCodeSegment` that contains the bytecode for that function.  Note that this means that there is not a single large chunk of bytecode in memory after compiling; each function carries with it an array of the bytecodes that represent it.

In the process of compiling, the compiler may emit error messages if the S-expressions are improperly formed.  These error messages are stored as `ParseMessage` objects in a standard `SmileList` attached to the `Compiler` instance itself.

Assuming there are no compiler errors, the `CompiledTables` object is what is then fed to `eval`.

-------------------

## Eval

Eval is responsible for actually running the bytecode representation of the program.

Eval is designed to run as quickly as is possible.  The majority of work is done in a single, large function, `Eval_RunCore()`, found in `eval.c`.  The code avoids heap instances where possible, preferring to use global variables and local registers for everything, and copying values back and forth as necessary.  The code makes very heavy use of C macros to optimize and inline everything it does, which can make it difficult to read, so this text is intended to help serve as a guide through it.

### Eval_Run()

Bytecode evaluation starts at `Eval_Run()`, which is passed a `CompiledTables` object and the `UserFunctionInfo` for the function to begin executing.  (This "root" function must take no arguments.)  `Eval_Run()` sets up a global closure for the _true_ global variables (like `String`), then a local closure for the global function itself, and then invokes `Eval_Continue()`.

### Eval_Continue()

The `Eval_Continue()` function does _not_ directly alter state; it takes the evaluation state that was passed into it and begins execution there.

There is a good reason for the split between `Eval_Run()` and `Eval_Continue()`:  The first time the code is executed, `Eval_Run()` is invoked.  If the code reaches a breakpoint, `Eval_Run()` will return.  To resume execution after the breakpoint (or anywhere else in the same function), you can call `Eval_Continue()`.

Internally, `Eval_Continue()` does little more than to call C `setjmp()` to set up a topmost exception handler, and then invokes `Eval_RunCore()`.  (It does a little additional work to handle the results of whatever `Eval_RunCore()` returns, or handle when an exception gets thrown, but really, its job is mostly about ensuring that there is a topmost exception handler.)

### Eval_RunCore()

This function consists of a giant `switch` statement, with one case for each instruction.  It is a very simple von Neumann machine:  The `switch` fetches the next opcode, and then each instruction is individually responsible for decoding the operation, executing it, moving the `byteCode` pointer to the next instruction, and jumping back to the start of the `switch` statement.

To avoid inefficiencies in the C compiler, each `case` of the `switch` statement uses a `goto` to jump back up to the start of the `switch` statement.  (It is an error to `break` or fall-through any cases in the `switch` statement.)

Each instruction is responsible for _all_ decode/execute work, including any changes to the instruction pointer itself.  Thus the _nop_ instruction is implemented as:

```C
	case Op_Nop:
		byteCode++;
		goto next;
```
`Eval_RunCore()` keeps the current `closure` pointer (which is used to access the closure's stack and local variables) and the current `byteCode` pointer in local register variables.  Because these local registers may be needed outside `Eval_RunCore()`, there are two macros, `LOAD_REGISTERS` and `STORE_REGISTERS`, that copy these critical local registers to and from their global-variable equivalents.  These two macros wrap all places where `Eval_RunCore()` may be entered or exited or invoke external code.

### Closures

A `Closure` is a heap object representing the current state of a function's local data.

In each closure, all values are maintained on a stack of `SmileArg` structs.  The stack is divided into three chunks:

- The lowest chunk is used for function arguments, which are copied from the stack of the caller.
- The middle chunk is used for local variables.
- The upper chunk is treated as a true stack, where temporary values are pushed and popped.

The `ClosureInfo` for each closure contains numbers describing how much stack space is required, so allocation of a runtime `Closure` can always be performed with a fixed size:  The actual stack does not need to grow or shrink at runtime (and is therefore really just an array).

It is worth pointing out that at runtime, function arguments and local variables are referenced exclusively by their stack index — not referenced by name — for efficiency.  The `ClosureInfo` contains lookup tables that allow stack indexes to be transformed into names for debugging purposes.

There are a host of macros to help interact with the current closure's stack efficiently.  These form roughly half the source code of `closure.h`, so it is worth reading the full list there, and worth reading them to see what each one does in detail.  But below is rough descriptions of the major macros and how they work.

#### Temporary-Value Macros

- `Closure_Push(closure, arg)`.  This pushes a `SmileArg` onto the top of the stack.
- `Closure_UnboxAndPush(closure, value)`.  This pushes a `SmileObject` onto the top of the stack, unboxing it into a `SmileArg` first (if it is a type that supports unboxing; if it does not, the unboxed data will be empty).  If you are pushing one of the unboxable types (bool, symbol, or any number of 64 bits or smaller), you _must_ use this form.
- `Closure_PushBoxed(closure, value)`.  This pushes a `SmileObject` onto the top of the stack, without attempting to unbox it.  This should only be used for types that do not support unboxing (heap types like String and List and Pair and UserObject).
- `Closure_PushedUnboxedInt64(closure, value)`.  There are several specialty macros for pushing unboxed data of a specific type:  Byte, Int16, Int32, Int64, Symbol, Bool, and so on.
- `Closure_Pop(closure)`.  This removes the top item from the stack, optionally returning it as a `SmileArg`.
- `Closure_GetTop(closure)`.  This peeks at the top `SmileArg` in the current closure without removing it.
- `Closure_SetTop(closure, arg)`.  This replaces the top `SmileArg` in the current closure.  It is an error to call this on an empty stack.
- `Closure_GetTemp(closure, depth)`.  This peeks at deeper `SmileArg` structs in the stack.  `Closure_GetTop(closure)` is exactly the same as `Closure_GetTemp(closure, 0)`.  A depth of `1` would access the value below the top; `2` would access the value below that, and so on.
- `Closure_SetTemp(closure, depth, arg)`. This replaces deeper `SmileArg` structs in the stack, analogous to a combination of `Closure_GetTemp()` and `Closure_SetTop()`.
- `Closure_PopCount(closure, count)` removes multiple items from the stack at once, discarding them.

#### Local-Variable Macros and Argument Macros

These have different names for historical reasons, but work identically.  That said, for clarity of code, you should use the `LocalVariable` form when you are referring to local variables, and the `Argument` form when you are referring to function arguments.

- `Closure_GetLocalVariable(closure, index)`.  This retrieves a local variable from the current closure, as a `SmileArg`.
- `Closure_GetLocalVariableInScope1(closure, index)`.  This retrieves a local variable from the parent closure.  There are similarly-named macros for the current scope (0), the parent scope (1), the grandparent scope (2), and so on, all the way up through ancestor 7.
- You may call the function `Closure_GetLocalVariableInScope(closure, scopeDepth, variableIndex)` to access deeper scopes beyond scope 7 or scopes of variable depth (which are unlikely to occur, but they could exist).
- There are analogous `Closure_SetLocalVariable()` and `Closure_SetLocalVariableInScope()` macros and functions to the above for modifying local variables in various scopes.
- There are analogous `Closure_GetArgument()` and `Closure_SetArgument()` macros and functions to all of the above for reading and modifying arguments in various scopes.

Because arguments and local variables share the same indexes, it is the responsibility of the bytecode to be certain which object it is accessing on the stack:  `LocalVariable(0)` is the same as `Argument(0)`.

#### Global Variables

True global variables — variables that come from the external environment — are stored outside the closure-and-index model.  There is a single root "global" closure in which the variables' values are accessed by simply looking them up, by symbol ID, in an `Int32Dict`.  Global variables are handled with the special `LdX` and `StX` instructions (which can be read as either "load unknown" or "load external", depending on whichever reads clearer to you).

###Function Calls

#### Calls and Recursion

Calls to functions and methods are designed to avoid recursing on the C stack wherever possible.  (The Smile interpreter is "stackless" in that sense.)

Because so much of a Smile program's time is spent calling things, there are both general-purpose calling instructions (like `Op_Call` and `Op_Met`) as well as optimized calling instructions (like `Op_Call0` and `Op_Met1`), as well as special-purpose calling instructions (like `Op_Add`).  Each of the rest of these could be boiled down to just invocations of `Op_Call`, but are kept separate to ensure that calls are as fast as possible.

#### User Functions

The `Op_Call` instruction, when invoking a Smile user function, first constructs a new `Closure` object, then records in it the `ByteCode` pointer and return `Closure` pointer, and then finally sets that function's `ByteCode` as the current `ByteCode`, and sets the new `Closure` as the current `Closure`.  The `Op_Ret` instruction, at the end of the user function, simply copies the values back.  Invoking a user function does not ever recurse the C stack deeper.

(There is a little extra work that is done to proxy arguments and return values between closures, but that's the gist of it.)

#### External Functions

When invoking an external function written in C, `Op_Call` simply passes the `SmileArg` structs from the current closure to the external C function.  It is up to the external C function to avoid invoking `eval` again.  All of the standard-library functions that come with Smile _do not_ recursively invoke `eval`.

While it is not strictly necessary, most C functions use "wrapper" functions with attached metadata to perform their type-checks where possible.  These type-checking wrappers are constructed automatically by calls to `SmileFunction_CreateExternalFunction()`, which can be passed argument counts and type checks.  (Future versions of the interpreter may use this metadata to perform type proofs and optimize execution even further.)

The signature of an external function is designed to mimic that of C `main()` itself:

```C
typedef SmileArg (*ExternalFunction)(Int argc, SmileArg *argv, void *param);
```

That is, your function will be passed in a count of arguments, an array of those arguments, and an optional void pointer to an environment (i.e., to any external data your function needs to maintain state; this pointer must be passed into the call to `SmileFunction_CreateExternalFunction`, and it will be passed back to you on every invocation of your function, unchanged).

Your function should return a `SmileArg` that is its resulting value.  If your function does not have a meaningful resulting value, you should return `SmileArg_From(NullObject)`.

Note that any data passed into your function will be unboxed, if it is a type that supports unboxing.  Your function _must_ also return unboxed data, if it returns data of a type that supports unboxing.

#### State-Machine Functions

Some external C functions, however, may need to be able to call back into `eval`.  For example, `List.each` needs to be able to invoke a function on each item in the list.  `eval` has special support for C functions that need to invoke Smile functions repeatedly.

`eval` provides a special kind of closure, the `ClosureStateMachine`, and the special `Op_StateMachStart` and `Op_StateMachEnd` instructions.   A C function can call `Eval_BeginStateMachine(start, body)`, passing in a pointer to a "start" function and a "body" function.  This call will switch `eval` into "state-machine mode", where it will repeatedly alternate between calling a Smile function and the C "body" function.  (The first time around, it will call the C "start" function instead of "body", to give the C code a chance to "set up the loop.")

A state machine should generally follow this sequence:

1. Call `Eval_BeginStateMachine(start, body)` to start up the `eval` state machine.

2. Populate the returned `closureStateMachine` with values that describe the initial state of the state machine.  (The state machine has enough room for 8 pointers, which should be sufficient for almost anything.)  For example, `List.each` populates it with the function to be called, the initial list cell, and the initial index:

   eachInfo = (EachInfo)closure->state;
   	eachInfo->function = function;
   	eachInfo->list = eachInfo->initialList = list;
   	eachInfo->index = 0;

3. `return (SmileArg){ NULL };` You have to return something from your function, but this return value will be ignored.

4. `eval` will then call your `start` function, inside the new closure.  Your `start` function should push onto the stack whatever is necessary to call the first Smile function (usually a `SmileFunction` object itself, and whatever its arguments are).  There is enough room in the closure's stack for 15 arguments, including the function.  Your `start` function should return how many arguments must be applied to the given function.

5. `eval` will invoke the given function, popping all the arguments, and leaving only that function's return value on the stack.

6. `eval` will then call your `body` function.  Your `body` function should examine the results of the Smile function, and either keep or discard them from the stack.  Then, just like the `start` function, It should then push the next Smile function to call, and its arguments, and return how many arguments are to be applied to the function.

7. `eval` will repeat steps 5 and 6 until either `start` or `body` return -1 for the number of arguments to apply.  This tells `eval` that the state machine is done.  `eval` will take the top `SmileArg` from the closure's stack as the state machine's return value, discard the closure, and return that `SmileArg` to the caller.


### Exceptions

`Eval_Continue()` and `Eval_RunCore()` both maintain exception handlers such that any code at any time may call `Smile_Throw(object)` (declared in `env.h`).

Outside of `eval`, `Smile_Throw()` immediately aborts the Smile runtime, since there is nothing to catch it.

Inside `eval`, `Smile_Throw()` can have one of two different behaviors:

- If a function has been registered as an exception handler, `Smile_Throw()` will cause an unconditional branch back to that function's declaring closure, push the exception object onto its stack, and then call the function normally.
- If no functions are registered as exception handlers, `Smile_Throw()` will use a C `longjmp()` to return to `Eval_Continue()`, which will return with an exception result.

In either case, if the thrown object is a user object, `Smile_Throw()` will also append to it a `stack-trace` property that contains information about the location at which the object was thrown.  Any other properties of the thrown object are the responsibility of the caller.