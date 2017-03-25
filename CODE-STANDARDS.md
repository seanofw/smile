
Smile Programming Language Interpreter Code Standard
====================================================


Overview
--------

High-quality software is designed first and foremost for its correctness and its maintainability.  A major requirement of software that is both correct and maintainable is consistency in its design and in its code.  This document, then lists the requirements of any source code that forms the C implementation of the Smile Programming Language Interpreter.

_______________________________________________________________________________


Terminology
-----------

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

_______________________________________________________________________________


Whitespace Conventions
----------------------

The Smile Interpreter's source code:

1. Must use tabs, not spaces, for indentation.

    > _Tabs are more efficient for storage; they help prevent accidents in indentation; and they are of viewer-customizable size._

2. Must place open curly braces for `for`, `if`, `else`, `while`, and `do` statements on the same line as the statement, not on the next line.

    > *Don't waste lines; write* `if (x) {`.

3. Must place closing curly braces for all statements on a line by themselves.

    > *Don't write* `} else {` *; put the* `else` *on the next line.*

4. Must place the open curly brace for a function on a line by itself below the function, unindented.

5. Must indent a closing curly brace of a statement to the same depth as the start of its opening curly brace.

    > *The following example shows correct indentation of curly braces:*
    >
    > ```c
    >  if (x) {
    >     if (y) {
    >         ...
    >     }
    >     else {
    >         ...
    >     }
    > }
    > else {
    >     ...
    > }
    > ```

6. Must not contain more than one blank line in sequence in a source file.

    > *Don't leave long gaps of unnecessary blank lines in source code.*

7. Must not contain a blank line before a closing `}`.

8. Must contain a blank line after every function body.

    > *Put blank lines between your functions.*

9. Must "collapse" a function declaration to a single line, wrapped to successive lines at no more than 100 characters; and successive lines of the function declaration must be indented.

    > *For example, do not write this:*
    >
    > ```c
    > void
    > f(
    > 	int x,
    > 	int y
    > )
    > {
    > }
    > ```

    > *Write this instead:*
    >
    > ```c
    > void f(int x, int y)
    > {
    > }
    > ```

10. Should contain whitespace to the left and right of every binary operator.

 > *Don't write* `1+2` *; write* `1 + 2` *. This rule is very nearly a "must" rule; you may only violate it for certain short operations like subscripting, like* `str[x+1]` *.*

