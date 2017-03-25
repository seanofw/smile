# SmileLib `src` Folder

This folder contains all of the C source code that implements the Smile compiler/interpreter and runtime.

Below is a useful roadmap when navigating the files here:

- `atomic` contains source code for implementing atomic-memory operations, like `Atomic_AddInt64()`.  There are separate implementations in here for each major platform that Smile supports.
- `dict` contains implementations of dictionaries, implemented as chained hash tables, for various types.   Dictionaries are used to map a known key to a specific value.
- `env` contains code to implement the Smile runtime environment.  The files in here are used for global initialization, global variables and known types, and the shared symbol table.
- `eval` contains the Smile evaluator.  When passed an S-expression, the evaluator runs it and returns its result.  It does so by first compiling the S-expression to an optimized bytecode form, and then running the bytecode.  Because `eval` is really implemented as both a compiler _and_ an interpreter, there is a separate subfolder for the "invisible" compiler portion of it.
- `numeric` primarily contains implementations of the Smile Real types (decimal floating point).  These types are implemented on top of the Intel Binary Integer Decimal library.  In the future, this will also contain the implementations of 128-bit integer and float and real types, as well as "big" forms (arbitrary-precision forms).
- `parsing` contains the Smile lexical analyzer and parser.  See the `README.md` in this folder for more details.