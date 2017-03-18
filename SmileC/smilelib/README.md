# Smile Core Library

This folder contains the entire Smile compiler/interpreter and runtime, including the garbage collector, decimal library, and bootstrap code.  The code found in here produces a DLL (shared library) that can be linked into anything to result in a runnable version of the Smile interpreter.  (The Smile command-line runner is merely a lightweight wrapper around all the code found in this folder.)

As a general rule, this folder is broken down into six major things:

- The `include` folder contains all of the headers needed by the Smile core, and all of the headers that are exposed to other programs.
- The `src` folder contains all of the C source code that implements the Smile core.
- The `gc` folder contains a (tweaked) copy of the Boehm garbage collector.
- The `decimal` folder contains a (tweaked) copy of the Intel Binary Integer Decimal library.
- The `bin`/`obj` folders contain the compiled output, with separate subfolders for each platform.
- All top-level files in this folder itself are either documentation or build scripts.