11. Must not contain whitespace after a prefix unary operator or before a postfix unary operator.

    > (*Don't write* `x ++` *or* `++ x` *; write* `x++` *and* `++x` *.*)

12. Must have a single space character after the keywords `if`, `else`, `while`, `do`, `for`, and `return`.

13. Must not have a space character after `(` or `[`, or before `)` or `]`.

14. Must have at least one whitespace character of some kind (space, tab, or newline) on both sides of a `{` or a `}`.

15. Variable declarations for pointer types must use whitespace that shows C's associativity rules.

    > *Write* `Byte *ptr` *and not* `Byte* ptr` *because the* `*` *in C belongs to the variable-name part of the declaration, not to the type-name part.*

16. Must not contain an extra space between the name of a function and its `(`, either when declaring the function or calling it.

    > *Don't write* `f (x)` *; write* `f(x)` *.*

17. Must indent all code within `(...)` or `[...]` or `{...}` by one tabstop relative to its parent indentation wherever that code appears on a new line.

    > *Correct example:*
    >
    > ```c
    > if (x < y) {
    >     f(y);
    >     if (x < z) {
    >         f(z + y);
    >     }
    > }
    > ```

18. When breaking long lines, must break either before a binary operator or after a comma.

    > *Correct example:*
    >
    > ```c
    > if (x < y && y < z && IsValid(z)
    >     || x < y && y < z && !IsValid(z)
    >     || x > y && y > z && !IsValid(z)) {
    > }
    > ```

_______________________________________________________________________________


Naming Conventions
------------------

Names in the Smile Interpreter's source code:

1. Must use `CamelCase` for all functions.

    > *All function names must start with a capital letter, and should not contain underscores, except when following rule #2 below.*

2. Should not contain `_` characters except for private/static data names, and for separating "namespaces" (*as in rule #3 below*).

3. Must use `Xxx_CamelCase` for functions specific to a particular data type, where the `Xxx` is identical to the type name.
    > *These functions must either take that data type as their first argument, or return that data type, or both.*

4. Must use `CamelCase` for `struct`, `union`, and `typedef` type names.

5. Must use `CAPS_WITH_UNDERSCORES` for all `#define`d macros, regardless of whether they contain simple const-value subtitutions or complex code.

6. Must use `camelCase` (note the initial lowercase letter) for all local variables and for all `struct` members.  However, `struct` members that are to be considered private data may (and should) start with an initial underscore (`_`).

7. Must name static data values as `_camelCase`, with the initial underscore (`_`) followed by a lowercase letter.

8. Must name public (extern) data values as `CamelCase`, or as `Xxx_CamelCase` for strongly-typed data, where the `Xxx` is identical to the type name.

9. Should contain the word `Struct` or `Int` (internal) in `struct` or `union` names, reserving "bare" names for use by `typedef`.

    > *For example,* `struct StringInt { };` *is internal, while* `typedef struct StringInt String;` *is public.*

_______________________________________________________________________________


Commenting Conventions
----------------------

The Smile Interpreter's source code:

1. Must use `//`-style comments.  Do not use classic C-style `/*` comments.

2. Must not contain "commented-out" code using `//` or `/*` comments. If code is unnecessary, it must be deleted.

3. Should contain `///`-style documentation comments before every public function.

4. Should put every comment on its own line, except when documenting a member of a `struct`.

    > *Don't add "comment tags" at the end of a line unless you have a good reason.*

5. Should prefix dangerous or complex code sequences with a `// TODO:` or `// WARNING:` comment.

6. Comments should be indented to match other declarations within the same scope.

_______________________________________________________________________________


Language Conventions
--------------------

The Smile Interpreter's source code:

1. Must be written in C.

    > *__Not__ in C++!*

2. Must conform to the C89 standard, except for the usage of `//`-style comments, and for code within the `types.h` portability layer.

    > *__Not__ the C99 standard, or C1x standard.*

3. Must be able to compile under GCC, Microsoft Visual C++, and clang.
    > *Avoid writing code that is platform-specific outside of `platform.h` files and anything that is necessarily platform-specific, like core file I/O operations.  In general, the majority of the interpreter should compile on anything, anywhere.*

4. Should avoid using `void*` wherever possible.  Prefer stronger pointer types where possible.

5. Should avoid using `char*` wherever possible.  Prefer using `String` objects instead.

6. Must not use `int`, `long`, `short`, `unsigned`, `float`, or `double`.  Use the safer, portable `Byte`, `SByte`, `Int16`, `UInt16`, `Int32`, `UInt32`, `Int64`, `UInt64', `Float32`, and `Float64` types.

7. Should avoid using `Int32` or `Int64` or `UInt32` or `UInt64` unless the size is truly important:  Prefer instead to use `Int` or `UInt`, whose size is always at least 32 bits but may vary to match the processor's native register size.

8. Must use `PtrInt` when casting a pointer to an integer type.

    > *Do not cast pointers to `Int` or `Int32` or `Int64` and expect those to be the correct size.*

9. Should avoid `#define` macros that contain non-type-parameterized code. Prefer using `Inline` functions.

10. Should avoid mutable `struct` types.  Prefer using immutable structures and creating new instances if a structure requires new data.

11. Should avoid heap allocation where possible.  Garbage collection isn't free, so if temporary memory can be allocated from the stack, it should be.

12. Should avoid using `goto`, except in state machines or in error-handling code.

13. Should avoid `return` in the middle of a function except when its purpose is early exit.

    > *To avoid deeply nesting most of a function inside an* `else` *clause, it is acceptable to use* `return` *to "early out" of a function, but* `return` *should otherwise be avoided*.

14. Must not use a redundant bare `return;` at the end of a `void` function.

15. Must place constant values on the *right* side of comparison operators.

    > *Don't write* `1 == x` *; write* `x == 1` *.*

16. Must not contain unnecessary parentheses, except when performing an assignment in an `if` statement, or around the `?:` operator, or when mixing arithmetic and bitwise operators, or to show correct precedence between `&` and `|` and `^` or between `&&` and `||`.

    > *Don't write* `if ((x < y) && (y < z))` *; write* `if (x < y && y < z)` *.*

    > *You may write* `if ((x = f(y)))` *to help defuse compilers that would*
    > *erroneously warn about* `=` *not being a* `==` *operator.*

    > *You may write* `x = (w ? y : z)` *to show that the nested operation is conceptually isolated from the assignment.*

    > *You may write* `if ((x < y && y < z) || (z < y && y < x))` *, even though the parentheses are unnecessary, to better visually represent the precedence of the and-vs-or operators, which can be unclear in C.*

17. Should not contain sets of integers that are all binary powers of two; instead, prefer bit-shifting.

    > *Don't do this:*
    > ```c
    > #define FOO 1
    > #define BAR 2
    > #define BAZ 4
    > #define QUX 8
    > #define GUM 16
    > ```

    > *Do this instead:*
    > ```c
    > #define FOO (1 << 0)
    > #define BAR (1 << 1)
    > #define BAZ (1 << 2)
    > #define QUX (1 << 3)
    > #define GUM (1 << 4)
    > ```

_______________________________________________________________________________


Build Requirements
------------------

The Smile Interpreter's source code:

1. Must compile on Linux, Windows, and MacOS X.
2. May require a Perl interpreter as part of the build chain on any platform.
3. On Windows, must compile under Visual Studio 2017 without requiring any extensions to be installed.
4. On Linux/MacOS X, must compile using `gcc` and GNU `make`.  Other external interpreters, like Ruby or Python, and other common build  tools, like `autoconf` and `m4`, must not be part of the build process.

_______________________________________________________________________________


Third-Party Library Requirements
--------------------------------

Third-party libraries should be avoided where reasonably technically feasible, as they contribute an unknown amount of security risk to the code base.  That said, some libraries are unavoidable.

For those libraries that are unavoidable and required for the interpreter to function, these requirements must be observed:

1. The library code must be compiled as part of the Smile build process.

    > *It must not be included in pre-compiled binary form, or as an external dependency that must be downloaded.*

2. The library must be written in C.

    > *Compiling the Smile interpreter __must not__ have any dependencies on a C++ compiler.*

3. The library's headers must not be exposed outside `smilelib` itself.

    > *Users of* `smilelib` *must not be able to* `#include` *any* `.h` *files from the third-party library directly or indirectly*.

4. The library's code must be isolated within a folder (directory) in the Smile source tree; an individual library must not span multiple folders.  If the library contains multiple components, they may be isolated within separate sub-folders within the library's primary folder.

5. Any functions externally-exposed by the library must be either wrapped or renamed so that all functions exposed follow the Smile C naming conventions above.

6. Third-party libraries should contain unit tests to demonstrate the library's correctness.  These tests must run as part of the standard Smile unit-test suite.

7. All third-party libraries must function correctly when compiled for 32-bit or for 64-bit processors.

8. All third-party libraries must compile and work correctly on Windows, Linux, and MacOS X, except where the library only is included to "shim" required functionality that an OS does not natively support.

9. Third-party libraries should contain no dependencies on other third-party libraries or on unusual OS components.

    > *The Smile Interpreter should be able to build easily on a "vanilla" OS install with no unusual components required.*
