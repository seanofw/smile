# Building Smile

## On Windows

You will need [Microsoft Visual Studio 2017](https://www.visualstudio.com)
installed, with its C/C++ compiler.  A standard Visual Studio ''.sln'' solution file,
along with associated ''.vcxproj'' project files, is provided.

You will also need to have the
[Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/install-win10)
installed.  The build expects to be able to invoke `bash` and `sed` and `date` and `perl`,
among other Un*x tools.  The build expects the GNU implementations of the tools as well.
We recommend the Ubuntu distribution.

To get the repository, simply `git clone` it into a new folder.  Do not clone
the repository into a path that includes spaces in its name.  This will cause
the build to fail.  (And we do *not* intend to fix this issue.)

To build, open the `.sln` solution file in Visual Studio and then simply
''Rebuild All''.  After it has built, run the ''SmileLibTests'' project to
run all of the unit tests and be certain that the interpreter was correctly
built.  The command-line executable for Smile can be found in the
''SmileRunner'' project.

Smile on Windows can be built in Debug or Release mode, for either x86 or
x64 architectures.

(Note: With effort, you could probably get it to build under VS2012/13/15
as well, but I only build and test it on VS2017 now.  Older versions of
Visual Studio are *not* recommended.  Visual Studio Code has not been tested,
and may not be able to build it.)

------------------------

## On Un*x

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

------------------------

## On MacOS X

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

