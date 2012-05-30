Kai
===

* Author: Samuel G. D. Williams (<http://www.oriontransfer.co.nz>)
* Copyright (C) 2009, 2011 Samuel G. D. Williams.
* Released under the MIT license.

Kai is a completely dynamic language stack based on `s-expressions` and `f-expressions`. It is designed to be a complete language with well-defined semantics.

It has a number of useful features:

* Arbitrary precision unsigned and fixed-point math by default.
* Easy to embed and extend into other C++ projects.

For documentation and examples, please see the main [project page][1].

[1]: http://www.oriontransfer.co.nz/research/kai

Implementation
--------------

### Object Model ###

Kai has a context-dependent object model which means that objects can be transparently moved between instances. Due to the indirection, there is a small performance hit due to the context-dependent lookup.

### Memory Model ###

Kai has a precise mark and sweep garbage collection with well-defined check points. The garbage collection is combined with a basic linked-list memory manager which includes a linked list of free allocations. The object allocator is designed for small object allocations between 32 and 256 bytes. It can handle larger objects but the performance will not be as good.

### Interpreter Model ###

The Kai stack frame serves as the fundamental unit of evaluation and lookup. Due to this, the interpreter context is stored entirely within the root stack frame from which futher evaluation occurs. Almost all Kai functions take the current stack frame as an argument.

Due to this, supporting multiple isolated interpreters is as easy as having separate stack frames. Isolated interpreters can be run on different threads and will not interact in any way (except by design).

Compatibility
-------------

Kai will build and work on any platform that provides a modern C++11 compiler, e.g. Ubuntu Precise+, Mac OS X 10.6+. Kai is primarily built and tested with `clang++`.

License
-------

Copyright (c) 2006, 2011 Samuel G. D. Williams. <http://www.oriontransfer.co.nz>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
