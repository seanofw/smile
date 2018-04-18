# Smile Programming Language Interpreter

**Copyright &copy; 2004-2018 Sean Werkema**

[![Travis CI Build Status](https://travis-ci.org/seanofw/smile.svg?branch=master)](https://travis-ci.org/seanofw/smile)
[![Coverity Scan Status](https://scan.coverity.com/projects/seanofw-smile/badge.svg?flat=1)](https://scan.coverity.com/projects/seanofw-smile)

-------------

**This is the C implementation of the Smile Programming Language.**

Smile is a functional, object-oriented, dynamically-typed programming
language with a flexible syntax.  It is heavily inspired by Lisp and
Smalltalk and JavaScript, and is as mutatable and extensible as a true
Lisp, but it reads more like Python or Ruby.

## Smile Language Information

  * [Smile Website](http://www.smile-lang.org)
  * [Unofficial Dev Blog](http://www.werkema.com)
  * [Code of Conduct](CODE-OF-CONDUCT.md)
  * [Contributing to Smile](CONTRIBUTING.md)
  * [List of Contributors](CONTRIBUTORS.md)

## C Interpreter Source Code

  * [Building the Interpreter](BUILD.md)
  * [Interpreter Code Standards](CODE-STANDARDS.md)
  * [Interpreter Source Code](smilelib)
    * [Logical Block Diagram](docs/Smile%20Interpreter%20Logical%20Block%20Diagram.png)
    * [Source Overview](smilelib/src)
      * [Parsing Overview](smilelib/src/parsing)
      * [Evaluation Overview](smilelib/src/eval)
    * [C Header Overview](smilelib/include)

(_Note: This C implementation is somewhat incomplete but growing,
and it will soon become the official implementation.  It is **somewhat**
usable in its current form, and will likely pass the old C# implementation by
the end of 2018._)

-------------

## License

**Licensed under the Apache License, Version 2.0 (the "License")**;
you may not use this software except in compliance with the License.
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 .

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

> _A copy of the Apache license is also included in the source repository,
in the file [LICENSE](LICENSE)._
> 
> _**Short short version:** It's free as in speech and free as in beer.  You can use it any way you want, non-profit, private, public, or commercial, and you can even sell it, but you can't claim you wrote it, and since it's free, you can't complain or sue if it doesn't work or causes trouble for you._

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

