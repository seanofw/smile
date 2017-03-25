# Includes

This folder contains all of the header files for the Smile interpreter.  Some of these are designed to be included externally, and some are not.  As a general rule, anything with the name `internal` is designed to be only used inside the interpreter, and should not be used outside.

All of the header files are wrapped with safe `#ifdef/define __HEADER_H__` blocks, so they can be included multiple times.

---------------------

Note that the headers for the Boehm garbage collector and Intel decimal math library are kept outside of here, in their original source trees, but they are referenced by other `#includes` in here.  To access the Boehm GC, you can simply include `gc.h`, and to access the decimal numerics, you only need to pull in `numeric/real.h`.