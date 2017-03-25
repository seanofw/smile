# Smile Programming Language Interpreter

**Copyright &copy; 2004-2017 Sean Werkema**

-------------

**This is the C implementation of the Smile Programming Language.**

Smile is a functional, object-oriented, dynamically-typed programming
language with a flexible syntax.  It is heavily inspired by Lisp and
Smalltalk and JavaScript, and is as mutatable and extensible as a true
Lisp, but it reads more like Python or Ruby.

More information on Smile can be found at http://www.smile-lang.org.  An
informal development blog can also be found at http://www.werkema.com.

(_Note: This implementation is currently incomplete but growing,
and it will eventually become the official implementation.  It is **not**
usable in its current form (yet).  If you want the much-more-complete and
fairly-usable (but slow) C# implementation, you'll need to look elsewhere,
as it's not hosted here._)

-------------

## Building Smile

### On Windows

You will need Microsoft Visual Studio 2012 or 2013.  A ''.sln'' solution
file, along with associated ''.vcxproj'' project files, is provided.  Open
the solution file and then ''Rebuild All''.

Smile on Windows can be built in Debug or Release mode, for either x86 or
x64 architectures.

### On Un*x

(**Warning:** The Smile interpreter hasn't been built on Linux in some time.
Getting it to run here may require some hacking.  It _should_ be able to run,
but most of the development focus has been on core functionality, not platform
compatibility.)

Go into the SmileC directory and run these two commands.

- ''make dep''
- ''make''

If your environment is supported, it will build, which should produce
no warnings or errors.  You can verify the build by running ''make check'',
which runs the thorough unit-test suite, and which should output no errors.

A successful build may be installed with ''make install''.

Supported, tested build environments use the GNU build chain
(Make and GCC).  These are the current test platforms:

- Linux x64 (GCC 5.x)
- Cygwin x86 (GCC 4.x)

Anything not on that list isn't guaranteed, but might work.  Note that
Cygwin x64 seems to have problems compiling the GC right now.

Many configuration options are available in ''SmileC/Makefile.conf'',
if you have customization requirements for the buildor installation.

### On MacOS X

(**Warning:** The Smile interpreter hasn't been built on MacOS in some time.
Getting it to run here may require some hacking.  It _should_ be able to run,
but most of the development focus has been on core functionality, not platform
compatibility.)

Test builds have been made on MacOS X x86 64-bit.  In theory, it follows
the same sequence as Un*x above:

- ''make dep''
- ''make''

It will build, which should produce no errors, but Clang currently generates
a number of warnings (we're working on that).  Once built, you can verify
the build by running ''make check'', which runs the thorough unit-test suite,
and which should output no errors.

### Other Un*x Support

If you want to add an unsupported exotic Un*x-like platform,
edit ''Makefile.conf'' and possibly ''scripts/Makefile.os'' and
''scripts/Makefile.extra'' as necessary.  Code changes may also be
required to support your platform, particularly in the garbage
collector, file I/O, text I/O, and decimal-number libraries:

- Smile currently uses the Boehm mostly-conservative garbage collector, which supports many platforms, but yours may not be among them.  If not, you may have some work ahead of you implementing some parts of the root-finding code for your platform.

- File I/O is necessarily platform-specific.  Most Un*x-like systems should be supported fairly well, but your mileage may vary.

- Text I/O is done using a simple wrapper library around ncurses, which should support most Un*xen, but curses/ncurses are famously finicky and may require tweaks for more exotic OSes.

- Decimal-number support is courtesy the Intel Decimal Floating-Point Math Library, which is IEEE 754-2008-compatible.  This library has been customized a little for Smile, and works correctly on Windows and Linux and MacOS x86/x64 systems, but may not behave nicely on others.

-------------

## License

**Licensed under the Apache License, Version 2.0 (the "License")**;
you may not use this software except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

_(A copy of the Apache license is also included in the source repository,
in the file ''LICENSE''.)_

_(**Short short version:** It's free as in speech and free as in beer.
You can use it any way you want, but you can't claim you wrote it,
and since it's free, you can't complain if it doesn't work for you.)_

-------------

## Third-Party Software

Smile uses third-party software libraries for some functionality:

- The Boehm mostly-conservative garbage collector.  http://www.hboehm.info/gc/
- The Intel Decimal Floating-Point Math Library.  https://software.intel.com/en-us/articles/intel-decimal-floating-point-math-library

These software libraries are covered under their own licenses, but all use forms of
non-restrictive open-source licenses (in the cases above, the MIT and BSD licenses,
respectively).

Smile does not contain commercially-licensed third-party software.
Smile does not contain GPL- or LGPL-licensed third-party software.

