# Smile Programming Language Interpreter

**Copyright &copy; 2004-2015 Sean Werkema**

<hr />

**This is the C implementation of the Smile Programming Language.**

Smile is a functional, object-oriented, dynamically-typed programming
language with a flexible syntax.  It is heavily inspired by Lisp and
Smalltalk and JavaScript, and is as mutatable and extensible as a true
Lisp, but it reads more like Python or Ruby.

More information on Smile can be found at http://www.smile-lang.org.  An
informal development blog can also be found at http://www.werkema.com.

(_Note: This implementation is currently **very** incomplete but growing,
and it will eventually become the official implementation.  It is **not**
usable in its current form (yet).  If you want the much-more-complete and
fairly-usable (but slow) C# implementation, you'll need to look elsewhere,
as it's not hosted here._)

<hr />

## Building Smile

### On Windows

You will need Microsoft Visual Studio 2012 or 2013.  A ''.sln'' solution
file, along with associated ''.vcxproj'' project files, is provided.  Open
the solution file and then ''Rebuild All''.

Smile on Windows can be built in Debug or Release mode, for either x86 or
x64 architectures.

### On Un*x

Just go into the SmileC directory and run these two commands.
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

Anything not on that list isn't guaranteed, but might work.

Many configuration options are available in ''SmileC/Makefile.conf'',
if you have customization requirements for the buildor installation.

### Other Un*x Support

If you want to add an unsupported exotic Un*x-like platform,
edit ''Makefile.conf'' and possibly ''scripts/Makefile.os'' and
''scripts/Makefile.extra'' as necessary.  Code changes may also be
required to support your platform, particularly in the garbage
collector, file I/O, text I/O, and decimal-number libraries:

  - Smile currently uses the Boehm mostly-conservative garbage collector, which supports many platforms, but yours may not be among them.  If not, you may have some work ahead of you implementing some parts of the root-finding code for your platform.

  - File I/O is necessarily platform-specific.  Most Un*x-like systems should be supported fairly well, but your mileage may vary.

  - Text I/O is done using a simple wrapper library around ncurses, which should support most Un*xen, but curses/ncurses are famously finicky and may require tweaks for more exotic OSes.

  - Decimal-number support is courtesy the Intel Decimal Floating-Point Math Library, which is IEEE 754-2008-compatible.  This library has been customized a little for Smile, and works correctly on Windows and Linux x86/x64 systems, but 

<hr />

**Licensed under the Apache License, Version 2.0 (the "License")**;
you may not use this software except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
